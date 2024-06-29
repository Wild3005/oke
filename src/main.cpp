// pub velocity to serial
// sub joy odometry vision
// Description: Main node for robot control
// romusa_messages adalah message global yang digunakan oleh semua node
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sensor_msgs/Joy.h>
#include <geometry_msgs/Twist.h>
#include <iostream>
#include <cmath>
#include "sensor_msgs/LaserScan.h"
#include <vector>

#include <chrono>
#include <thread>

#include <romusa_messages/ObjectDetection.h>
#include <romusa_messages/odom.h>
#include <romusa_messages/cmd_vel.h>
#include <romusa_messages/cmd_slider.h>
#include <romusa_messages/vac.h>
#include <romusa_messages/enc_slider.h>
#include <romusa_messages/posGlob.h>

#include <romusarival/main.hpp>
#include <romusarival/filter.hpp>
#include "ros/ros.h"

JoyButton jb;
sequenceMove stateRobot; // cant use idk why

ros::Time current_time;

ros::Publisher pub_vel;
ros::Publisher pub_slider;
ros::Publisher pub_vacuum;
ros::Publisher pub_posGlobalRos;

ros::Subscriber sub_joy;
ros::Subscriber sub_odom;
ros::Subscriber sub_vision;
ros::Subscriber sub_encSlider;
ros::Subscriber lidar_sub;

// position control
void jalan_y(float SP_Position[3], float SP_Speed[3]);
void jalan_x(float SP_POS[3], float SP_SPEED[3]);

// callback to subscribe msg
void odom_callback(const romusa_messages::odom::ConstPtr &odom_msg);
void slider_callback(const romusa_messages::enc_slider::ConstPtr &enc_slider_msg);
void vision_callback(const romusa_messages::ObjectDetection::ConstPtr &object_detection_msg);
void joy_callback(const sensor_msgs::Joy::ConstPtr &msg);
void lidar_callback(const sensor_msgs::LaserScan::ConstPtr &msg);

