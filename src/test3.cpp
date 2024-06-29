#include <opencv2/opencv.hpp>
#include <iostream>

cv::Mat adjustBrightness(const cv::Mat& image, int beta) {
    cv::Mat adjusted;
    image.convertTo(adjusted, -1, 1, beta);
    return adjusted;
}

int main() {
    // Baca gambar
    cv::Mat image = cv::imread("../sample1.jpeg");
    if (image.empty()) {
        std::cerr << "Gagal membaca gambar" << std::endl;
        return -1;
    }

    // Tampilkan gambar asli
    cv::imshow("Original Image", image);

    // Tentukan koordinat piksel dan daerah
    int x = 100, y = 100;
    int x1 = 50, y1 = 50, x2 = 150, y2 = 150;

    // Dapatkan nilai warna pada piksel tertentu
    cv::Vec3b pixelValue = image.at<cv::Vec3b>(y, x);
    std::cout << "Nilai warna pada piksel (" << x << ", " << y << "): "
              << static_cast<int>(pixelValue[0]) << ", "
              << static_cast<int>(pixelValue[1]) << ", "
              << static_cast<int>(pixelValue[2]) << std::endl;

    // Dapatkan nilai rata-rata warna pada daerah tertentu
    cv::Rect regionOfInterest(x1, y1, x2 - x1, y2 - y1);
    cv::Mat roi = image(regionOfInterest);
    cv::Scalar meanColor = cv::mean(roi);
    std::cout << "Nilai rata-rata warna pada daerah (" << x1 << ", " << y1
              << ") ke (" << x2 << ", " << y2 << "): "
              << meanColor[0] << ", " << meanColor[1] << ", "
              << meanColor[2] << std::endl;

    // Menyesuaikan kecerahan berdasarkan rata-rata warna
    int beta = -50;  // Contoh penyesuaian kecerahan
    cv::Mat adjustedImage = adjustBrightness(image, beta);

    // Tampilkan gambar yang sudah disesuaikan
    cv::imshow("Adjusted Image", adjustedImage);
    cv::waitKey(0);
    cv::destroyAllWindows();

    return 0;
}
