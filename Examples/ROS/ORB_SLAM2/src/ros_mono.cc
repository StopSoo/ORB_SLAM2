#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>

#include<ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include<opencv2/core/core.hpp>
#include"../../../include/System.h"

using namespace std;

class ImageGrabber
{
public:
    ImageGrabber(ORB_SLAM2::System* pSLAM):mpSLAM(pSLAM){}

    void GrabImage(const sensor_msgs::ImageConstPtr& msg);

    ORB_SLAM2::System* mpSLAM;
};

int main(int argc, char **argv)
{
    ros::init(argc, argv, "Mono");
    ros::start();

    if (argc != 4)
    {
        cerr << endl << "Usage: rosrun ORB_SLAM2 Mono path_to_vocabulary path_to_settings" << endl;        
        ros::shutdown();
        return 1;
    }    

    ORB_SLAM2::System SLAM(argv[1],argv[2],ORB_SLAM2::System::MONOCULAR,true);

    ros::NodeHandle nodeHandler;

    // 카메라 3대에 맞게 객체 생성
    ImageGrabber igb_1(&SLAM);
    ImageGrabber igb_2(&SLAM);
    ImageGrabber igb_3(&SLAM);

    ros::Subscriber sub_1 = nodeHandler.subscribe("/camera/image_raw", 1, &ImageGrabber::GrabImage, &igb_1);
    ros::Subscriber sub_2 = nodeHandler.subscribe("/camera/image_raw", 1, &ImageGrabber::GrabImage, &igb_2);
    ros::Subscriber sub_3 = nodeHandler.subscribe("/camera/image_raw", 1, &ImageGrabber::GrabImage, &igb_3);

    ros::spin();

    SLAM.Shutdown();

    // 카메라 궤적 저장 
    SLAM.SaveKeyFrameTrajectoryTUM("KeyFrameTrajectory.txt");

    ros::shutdown();

    return 0;
}

void ImageGrabber::GrabImage(const sensor_msgs::ImageConstPtr& msg)
{
    // ros 이미지를 cv::Mat 형태로 변경
    cv_bridge::CvImageConstPtr cv_ptr;
    try
    {
        cv_ptr = cv_bridge::toCvShare(msg);
    }
    catch (cv_bridge::Exception& e)
    {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }
    // 트래킹 실행
    mpSLAM->TrackMonocular(cv_ptr->image,cv_ptr->header.stamp.toSec());
}


