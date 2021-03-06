/*
 * frame.h
 *
 *  Created on: Sep 20, 2016
 *      Author: kevinsheridan
 */

#ifndef PAUVSI_M7_INCLUDE_PAUVSI_VO_FRAME_H_
#define PAUVSI_M7_INCLUDE_PAUVSI_VO_FRAME_H_

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/features2d.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/video.hpp"
#include <vector>
#include <algorithm>
#include <string>
#include <ros/ros.h>

#include "VIOFeature2D.hpp"
#include "VIOState.hpp"



#define DEFAULT_FEATURE_SEARCH_RANGE 5
#define MAXIMUM_ID_NUM 1000000000 //this is the maximum size that a feature ID should be to ensure there are no overflow issues.

class Frame
{

private:
	cv::Ptr<cv::xfeatures2d::BriefDescriptorExtractor> descriptionExtractor;
	bool frameSet;

public:
	ros::Time timeImageCreated;
	cv::Mat image;
	cv::Mat K;
	cv::Mat D;
	std::vector<VIOFeature2D> features; //the feature vector for this frame
	VIOState state;
	//this ensures that all features have a unique ID
	int nextFeatureID; // the id of the next feature that is added to this frame

	/*
	 * initilize and set frame
	 * the next feature id will be 0;
	 */
	Frame(cv::Mat img, ros::Time t)
	{
		this->image = img;
		this->timeImageCreated = t;
		descriptionExtractor = cv::xfeatures2d::BriefDescriptorExtractor::create();
		frameSet = true;
		nextFeatureID = 0;
		state = VIOState();
	}

	/*
	 * initilize and set frame
	 * The startingID  should be equal to the lastFrame's last feature's ID
	 * if the startingID is too large it will wrap to 0
	 */
	Frame(cv::Mat img, ros::Time t, int startingID)
	{
		this->image = img;
		this->timeImageCreated = t;
		descriptionExtractor = cv::xfeatures2d::BriefDescriptorExtractor::create();
		frameSet = true;
		if(startingID > MAXIMUM_ID_NUM){
			nextFeatureID = 0;
			ROS_WARN("2D FEATURE ID's ARE OVER FLOWING!");
		}
		else{
			nextFeatureID = startingID;
		}
		state = VIOState();
	}

	/*
	 * initilize frame but don't set it
	 * nextFeatureID will be set to zero
	 */
	Frame()
	{
		descriptionExtractor = cv::xfeatures2d::BriefDescriptorExtractor::create();
		frameSet = false;
		nextFeatureID = 0; // assume that this frame starts at zero featureID
		state = VIOState();
	}

	bool isFrameSet(){
		return frameSet;
	}

	/*
	 * gets the time since this image was captured
	 */
	double getAgeSeconds()
	{
		return ros::Time::now().toSec() - this->timeImageCreated.toSec();
	}

	bool addFeature(cv::KeyPoint _corner){
		//ROS_DEBUG_STREAM("adding feature with ID " << nextFeatureID);
		features.push_back(VIOFeature2D(_corner, nextFeatureID));
		nextFeatureID++; // iterate the nextFeatureID
		return true;
	}

	bool addFeature(VIOFeature2D feat){
		//ROS_DEBUG_STREAM("adding feature with ID " << nextFeatureID);
		//set the feature id
		feat.setFeatureID(nextFeatureID);
		features.push_back(feat); // add it to the features vector
		nextFeatureID++; // iterate the nextFeatureID
	}

	/*
	 * get the fast corners from this image
	 * and add them to the frames features
	 *
	 * This function does not check for identical features
	 */
	std::vector<VIOFeature2D> getFASTCorners(int threshold){
		std::vector<cv::KeyPoint> corners;
		cv::FAST(this->image, corners, threshold, true); // detect with nonmax suppression

		int startingID = features.size(); // the starting ID is the current size of the feature vector

		std::vector<VIOFeature2D> feats;
		for(int i=0; i < corners.size(); i++)
		{
			//use a negative id because it is not important to give it a relevant id yet
			feats.push_back(VIOFeature2D(corners.at(i), -1)); // ensure that id's do not repeat
			//ROS_DEBUG_STREAM("corner id " << corners.at(i).response);
		}

		return feats;
	}

