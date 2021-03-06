/*
 * vioDraw.cpp
 *
 *  Created on: Jan 16, 2017
 *      Author: kevin
 */


#include "vio.h"

cv::Mat drawEpiLines(cv::Matx33f F, cv::Point2f pt, cv::Matx33f tK, cv::Mat img)
{
	cv::Matx31f u;

	u(0) = pt.x;
			u(1) = pt.y;
			//u(2) = 1.0;
			/*u = tK * u;
			u(0) /= u(2);
			u(1) /= u(2);*/

			// Draw the epipolar lines
			std::vector<cv::Vec3f> lines1;
			std::vector<cv::Point2f> ptt;
			ptt.push_back(cv::Point2f(u(0), u(1)));
			cv::computeCorrespondEpilines(ptt, 2, F, lines1);

			//ROS_DEBUG_STREAM("abc: " << lines1[0]);

			cv::Matx31f pt1, pt2;
			pt1(0) = -2;
			pt1(1) = -(lines1[0][2] + lines1[0][0]*-2)/lines1[0][1];
			pt1(2) = 1.0;
			pt1 = tK * pt1;

			pt2(0) = 2;
			pt2(1) = -(lines1[0][2]+lines1[0][0]*2)/lines1[0][1];
			pt2(2) = 1.0;
			pt2 = tK * pt2;

			cv::line(img,
					cv::Point(pt1(0), pt1(1)),
					cv::Point(pt2(0), pt2(1)),
					cv::Scalar(rand()%255, rand()%255, rand()%255));

			return img;
}

