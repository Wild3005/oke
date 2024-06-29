#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include<opencv2/imgproc.hpp>
#include<iostream>

using namespace std;
using namespace cv;

Mat imgasli,imggray,imgcanny,imgblur,imgthre, imgdil,imgerode;
Mat preprosesing(Mat img){
    cvtColor(img, imggray,COLOR_BGR2GRAY);
    GaussianBlur(imggray, imgblur, Size(3 ,3),3,0);
    Canny(imgblur, imgcanny, 25, 75);

    Mat kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
    dilate(imgcanny, imgdil, kernel);
    //erode(imgdil, imgerode, kernel);
    return imgdil;
}

int main(){
    string path = "/home/ichbinwil/Downloads/sampletext1.png";
    imgasli = imread(path);
    resize(imgasli, imgasli, Size(), 0.5, 0.5);

    imgthre = preprosesing(imgasli);
    imshow("asli",imgasli);
    imshow("final",imgthre);

}