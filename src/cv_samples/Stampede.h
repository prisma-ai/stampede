//
// Created by soultoxik on 11.11.2021.
//

#ifndef GRAPH_PROC_SRC_CV_SAMPLES_STAMPEDE_H_
#define GRAPH_PROC_SRC_CV_SAMPLES_STAMPEDE_H_

#include "Ops.h"
#include <spd/All.h>

using PortraitSegmentation = std::tuple < cv::Mat, cv::Mat, cv::Mat>;

declare_node(PortraitSegmentationNode, spd::Unit, PortraitSegmentation, cv::Mat&)
PortraitSegmentation PortraitSegmentationNode::runImpl( cv::Mat& image) {
  return portraitSegmentation(image);
}

declare_node(StyleTransferNode, std::string, cv::Mat, PortraitSegmentation)
cv::Mat StyleTransferNode::runImpl( PortraitSegmentation segmentation) {
  return styleTransfer(std::get<0>(segmentation), config);
}

declare_node(PortraitRetouchNode, float, cv::Mat, cv::Mat)
cv::Mat PortraitRetouchNode::runImpl( cv::Mat image) {
  return portraitRetouch(image, config);
}

declare_node(BackgroundRetouchNode, float, cv::Mat, cv::Mat)
cv::Mat BackgroundRetouchNode::runImpl( cv::Mat image) {
  return backgroundRetouch(image, config);
}
declare_node(BlendingNode, float, cv::Mat, PortraitSegmentation, cv::Mat, cv::Mat)
cv::Mat BlendingNode::runImpl( PortraitSegmentation segmentation, cv::Mat background, cv::Mat portrait) {
  return blending(background, portrait, std::get<0>(segmentation), config);
}


#endif //GRAPH_PROC_SRC_CV_SAMPLES_STAMPEDE_H_
