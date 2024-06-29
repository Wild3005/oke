#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
using namespace cv;
using namespace std;

int h_low = 0;
int h_hi = 255;

void on_trackbar(int, void* data) {
    Mat* img = (Mat*)data;
    Mat gray, thr;
    cvtColor(*img, gray, cv::COLOR_BGR2GRAY);
    // blur(gray, gray, Size(3, 3));
    GaussianBlur(gray,gray,cv::Size(5,5),0);

    Canny(gray, thr, h_low, h_hi, 3, false);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
    findContours(thr.clone(), contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, Point(0, 0));

    Mat drawing = img->clone();
    vector<vector<Point>> contours_poly(contours.size());
    for (int i = 0; i < contours.size(); i++) {
        approxPolyDP(Mat(contours[i]), contours_poly[i], 10, true);
        if (contours_poly[i].size() > 15) {
            drawContours(drawing, contours_poly, i, Scalar(0, 255, 0), 2, 8, vector<Vec4i>(), 0, Point());
        } else {
            drawContours(drawing, contours_poly, i, Scalar(0, 0, 255), 2, 8, vector<Vec4i>(), 0, Point());
        }
    }

    Mat resizedFrame;
    resize(drawing, resizedFrame, Size(), 0.2, 0.2);
    imshow("kamera", resizedFrame);

    Mat resizedThr;
    resize(thr, resizedThr, Size(), 0.2, 0.2);
    imshow("kamera2", resizedThr);
}

int main(int argc, char** argv) {
    Mat src = imread("../biru.jpg", 1);
    if (src.empty()) {
        printf("Error opening image\n");
        return -1;
    }

    namedWindow("My window", cv::WINDOW_AUTOSIZE);
    createTrackbar("Low Threshold", "My window", &h_low, 255, on_trackbar, &src);
    createTrackbar("High Threshold", "My window", &h_hi, 255, on_trackbar, &src);

    on_trackbar(0, &src); // Initial call to display the image

    waitKey(0);
    return 0;
}