int main(int argc, char **argv)
{
  ros::init(argc, argv, "main_node");
  ros::NodeHandle nh;
  ros::Rate loop_rate(100);

  sub_joy = nh.subscribe<sensor_msgs::Joy>("joy", 1, joy_callback);
  sub_odom = nh.subscribe<romusa_messages::odom>("odom", 1000, odom_callback);
  sub_vision = nh.subscribe<romusa_messages::ObjectDetection>("object_detection", 1000, vision_callback);
  sub_encSlider = nh.subscribe<romusa_messages::enc_slider>("encSlider", 1000, slider_callback);
  lidar_sub = nh.subscribe<sensor_msgs::LaserScan>("scan", 10, lidar_callback);

  pub_vel = nh.advertise<romusa_messages::cmd_vel>("velocity", 1000);
  pub_slider = nh.advertise<romusa_messages::cmd_slider>("slider", 1000);
  pub_vacuum = nh.advertise<romusa_messages::vac>("vacuum", 1000);
  pub_posGlobalRos = nh.advertise<romusa_messages::posGlob>("posGlobal", 1000);

  while (ros::ok())
  {
    romusa_messages::cmd_vel vel_msg;
    romusa_messages::cmd_slider slider_msg;
    romusa_messages::vac vac_msg;
    romusa_messages::posGlob posGlobalRos_msg;

    static ros::Time previous_time;

    // publish position calculate by ROS
    posGlobalRos_msg.Xpos = Odom_Pose[0];
    posGlobalRos_msg.Ypos = Odom_Pose[1];
    posGlobalRos_msg.YawPos = RoboPose_Global[2];

    // if (status_button == 1)
    // {
    //   status_auto = 1;
    //   printf("button pressed, Go!!!\n");
    // }
    if (jb.buttons[1] == 1)
    {
      status_auto = 1;
      printf("button pressed, Go!!!\n");
    }
    if (jb.buttons[2] == 1)
    {
      status_auto = 0;
      printf("Manual\n");
      ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], RoboPose_Global[2], x_lidar, y_lidar_new);
    }
    switch (status_auto) // undian A
    {
    case 0:
      if (jb.axes[1] > 0.1 || jb.axes[1] < -0.1)
      {
        RoboSpeed[0] = 0.5 * jb.axes[1];
      }
      else
      {
        RoboSpeed[0] = 0;
      }
      if (jb.axes[0] > 0.1 || jb.axes[0] < -0.1)
      {
        RoboSpeed[1] = 0.5 * jb.axes[0];
      }
      else
      {
        RoboSpeed[1] = 0;
      }
      if (jb.axes[2] > 0.1 || jb.axes[2] < -0.1)
      {
        RoboSpeed[2] = 4 * jb.axes[2];
      }
      else
      {
        RoboSpeed[2] = 0;
      }
      if (jb.buttons[3] == 1) // triangel
      {
        vac_msg.stat_pneumatic = 1;
      }
      if (jb.buttons[4] == 1) // L1
      {
        slider_msg.target_arm = 1580;
        slider_msg.limit_arm = 200;
      }
      if (jb.buttons[6] == 1) // L2
      {
        slider_msg.target_arm = 0;
        slider_msg.limit_arm = 200;
      }
      if (jb.buttons[5] == 1) // R1
      {
        slider_msg.target_lifter = 610;
        slider_msg.limit_lifter = 200;
      }
      if (jb.buttons[7] == 1) // R2
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
      }
      if (jb.buttons[9] == 1) // options
      {
        vac_msg.stat_vacuum = 1;
      }
      if (jb.buttons[0] == 1)
      {
        FirstRun = 0;
      }
      vel_msg.velX = RoboSpeed[0];
      vel_msg.velY = RoboSpeed[1];
      vel_msg.velTheta = RoboSpeed[2];
      break;
    case 1: // maju ke sampah 5
      target[0] = 1.16;
      target[1] = -0.02;
      target[2] = 0;
      speed[0] = 1.0;
      speed[1] = 0.15;
      speed[2] = 2.0;
      jalan_x(target, speed);
      vel_msg.velX = RoboSpeed[0];
      vel_msg.velY = RoboSpeed[1];
      vel_msg.velTheta = RoboSpeed[2];
      if (fabs(target[0] - x_lidar) < 0.02)
      {
        slider_msg.target_lifter = 220;
        slider_msg.limit_lifter = 150;
        if (fabs(slider_msg.target_lifter - posLifter) < 10)
        {
          // status_auto = 3;
          status_auto = 14;
          prev_state = 2;
          ROS_INFO("ROMUSA get Sampah 5");
          ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], RoboPose_Global[2], x_lidar, y_lidar_new);
          previous_time = ros::Time::now();
        }
      }
      break;
    case 3:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -0.3;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 810;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 10)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 30)
            {
              // status_auto = 4;
              status_auto = 16;
              prev_state = 3;
              ROS_INFO("ROMUSA get Sampah 6");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 4:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -0.6;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 0;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter)< 30)
            {
              // status_auto = 5;
              status_auto = 12;
              prev_state = 4;
              ROS_INFO("ROMUSA get Sampah 7");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 5:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -0.9;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 810;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 30)
            {
              // status_auto = 6;
              status_auto = 15;
              prev_state = 5;
              ROS_INFO("ROMUSA get Sampah 8");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 6:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -1.2;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 0;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter)< 30)
            {
              // status_auto = 7;
              status_auto = 14;
              prev_state = 6;
              ROS_INFO("ROMUSA get Sampah 9");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 7:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -1.5;
        target[2] = 0;
        speed[0] = 0.01;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 850;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm)< 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter)< 30)
            {
              // status_auto = 8;
              status_auto = 14;
              prev_state = 7;
              ROS_INFO("ROMUSA get Sampah 10");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 8:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = 0.3;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 810;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (slider_msg.target_lifter - posLifter < 30)
            {
              // status_auto = 9;
              status_auto = 15;
              prev_state = 8;
              ROS_INFO("ROMUSA get Sampah 4");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 9:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = 0.6;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 0;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm)< 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 30)
            {
              // status_auto = 10;
              status_auto = 14;
              prev_state = 9;
              ROS_INFO("ROMUSA get Sampah 3");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 10:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = 0.9;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 810;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 30)
            {
              // status_auto = 11;
              status_auto = 13;
              prev_state = 10;
              ROS_INFO("ROMUSA get Sampah 2");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 11:
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = 1.2;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 0;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 20)
          {
            slider_msg.target_lifter = 250;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 30)
            {
              // status_auto = 0;
              status_auto = 16;
              prev_state = -1;
              ROS_INFO("ROMUSA get Sampah 1");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 12: // ferro
      vac_msg.stat_vacuum = 1;
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -0.55;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 1580;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 30)
          {
            slider_msg.target_lifter = 500;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 10)
            { 
              status_auto = prev_state + 1;
              ROS_INFO("ROMUSA put Ferro");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 13: // non ferro
      vac_msg.stat_vacuum = 1;
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -0.35;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 1580;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm)< 30)
          {
            slider_msg.target_lifter = 500;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 10)
            {
              status_auto = prev_state + 1;
              ROS_INFO("ROMUSA put Non Ferro");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 14: // plastik
      vac_msg.stat_vacuum = 1;
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = -0.1;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 1580;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 30)
          {
            slider_msg.target_lifter = 500;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 10)
            {
              status_auto = prev_state + 1;
              ROS_INFO("ROMUSA put Plastik");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 15: // kertas
      vac_msg.stat_vacuum = 1;
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = 0.1;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 1580;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 30)
          {
            slider_msg.target_lifter = 500;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 10)
            {
              status_auto = prev_state + 1;
              ROS_INFO("ROMUSA put Kertas");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    case 16: // daun
      vac_msg.stat_vacuum = 1;
      if (ros::Time::now() - previous_time > ros::Duration(2.0))
      {
        slider_msg.target_lifter = 0;
        slider_msg.limit_lifter = 200;
        target[0] = 1.16;
        target[1] = 0.25;
        target[2] = 0;
        speed[0] = 0.2;
        speed[1] = 0.5;
        speed[2] = 2.0;
        jalan_y(target, speed);
        vel_msg.velX = RoboSpeed[0];
        vel_msg.velY = RoboSpeed[1];
        vel_msg.velTheta = RoboSpeed[2];
        if (fabs(target[1] - Odom_Pose[1]) < 0.01)
        {
          slider_msg.target_arm = 1580;
          slider_msg.limit_arm = 200;
          if (fabs(slider_msg.target_arm - posArm) < 30)
          {
            slider_msg.target_lifter = 500;
            slider_msg.limit_lifter = 200;
            if (fabs(slider_msg.target_lifter - posLifter) < 10)
            {
              status_auto = prev_state + 1;
              ROS_INFO("ROMUSA put Daun");
              ROS_INFO("Odom Pose: %f %f %f ||%f %f", Odom_Pose[0], Odom_Pose[1], Odom_Pose[2], x_lidar, y_lidar_new);
              previous_time = ros::Time::now();
            }
          }
        }
      }
      break;
    }
    if (FirstRun == 0)
    {
      Odom_Pose_Offset[0] = Odom_Pose_temp[0];
      Odom_Pose_Offset[1] = Odom_Pose_temp[1];
      RoboPose_Offset[2] = pose_temp[2]; // gyro
      if (++timeRun > 30)
      {
        FirstRun = 1;
        timeRun = 0;
        ROS_INFO("Reset Position");
      }
    }
    else
    {
      Odom_Pose[0] = Odom_Pose_temp[0] - Odom_Pose_Offset[0];
      Odom_Pose[1] = Odom_Pose_temp[1] - Odom_Pose_Offset[1];
      RoboPose_Global[2] = pose_temp[2] - RoboPose_Offset[2];

      if (firstState > 100)
      {
        if (y_lidar_new < -0.1)
        {
          Odom_Pose[1] = Odom_Pose[1] * 0.9 + y_lidar_new * 0.1;
        }
      }
      if (RoboPose_Global[2] > M_PI)
        RoboPose_Global[2] -= 2 * M_PI;
      else if (RoboPose_Global[2] < -M_PI)
        RoboPose_Global[2] += 2 * M_PI;
    }
    pub_vel.publish(vel_msg);
    pub_slider.publish(slider_msg);
    pub_vacuum.publish(vac_msg);
    pub_posGlobalRos.publish(posGlobalRos_msg);
    ros::spinOnce();
    loop_rate.sleep();
  }
}

