#include<iostream>
#include<opencv2/opencv.hpp>

typedef struct{
    std::string nama;
    cv::Scalar color;
    cv::Point2f titik;
    float radius;
    cv::Scalar limitA;
    cv::Scalar limitB;
}data;

//inisiasi glonal

int main(){
    cv::VideoCapture kamera(2);
    if(!kamera.isOpened()){
        std::cerr << "masalah kamera" << std::endl;
        return 1;
    }
    int kapasitas = 5; //update disini
    bool pernah[kapasitas] = {false};
    data dataset[kapasitas] = {
        //{nama,warna,titik,radius,warna atas, warna bawah}
        //{"kkuning",cv::Scalar(33,90,255),cv::Point2f(0,0),0,cv::Scalar(30,115,255),cv::Scalar(0,38,173)},
        {"kkuning",cv::Scalar(33,90,255),cv::Point2f(0,0),0,cv::Scalar(39,123,216),cv::Scalar(8,45,132)},
        //{"nferro",cv::Scalar(0, 255, 0),cv::Point2f(0,0),0,cv::Scalar(9,102,255),cv::Scalar(0,53,138)},
        {"phijau",cv::Scalar(110, 255, 132),cv::Point2f(0,0),0,cv::Scalar(105, 255, 147),cv::Scalar(79, 73, 33)},
        {"daun",cv::Scalar(0, 255, 255),cv::Point2f(0,0),0,cv::Scalar(49,255,183),cv::Scalar(24,142,153)},
        
        {"nferro",cv::Scalar(0, 255, 0),cv::Point2f(0,0),0,cv::Scalar(18,167,176),cv::Scalar(5,81,105)},////
        //{"ferro",cv::Scalar(255, 255, 255),cv::Point2f(0,0),0,cv::Scalar(127, 38, 203),cv::Scalar(85, 8, 64)},
        //{"ferro",cv::Scalar(255, 255, 255),cv::Point2f(0,0),0,cv::Scalar(141, 105, 129),cv::Scalar(100, 0, 59)},
        {"kkoran",cv::Scalar(135, 135, 135),cv::Point2f(0,0),0,cv::Scalar(180, 106, 102),cv::Scalar(0, 0, 0)}
        }; //update disini

    int adjust =0;
    bool giliran = true;
    cv::Mat frame;
    while(1){
        kamera >> frame;
        if(frame.empty()){
            std::cerr << "frame kosong" << std::endl;
            return 1;
        }
        cv::Mat frametemp;
        frametemp = frame.clone();
        //cv::GaussianBlur(frame,frame,cv::Size(15,15),0);
        frame.convertTo(frame,-1,1,adjust);
        if(adjust<0&&adjust>=-100&&!giliran){
            adjust -=1;
            if(adjust == -100) giliran = true;
        }else if(adjust>=0&&adjust<=100&&giliran){
            adjust +=1;
            if(adjust == 100) giliran = false;
        }else if(adjust>=0&&adjust<=100&&!giliran){
            adjust -=1;
        }else if(adjust<0&&adjust>=-100&&giliran){
            adjust +=1;
        }
        std::string teks = "adjust : " +  std::to_string(adjust);
        cv::putText(frametemp,teks,cv::Point(300,50),cv::FONT_HERSHEY_COMPLEX,0.5,cv::Scalar(255,255,255),1);
        cv::Mat dummy;
        cv::cvtColor(frame,dummy,cv::COLOR_BGR2HSV);

        cv::Mat limcolor[kapasitas];
        for(int i=0; i<kapasitas; i++){
            cv::inRange(dummy,dataset[i].limitB,dataset[i].limitA,limcolor[i]);
        }

        std::vector<std::vector<cv::Point>> kontur[kapasitas];
        for(int i=0;i<kapasitas;i++){
            cv::findContours(limcolor[i],kontur[i],cv::RETR_EXTERNAL,cv::CHAIN_APPROX_SIMPLE);
        }

        double min_area = 1000;
        double min_area_rectA = 350;
        double min_area_rectB = 150;

        for(int i=0;i<kapasitas;i++){
            if(i == kapasitas - 1){
                for(const auto& scan : kontur[kapasitas - 1]){
                    if((cv::contourArea(scan) < min_area_rectA)&&(cv::contourArea(scan) > min_area_rectB)){
                        cv::Rect box = cv::boundingRect(scan);
                        dataset[kapasitas - 1].titik = cv::Point2f(box.x+box.width/2,box.y+box.height);

                        //debugging
                        //cv::rectangle(frametemp,box,cv::Scalar(0,0,0),2);
                        //std::string text = "(" + std::to_string(dataset[kapasitas - 1].titik.x) + ","+ std::to_string(dataset[kapasitas - 1].titik.y)+")";
                        //cv::putText(frametemp,text,cv::Point(300,50),cv::FONT_HERSHEY_COMPLEX,0.5,cv::Scalar(255,255,255),1);
                    }
                }
            }else{
                int indeks = 0;
                for(const auto& scan : kontur[i]){
                    if(cv::contourArea(scan) > min_area){
                        cv::Point2f titiktemp;
                        float radius;
                        cv::minEnclosingCircle(scan,titiktemp,radius);
                        dataset[i].titik = titiktemp;
                        dataset[i].radius = radius;

                        //debugging
                        cv::circle(frametemp, titiktemp, static_cast<int>(radius), dataset[i].color, 2);
                        //std::string text = "(" + std::to_string(titiktemp.x) + ","+ std::to_string(titiktemp.y)+")";
                        //cv::putText(frametemp,text,cv::Point(20,20),cv::FONT_HERSHEY_COMPLEX,0.5,cv::Scalar(255,255,255),1);
                        pernah[indeks]=true;
                        
                    }
                    indeks=+1;
                }
            }
        }

        double jarak = cv::norm(dataset[0].titik - dataset[1].titik);// titik nf
        double jarak2 = cv::norm(dataset[2].titik - dataset[3].titik);// titik kkoran

        //kondisi-kondisi
        if((jarak < dataset[0].radius)||(jarak < dataset[1].radius)){
            cv::putText(frametemp,"non_ferro",cv::Point(400,100),cv::FONT_HERSHEY_COMPLEX,0.5,cv::Scalar(255,255,255),1);
        }
        else if(jarak2 < dataset[2].radius){
            cv::putText(frametemp,"koran",cv::Point(400,100),cv::FONT_HERSHEY_COMPLEX,0.5,cv::Scalar(255,255,255),1);
        }else{
            for(int i = 0;i<kapasitas;i++){
                if(pernah[i]){
                    cv::putText(frametemp,dataset[i].nama,cv::Point(500,100),cv::FONT_HERSHEY_COMPLEX,0.5,cv::Scalar(255,255,255),1);
                }
            }
        }
        //set ulang data
        for(int i=0;i<kapasitas;i++){
            dataset[i].titik = cv::Point(0,0);
            dataset[i].radius = 0;
            pernah[i]=false;
        }

        cv::putText(frametemp,"terdeteksi: ",cv::Point(300,100),cv::FONT_HERSHEY_COMPLEX,0.5,cv::Scalar(255,255,255),1);

        cv::imshow("oke",frametemp);
        if(cv::waitKey(30)== 32){
            break;
        }
    }
}