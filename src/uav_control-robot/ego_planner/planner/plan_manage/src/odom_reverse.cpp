#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <std_msgs/Float64.h>
#include <std_msgs/Int32.h>
#include <tf/tf.h>

#include <cmath>


// Initial odom from airsim
ros::Subscriber *airsimOdomSubPtr;
// Reversed odom for ego-planner
ros::Publisher *reversedOdomPubPtr;
nav_msgs::Odometry reversedOdom;

void OdomReverseCallback(const nav_msgs::Odometry &airsimOdom)
{
  //记录开始时间
  static ros::Time last_time = ros::Time::now();
    ROS_INFO("now - last %f ", ros::Time::now().toSec() - last_time.toSec());
    last_time = ros::Time::now();
   ros::Time begin = ros::Time::now();
reversedOdom.header.frame_id = airsimOdom.header.frame_id;

reversedOdom.pose.pose.position.x = airsimOdom.pose.pose.position.x;
reversedOdom.pose.pose.position.y = airsimOdom.pose.pose.position.y;
reversedOdom.pose.pose.position.z = airsimOdom.pose.pose.position.z;

// Convert the msg quaternion to tf quaternion
tf::Quaternion airsimQuaternion;
airsimQuaternion.setX(airsimOdom.pose.pose.orientation.x);
airsimQuaternion.setY(airsimOdom.pose.pose.orientation.y);
airsimQuaternion.setZ(airsimOdom.pose.pose.orientation.z);
airsimQuaternion.setW(airsimOdom.pose.pose.orientation.w);
// Get RPY
tf::Matrix3x3 airsimTransform(airsimQuaternion);
double roll, pitch, yaw;
airsimTransform.getRPY(roll,pitch, yaw);
// Reverse
tf::Quaternion reversedQuaternion;
reversedQuaternion.setRPY(roll, pitch, yaw);
reversedOdom.pose.pose.orientation.x = reversedQuaternion.getX();
reversedOdom.pose.pose.orientation.y = reversedQuaternion.getY();
reversedOdom.pose.pose.orientation.z = reversedQuaternion.getZ();
reversedOdom.pose.pose.orientation.w = reversedQuaternion.getW();
reversedOdomPubPtr->publish(reversedOdom);
// TODO: Reverse the twist part
//记录结束时间
ros::Time end = ros::Time::now();
//计算运行时间
double time = end.toSec() - begin.toSec();
ROS_INFO("time: %f", time);

}

int main(int argc, char **argv)
{
  //Initiate ROS
    ros::init(argc, argv, "odom_reverse");
    ros::NodeHandle nodeHandle;

    ros::Subscriber airsimOdomSub = nodeHandle.subscribe("/mavros/vision_pose/pose", 10, &OdomReverseCallback);
    ros::Publisher  reversedOdomPub = nodeHandle.advertise<nav_msgs::Odometry>("/mavros/gazebo_pose/pose", 1);
  	airsimOdomSubPtr = &airsimOdomSub;
      	reversedOdomPubPtr = &reversedOdomPub;
    //Create an object of class SubscribeAndPublish that will take care of everything
    ros::spin();

  return 0;
}