	/*
	 * gets the a keypoint vector form the feature vector
	 */
	std::vector<cv::KeyPoint> getKeyPointVectorFromFeatures(){
		std::vector<cv::KeyPoint> kp;
		for (int i = 0; i < this->features.size(); i++)
		{
			kp.push_back(this->features.at(i).getFeature());
		}
		return kp;
	}

	std::vector<cv::KeyPoint> getUndistortedKeyPointVectorFromFeatures(){
		std::vector<cv::KeyPoint> kp;
		for (int i = 0; i < this->features.size(); i++)
		{
			if(this->features.at(i).isUndistorted())
			{
				cv::KeyPoint p;
				p.pt = (this->features.at(i).getUndistorted(true));
				kp.push_back(p);
			}
		}
		return kp;
	}

	/*
	 * gets the a keypoint vector form the feature vector
	 */
	std::vector<cv::KeyPoint> getKeyPointVectorFromFeatures(std::vector<VIOFeature2D> featureVector){
		std::vector<cv::KeyPoint> kp;
		for (int i = 0; i < featureVector.size(); i++)
		{
			kp.push_back(featureVector.at(i).getFeature());
		}
		return kp;
	}


	/*
	 * gets a point2f vector from the local feature vector
	 */
	std::vector<cv::Point2f> getPoint2fVectorFromFeatures(){
		std::vector<cv::Point2f> points;
		for (int i = 0; i < this->features.size(); i++)
		{
			points.push_back(this->features.at(i).getFeature().pt);
		}
		return points;
	}

	/*
	 * describes an individual feature
	 */
	cv::Mat extractBRIEFDescriptor(VIOFeature2D feature){
		cv::Mat descriptor;
		std::vector<cv::KeyPoint> kp(1); // must be in the form of a vector
		kp.push_back((feature.getFeature()));
		descriptionExtractor->compute(this->image, kp, descriptor);
		return descriptor;
	}

	/*
	 * checks all features and undistorts them if they are not undistorted
	 */
	void undistortFeatures()
	{


		for(int i = 0; i < this->features.size(); i++)
		{
			if(!this->features.at(i).isUndistorted())
			{
				this->features.at(i).undistort(this->K, this->D);
			}
		}
	}

	/*
	 * searches for all features in a the local feature vector that have not been described
	 * and describes them using the BRIEF algorithm
	 */
	bool describeFeaturesWithBRIEF(){
		if(features.size() == 0)
		{
			return false;
		}

		std::vector<unsigned int> featureIndexes; // the index of the feature keypoints
		std::vector<cv::KeyPoint> featureKPs; // feature keypoints to be described

		// find all features which must be described
		for(int i = 0; i < features.size(); i++)
		{
			if(!features.at(i).isFeatureDescribed())
			{
				featureIndexes.push_back(i);
				featureKPs.push_back(features.at(i).getFeature());
			}
		}

		cv::Mat descriptions;
		descriptionExtractor->compute(this->image, featureKPs, descriptions); //describe all un-described keypoints

		for(int i = 0; i < descriptions.rows; i++)
		{
			cv::Mat desc = descriptions.row(i);
			features.at(featureIndexes.at(i)).setFeatureDescription(desc);
		}

		return true;

	}
	/*
	 * take a feature vector and describe each of the features
	 */
	cv::Mat describeFeaturesWithBRIEF(cv::Mat image, std::vector<VIOFeature2D> featureVector){
		//ROS_DEBUG("begin to describe feature with brief");
		std::vector<cv::KeyPoint> kp = this->getKeyPointVectorFromFeatures(featureVector);
		cv::Mat description;
		descriptionExtractor->compute(image, kp, description);
		//ROS_DEBUG_STREAM("found description with the size" << description.rows << " X " << description.cols);
		return description;
	}