// jalan kanan kiri
void jalan_y(float sp_pos[3], float sp_speed[3])
{
  float error[3];
  float P[3], I[3], D[3];
  static float sum_error[3], prev_error[3];
  float output_speed[3];
  float kp[3] = {5, 5, 10.0}, ki[3] = {0, 0, 0}, kd[3] = {0, 0, 0};

  for (int i = 0; i < 3; i++)
  {
    error[0] = sp_pos[0] - x_lidar;
    error[1] = sp_pos[1] - Odom_Pose[1];
    error[2] = sp_pos[2] - RoboPose_Global[2];
    if (i == 2)
    {
      if (RoboPose_Global[i] > M_PI)
        RoboPose_Global[i] -= 2 * M_PI;
      else if (RoboPose_Global[i] < -M_PI)
        RoboPose_Global[i] += 2 * M_PI;
    }
    sum_error[i] += error[i];
    if (sum_error[i] > sp_speed[i])
      sum_error[i] = sp_speed[i];
    else if (sum_error[i] < -sp_speed[i])
      sum_error[i] = -sp_speed[i];
    P[i] = error[i] * kp[i];
    I[i] = ki[i] * sum_error[i];
    D[i] = kd[i] * (error[i] - prev_error[i]);

    prev_error[i] = error[i];

    output_speed[i] = P[i] + I[i] + D[i];
    if (output_speed[i] > sp_speed[i])
      output_speed[i] = sp_speed[i];
    else if (output_speed[i] < -sp_speed[i])
      output_speed[i] = -sp_speed[i];

    RoboSpeed[i] = output_speed[i];
  }
}

