#include <rclcpp/rclcpp.hpp>
#include <image_transport/image_transport.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/dnn.hpp>
#include <openvino/openvino.hpp>

#include <chrono>
#include <algorithm>
#include <functional>
#include <memory>
#include <string>

using namespace ov;
using namespace cv;
using namespace std;

const float SCORE_THRESHOLD = 0.2;
const float NMS_THRESHOLD = 0.4;
const float CONFIDENCE_THRESHOLD = 0.5;

class ImageSubscriberNode : public rclcpp::Node {
public:
  Mat img;
  ov::CompiledModel compiled_model;
  ov::InferRequest infer_request;
  struct Detection{
        int class_id;
        float confidence;
        Rect box;
    };

  struct Resize{
      Mat resized_image;
      int dw;
      int dh;
  };
  ImageSubscriberNode() : Node("image_subscriber_node2") {
    // // Image transport subscriber
    // it_ = std::make_shared<image_transport::ImageTransport>(this->shared_from_this());
    // img_sub_ = it_->subscribe(
    //   "camera/image", // Topic yang sama dengan publisher
    //   10,
    //   std::bind(&ImageSubscriberNode::image_callback, this, std::placeholders::_1));
  }
  ~ImageSubscriberNode() {
      cv::destroyAllWindows();
  }

  void init(){
    it_ = std::make_shared<image_transport::ImageTransport>(this->shared_from_this());
    img_sub_ = it_->subscribe("topic_2",5,std::bind(&ImageSubscriberNode::image_callback, this, std::placeholders::_1));
    Core core;
    shared_ptr<Model> model = core.read_model("/home/ichbinwil/Documents/face_detect_openvino_model/face_detect.xml");
    preprocess::PrePostProcessor ppp = preprocess::PrePostProcessor(model);
    ppp.input().tensor().set_element_type(element::u8).set_layout("NHWC").set_color_format(preprocess::ColorFormat::BGR);
    ppp.input().preprocess().convert_element_type(element::f32).convert_color(preprocess::ColorFormat::RGB).scale({255.,255.,255.});
    ppp.input().model().set_layout("NCHW");
    model = ppp.build();
    compiled_model = core.compile_model(model,"GPU");
    infer_request = compiled_model.create_infer_request();
  }

private:
  float fps = 0.0;
  std::shared_ptr<image_transport::ImageTransport> it_;
  image_transport::Subscriber img_sub_;

  Resize resize_and_pad(Mat& img,Size new_shape){
        float width = img.cols;
        float height = img.rows;
        float r = (float)(new_shape.width/max(width,height));
        int new_unpadW = int(round(width*r));
        int new_unpadH = int(round(height*r));
        Resize resize;
        cv::resize(img, resize.resized_image, Size(new_unpadW,new_unpadH), 0, 0,INTER_AREA);

        resize.dw = new_shape.width - new_unpadW;
        resize.dh = new_shape.height - new_unpadH;
        Scalar color = Scalar(100,100,100);
        copyMakeBorder(resize.resized_image, resize.resized_image, 0, resize.dh, 0, resize.dw, cv::BORDER_CONSTANT, color);

        return resize;
    }

