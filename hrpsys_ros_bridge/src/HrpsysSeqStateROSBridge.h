// -*- C++ -*-
/*!
 * @file  HrpsysSeqStateROSBridge.h * @brief hrpsys seq state - ros bridge * @date  $Date$ 
 *
 * $Id$ 
 */
#ifndef HRPSYSSEQSTATEROSBRIDGE_H
#define HRPSYSSEQSTATEROSBRIDGE_H

#include "HrpsysSeqStateROSBridgeImpl.h"

// rtm
#include <rtm/CorbaNaming.h>

// ros
#include "ros/ros.h"
#include "rosgraph_msgs/Clock.h"
#include "std_msgs/Int32.h"
#include "sensor_msgs/JointState.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/WrenchStamped.h"
#include "actionlib/server/simple_action_server.h"
#include "control_msgs/FollowJointTrajectoryAction.h"
#ifdef USE_PR2_CONTROLLERS_MSGS
#include "pr2_controllers_msgs/JointTrajectoryAction.h"
#include "pr2_controllers_msgs/JointTrajectoryControllerState.h"
#else
#include "control_msgs/JointTrajectoryControllerState.h"
#endif
#include "dynamic_reconfigure/Reconfigure.h"
#include "hrpsys_ros_bridge/MotorStates.h"
#include "hrpsys_ros_bridge/ContactStatesStamped.h"
#include "diagnostic_msgs/DiagnosticArray.h"
#include "sensor_msgs/Imu.h"
#include "hrpsys_ros_bridge/SetSensorTransformation.h"
#include "sensor_msgs/Joy.h"
#include "geometry_msgs/Vector3.h"
#include "sys/time.h"

extern const char* hrpsysseqstaterosbridgeimpl_spec[];

class HrpsysSeqStateROSBridge  : public HrpsysSeqStateROSBridgeImpl
{
 public:
  HrpsysSeqStateROSBridge(RTC::Manager* manager) ;
  ~HrpsysSeqStateROSBridge();

  RTC::ReturnCode_t onInitialize();
  RTC::ReturnCode_t onFinalize();
  RTC::ReturnCode_t onExecute(RTC::UniqueId ec_id);

  void onJointTrajectory(trajectory_msgs::JointTrajectory trajectory);
#ifdef USE_PR2_CONTROLLERS_MSGS
  void onJointTrajectoryActionGoal();
  void onJointTrajectoryActionPreempt();
#endif
  void onFollowJointTrajectoryActionGoal();
  void onFollowJointTrajectoryActionPreempt();
  void onTrajectoryCommandCB(const trajectory_msgs::JointTrajectoryConstPtr& msg);
  void onJoySubCB(const sensor_msgs::JoyConstPtr& msg);
  void onZedVelSubCB(const geometry_msgs::Vector3ConstPtr& msg);
  void onZedOdomSubCB(const nav_msgs::OdometryConstPtr& msg);
  bool sendMsg (dynamic_reconfigure::Reconfigure::Request &req,
                dynamic_reconfigure::Reconfigure::Response &res);
  bool setSensorTransformation(hrpsys_ros_bridge::SetSensorTransformation::Request& req,
                               hrpsys_ros_bridge::SetSensorTransformation::Response& res);
 private:
  ros::NodeHandle nh;
  ros::Publisher joint_state_pub, joint_controller_state_pub, mot_states_pub, diagnostics_pub, clock_pub, zmp_pub, ref_cp_pub, act_cp_pub, odom_pub, imu_pub, em_mode_pub, ref_contact_states_pub, act_contact_states_pub;
  ros::Subscriber trajectory_command_sub;
  ros::Subscriber joy_sub;
  ros::Subscriber linear_vel_sub;
  ros::Subscriber zed_odom_sub;
  std::vector<ros::Publisher> fsensor_pub, cop_pub;
#ifdef USE_PR2_CONTROLLERS_MSGS
  actionlib::SimpleActionServer<pr2_controllers_msgs::JointTrajectoryAction> joint_trajectory_server;
#endif
  actionlib::SimpleActionServer<control_msgs::FollowJointTrajectoryAction> follow_joint_trajectory_server;
  ros::ServiceServer sendmsg_srv;
  ros::ServiceServer set_sensor_transformation_srv;
  bool interpolationp, use_sim_time, use_hrpsys_time;
  bool publish_sensor_transforms;
  tf::TransformBroadcaster br;

  coil::Mutex m_mutex;
  coil::TimeMeasure tm;
  sensor_msgs::JointState prev_joint_state;

  std::string nameserver;
  std::string rootlink_name;

  ros::Subscriber clock_sub;

  nav_msgs::Odometry prev_odom;
  bool prev_odom_acquired;
  hrp::Vector3 prev_rpy;
  void clock_cb(const rosgraph_msgs::ClockPtr& str) {};

  bool follow_action_initialized;
  ros::Time traj_start_tm;

  boost::mutex tf_mutex;
  double tf_rate;
  double V_d_max = 0.3; // segway demo
  // double V_d_max = 0.1; // seesaw demo best (20200910)
  double Omega_d_max = 0.8;
  // double Foot_pos_d_max = 0.025;
  // double Foot_pos_d_max = 0.002;
  // double Foot_pos_d_max = 0.1; // ref_zmp y
  // double Foot_pos_d_max = 0.3; // ref_zmp y
  // double Foot_pos_d_max = 0.6; // ref_zmp y
  // double Foot_pos_d_max = 1.0; // ref_zmp y
  // double Foot_pos_d_max = 0.1; // new_refzmp y
  // double Foot_pos_d_max = 0.002; // rootLink pos y
  // double Foot_pos_d_max = 0.008; // rootLink pos y
  double Foot_pos_d_max = 0.0; // ref_root_rpy(0) [deg]   Disable for Segway (2019/01/19)
  // double Foot_pos_d_max = 20.0; // ref_root_rpy(0) [deg]   Ninebot best parameter (2019/01/19)
  // double Foot_pos_d_max = 0.3; // ref_root_pos(1) [m]
  // double Oneleg_roll_d_max = 0.1; // new_refzmp y [m]
  double Oneleg_roll_d_max = 0.15; // new_refzmp y [m]
  // double Oneleg_roll_d_max = 1.0; // Delta stikp[0].d_foot_rpy[0] [deg]
  double v_d, omega_d, foot_pos_d, oneleg_roll_d;
  int ninebot_start_learning_ps3joy, ninebot_stop_learning_ps3joy, ninebot_vd_on_ps3joy, ninebot_vd_off_ps3joy;
  geometry_msgs::Vector3 linear_vel_local_msg;
  nav_msgs::Odometry zed_odom_msg;
  ros::Timer periodic_update_timer;
  std::vector<geometry_msgs::TransformStamped> tf_transforms;
  void periodicTimerCallback(const ros::TimerEvent& event);
  
  // odometry relatives
  void updateOdometry(const hrp::Vector3 &trans, const hrp::Matrix33 &R, const ros::Time &stamp);

  // imu relatives
  void updateImu(tf::Transform &base, bool is_base_valid, const ros::Time &stamp);
  
  // sensor relatives
  void updateSensorTransform(const ros::Time &stamp);
  std::map<std::string, geometry_msgs::Transform> sensor_transformations;
  boost::mutex sensor_transformation_mutex;
};


extern "C"
{
  DLL_EXPORT void HrpsysSeqStateROSBridgeInit(RTC::Manager* manager);
};

#endif // HRPSYSSEQSTATEROSBRIDGE_H