// jalan maju mundur
void jalan_x(float SP_POS[3], float SP_SPEED[3])
{
  float Error[3];
  float Propo[3], Integral[3], Derivative[3];
  static float sumError[3], prevError[3];
  float out_speed[3];
  float KP[3] = {5, 5, 6.0}, KI[3] = {0, 0, 0}, KD[3] = {0, 0, 0};

  for (int i = 0; i < 3; i++)
  {
    Error[0] = SP_POS[0] - x_lidar;
    Error[1] = SP_POS[1] - Odom_Pose[1];
    Error[2] = SP_POS[2] - RoboPose_Global[2];
    if (i == 2)
    {
      if (RoboPose_Global[i] > M_PI)
        RoboPose_Global[i] -= 2 * M_PI;
      else if (RoboPose_Global[i] < -M_PI)
        RoboPose_Global[i] += 2 * M_PI;
    }
    sumError[i] += Error[i];
    if (sumError[i] > SP_SPEED[i])
      sumError[i] = SP_SPEED[i];
    else if (sumError[i] < -SP_SPEED[i])
      sumError[i] = -SP_SPEED[i];
    Propo[i] = KP[i] * Error[i];
    Integral[i] = KI[i] * sumError[i];
    Derivative[i] = KD[i] * (Error[i] - prevError[i]);

    prevError[i] = Error[i];

    out_speed[i] = Propo[i] + Integral[i] + Derivative[i];

    if (out_speed[i] > SP_SPEED[i])
      out_speed[i] = SP_SPEED[i];
    else if (out_speed[i] < -SP_SPEED[i])
      out_speed[i] = -SP_SPEED[i];

    RoboSpeed[i] = out_speed[i];
  }
}

