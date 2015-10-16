#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <string>
#include <sstream>
#include <Windows.h>
#include "lsd_opencv.hpp"
using namespace std;
using namespace cv;
//control bar
int threshold_value1 = 0;
int threshold_value2 = 255;
char* trackbar_value1 = "H_low Value";
char* trackbar_value2 = "H_high Value";
//
void Threshold_Demo(int, void*)
{}
string num2str(int i){
	stringstream s;
	s << i;
	return s.str();
}
int main(int argc, char** argv)
{
	//bar window
	cv::namedWindow("BarValueThres");
	cv::namedWindow("video");
	string ss("");
	cv::VideoCapture videoCapture(0);
	videoCapture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	videoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	//内参
	/*Mat intrinsic = (Mat_<double>(3, 3) << 392.6625, 0, 593.9263,
		0, 390.7315, 329.4303,
		0, 0, 1.0000);
	Mat distortion = (Mat_<double>(4, 1) << -0.204700391393686, 0.0298622006125642, 0.00612432307199318, 0.000947722538938783);
	*/
	Mat frame;
	Mat distortframe;
	int index = 0;
	//显示视屏
	char c = 0;
	while (1)
	{
		videoCapture >> frame;
		//create bar
		createTrackbar(trackbar_value1,
			"BarValueThres", &threshold_value1,
			255, Threshold_Demo);

		createTrackbar(trackbar_value2,
			"BarValueThres", &threshold_value2,
			255, Threshold_Demo);
		//
		if (!frame.data)
			continue;
		Mat gray,gray1;
		cvtColor(frame, gray, CV_RGB2GRAY);
		inRange(gray, threshold_value1, threshold_value2, gray1);
	    //line detect
		Mat roiImageH;
		//cvtColor(gray1, roiImageH, CV_RGB2HSV);//just input your image here
		cvtColor(gray1, gray1, CV_GRAY2RGB);
		cvtColor(gray1, roiImageH, CV_RGB2HSV);
		vector<Mat> splited;
		split(roiImageH, splited);
		Mat image = splited[2];
		Ptr<LineSegmentDetector> lsd_std = createLineSegmentDetectorPtr(LSD_REFINE_STD);
		double start = double(getTickCount());
		vector<Vec4i> lines_std;
		lsd_std->detect(image, lines_std);
		//show line
		Mat drawnLines(frame);
		lsd_std->drawSegments(drawnLines, lines_std);
		imshow("Standard refinement", drawnLines);
		waitKey(30);
		//
		imshow("rgb", frame);
		imshow("video", gray);
		imshow("extract", gray1);
		//imshow("disvideo", distortframe);
		if (c == 32){ //c==32
			imwrite((num2str(index) + ".jpg").c_str(), frame);
			cout << index << endl;
			++index;
			//Sleep(2000);
		}

		c = cvWaitKey(30);
		//cout<<int(c)<<endl;
		if (c == 27) break;
	}
	cvDestroyWindow("video");
};