	/*
	 * compares two descriptors
	 * 0 is perfect match
	 * 256 is the worst possible match
	 * expects two row vectors in cvMats with uchars
	 * assumes that the two description vectors match in size
	 */
	int compareDescriptors(cv::Mat desc1, cv::Mat desc2){
		int sumError = 0;
		int mask;
		for(int i = 0; i < desc1.cols; i++)
		{
			mask = desc1.at<uchar>(0, i) ^ desc2.at<uchar>(0, i);
			for(int j = 0; j<8; ++j)
			{
				sumError += (mask >> j) & 1;
			}
			//error += std::abs(desc2.at<uchar>(0, i) - desc1.at<uchar>(0, i));
		}
		return sumError;
	}

	static bool wayToSort(VIOFeature2D i, VIOFeature2D j)
	{
		bool v = i.getQuality()<j.getQuality();
		return i.getQuality()<j.getQuality();
	}

	/* Takes Threshold for FAST corner detection and KillRadius of the Region of Interest
	 * Defines the quality of all the features and then Sorts them in ascending order of quality.
	 * 80% of quality depends on feature response and 20% on radius within region of interest.
	 */
	void rankFeatures(int fastThreshold, int killRadius)
	{
		float scaleValue = 0.2 * fastThreshold;
		int size = features.size();
		for(int i=0; i<size; ++i)
		{
			if(features.at(i).getQuality() != -1.0)
			{
				//Scaled to 80% from response value and 20% from distance from center
				features.at(i).setQuality(0.8*features.at(i).getResponse() +
						0.2*features.at(i).getResponse() *
						(1/(1+exp(-(5-(10*features.at(i).getDistanceFromFrameCenter()/killRadius))))));
				//Implemented sigmoid function and scaled to [-5,5]. Actual sigmoid => sigmoid(killRadius/2 - DistanceFromCenter
			}
		}
		std::sort(features.begin(), features.end(), wayToSort);

		return;
	}

	/* Takes Threshold for FAST corner detection and KillRadius of the Region of Interest
	 * Defines the quality of all the features and then Sorts them in ascending order of quality.
	 * 80% of quality depends on feature response and 20% on radius within region of interest.
	 *
	 * overloaded uses referenced feature vector
	 */
	void rankFeatures(std::vector<VIOFeature2D>& features, int fastThreshold, int killRadius)
	{
		float scaleValue = 0.2 * fastThreshold;
		int size = features.size();
		for(int i=0; i<size; ++i)
		{
			if(features.at(i).getQuality() != -1.0)
			{
				//Scaled to 80% from response value and 20% from distance from center
				features.at(i).setQuality(0.8*features.at(i).getResponse() +
						0.2*features.at(i).getResponse() *
						(1/(1+exp(-(5-(10*features.at(i).getDistanceFromFrameCenter()/killRadius))))));
				//Implemented sigmoid function and scaled to [-5,5]. Actual sigmoid => sigmoid(killRadius/2 - DistanceFromCenter
			}
		}
		std::sort(features.begin(), features.end(), wayToSort);

		return;
	}

	/*
	 * Checks all local frame features for whether or not a feature is outside of the kill radius.
	 * It will kill the feature if it is
	 * It will set the feature's distance to center if it is not
	 * this function ensures that the feature id's remain in ascending order
	 */
	void cleanUpFeaturesByKillRadius(float killRadius)
	{
		cv::Point2f imageCenter = cv::Point2f((float)(this->image.cols / 2), (float)(this->image.rows / 2));
		std::vector<VIOFeature2D> cleanFeatures;
		for(int i = 0; i < this->features.size(); i++)
		{
			if(this->getAndSetFeatureRadius(this->features.at(i), imageCenter) <= killRadius)
			{
				cleanFeatures.push_back(this->features.at(i));
			}
			else
			{
				ROS_DEBUG_STREAM_THROTTLE(2, "removing a feature with radius " << this->features.at(i).getDistanceFromFrameCenter());
			}
		}

		//finally set the local feature vector to the new featureVector
		this->features = cleanFeatures;
	}