void odom_callback(const romusa_messages::odom::ConstPtr &odom_msg)
{
  pose_temp[0] = odom_msg->posX;
  pose_temp[1] = odom_msg->posY;
  pose_temp[2] = odom_msg->posTheta;

  localposX = odom_msg->posLocal_X;
  localposY = odom_msg->posLocal_Y;

  localSpeedX = localposX - prev_localposX;
  localSpeedY = localposY - prev_localposY;

  prev_localposX = localposX;
  prev_localposY = localposY;

  if (firstState == 0)
    firstState = 1;

  static const float offset_x = 0; // dalam m
  static const float offset_y = 0; // dalam m
  static float x_buffer = 0.09;
  static float y_buffer = 0.05;
  // static float x_buffer = 0.0;
  // static float y_buffer = 0.0;
  static float theta_buffer = 0;

  if (firstState > 100)
  {
    //   Odom_Pose[0] += cosf(RoboPose_Global[2]) * localSpeedX - sinf(RoboPose_Global[2]) * localSpeedY;
    //   Odom_Pose[1] += sinf(RoboPose_Global[2]) * localSpeedX + cosf(RoboPose_Global[2]) * localSpeedY;
    //   Odom_Pose[2] = RoboPose_Global[2];
    // di buffer dulu untuk mengoffset center odometry dan di integral dari local speed odom ke posisi lokal odom
    x_buffer += cosf(RoboPose_Global[2]) * localSpeedX - sinf(RoboPose_Global[2]) * localSpeedY;
    y_buffer += sinf(RoboPose_Global[2]) * localSpeedX + cosf(RoboPose_Global[2]) * localSpeedY;
    theta_buffer = RoboPose_Global[2];

    static const float offset_x = -0.09; // dalam m
    static const float offset_y = -0.05; // dalam m
    // static const float offset_x = 0; // dalam m
    // static const float offset_y = 0; // dalam m

    Odom_Pose_temp[0] = x_buffer + offset_x * cosf(theta_buffer) - offset_y * sinf(theta_buffer);
    Odom_Pose_temp[1] = y_buffer + offset_x * sinf(theta_buffer) + offset_y * cosf(theta_buffer);
    Odom_Pose_temp[2] = theta_buffer;
  }
  else
  {
    firstState++;
  }
}

void lidar_callback(const sensor_msgs::LaserScan::ConstPtr &msg)
{
  lidar_polar = msg->ranges; // datanya kesimpan di lidar_polar. total data = 180/0.25 = 720 (jd vectornya harusnya size 720)

  static float diff = 0;

  diff = (lidar_polar[540] - lidar_polar[180]) * 0.05 + diff * 0.95;
  yaw_lidar = diff * 125;

  for (int i = 0; i < 360; i++)
  {
    delta_lidar[i] = lidar_polar[i] - lidar_polar[i + 1];
  }

  x_lidar = 1.35 - lidar_polar[360];

  for (int i = 359; i >= 0; i--)
  {
    if (delta_lidar[i] > 0.1)
    {
      idx_search = i;
      break;
    }
  }

  for (int i = 359; i >= 0; i--)
  {
    // angle lidar from 0 until 180 scan from right to left
    // y lidar equal to range times cos of angle lidar polar [idx search+1]
    y_lidar = (y_lidar * 0.99) + ((lidar_polar[idx_search + 1] * cosf((idx_search + 1) * 0.25 * M_PI / 180)) * 0.01); // filter
    y_lidar_new = y_lidar - 1.45;                                                                                     // 1.45 itu jarak start dari lidar ke meja
  }
}

void slider_callback(const romusa_messages::enc_slider::ConstPtr &enc_slider_msg)
{
  // pose_temp_arm = enc_slider_msg->encArm;
  // pose_temp_lifter = enc_slider_msg->encLifter;
  posArm = enc_slider_msg->encArm;
  posLifter = enc_slider_msg->encLifter;
  status_button = enc_slider_msg->trigButton;
}

void vision_callback(const romusa_messages::ObjectDetection::ConstPtr &object_detection_msg)
{
  nomor_sampah = object_detection_msg->class_id;
  for (int i = 0; i < 6; i++)
  {
    centerX[i] = object_detection_msg->center_x[i];
    centerY[i] = object_detection_msg->center_y[i];
  }
}

void joy_callback(const sensor_msgs::Joy::ConstPtr &msg)
{
  for (int i = 0; i < msg->axes.size(); i++)
  {
    jb.axes[i] = msg->axes[i];
  }
  for (int i = 0; i < msg->buttons.size(); i++)
  {
    jb.buttons[i] = msg->buttons[i];
  }
}