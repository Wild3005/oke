#include <iostream>
#include <opencv4/opencv2/opencv.hpp>

typedef struct {
    std::string nama;
    cv::Scalar color;
    cv::Point2f titik;
    cv::Scalar limitA;
    cv::Scalar limitB;
} data;

int main() {
    cv::VideoCapture kamera(2);
    
    if (!kamera.isOpened()) {
        std::cerr << "masalah kamera" << std::endl;
        return 1;
    }

    int kapasitas = 5;
    data dataset[kapasitas] = {
        //{"SDaun", cv::Scalar(0, 0, 0), cv::Point2f(0, 0), cv::Scalar(44, 198, 205), cv::Scalar(23, 128, 185)},// daun sample kemarin
        {"SDaun", cv::Scalar(0, 0, 0), cv::Point2f(0, 0), cv::Scalar(65, 160, 200), cv::Scalar(35, 60, 174)},
        //{"SPlastik", cv::Scalar(36, 209, 255), cv::Point2f(0, 0), cv::Scalar(28, 213, 220), cv::Scalar(18, 147, 165)},//plastik sampel 1
        {"SKertas", cv::Scalar(94, 129, 255), cv::Point2f(0, 0), cv::Scalar(100, 255, 181), cv::Scalar(85, 155, 154)},
        //{"SNFerro", cv::Scalar(255, 255, 255), cv::Point2f(0, 0), cv::Scalar(5, 5, 210), cv::Scalar(0, 0, 200)},//kondisi cukup cahaya
        {"SNFerro", cv::Scalar(255, 255, 255), cv::Point2f(0, 0), cv::Scalar(5, 5, 210), cv::Scalar(0, 0, 180)},//kondisi kurang cahaya
        //{"SFerro", cv::Scalar(19, 121, 170), cv::Point2f(0, 0), cv::Scalar(17, 189, 215), cv::Scalar(0, 127, 141)},//ferro sample1
        {"SPlastik", cv::Scalar(36, 209, 255), cv::Point2f(0, 0), cv::Scalar(28, 225, 225), cv::Scalar(0, 100, 100)},//plastik sample 2
        {"SFerro", cv::Scalar(19, 121, 170), cv::Point2f(0, 0), cv::Scalar(30, 185, 190), cv::Scalar(0, 148, 157)}

    };

    cv::Mat frame;
    while (true) {
        kamera >> frame;
        if (frame.empty()) {
            std::cerr << "frame kosong" << std::endl;
            return 1;
        }

        // Reduce brightness (optional, adjust alpha and beta as needed)
        // frame.convertTo(frame, -1, 1, -10);

        cv::Mat framefix = frame.clone();
        //cv::GaussianBlur(frame, frame, cv::Size(15, 15), 0);
        cv::Mat dummy;
        cv::cvtColor(frame, dummy, cv::COLOR_BGR2HSV);

        cv::Mat limcolor[kapasitas];
        for (int i = 0; i < kapasitas; i++) {
            cv::inRange(dummy, dataset[i].limitB, dataset[i].limitA, limcolor[i]);
        }

        std::vector<std::vector<cv::Point>> kontur[kapasitas];
        for (int i = 0; i < kapasitas; i++) {
            cv::findContours(limcolor[i], kontur[i], cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        }

        int area = 1000;

        for (int i = 0; i < kapasitas; i++) {
            for (auto& scan : kontur[i]) {
                if (cv::contourArea(scan) > area) {
                    cv::Rect box = cv::boundingRect(scan);
                    dataset[i].titik = cv::Point2f(box.x + box.width / 2, box.y + box.height / 2);
                    cv::rectangle(framefix, box, dataset[i].color, 2);
                    std::string text = "(" + std::to_string(dataset[i].titik.x) + "," + std::to_string(dataset[i].titik.y) + ")";
e                }
            }
        }

        // Mengubah ukuran framefix menjadi lebih kecil
        cv::Mat resizedFrame;
        cv::resize(framefix, resizedFrame, cv::Size(), 0.5, 0.5);  // Mengubah ukuran menjadi setengah dari ukuran asli

        cv::imshow("kamera", resizedFrame);
        if (cv::waitKey(30) == 32) {
            break;
        }
    }
    return 0;
}
