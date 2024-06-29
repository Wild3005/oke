#include<iostream>
#include<opencv2/opencv.hpp>

int main(){
    cv::VideoCapture kamera(2);
    if(!kamera.isOpened()){
        std::cerr << "masalah kamera" << std::endl;
        return 1;
    }
    cv::Mat frame;
    while(1){
        kamera >> frame;
        if(frame.empty()){
            std::cerr << "frame kosong" << std::endl;
            return 1;
        }
        cv::imshow("kamera",frame);
        if(cv::waitKey(30)== 32){
            break;
        }
    }
    return 0;
}