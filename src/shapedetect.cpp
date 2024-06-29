#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(){
    VideoCapture kamera(0);;
    if(!kamera.isOpened()){
        cerr << "masalah kamera" << endl;
        return 1;
    }
    Mat frame;
    while(1){
        kamera >> frame;
        if(frame.empty()){
            std::cerr << "frame kosong" << std::endl;
            return 1;
        }
        Mat layar = frame.clone();
        GaussianBlur(frame,frame,cv::Size(5,5),0);
        Mat gray;
        cvtColor(frame,gray,COLOR_BGR2GRAY);
        Mat color_h, color_l;
        inRange(gray,color_l,color_h,dataset[i].limitA,limcolor[i]);
        findContours(limcolor[i],kontur[i],cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);


        imshow("kamera",gray);
        if(cv::waitKey(30)== 32){
            break;
        }
    }

}