	/*
	 * Checks all local frame features for whether or not a feature is outside of the kill radius.
	 * It will kill the feature if it is
	 * It will set the feature's distance to center if it is not
	 * this function ensures that the feature id's remain in ascending order
	 *
	 * overloaded:
	 * uses a referenced feature vector
	 */
	void cleanUpFeaturesByKillRadius(std::vector<VIOFeature2D>& feats, float killRadius)
	{
		cv::Point2f imageCenter = cv::Point2f((float)(this->image.cols / 2), (float)(this->image.rows / 2));
		std::vector<VIOFeature2D> cleanFeatures;
		for(int i = 0; i < features.size(); i++)
		{
			if(this->getAndSetFeatureRadius(features.at(i), imageCenter) <= killRadius)
			{
				cleanFeatures.push_back(features.at(i));
			}
			else
			{
				//ROS_DEBUG_STREAM_THROTTLE(2, "removing a feature with radius " << features.at(i).getDistanceFromFrameCenter());
			}
		}

		//finally set the local feature vector to the new featureVector
		features = cleanFeatures;
	}

	/*
	 * this will check the feature's radius and both set an its radius
	 */
	float getAndSetFeatureRadius(VIOFeature2D& feat, cv::Point2f imageCenter){
		float dx = feat.getFeaturePosition().x - imageCenter.x;
		float dy = feat.getFeaturePosition().y - imageCenter.y;
		feat.setDistanceFromFrameCenter(sqrt(dx * dx + dy * dy));
		return feat.getDistanceFromFrameCenter();
	}

	/*
	 * uses manhattan method to find distance between two pixels
	 */
	float manhattan(cv::Point2f p1, cv::Point2f p2){
		return abs(p2.x - p1.x) + abs(p2.y - p1.y);
	}

	/*
	 * removes redundant features from the clean vector by comparing
	 * its features with the compare vector and checking their
	 * distances with min_feature_dist
	 */
	void removeRedundantFeature(std::vector<VIOFeature2D>& toClean, std::vector<VIOFeature2D> compare, int min_feature_dist)
	{
		std::vector<VIOFeature2D> cleaned;
		bool removed = false;

		for(int i = 0; i < toClean.size(); i++)
		{
			for(int j = 0; j < compare.size(); j++)
			{
				if(manhattan(toClean.at(i).getFeature().pt, compare.at(j).getFeature().pt) < min_feature_dist)
				{
					removed = true;
					break;
				}
			}

			//if it has not been found to be redundant
			// push it back into the cleaned vector
			if(!removed)
			{
				cleaned.push_back(toClean.at(i));
			}

			removed = false;
		}

		//ROS_DEBUG_STREAM("After cleaning " << toClean.size() << " features, we are left with " << cleaned.size() << " features");
		toClean = cleaned;
	}

	/*
	 * this will use the fast algorithm to find new features in the image
	 * it will then kill all features outside the kill radius
	 * then it will compare each feature with all features in the frame and ensure that it is not the same
	 * finally the remaining features will be ranked and the top n features will be added to the feature vector.
	 *
	 * nFeatures: number of features to add to the frame
	 * fast_threshold: the threshold to use with the fast algorithm
	 * kill_radius: the radius to kill features at
	 * min_feature_dist: the minimum distance between new and old features
	 */
	void getAndAddNewFeatures(int nFeatures, int fast_threshold, float kill_radius, int min_feature_dist)
	{
		std::vector<VIOFeature2D> candidates;

		//get new features
		candidates = this->getFASTCorners(fast_threshold);

		//clean features by kill radius
		this->cleanUpFeaturesByKillRadius(candidates, kill_radius);

		//clean features by redundancy
		//ROS_DEBUG_STREAM("before " << candidates.size());
		this->removeRedundantFeature(candidates, this->features, min_feature_dist);
		//ROS_DEBUG_STREAM("after " << candidates.size());

		//rank the features
		this->rankFeatures(candidates, fast_threshold, kill_radius);

		//if after all this we have too few features only add all of them
		if(candidates.size() < nFeatures)
		{
			for(int i = 0; i < candidates.size(); i++)
			{
				this->addFeature(candidates.at(i));
			}
		}
		else
		{
			for(int i = 0; i < nFeatures; i++)
			{
				this->addFeature(candidates.at(i));
			}
		}

	}


};



#endif /* PAUVSI_M7_INCLUDE_PAUVSI_VO_FRAME_H_ */
