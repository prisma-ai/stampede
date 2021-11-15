//
// Created by soultoxik on 11.11.2021.
//

#ifndef GRAPH_PROC_SRC_CV_SAMPLES_VANILLA_H_
#define GRAPH_PROC_SRC_CV_SAMPLES_VANILLA_H_

#include "Ops.h"


cv::Mat vanilla(cv::Mat inputImage, float portraitIntensity, float backgroundIntensity, float blendingIntensity) {
  auto [background, portrait, mask] = portraitSegmentation(inputImage);

  auto stylishBackground = styleTransfer(background, "background");
  auto stylishPortrait = styleTransfer(portrait, "portrait");

  auto fineBackground = backgroundRetouch(stylishBackground, backgroundIntensity);
  auto finePortrait = portraitRetouch(stylishPortrait, portraitIntensity);

  return blending(fineBackground, finePortrait, mask, blendingIntensity);
}


#endif //GRAPH_PROC_SRC_CV_SAMPLES_VANILLA_H_
