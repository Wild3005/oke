#include<iostream>
#include<opencv2/opencv.hpp>

//DATADATA GLOBAL
const int kapasitas = 5;
std::string nama[kapasitas] = {"nf","phijau","kkuning","f","pabu2"};
cv::Scalar color[kapasitas] = {cv::Scalar(0, 255, 0),cv::Scalar(0, 0, 255),cv::Scalar(33,90,255),cv::Scalar(255,255,255),cv::Scalar(169,20,176)};
cv::Point letak[kapasitas] = {cv::Point(50, 80),cv::Point(130, 80),cv::Point(180, 80),cv::Point(210, 80),cv::Point(240,80)};
cv::Point letakdebug[kapasitas] = {cv::Point(30, 30),cv::Point(100, 30),cv::Point(150, 30),cv::Point(175, 30),cv::Point(200,30)};
for(int i=0;i<kapasitas;i++){

cv::Mat frametemp;
void debug(int cx, int cy, int i, const cv::Point& letak, const cv::Scalar& color) {
    if(cx > 1 && cy > 1) {
        std::string text = "(" + std::to_string(cx) + "," + std::to_string(cy) + ")";
        cv::putText(frametemp, text, letak, cv::FONT_HERSHEY_SIMPLEX, 0.5, color[i], 2);
    }
}

bool isInside(cv::Point point, std::vector<cv::Point>& contour) {
    return cv::pointPolygonTest(contour, point, 1) >= 0;
}

int main(){
    cv::VideoCapture kamera(2);
    if(!kamera.isOpened()){
        std::cerr << "kesalahan kamera" << std::endl;
        return 1;
    }
    cv::Mat frame;
    while(true){
        kamera >> frame;

        if(frame.empty()){
            std::cerr << "kesalahan frame" << std::endl;
            return 1;
        }

        frametemp = frame.clone();
        cv::GaussianBlur(frame, frame, cv::Size(15, 15), 0);

        cv::Mat dummy;
        cv::cvtColor(frame, dummy, cv::COLOR_BGR2HSV);

        //DATADATA
        //list nf,p_hijau,k_kuning
        cv::Scalar limitsetatas [kapasitas] = {cv::Scalar(9,102,255),cv::Scalar(110,255,132),cv::Scalar(30,115,255),cv::Scalar(157,34,255),cv::Scalar(169,20,176)};
        cv::Scalar limitsetbawah [kapasitas] = {cv::Scalar(0,53,138),cv::Scalar(84,124,78),cv::Scalar(0,38,173),cv::Scalar(0,0,175),cv::Scalar(127,0,131)};

        cv::Mat lim_color[kapasitas];
        for(int i=0;i<kapasitas;i++){
            cv::inRange(dummy,limitsetbawah[i],limitsetatas[i],lim_color[i]);
        }

        std::vector<std::vector<cv::Point>> kontur[kapasitas];
        for(int i=0;i<kapasitas;i++){
            cv::findContours(lim_color[i], kontur[i], cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
        }

        int num_kontur[kapasitas];

        double area_min = 1000;

        int cx[kapasitas],cy[kapasitas];
        for(int i=0;i<kapasitas;i++){
            for(const auto& scan : kontur[i]){
                if(cv::contourArea(scan) > area_min){
                    cv::Moments momen = cv::moments(scan);
                    cx[i] = static_cast<int>(momen.m10/momen.m00);
                    cy[i] = static_cast<int>(momen.m01/momen.m00);
                    cv::Point2f tengah;
                    float radius;
                    cv::minEnclosingCircle(scan,tengah,radius);
                    cv::circle(frametemp, tengah, static_cast<int>(radius), color[i], 2);
                    cv::putText(frametemp, std::to_string(radius) , cv::Point(500,50), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
                    
                    //info kontur
                    debug(cx[i],cy[i],i,letakdebug[i],color[i]);

                    //jumlah kontur
                    int num_kontur = kontur[i].size();
                    cv::putText(frametemp, std::to_string(num_kontur) , letak[i], cv::FONT_HERSHEY_SIMPLEX, 0.5, color[i], 2);
                    /*if(radius >= 144 &&radius <= 150){
                        cv::putText(frame, "KERTAS" , cv::Point(250, 50), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
                    }*/
                    
                }
            }
            //cek kondisi
            //nf
            if (!kontur[2].empty() && isInside(cv::Point(cx[0], cy[0]), kontur[2][0])) {
                cv::putText(frametemp, "NON_FERRO" , cv::Point(250, 150), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 2);
            }
        }


        cv::imshow("test",frametemp);

        if(cv::waitKey(30) == 32){
            break;
        }

    }
    return 0;
}