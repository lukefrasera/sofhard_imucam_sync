#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ros/ros.h"
#include "sensor_msgs/IMU.h"
#include <string>

class DoubleBufferList {
 public:
  std::vector<ros::Time> *active, *locked;
  DoubleBufferList() {
    active = new std::vector<ros::Time>;
    locked = new std::vector<ros::Time>;
  }
  ~DoubleBufferList() {
    delete active;
    delete locked;
  }
  void Swap() {
    std::vector<ros::Time> *temp;
    temp = active;
    active = locked;
    locked = temp;
  }
}

class CamSyncPub {
 private:
  ros::Nodehandle nh;
  ros::Subscriber imu_sub;
  image_transport::ImageTransport it_;
  image_transport::Publisher image_pub;
  image_transport::Subscriber image_sub;
  DoubleBufferList imu_timestamp_list;

 public:
  CamSyncPub(std::string topic) : it_(nh) {
    image_pub = it_.advertise(topic, 1);
    image_sub = it_.subscribe("usb_cam/image_raw", 1);
    imu_sub.nh.subscribe("/imu0");
  }
  ~CamSyncPub();

 private:
  void imucb(/*sensor_msgs::Imu& msg*/) {
    // Change message type to imu timestamp and bool whether it was a camera trigger
    // Queue timestamps based on the most recent trigger
    if (msg->trigger) {
      imu_timestamp_list.active->clear();
    }
    // Append the current imu timestamp
  }

  void imagecb(sensor_msgs::ImageConstPtr& msgs) {
    imu_timestamp_list.Swap();
    // acquire double buffered list
    // Determine correct timestamp of camera image
    //   - timestamp = (last_imu_time_in_list - trigger_time) * alpha[0-1] + trigger_time
    //   - maybe: select the closest relevant imu time
    msgs.header.timestamp = timestamp;
    image_pub.publish(msgs);
    imu_timestamp_list.locked->clear();
  }

};



int main(int argc, char *argv[] ) {
  // Initialize ros environment
  ros::init(argc, argv, "imu_sync_image");

  ros::NodeHandle nh("~");

  std::string pub_topic;

  if (!nh.getParam("pub_topic", pub_topic, "/cam0/raw")) {
    ROS_INFO("using default published topic: %s", pub_topic);
  }

  CamSyncPub pub(pub_topic);

  ros::spin();

  return 0;
}
