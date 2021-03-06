cmake_minimum_required(VERSION 2.8.3)
project(pauvsi_vio)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  image_transport
  roscpp
  sensor_msgs
  std_msgs
  dynamic_reconfigure
  cmake_modules
  tf
)

find_package(Eigen3 REQUIRED)

find_package(OpenCV REQUIRED)


include_directories(
	${OpenCV_INCLUDE_DIRS}
	include
	include/pauvsi_vio
	${catkin_INCLUDE_DIRS}
	${Eigen_INCLUDE_DIRS}
)

catkin_package(
   INCLUDE_DIRS include
#  LIBRARIES pauvsi_vio
#  CATKIN_DEPENDS cv_bridge image_transport roscpp sensor_msgs std_msgs
#  DEPENDS system_lib
   DEPENDS eigen opencv
)

add_library(keyframeinfo include/pauvsi_vio/KeyFrameInfo.hpp)
set_target_properties(keyframeinfo PROPERTIES LINKER_LANGUAGE CXX)

add_library(visualmeasurement include/pauvsi_vio/VisualMeasurement.hpp)
set_target_properties(visualmeasurement PROPERTIES LINKER_LANGUAGE CXX)

add_library(viostate include/pauvsi_vio/VIOState.hpp)
set_target_properties(viostate PROPERTIES LINKER_LANGUAGE CXX)

add_library(egoMotionEstimator include/pauvsi_vio/EgoMotionEstimator.hpp)
set_target_properties(egoMotionEstimator PROPERTIES LINKER_LANGUAGE CXX)

add_library(vioekf include/pauvsi_vio/VIOEKF.cpp)

add_library(featureTracker include/pauvsi_vio/FeatureTracker.cpp)

add_library(vioLine include/pauvsi_vio/VIOLine.hpp)
set_target_properties(vioLine PROPERTIES LINKER_LANGUAGE CXX)

add_library(vioFeature3d include/pauvsi_vio/VIOFeature3D.hpp)
set_target_properties(vioFeature3d PROPERTIES LINKER_LANGUAGE CXX)

add_library(vioFeature2d include/pauvsi_vio/VIOFeature2D.hpp)
set_target_properties(vioFeature2d PROPERTIES LINKER_LANGUAGE CXX)

add_library(frame include/pauvsi_vio/Frame.hpp)
set_target_properties(frame PROPERTIES LINKER_LANGUAGE CXX)

add_library(vio include/pauvsi_vio/vio.cpp include/pauvsi_vio/vioMotion.cpp include/pauvsi_vio/vioDraw.cpp include/pauvsi_vio/vioTriangulate.cpp)

add_executable(pauvsi_vio src/pauvsi_vio.cpp)
target_link_libraries(visualmeasurement ${catkin_LIBRARIES} ${Eigen_LIBRARIES})
target_link_libraries(viostate ${catkin_LIBRARIES} ${Eigen_LIBRARIES} ${OpenCV_LIBRARIES} visualmeasurement)
target_link_libraries(vioekf ${catkin_LIBRARIES} ${Eigen_LIBRARIES} viostate visualmeasurement)
target_link_libraries(egoMotionEstimator frame ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} vioFeature3d vioFeature2d visualmeasurement)
target_link_libraries(featureTracker frame ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})
target_link_libraries(vioFeature3d vioLine)
target_link_libraries(frame vioFeature3d ${Eigen_LIBRARIES})
target_link_libraries(frame vioFeature2d viostate)
target_link_libraries(vio frame ${catkin_LIBRARIES} featureTracker vioekf egoMotionEstimator viostate visualmeasurement vioFeature3d keyframeinfo)
target_link_libraries(pauvsi_vio ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Eigen_LIBRARIES} vio)

