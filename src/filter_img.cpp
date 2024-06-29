#include<iostream>
#include<opencv4/opencv2/opencv.hpp>
using namespace std;
using namespace cv;

int main (){
    Mat src = imread("../biruresize.jpg");
	if (src.empty())
		return 1;

    Mat red, green, blue;
        
    inRange(src,Scalar(0,120,70),Scalar(10,255,255),red);
    inRange(src,Scalar(35,100,50),Scalar(85,255,255),green);
    inRange(src,Scalar(240,125,50),Scalar(255,255,255),blue);

    Mat result_range = red + green + blue;

    imshow("combine",result_range);
    imshow("merah",red);
    imshow("hijau",green);
    imshow("biru",blue);

    waitKey(0);
}