//
// MIT License
//
// Copyright (c) 2017 MRSD Team D - LoCo
// The Robotics Institute, Carnegie Mellon University
// http://mrsdprojects.ri.cmu.edu/2016teamd/
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifndef _TRAJ_SERVER_H_
#define _TRAJ_SERVER_H_

#include "ros/ros.h"
#include <actionlib/server/simple_action_server.h>
#include <ilqr_loco/TrajExecAction.h>

#include <math.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/PoseStamped.h>
#include <tf/transform_datatypes.h>
#include <nav_msgs/Path.h>
#include <fstream>
#include <string>
#include <sstream>

#include "try_get_param.h"

//TODO is there a way for server to know who called it?

class TrajServer
{
public:
  TrajServer():
    as(nh, "traj_server", boost::bind(&TrajServer::execute_trajectory, this,
    _1), false), traj_action("traj_server"), cur_yaw_(0), cur_integral_(0),
    prev_error_(0), dt(0.05)
    {
	    ROS_INFO("Starting traj server.");

      as.start();
      cmd_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 1);
      path_pub = nh.advertise<nav_msgs::Path>("path", 1);
      state_sub  = nh.subscribe("odometry/filtered", 1, &TrajServer::stateCb, this);

      LoadParams();
      ROS_INFO("Started iLQR executer node. Send me actions!");

      t_ = ros::Time::now().toSec();
    }

private:
  ros::NodeHandle nh;
  actionlib::SimpleActionServer<ilqr_loco::TrajExecAction> as;

  std::string traj_action;

  // If a command was planned to be executed more than this many seconds, then
  // client will ignore it and move to processing next command.
  double old_msg_thres;

  // create messages that are used to publish feedback/result
  ilqr_loco::TrajExecFeedback feedback_;
  ilqr_loco::TrajExecResult result_;

  //PID control
  double cur_yaw_;
  double cur_integral_;
  double prev_error_;
  float kp_, ki_, kd_, dt, t_;

  ros::Publisher cmd_pub;
  ros::Publisher path_pub;
  ros::Subscriber state_sub;

  void LoadParams();
  void execute_trajectory(const ilqr_loco::TrajExecGoalConstPtr &goal);
  geometry_msgs::Twist pid_correct_yaw(geometry_msgs::Twist orig_twist, nav_msgs::Odometry state);
  void stateCb(const nav_msgs::Odometry &msg);
  void PublishPath(const ilqr_loco::TrajExecGoalConstPtr &goal);

  double clamp(double val, double min_val, double max_val)
  {
    return std::max(min_val, std::min(val, max_val));
  }

};

#endif
