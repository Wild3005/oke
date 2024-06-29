#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv2/tracking/tracker.hpp>

typedef struct {
    std::string nama;
    cv::Scalar color;
    cv::Point2f titik;
    cv::Scalar limitA;
    cv::Scalar limitB;
} data;

bool isInside(const cv::Rect& smallBox, const cv::Rect& largeBox) {
    return (smallBox.x >= largeBox.x && smallBox.y >= largeBox.y &&
            smallBox.x + smallBox.width <= largeBox.x + largeBox.width &&
            smallBox.y + smallBox.height <= largeBox.y + largeBox.height);
}

int main() {
    cv::VideoCapture kamera(2);
    
    if (!kamera.isOpened()) {
        std::cerr << "masalah kamera" << std::endl;
        return 1;
    }

    int kapasitas = 5;
    data dataset[kapasitas] = {
        {"SDaun", cv::Scalar(0, 0, 0), cv::Point2f(0, 0), cv::Scalar(65, 160, 200), cv::Scalar(35, 60, 174)},
        {"SPlastik", cv::Scalar(36, 209, 255), cv::Point2f(0, 0), cv::Scalar(30, 225, 225), cv::Scalar(11, 130, 237)},
        {"SKertas", cv::Scalar(94, 129, 255), cv::Point2f(0, 0), cv::Scalar(100, 255, 181), cv::Scalar(85, 155, 154)},
        {"SNFerro", cv::Scalar(255, 255, 255), cv::Point2f(0, 0), cv::Scalar(5, 5, 210), cv::Scalar(0, 0, 180)},
        {"SFerro", cv::Scalar(19, 121, 170), cv::Point2f(0, 0), cv::Scalar(30, 185, 190), cv::Scalar(0, 148, 157)}
    };

    int adjust = 0;
    bool giliran = true;
    cv::Mat frame;

    // Inisialisasi tracker CSRT untuk setiap objek dalam dataset
    std::vector<cv::Ptr<cv::TrackerCSRT>> trackers(kapasitas);
    for (int i = 0; i < kapasitas; ++i) {
        trackers[i] = cv::TrackerCSRT::create();
    }

    // Flag untuk menandakan apakah tracker telah diinisialisasi
    std::vector<bool> initialized(kapasitas, false);

    while (true) {
        kamera >> frame;
        if (frame.empty()) {
            std::cerr << "frame kosong" << std::endl;
            return 1;
        }

        cv::Mat framefix = frame.clone();
        cv::Mat dummy;
        //frame.convertTo(framefix,-1,1,-10);
        cv::cvtColor(framefix, dummy, cv::COLOR_BGR2HSV);

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
            std::vector<cv::Rect> boundingBoxes;
            for (auto& scan : kontur[i]) {
                if (cv::contourArea(scan) > area) {
                    cv::Rect box = cv::boundingRect(scan);
                    boundingBoxes.push_back(box);
                }
            }

            // Filter bounding boxes to keep only the largest non-overlapping boxes
            std::vector<cv::Rect> filteredBoxes;
            for (const auto& box : boundingBoxes) {
                bool shouldAdd = true;
                for (auto it = filteredBoxes.begin(); it != filteredBoxes.end();) {
                    if (isInside(box, *it)) {
                        shouldAdd = false;
                        break;
                    } else if (isInside(*it, box)) {
                        it = filteredBoxes.erase(it);
                    } else {
                        ++it;
                    }
                }
                if (shouldAdd) {
                    filteredBoxes.push_back(box);
                }
            }

            for (const auto& box : filteredBoxes) {
                dataset[i].titik = cv::Point2f(box.x + box.width / 2, box.y + box.height / 2);
                cv::rectangle(framefix, box, dataset[i].color, 2);
                std::string text = "(" + std::to_string(dataset[i].titik.x) + "," + std::to_string(dataset[i].titik.y) + ")";
                cv::putText(framefix, text, cv::Point(box.x, box.y - 10), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

                // Inisialisasi tracker jika belum diinisialisasi
                if (!initialized[i]) {
                    trackers[i]->init(frame, box);
                    initialized[i] = true;
                }
            }
        }

        // Update tracker dan gambar hasil pelacakan
        for (int i = 0; i < kapasitas; ++i) {
            if (initialized[i]) {
                cv::Rect2d newBox;
                if (trackers[i]->update(frame, newBox)) {
                    // cv::rectangle(framefix, newBox, dataset[i].color, 2);
                    // dataset[i].titik = cv::Point2f(newBox.x + newBox.width / 2, newBox.y + newBox.height / 2);
                    // std::string text = "(" + std::to_string(dataset[i].titik.x) + "," + std::to_string(dataset[i].titik.y) + ")";
                    // cv::putText(framefix, text, cv::Point(newBox.x, newBox.y - 10), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
                }
            }
        }

        std::string teks = "adjust : " + std::to_string(adjust);
        cv::putText(framefix, teks, cv::Point(300, 50), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

        cv::putText(framefix, "oke", cv::Point(300, 200), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);

        cv::imshow("kamera 2", framefix);
        if (cv::waitKey(30) == 32) {
            break;
        }
    }
    return 0;
}
