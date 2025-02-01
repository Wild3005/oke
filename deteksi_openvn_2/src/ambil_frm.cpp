#include <iostream>
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.hpp>


class Get_Frame : public rclcpp::Node{
public:
Get_Frame():Node("Get_Frame"){
cap_.open("/dev/v4l/by-id/usb-BC-231018-A_XWF_1080P_PC_Camera-video-index0", cv::CAP_V4L2);
  if(!cap_.isOpened()){
    cap_.open("/dev/v4l/by-id/usb-Generic_HP_TrueVision_HD_Camera_0001-video-index0", cv::CAP_V4L2);
    if(!cap_.isOpened()){
      RCLCPP_ERROR(this->get_logger(), "Error opening camera");
      rclcpp::shutdown();
    }
  }
  timer_ = this->create_wall_timer(std::chrono::milliseconds(30), std::bind(&Get_Frame::get_, this));
}

void init(){
  it_ = std::make_shared<image_transport::ImageTransport>(this->shared_from_this());
  img_publisher_ = it_->advertise("topic2", 10);
}
private:
cv::VideoCapture cap_;
rclcpp::TimerBase::SharedPtr timer_;
std::shared_ptr<image_transport::ImageTransport> it_;
image_transport::Publisher img_publisher_;

void get_(){
  cv::Mat frame;
  cap_ >> frame;
  if(frame.empty()){
    RCLCPP_WARN(this->get_logger(),"frame kosong");
    return;
  }
  sensor_msgs::msg::Image::SharedPtr msg = cv_bridge::CvImage(std_msgs::msg::Header(),"bgr8",frame).toImageMsg();
  img_publisher_.publish(msg);
}
};

int main(int argc, char ** argv){
  rclcpp::init(argc,argv);
  auto node = std::make_shared<Get_Frame>();
  node->init();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}