void VIO::drawKeyFrames()
{
	cv::Mat img1, img2, img3, img4, img5;
	img1 = currentFrame().image;
	img2 = frameBuffer.at(keyFrames.at(0).frameBufferIndex).image;
	img3 = frameBuffer.at(keyFrames.at(1).frameBufferIndex).image;
	img4 = frameBuffer.at(keyFrames.at(2).frameBufferIndex).image;
	img5 = frameBuffer.at(keyFrames.at(3).frameBufferIndex).image;

	cv::cvtColor(img1, img1, CV_GRAY2BGR);
	cv::cvtColor(img2, img2, CV_GRAY2BGR);
	cv::cvtColor(img3, img3, CV_GRAY2BGR);
	cv::cvtColor(img4, img4, CV_GRAY2BGR);
	cv::cvtColor(img5, img5, CV_GRAY2BGR);

	cv::Matx33f F;
	cv::Matx33f tK;

	//do work on images
	Frame cf = currentFrame();
	cf.K.copyTo(tK);

	for(int i = 0; i < cf.features.size(); i++)
	{
		cv::Matx31f u;
		u(0) = cf.features.at(i).getFeaturePosition().x;
		u(1) = cf.features.at(i).getFeaturePosition().y;

		cv::drawMarker(img1, cv::Point2f(u(0), u(1)), cv::Scalar(0, 255, 255), cv::MARKER_DIAMOND, 6);
	}
	F = keyFrames.at(0).F;
	//ROS_DEBUG_STREAM("feat: " <<  keyFrames.at(0).matchedFeatures.size());
	for(auto e : keyFrames.at(0).matchedFeatures)
	{
		cv::Matx31f u;
		u(0) = e.getFeaturePosition().x;
		u(1) = e.getFeaturePosition().y;

		cv::drawMarker(img2, cv::Point2f(u(0), u(1)), cv::Scalar(0, 255, 255), cv::MARKER_DIAMOND, 12);

		img2 = drawEpiLines(F, e.getUndistorted(), tK, img2);
	}
	//F = keyFrames.at(1).F;
	for(auto e : keyFrames.at(1).matchedFeatures)
	{
		cv::Matx31f u;
		u(0) = e.getFeaturePosition().x;
		u(1) = e.getFeaturePosition().y;

		cv::drawMarker(img3, cv::Point2f(u(0), u(1)), cv::Scalar(0, 255, 255), cv::MARKER_DIAMOND, 12);

		//img3 = drawEpiLines(F, e.getUndistorted(), tK, img3);
	}
	//F = keyFrames.at(2).F;
	for(auto e : keyFrames.at(2).matchedFeatures)
	{
		cv::Matx31f u;
		u(0) = e.getFeaturePosition().x;
		u(1) = e.getFeaturePosition().y;

		cv::drawMarker(img4, cv::Point2f(u(0), u(1)), cv::Scalar(0, 255, 255), cv::MARKER_DIAMOND, 12);

		//img4 = drawEpiLines(F, e.getUndistorted(), tK, img4);
	}

	//F = tK.t() * keyFrames.at(3).F * tK; // convert the essential mat into the fundamental mat
	//F = keyFrames.at(3).F;

	for(auto e : keyFrames.at(3).matchedFeatures)
	{
		cv::Matx31f u;
		u(0) = e.getFeaturePosition().x;
		u(1) = e.getFeaturePosition().y;

		cv::drawMarker(img5, cv::Point2f(u(0), u(1)), cv::Scalar(0, 255, 255), cv::MARKER_DIAMOND, 12);

		//img5 = drawEpiLines(F, e.getUndistorted(), tK, img5);

	}



	cv::Mat img2_s, img3_s, img4_s, img5_s;
	cv::resize(img2, img2_s, cv::Size(320, 256));
	cv::resize(img3, img3_s, cv::Size(320, 256));
	cv::resize(img4, img4_s, cv::Size(320, 256));
	cv::resize(img5, img5_s, cv::Size(320, 256));


	cv::Mat final = cv::Mat(cv::Size(640, 1024), CV_8UC3);
	//cv::Mat roi1 = cv::Mat(final, cv::Rect(0, 0, 640, 512));
	img1.copyTo(final(cv::Rect(0, 0, 640, 512)));
	//cv::Mat roi2 = cv::Mat(final, cv::Rect(0, 512, 320, 256));
	img2_s.copyTo(final(cv::Rect(0, 512, 320, 256)));
	//cv::Mat roi3 = cv::Mat(final, cv::Rect(320, 512, 320, 256));
	img3_s.copyTo(final(cv::Rect(320, 512, 320, 256)));
	//cv::Mat roi4 = cv::Mat(final, cv::Rect(0, 768, 320, 256));
	img4_s.copyTo(final(cv::Rect(0, 768, 320, 256)));
	//cv::Mat roi5 = cv::Mat(final, cv::Rect(320, 768, 320, 256));
	img5_s.copyTo(final(cv::Rect(320, 768, 320, 256)));


	//cv::namedWindow("debug", cv::WINDOW_AUTOSIZE);
	cv::imshow("debug", final);
	cv::waitKey(1);
}

// LEGACY - VERSION 2