  void image_callback(const sensor_msgs::msg::Image::ConstSharedPtr &msg) {
    try {
      // Konversi dari ROS Image ke OpenCV Mat
      Resize res;
      img = cv_bridge::toCvCopy(msg, "bgr8")->image;

      res = resize_and_pad(img,Size(640,640));
      auto start_time = chrono::high_resolution_clock::now();
      cv::waitKey(10);
      float *input_data = (float*) res.resized_image.data;
        Tensor input_tensor = Tensor(compiled_model.input().get_element_type(), compiled_model.input().get_shape(), input_data);
        infer_request.set_input_tensor(input_tensor);
        infer_request.infer();
        // size_t num_outputs = compiled_model.outputs().size();
        // size_t num_inputs = compiled_model.inputs().size();
        // if (num_outputs != 1) {
        //     RCLCPP_ERROR(this->get_logger(), "Model has %zu output tensors, expected 1", num_outputs);
        //     RCLCPP_INFO(this->get_logger(), "Model has %zu input tensors, expected 1", num_inputs);
        //     return;
        // }
        const Tensor &output_tensor = infer_request.get_output_tensor(0);
        Shape output_shape = output_tensor.get_shape();
        float *detections = output_tensor.data<float>();
        vector<cv::Rect> boxes;
        vector<int> class_ids;
        vector<float> confidences;
        for (size_t i = 0; i < output_shape[1]; i++){
            float *detection = &detections[i * output_shape[2]];
            float confidence = detection[4];
            if (confidence >= CONFIDENCE_THRESHOLD){
                float *classes_scores = &detection[5];
                cv::Mat scores(1, output_shape[2] - 5, CV_32FC1, classes_scores);
                cv::Point class_id;
                double max_class_score;
                cv::minMaxLoc(scores, 0, &max_class_score, 0, &class_id);

                if (max_class_score > SCORE_THRESHOLD){

                    confidences.push_back(confidence);

                    class_ids.push_back(class_id.x);

                    float x = detection[0];
                    float y = detection[1];
                    float w = detection[2];
                    float h = detection[3];

                    float xmin = x - (w / 2);
                    float ymin = y - (h / 2);

                    boxes.push_back(cv::Rect(xmin, ymin, w, h));
                }
            }
        }
        std::vector<int> nms_result;
        cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);
        std::vector<Detection> output;
        for (size_t i = 0; i < nms_result.size(); i++)
        {
            Detection result;
            int idx = nms_result[i];
            result.class_id = class_ids[idx];
            result.confidence = confidences[idx];
            result.box = boxes[idx];
            output.push_back(result);
        }
        for (size_t i = 0; i < output.size(); i++)
        {
            auto detection = output[i];
            auto box = detection.box;
            auto classId = detection.class_id;
            auto confidence = detection.confidence;
            float rx = (float)img.cols / (float)(res.resized_image.cols - res.dw);
            float ry = (float)img.rows / (float)(res.resized_image.rows - res.dh);
            box.x = rx * box.x;
            box.y = ry * box.y;
            box.width = rx * box.width;
            box.height = ry * box.height;
            cout << "Bbox" << i + 1 << ": Class: " << classId << " "
                << "Confidence: " << confidence << " Scaled coords: [ "
                << "cx: " << (float)(box.x + (box.width / 2)) / img.cols << ", "
                << "cy: " << (float)(box.y + (box.height / 2)) / img.rows << ", "
                << "w: " << (float)box.width / img.cols << ", "
                << "h: " << (float)box.height / img.rows << " ]" << endl;
            float xmax = box.x + box.width;
            float ymax = box.y + box.height;
            // info_obj.center_x = (float)(box.x + (box.width / 2)) / img.cols;
            // info_obj.center_y = (float)(box.y + (box.height / 2)) / img.rows;
            cv::rectangle(img, cv::Point(box.x, box.y), cv::Point(xmax, ymax), cv::Scalar(0, 255, 0), 3);
            cv::rectangle(img, cv::Point(box.x, box.y - 20), cv::Point(xmax, box.y), cv::Scalar(0, 255, 0), cv::FILLED);
            cv::putText(img, std::to_string(classId), cv::Point(box.x, box.y - 5), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
        }
        auto end_time = std::chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = end_time - start_time;
        fps = 1.0 / elapsed.count(); // Accurate frame rate calculation
        // RCLCPP_INFO(this->get_logger(), "FPS: %.2f", fps);
        putText(img, "FPS:" + to_string(fps),cv::Point(10,30),cv::FONT_HERSHEY_SIMPLEX,1.0,cv::Scalar(0,0,0),2);
        // RCLCPP_INFO(this->get_logger(), "FPS: %f", fps);
        // obj_pub->publish(this->info_obj);
        imshow("Detection Result 2", img);
        cv::waitKey(1); // Wajib untuk memproses event GUI OpenCV
    } catch (const cv_bridge::Exception &e) {
      RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
    }
  }
};

int main(int argc, char **argv) {
  rclcpp::init(argc, argv);
  auto node = std::make_shared<ImageSubscriberNode>();
  node->init();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}