#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // Variabel-variabel yang diperlukan
    bool keadaan = false;
    std::vector<std::vector<cv::Point>> kontur;

    // Membuat gambar lingkaran
    cv::Mat circleoke = cv::Mat::zeros(cv::Size(500, 500), CV_8U);
    cv::Point2f titik(200, 200);
    cv::Point2f titikuji(50, 250); // Mengubah posisi titikuji ke luar lingkaran
    cv::circle(circleoke, titik, 100, cv::Scalar(255, 255, 255), 1);
    cv::circle(circleoke, titikuji, 1, cv::Scalar(255, 255, 255, 1));

    // Mencari kontur dari gambar lingkaran
    cv::findContours(circleoke, kontur, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    // Menampilkan hasil kontur jika titik uji berada di dalamnya
    for (const auto& scan : kontur) {
        double distance = cv::norm(titikuji - titik); // Menghitung jarak antara titik uji dan pusat lingkaran
        if (distance < 100) { // Memeriksa apakah titik uji berada di dalam kontur dan jaraknya tidak lebih besar dari jari-jari lingkaran
            // Menampilkan teks "berhasil" jika titik uji berada di dalam kontur
            cv::putText(circleoke, "berhasil", cv::Point(350, 400), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 255), 2);
            break; // Keluar dari loop karena sudah menemukan kontur yang memuat titik
        }
    }

    // Menampilkan gambar
    cv::imshow("oke", circleoke);
    cv::waitKey(0);

    return 0;
}
