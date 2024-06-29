#include <opencv2/opencv.hpp>
#include <string>

using namespace cv;
using namespace std;

// Variabel global
int variable = 0;

// Callback function untuk trackbar
void on_trackbar(int, void*)
{
    // Fungsi ini akan dipanggil setiap kali nilai trackbar berubah
}

int main()
{
    // Membuat jendela
    string windowName = "Trackbar Example";
    namedWindow(windowName, WINDOW_AUTOSIZE);

    // Membuat trackbar
    int max_value = 180;
    createTrackbar("Value", windowName, &variable, max_value, on_trackbar);

    while (true)
    {
        // Membuat gambar kosong
        Mat frame = Mat::zeros(Size(640, 480), CV_8UC3);

        // Mengubah variabel integer menjadi string
        string text = "Value: " + to_string(variable);

        // Menempatkan teks pada frame
        putText(frame, text, Point(50, 50), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255), 2);

        // Menampilkan frame
        imshow(windowName, frame);

        // Menunggu selama 30 ms atau sampai tombol 'Esc' ditekan
        if (waitKey(30) == 27)
        {
            break;
        }
    }

    // Menutup semua jendela
    destroyAllWindows();
    return 0;
}
