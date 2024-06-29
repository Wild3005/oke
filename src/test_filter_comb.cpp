#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main(){
    VideoCapture cap(0);
    if(!cap.isOpened()){
        cerr << "masalah kamera" << endl;
        return 1;
    }
    Mat frame;
    while(1){
        cap >> frame;
        if(frame.empty()){
            cerr << "frame kosong" << endl;
            return 1;
        }

        cvtColor(frame,frame,COLOR_BGR2RGB);
        Mat red, green, blue;
        
        inRange(frame,Scalar(0,120,70),Scalar(10,255,255),red);
        inRange(frame,Scalar(35,100,50),Scalar(85,255,255),green);
        inRange(frame,Scalar(100,150,50),Scalar(140,255,255),blue);

        

        

    }
}
