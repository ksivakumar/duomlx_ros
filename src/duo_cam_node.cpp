#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <duo_cam/DUOLib.h>
#include <duo_cam/extralib.h>
#include <sensor_msgs/Imu.h>
#include <sstream>

#define WIDTH  752
#define HEIGHT 480
#define FPS    30

int main(int argc, char** argv)
{

     ros::init(argc, argv, "duo_image_publisher");
     ros::NodeHandle nh;
     image_transport::ImageTransport it(nh);
     image_transport::Publisher lpub = it.advertise("stereo/leftimage", 1);
     image_transport::Publisher rpub = it.advertise("stereo/rightimage", 1);
     ros::Publisher imupub = nh.advertise<sensor_msgs::Imu>("stereo/imu", 1);

     if(!OpenDUOCamera(WIDTH, HEIGHT, FPS))
     {
           return 0;
     }

     IplImage *left = cvCreateImageHeader(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);
     IplImage *right = cvCreateImageHeader(cvSize(WIDTH, HEIGHT), IPL_DEPTH_8U, 1);

     cv::Mat leftframe, rightframe;
     sensor_msgs::ImagePtr leftmsg;
     sensor_msgs::ImagePtr rightmsg;
     sensor_msgs::Imu imu;
     ros::Rate loop_rate(FPS);
     
     while(nh.ok()) {
          PDUOFrame pFrameData = GetDUOFrame();
          if(pFrameData == NULL) continue;
          left->imageData = (char*)pFrameData->leftData;
          right->imageData = (char*)pFrameData->rightData;
          leftframe = left;
          rightframe = right;
          imu.linear_acceleration.x = pFrameData->accelData[0];
          imu.linear_acceleration.y = pFrameData->accelData[1];
          imu.linear_acceleration.z = pFrameData->accelData[2];
          imu.angular_velocity.x = pFrameData->gyroData[0];
          imu.angular_velocity.y = pFrameData->gyroData[1];
          imu.angular_velocity.z = pFrameData->gyroData[2];

          if (!leftframe.empty() && !rightframe.empty()) {
               leftmsg = cv_bridge::CvImage(std_msgs::Header(), "mono8", leftframe).toImageMsg();
               rightmsg = cv_bridge::CvImage(std_msgs::Header(), "mono8", rightframe).toImageMsg();
               lpub.publish(leftmsg);
               rpub.publish(rightmsg);
               imupub.publish(imu);
               cv::waitKey(1);
          }
          ros::spinOnce();
          loop_rate.sleep();
     }

     cvReleaseImageHeader(&left);
     cvReleaseImageHeader(&right);
     CloseDUOCamera();
     return 0;
}
