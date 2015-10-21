#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#include <string>
#include <sstream>
#include <fstream>
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
double cameraPara[9] = { 387.9441, 0, 307.5401,
0, 385.1973, 269.5151,
0, 0, 1.0000 };
double distorPara[4] = { 0.1420, -0.2269, 0.0017, -0.0034 };
double para[5] = { 0.4, -0.9, 0, 0, 1010.1 - 259 };
double a[640], b[640], c[640];
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

	Mat(3, 3, CV_64FC1, cameraPara).copyTo(cameraMatrix);
	Mat(4, 1, CV_64FC1, distorPara).copyTo(distMatrix);//深复制

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
	cout << cameraMatrix << endl << distMatrix << endl;
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
}
void wrongPointDetect(double p[])
{
	
	for (int i = 1; i < 640-1; i++)
	{
		if (fabs(p[i - 1] - p[i + 1])<10 && fabs(p[i] - p[i - 1])>50)
			p[i] = (p[i + 1] + p[i - 1]) / 2;

	}
}
void dataProcess(int start, int end, int step)
{
	bool first = true;
	Mat A[640], B[640];
	double depth[640];
	int index = 1;
	for (int s = start; s <= end; s += step)
	{
		static ifstream infile;
		if (!infile.is_open()) {
			infile.open("depth.txt", ios::in);
		}
		for (int i = 0; i < 640; i++)
		{
			double a;
			infile >> a;
			/*if (a<0 || a>5000)
				a = 0;*/
			depth[i] = a;
			//cout << a << " ";
		}

		for (int i = 0; i < 640; i++)
		{
			
			double num = depth[i];
			double temp[3] = { num*num, num, 1 };
			A[i ].push_back(Mat(1, 3, CV_64FC1, temp));
			/*double temp[2] = {  num, 1 };
			A[i].push_back(Mat(1, 2, CV_64FC1, temp));*/
			B[i ].push_back(Mat(1, 1, CV_64FC1, s));
		}
	}
	/*Mat P = A[140].t()*A[140];
	Mat Q = A[140].t()*B[140];
	cout << P << endl;
	cout << Q << endl;
	cout << P.inv()*Q << endl;*/
	for (int i = 0; i < 640; i++)
	{
		Mat AA = A[i].t()* A[i];
		Mat BB = A[i].t()* B[i];
		Mat x = AA.inv()*BB;
		//cout << x << endl;
		a[i] = x.at<double>(0, 0);
		b[i] = x.at<double>(1, 0);
		c[i] = x.at<double>(2, 0);
	}



}
Mat aa = Mat(1, 640, CV_64FC1, a);
Mat bb = Mat(1, 640, CV_64FC1, b);
Mat cc = Mat(1, 640, CV_64FC1, c);
void yanzheng()
{

	double depth[640];
	static ifstream infile;
	infile.close();
	for (int k = 0; k < 9; k++)
	{
		
		if (!infile.is_open()) {
			infile.open("depth.txt", ios::in);
		}
		for (int i = 0; i < 640; i++)
		{
			double a;
			infile >> a;
			if (a<0 || a>5000)
				a = 0;
			depth[i] = a;
			/*if (i % 50 == 0)
				cout << a << endl;*/
		}
		Mat x = Mat(1, 640, CV_64FC1, depth);
		Mat result;
		Mat A, B;
		result = aa.mul(x).mul(x) + bb.mul(x)+cc;
		/*for (int j = 0; j < 640; j += 20)
		{
			cout << result.at<double>(0, j) << " ";
		}
		cout << endl;*/
	}
	


}
int main(int argc, char** argv)
{
	//bar window
	double p[640];
	double depth[640];
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
	dataProcess(800, 4000, 400);
	yanzheng();
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
					
					y = (x - x2) / (x1 - x2)*(y1 - y2) + y2;
					getmin.push_back(y);
				}
				

				//circle(frame, Point2f(x1, y1), 3, Scalar(255, 0, 0));
				//circle(frame, Point2f(x2, y2), 3, Scalar(255, 0, 0));
			}
			sort(getmin.begin(), getmin.end(), my_cmp);
			if (getmin.size() > 1)
			{
				y = (getmin[0] + getmin[1]) / 2;
				p[k]=y;
				
			}

		}
		wrongPointDetect(p);
		
		for (int k = 0; k < 640; k++)
		{
			
				double a, b;
				circle(frame, Point2f(k, p[k]), 5, Scalar(255, 0, 0));

				Mat p_origin, p_after;
				double aa[2] = { k, p[k] };
				p_origin.push_back(Mat(1, 1, CV_64FC2, aa));
				undistortPoints(p_origin, p_after, cameraMatrix, distMatrix);
				//cout << cameraMatrix << endl << distMatrix << endl;
				//cout << p_origin << endl;

				double x0 = p_after.at<double>(0, 0);
				double y0 = p_after.at<double>(0, 1);
				double fx = 388.2391, cx = 307.5625, fy = 385.5123, cy = 269.5769;
				x0 = x0*fx + cx;
				y0 = y0*fy + cy;
				//cout << x0<<" "<<y0 << endl;

				getPoint3D(x0, y0, a, b);
				depth[k] = b;
				/*if (k % 50 == 0)
					cout << b << " ";*/
			
		}

		//利用线性优化系数进行优化
		Mat x = Mat(1, 640, CV_64FC1, depth);
		Mat result;
		Mat A, B;
		result = aa.mul(x).mul(x) + bb.mul(x) + cc;
		for (int j = 0; j < 640; j += 20)
		{
			cout << result.at<double>(0, j) << " ";
		}
		cout << endl;
		//cout << endl;
		
		//show line
		Mat drawnLines(frame);
		lsd_std->drawSegments(drawnLines, out_line);
		imshow("Standard refinement", drawnLines);

		//imshow("rgb", frame);
		imshow("video", gray);
		imshow("extract", gray1);
		//imshow("disvideo", distortframe);
		if (c == 32){ //c==32
			/*	imwrite((num2str(index) + ".jpg").c_str(), frame);
				cout << index << endl;
				++index;
				Sleep(2000);*/
			static ofstream outfile;
			if (!outfile.is_open()) {
				cout << "not open" << endl;
				outfile.open("depth1.txt", ios::out);//文件名改成自己的
			}
			for (int i = 0; i < 639; i++)
			{
				outfile << depth[i] << '\t';
			}
			outfile << depth[639] << endl;
		}

		c = cvWaitKey(30);
		//cout<<int(c)<<endl;
		if (c == 27) break;
	}
	cvDestroyWindow("video");
};