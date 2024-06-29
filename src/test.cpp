// dependencies
#include <functional>
#include <memory>
#include <thread>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cv_bridge/cv_bridge.h>
#include <iostream>
#include <cmath>
#include <yaml-cpp/yaml.h>

#include "rclcpp/rclcpp.hpp"

// msg
#include "msg_bank/msg/bottom_detection.hpp"

// UNTUK PATH WORKSPACE
#include <rcpputils/get_env.hpp>
#include <rcpputils/filesystem_helper.hpp>

using namespace cv;
using namespace std::placeholders;
using namespace std::chrono_literals;
using namespace std;
using namespace msg_bank::msg;

std::string workspaceDir = rcpputils::get_env_var("COLCON_PREFIX_PATH");
std::string masterYamlPath = workspaceDir + "/../src/yaml/master.yaml";
std::string colorYamlPath = workspaceDir + "/../src/yaml/color.yaml";

YAML::Node master = YAML::LoadFile(masterYamlPath);
YAML::Node color = YAML::LoadFile(colorYamlPath);


VideoCapture cap(master["indexBottom"].as<int>());

// HSV range variables
int h_low = color["h_low"].as<int>(), s_low =  color["s_low"].as<int>(), v_low =  color["v_low"].as<int>();
int h_hi =  color["h_hi"].as<int>(), s_hi =  color["s_hi"].as<int>(), v_hi =  color["v_hi"].as<int>();


class BottomDetectionNode : public rclcpp::Node
{
public:
    // Inisialisasi variabel
    Mat image;

    // BottomDetection Atributes
    BottomDetection center_object;

    // Publisher
    rclcpp::Publisher<BottomDetection>::SharedPtr center_object_publisher;
    // Timer Base
    rclcpp::TimerBase::SharedPtr timer_publisher;

    Mat detectColor(Mat &image)
    {
        Mat frame, hsv, gray, maskHSV, resultHSV, imgThresholded;
        // GaussianBlur(frame, matblur, Size(10,10), 0);
        cvtColor(image, hsv, COLOR_BGR2HSV);
        Scalar minHSV = Scalar(h_low, s_low, v_low);
        Scalar maxHSV = Scalar(h_hi, s_hi, v_hi);
        inRange(hsv, minHSV, maxHSV, maskHSV);
        bitwise_and(hsv, hsv, resultHSV, maskHSV);
        erode(maskHSV, maskHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
        dilate(maskHSV, maskHSV, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

        return maskHSV;
    }

    Point findCenter(Mat &maskHSV)
    {
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        Point center_point;
        findContours(maskHSV, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);
        int maxArea = 0;
        // drawContours(frame, contours, -1,(50,255,50),3);
        if (contours.size() != 0)
        {
            center_object.is_detected = true;
            for (size_t i = 0; i < contours.size(); ++i)
            {
                Rect boundRect = boundingRect(contours[i]);
                int area = boundRect.width * boundRect.height;
                if (area > maxArea)
                {
                    maxArea = area;
                    center_point.x = boundRect.x + boundRect.width / 2;
                    center_point.y = boundRect.y + boundRect.height / 2;
                }
            }
        }
        else
        {
            center_object.is_detected = false;
            center_point.x = 0;
            center_point.y = 0;
        }
        return center_point;
    }

    BottomDetectionNode() : Node("camera")
    {

        // Publisher
        this->center_object_publisher = this->create_publisher<BottomDetection>(
            "center_object", 10);

        // Timer base
        this->timer_publisher = this->create_wall_timer(33ms, std::bind(&BottomDetectionNode::center_callback, this));

        // Create trackbars
        namedWindow("My window");
        createTrackbar("Hue Min", "My window", &h_low, 255);
        createTrackbar("Sat Min", "My window", &s_low, 255);
        createTrackbar("Val Min", "My window", &v_low, 255);
        createTrackbar("Hue Max", "My window", &h_hi, 255);
        createTrackbar("Sat Max", "My window", &s_hi, 255);
        createTrackbar("Val Max", "My window", &v_hi, 255);

        // Check if the video capture is opened successfully
        if (!cap.isOpened())
        {
            RCLCPP_ERROR(this->get_logger(), "Error opening video stream or file");
            rclcpp::shutdown();
        }
    }

private:
    // callback center object
    void center_callback()
    {
        // Deklarasi variabel
        Mat maskHSV;
        Point center_point, center_window;

        cap.read(this->image);

        maskHSV = detectColor(this->image);

        waitKey(1);

        center_point = findCenter(maskHSV);

        center_window = Point(image.cols / 2, image.rows / 2);

        center_object.x_center = center_point.x;
        center_object.y_center = center_point.y;
        center_object.center_window_x = center_window.x;
        center_object.center_window_y = center_window.y;

        circle(image, center_point, 4, Scalar(0, 255, 255), -1);
        circle(image, center_window, 4, Scalar(0, 255, 255), -1);

        imshow("contours", maskHSV);
        imshow("frame", image);

        center_object_publisher->publish(this->center_object);
    }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BottomDetectionNode>());
    rclcpp::shutdown();
    return 0;
}