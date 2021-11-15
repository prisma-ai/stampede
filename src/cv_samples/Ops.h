//
// Created by soultoxik on 11.11.2021.
//

#ifndef GRAPH_PROC_SRC_CV_SAMPLES_OPS_H_
#define GRAPH_PROC_SRC_CV_SAMPLES_OPS_H_

#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <numeric>
#include <thread>
#include <future>


using namespace std::chrono_literals;

constexpr auto HEAVY_OP = 100ms;
constexpr auto LIGHT_OP = 10ms;


std::tuple<cv::Mat, cv::Mat, cv::Mat> portraitSegmentation(cv::Mat& img) {
  // XXX: stub
  std::this_thread::sleep_for(HEAVY_OP);

  return { img, img, img};
}

cv::Mat styleTransfer(cv::Mat& img, std::string modelName) {
  // XXX: stub
  std::this_thread::sleep_for(HEAVY_OP);

  return img;
}


cv::Mat portraitRetouch(cv::Mat& img, float intensity) {
  std::this_thread::sleep_for(LIGHT_OP);

  return img;
}

cv::Mat backgroundRetouch(cv::Mat& img, float intensity) {
  std::this_thread::sleep_for(LIGHT_OP);

  return img;
}

cv::Mat blending(cv::Mat& background, cv::Mat portrait, cv::Mat mask, float intensity) {
  std::this_thread::sleep_for(LIGHT_OP);

  return background;
}
#endif //GRAPH_PROC_SRC_CV_SAMPLES_OPS_H_
