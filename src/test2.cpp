#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Point2f batas;

int main(){
    VideoCapture cam(2);
    if(!cam.isOpened()){
        cerr << "Error: Kamera tidak dapat dibuka!" << endl;
        return 1;
    }

    Mat frame;
    Mat framefix;
    Mat hsv;

    Scalar lim_atas(180, 233, 255), lim_bawah(0, 0, 136);

    while(true){
        cam >> frame;
        if(frame.empty()){
            cerr << "Error: Frame kosong!" << endl;
            break;
        }

        framefix = frame.clone();
        
        cvtColor(frame, hsv, COLOR_BGR2HSV);

        Mat limcolor;
        inRange(hsv, lim_bawah, lim_atas, limcolor);

        std::vector<std::vector<Point>> kontur;
        findContours(limcolor, kontur, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        for (const auto& box : kontur) {
            if (contourArea(box) > 100) { // Menghindari kontur kecil
                Rect box_bounding = boundingRect(box);
                batas = Point2f(box_bounding.x + box_bounding.width / 2, box_bounding.y + box_bounding.height / 2);
                rectangle(framefix, box_bounding, Scalar(0, 255, 0), 2); // Kontur dengan warna hijau
            }
        }

        Mat resizedFrame;
        resize(framefix, resizedFrame, Size(), 0.5, 0.5);  // Mengubah ukuran menjadi setengah dari ukuran asli

        imshow("kamera", resizedFrame);
        if (waitKey(30) == 32) { // Spasi untuk keluar
            break;
        }
    }

    cam.release();
    destroyAllWindows();
    return 0;
}
