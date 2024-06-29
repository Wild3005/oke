#include "ros/ros.h"
#include <functional>
#include <memory>
#include <thread>
#include <iostream>
#include <cmath>
#include "opencv2/opencv.hpp"
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <message_file/box_trash.h>

// // HSV range variables
// int h_low = color["h_low"].as<int>(), s_low =  color["s_low"].as<int>(), v_low =  color["v_low"].as<int>();
// int h_hi =  color["h_hi"].as<int>(), s_hi =  color["s_hi"].as<int>(), v_hi =  color["v_hi"].as<int>();

class BottomDetectionNode {
public:
    // Inisialisasi variabel
    cv::Mat image;
    cv::VideoCapture cap;

    // BottomDetection Attributes
    message_file::center_msgs data_message;

    // Publisher
    ros::Publisher center_object_publisher;

    // Timer
    ros::Timer timer_publisher;

    BottomDetectionNode() {
        // NodeHandle
        ros::NodeHandle nh;

        // Publisher
        this->center_object_publisher = this->nh.advertise<std_msgs::String>("center_object", 1000);

        // Timer
        this->timer_publisher = this->nh.createTimer(ros::Duration(0.033), &BottomDetectionNode::center_callback, this);

    // // Create trackbars
    //     namedWindow("My window");
    //     createTrackbar("Hue Min", "My window", &h_low, 255);
    //     createTrackbar("Sat Min", "My window", &s_low, 255);
    //     createTrackbar("Val Min", "My window", &v_low, 255);
    //     createTrackbar("Hue Max", "My window", &h_hi, 255);
    //     createTrackbar("Sat Max", "My window", &s_hi, 255);
    //     createTrackbar("Val Max", "My window", &v_hi, 255);

        // Video capture initialization
        cap.open(0);
        if (!cap.isOpened()) {
            ROS_ERROR("Failed to open video capture device");
            return;
        }
    }

    cv::Point findCenter(cv::Mat& maskHSV) {
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::Point center_point;
        cv::findContours(maskHSV, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        int maxArea = 0;

        if (!contours.empty()) {
            data_message.is_detected = true;
            for (size_t i = 0; i < contours.size(); ++i) {
                cv::Rect boundRect = cv::boundingRect(contours[i]);
                int area = boundRect.width * boundRect.height;
                if (area > maxArea) {
                    maxArea = area;
                    center_point.x = boundRect.x + boundRect.width / 2;
                    center_point.y = boundRect.y + boundRect.height / 2;
                }
            }
        } else {
            data_message.is_detected = false;
            center_point.x = 0;
            center_point.y = 0;
        }
        return center_point;
    }

    cv::Mat detectColor(cv::Mat &image) {
        cv::Mat hsv, maskHSV;
        cv::cvtColor(image, hsv, cv::COLOR_BGR2HSV);
        cv::Scalar minHSV = cv::Scalar(0, 100, 100);  // Adjust as needed
        cv::Scalar maxHSV = cv::Scalar(10, 255, 255); // Adjust as needed
        cv::inRange(hsv, minHSV, maxHSV, maskHSV);
        cv::erode(maskHSV, maskHSV, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
        cv::dilate(maskHSV, maskHSV, cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5)));
        return maskHSV;
    }

private:
    void center_callback(const ros::TimerEvent&) {
        // Deklarasi variabel
        cv::Mat maskHSV;
        cv::Point center_point, center_window;

        cap.read(this->image);

        maskHSV = detectColor(this->image);

        cv::waitKey(1);

        center_point = findCenter(maskHSV);

        center_window = cv::Point(image.cols / 2, image.rows / 2);

        center_object.x_center = center_point.x;
        center_object.y_center = center_point.y;
        cv::circle(image, center_point, 4, cv::Scalar(0, 255, 255), -1);
        cv::circle(image, center_window, 4, cv::Scalar(0, 255, 255), -1);

        cv::imshow("contours", maskHSV);
        cv::imshow("frame", image);

        center_object_publisher.publish(this->center_object);
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "bottom_detection_node");
    BottomDetectionNode node;
    ros::spin();
    return 0;
}
