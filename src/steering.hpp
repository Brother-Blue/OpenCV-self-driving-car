//
// Created by hassan on 2021-05-14.
//

#ifndef STEERING_STEERING_H
#define STEERING_STEERING_H
// Include the OpenDLV Standard Message Set that contains messages that are usually exchanged for automotive or robotic applications
#include "opendlv-standard-message-set.hpp"

// Include the GUI and image processing header files from OpenCV
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
class Steering {
    public:
        double steeringAccuracy();
        bool isOnLeft(Point pos);
        double getDistance(cv::Point pos1, cv::Point pos2);
        double trackCones();
        bool getBlueCones(cv::Mat detectImage, cv::Mat drawImage, cv::Scalar color);
        bool getYellowCones(cv::Mat detectImage, cv::Mat drawImage, cv::Scalar color);
};
#endif //STEERING_STEERING_H
