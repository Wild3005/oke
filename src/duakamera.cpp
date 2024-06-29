#include<iostream>
#include<opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(){
    VideoCapture kamera(0);
    VideoCapture kamera2(2);
    if(!kamera.isOpened() && !kamera2.isOpened()){
        return 1;
    }
    namedWindow("kamera laptop", WINDOW_AUTOSIZE);
    namedWindow("kamera daxa", WINDOW_AUTOSIZE);

    Mat frame1;
    Mat frame2;

    while(true){
        kamera >> frame1;
        kamera2 >> frame2;

        if(frame1.empty() && frame2.empty()){
            return 1;
        }

        imshow("kamera laptop", frame1);
        imshow("kamera daxa", frame2);
        if(waitKey(30)==32){
            break;
        }
    }

}