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
int threshold_value1 = 200;
int threshold_value2 = 255;
int threshold_value3 = 60;
int threshold_value4 = 120;
char* trackbar_value1 = "H_low Value";
char* trackbar_value2 = "H_high Value";
char* trackbar_value3 = "angle_low Value";
char* trackbar_value4 = "angle_high Value";
Mat cameraMatrix, distMatrix, warp3D, warp3DInv;
//
void Threshold_Demo(int, void*)
{}
string num2str(int i){
	stringstream s;
	s << i;
	return s.str();
}
int my_cmp(double p1, double  p2)
{
	return p1> p2;
}
void updatePara(){


	double cameraPara[9] = { 387.9441, 0, 307.5401,
		0, 385.1973, 269.5151,
		0, 0, 1.0000 };
	double distorPara[4] = { 0.1420, -0.2269, 0.0017, -0.0034 };
	double para[5] = { 0.4648, -0.8854, 0.0045, 0.0084, 698.9701-259 };
	cameraMatrix = Mat(3, 3, CV_64FC1, cameraPara);
	distMatrix = Mat(1, 4, CV_64FC1, distorPara);

	double q0 = para[0], q1 = para[1], q2 = para[2], q3 = para[3], h = para[4];
	double R[9] = {
		2 * q0*q0 + 2 * q1*q1 - 1, 2 * q1*q2 - 2 * q0*q3, 2 * q1*q3 + 2 * q0*q2,
		2 * q1*q2 + 2 * q0*q3, 2 * q0*q0 + 2 * q2*q2 - 1, 2 * q2*q3 - 2 * q0*q1,
		2 * q1*q3 - 2 * q0*q2, 2 * q2*q3 + 2 * q0*q1, 2 * q0*q0 + 2 * q3*q3 - 1
	};

	Mat R_M(3, 3, CV_64F, (void *)R);
	warp3D = R_M*cameraMatrix.inv();
	warp3D.row(0) *= -h;
	warp3D.row(1) *= -h;

	warp3DInv = warp3D.inv();
	//warp3DBias = warp3D;
	//warp3DBias.row(0) += 500;
	//warp3DBias.row(1) -= 1000;
}
void getPoint3D(double u, double v, double &x, double &y){
	double axisInImage[3] = { u, v, 1 };
	cv::Mat axisImage = cv::Mat(3, 1, CV_64FC1, axisInImage);
	cv::Mat_<double> divder = warp3D.row(2)*axisImage; double divderD = divder(0, 0);
	cv::Mat_<double> x_ = warp3D.row(0)*axisImage / divderD;
	cv::Mat_<double> y_ = warp3D.row(1)*axisImage / divderD;
	x = x_(0, 0);
	y = y_(0, 0);
};
int main(int argc, char** argv)
{
	//bar window
	double p[640][2];
	cv::namedWindow("BarValueThres");
	cv::namedWindow("video");
	string ss("");
	cv::VideoCapture videoCapture(0);
	videoCapture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	videoCapture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	//�ڲ�
	/*Mat intrinsic = (Mat_<double>(3, 3) << 392.6625, 0, 593.9263,
		0, 390.7315, 329.4303,
		0, 0, 1.0000);
	Mat distortion = (Mat_<double>(4, 1) << -0.204700391393686, 0.0298622006125642, 0.00612432307199318, 0.000947722538938783);
	*/
	Mat frame;
	Mat distortframe;
	int index = 0;
	//��ʾ����
	char c = 0;
	updatePara();
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
		createTrackbar(trackbar_value3,
			"BarValueThres", &threshold_value3,
			180, Threshold_Demo);

		createTrackbar(trackbar_value4,
			"BarValueThres", &threshold_value4,
			180, Threshold_Demo);
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
		//double start = double(getTickCount());
		vector<Vec4i> lines_std,out_line;
		
		
		lsd_std->detect(image, lines_std);
		//filter lines
		lsd_std->filterOutAngle(lines_std, out_line,threshold_value3, threshold_value4);
		//
		for (int k = 0; k < 640; k++)
		{
			vector<double> getmin;
			double x = k, y;
			for (int i = 0; i < out_line.size(); i++)
			{
				double x1 = out_line[i][0];
				double y1 = out_line[i][1];
				double x2 = out_line[i][2];
				double y2 = out_line[i][3];
				if (k >= x1&&k <= x2 || k >= x2&&k <= x1)
				{
					
					double y = (x - x2) / (x1 - x2)*(y1 - y2) + y2;
					getmin.push_back(y);
				}
				

				//circle(frame, Point2f(x1, y1), 3, Scalar(255, 0, 0));
				//circle(frame, Point2f(x2, y2), 3, Scalar(255, 0, 0));
			}
			sort(getmin.begin(), getmin.end(), my_cmp);
			if (getmin.size() > 1)
			{
				y = (getmin[0] + getmin[1]) / 2;
				p[k][0] = x;
				p[k][1] = y;
				
			}
			if (k % 50 == 0)
			{
				double a, b;
				circle(frame, Point2f(x, y), 5, Scalar(255, 0, 0));
				getPoint3D(x, y, a, b);
				cout << b << " ";
			}



		}
		cout << endl;
		
		//show line
		Mat drawnLines(frame);
		lsd_std->drawSegments(drawnLines, out_line);
		imshow("Standard refinement", drawnLines);

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