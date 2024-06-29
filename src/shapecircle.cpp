#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
    VideoCapture kamera(0);
    if (!kamera.isOpened()) {
        cerr << "masalah kamera" << endl;
        return 1;
    }

    Mat frame;
    while (true) {
        kamera >> frame;
        if (frame.empty()) {
            cerr << "frame kosong" << endl;
            return 1;
        }

        Mat layar = frame.clone();
        GaussianBlur(frame, frame, Size(9, 9), 2, 2);  // Mengurangi noise

        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);  // Mengkonversi ke grayscale

        Mat edges;
        Canny(gray, edges, 50, 150);  // Mendeteksi tepi menggunakan Canny

        // Deteksi lingkaran menggunakan Hough Circle Transform
        vector<Vec3f> circles;
        HoughCircles(gray, circles, HOUGH_GRADIENT, 1, gray.rows / 8, 200, 50, 0, 0);

        // Menggambar lingkaran pada frame asli
        for (size_t i = 0; i < circles.size(); i++) {
            Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
            int radius = cvRound(circles[i][2]);
            circle(layar, center, 3, Scalar(0, 255, 0), -1);  // Pusat lingkaran
            circle(layar, center, radius, Scalar(0, 0, 255), 3);  // Garis luar lingkaran
        }

        imshow("kamera", layar);

        if (waitKey(30) == 32) {  // Keluar jika tombol spasi ditekan
            break;
        }
    }

    return 0;
}