template <typename T1, typename T2>
void VIO::drawEpipolarLines(const std::string& title, const cv::Matx<T1,3,3> F,
		const cv::Mat& img1, const cv::Mat& img2,
		const std::vector<cv::Point_<T2> > points1,
		const std::vector<cv::Point_<T2> > points2,
		const float inlierDistance)
{
	CV_Assert(img1.size() == img2.size() && img1.type() == img2.type());
	cv::Mat outImg(img1.rows, img1.cols*2, CV_8UC3);
	cv::Rect rect1(0,0, img1.cols, img1.rows);
	cv::Rect rect2(img1.cols, 0, img1.cols, img1.rows);
	/*
	 * Allow color drawing
	 */
	if (img1.type() == CV_8U)
	{
		cv::cvtColor(img1, outImg(rect1), CV_GRAY2BGR);
		cv::cvtColor(img2, outImg(rect2), CV_GRAY2BGR);
	}
	else
	{
		img1.copyTo(outImg(rect1));
		img2.copyTo(outImg(rect2));
	}
	std::vector<cv::Vec<T2,3> > epilines1, epilines2;
	cv::computeCorrespondEpilines(points1, 1, F, epilines1); //Index starts with 1
	cv::computeCorrespondEpilines(points2, 2, F, epilines2);

	CV_Assert(points1.size() == points2.size() &&
			points2.size() == epilines1.size() &&
			epilines1.size() == epilines2.size());

	cv::RNG rng(0);
	for(size_t i=0; i<points1.size(); i++)
	{
		if(inlierDistance > 0)
		{
			if(distancePointLine(points1[i], epilines2[i]) > inlierDistance ||
					distancePointLine(points2[i], epilines1[i]) > inlierDistance)
			{
				//The point match is no inlier
				continue;
			}
		}
		/*
		 * Epipolar lines of the 1st point set are drawn in the 2nd image and vice-versa
		 */
		cv::Scalar color(rng(256),rng(256),rng(256));

		cv::line(outImg(rect2),
				cv::Point(0,-epilines1[i][2]/epilines1[i][1]),
				cv::Point(img1.cols,-(epilines1[i][2]+epilines1[i][0]*img1.cols)/epilines1[i][1]),
				color);
		cv::circle(outImg(rect1), points1[i], 3, color, -1, CV_AA);

		cv::line(outImg(rect1),
				cv::Point(0,-epilines2[i][2]/epilines2[i][1]),
				cv::Point(img2.cols,-(epilines2[i][2]+epilines2[i][0]*img2.cols)/epilines2[i][1]),
				color);
		cv::circle(outImg(rect2), points2[i], 3, color, -1, CV_AA);
	}
	cv::imshow(title, outImg);
	cv::waitKey(1);
}

template <typename T>
float VIO::distancePointLine(const cv::Point_<T> point, const cv::Vec<T,3>& line)
{
	//Line is given as a*x + b*y + c = 0
	return abs(line(0)*point.x + line(1)*point.y + line(2)) / sqrt(line(0)*line(0)+line(1)*line(1));
}

void VIO::viewMatches(std::vector<VIOFeature2D> ft1, std::vector<VIOFeature2D> ft2, Frame f1, Frame f2, std::vector<cv::Point2f> pt1_new, std::vector<cv::Point2f> pt2_new)
{
	cv::Mat img1 = f1.image;
	cv::Mat img2 = f2.image;

	cv::cvtColor(img1, img1, CV_GRAY2BGR);
	cv::cvtColor(img2, img2, CV_GRAY2BGR);

	cv::Matx33f tK = currentFrame().K;

	for(int i = 0; i < f1.features.size(); i++)
	{
		cv::Matx31f u;
		u(0) = f1.features.at(i).getUndistorted(true).x;
		u(1) = f1.features.at(i).getUndistorted(true).y;
		u(2) = 1.0;
		u = tK * u;

		cv::drawMarker(img1, cv::Point2f(u(0) / u(2), u(1) / u(2)), cv::Scalar(255, 0, 0), cv::MARKER_DIAMOND, 4);
	}

	for(int i = 0; i < f2.features.size(); i++)
	{
		cv::Matx31f u;
		u(0) = f2.features.at(i).getUndistorted(true).x;
		u(1) = f2.features.at(i).getUndistorted(true).y;
		u(2) = 1.0;
		u = tK * u;

		cv::drawMarker(img2, cv::Point2f(u(0) / u(2), u(1) / u(2)), cv::Scalar(255, 0, 0), cv::MARKER_DIAMOND, 4);
	}

	for(int i = 0; i < ft1.size(); i++)
	{
		cv::drawMarker(img1, ft1.at(i).getFeaturePosition(), cv::Scalar(0, 255, 0), cv::MARKER_SQUARE, 10);
	}

	for(int i = 0; i < ft2.size(); i++)
	{
		cv::drawMarker(img2, ft2.at(i).getFeaturePosition(), cv::Scalar(0, 255, 0), cv::MARKER_SQUARE, 10);
	}

	for(int i = 0; i < pt1_new.size(); i++)
	{
		cv::Matx31f u;
		u(0) = pt1_new.at(i).x;
		u(1) = pt1_new.at(i).y;
		u(2) = 1.0;
		u = tK * u;

		cv::drawMarker(img1, cv::Point2f(u(0) / u(2), u(1) / u(2)), cv::Scalar(0, 0, 255), cv::MARKER_TRIANGLE_UP, 8);
	}

	for(int i = 0; i < pt2_new.size(); i++)
	{
		cv::Matx31f u;
		u(0) = pt2_new.at(i).x;
		u(1) = pt2_new.at(i).y;
		u(2) = 1.0;
		u = tK * u;

		cv::drawMarker(img2, cv::Point2f(u(0) / u(2), u(1) / u(2)), cv::Scalar(0, 0, 255), cv::MARKER_TRIANGLE_UP, 8);
	}

	//img1 = this->reproject3dPoints(img1, f1.state);
	//img2 = this->reproject3dPoints(img2, f2.state);

	cv::Mat img;
	cv::vconcat(img2, img1, img);

	cv::imshow("matches", img);
	cv::waitKey(30);
}

