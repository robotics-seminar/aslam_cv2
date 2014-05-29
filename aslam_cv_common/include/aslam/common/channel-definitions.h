#ifndef ASLAM_CV_COMMON_CHANNEL_DEFINITIONS_H_
#define ASLAM_CV_COMMON_CHANNEL_DEFINITIONS_H_

#include <Eigen/Dense>
#include <aslam/common/channel-declaration.h>

DECLARE_CHANNEL(VISUAL_KEYPOINT_MEASUREMENTS, Eigen::Matrix2Xd);
DECLARE_CHANNEL(VISUAL_KEYPOINT_MEASUREMENT_UNCERTAINTIES, Eigen::VectorXd);
DECLARE_CHANNEL(VISUAL_KEYPOINT_ORIENTATIONS, Eigen::VectorXd);
DECLARE_CHANNEL(VISUAL_KEYPOINT_SCALES, Eigen::VectorXd);
DECLARE_CHANNEL(BRISK_DESCRIPTORS, Eigen::Matrix<char, Eigen::Dynamic, Eigen::Dynamic>);

#endif  // ASLAM_CV_COMMON_CHANNEL_DEFINITIONS_H_