cv::Mat VIO::reproject3dPoints(cv::Mat img_in, VIOState x)
{
	tf::StampedTransform base2cam;
	try{
		this->ekf.tf_listener.lookupTransform(this->camera_frame, this->CoM_frame, ros::Time(0), base2cam);
	}
	catch(tf::TransformException& e){
		ROS_WARN_STREAM(e.what());
	}

	tf::Transform cam2world = (tf::Transform(x.getTFQuaternion(), tf::Vector3(x.x(), x.y(), x.z())) * base2cam).inverse();

	tf::Quaternion tf_q = cam2world.getRotation();
	cv::Mat temp = img_in;

	Eigen::Quaternionf q;
	q.w() = tf_q.w();
	q.x() = tf_q.x();
	q.y() = tf_q.y();
	q.z() = tf_q.z();

	cv::Matx33f tK = currentFrame().K;

	cv::Matx33f R;
	cv::eigen2cv(q.matrix(), R);

	cv::Matx31f t;
	t(0) = cam2world.getOrigin().getX();
	t(1) = cam2world.getOrigin().getY();
	t(2) = cam2world.getOrigin().getZ();

	cv::Matx34f P;
	cv::hconcat(R, t, P);

	//ROS_DEBUG_STREAM(active3DFeatures.begin() << " and " << active3DFeatures.end());

	for(int i = 0; i < active3DFeatures.size(); i++)
	{
		cv::Matx41f X;
		X(0) = active3DFeatures.at(i).position(0);
		X(1) = active3DFeatures.at(i).position(1);
		X(2) = active3DFeatures.at(i).position(2);
		X(3) = 1.0;

		cv::Matx31f u = tK * P * X;

		cv::drawMarker(temp, cv::Point2f(u(0) / u(2), u(1) / u(2)), cv::Scalar(0, 255, 255), cv::MARKER_CROSS, 6, 1);
		ROS_DEBUG_STREAM("reproj Pt: " << u(0)/u(2) << ", " << u(1)/u(2));
	}

	return temp;
}

/*
 * shows cv::Mat
 */
void VIO::viewImage(cv::Mat img){
	cv::imshow("test", img);
	cv::waitKey(30);
}

/*
 * draws frame with its features
 */
void VIO::viewImage(Frame frame){
	cv::Mat img;
	cv::drawKeypoints(frame.image, frame.getKeyPointVectorFromFeatures(), img, cv::Scalar(0, 0, 255));
	cv::drawKeypoints(img, frame.getUndistortedKeyPointVectorFromFeatures(), img, cv::Scalar(255, 0, 0));
	this->viewImage(img);

}

