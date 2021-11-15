//
// Created by soultoxik on 11.11.2021.
//

#ifndef GRAPH_PROC_SRC_CV_SAMPLES_OPTIMIZED_H_
#define GRAPH_PROC_SRC_CV_SAMPLES_OPTIMIZED_H_

#include "Ops.h"

cv::Mat optimized(cv::Mat inputImage,
                         float portraitIntensity,
                         float backgroundIntensity,
                         float blendingIntensity,
                         bool inputChanged,
                         std::unordered_map<std::string, cv::Mat> cache ) {
  cv::Mat background, portrait, mask;

  if(inputChanged) {
    auto [genB, genP, genM] = portraitSegmentation(inputImage);

    background = genB;
    portrait = genP;

    cache["portrait"] = portrait;
    cache["background"] = background;
    cache["mask"] = genM;

  } else {
    portrait = cache["portrait"];
    background = cache["background"];
    mask = cache["mask"];
  }

  cv::Mat stylishBackground, stylishPortrait;
  if(inputChanged) {
    auto stylishBackgroundTask = std::async(std::launch::async, [&background]() {
      return styleTransfer(background, "background");
    });

    auto stylishPortraitTask = std::async(std::launch::async, [&portrait]() {
      return styleTransfer(portrait, "portrait");
    });



    cache["stylishBackground"] = stylishBackgroundTask.get();
    cache["stylishPortrait"] = stylishPortraitTask.get();
  } else {
    stylishBackground = cache["stylishBackground"];
    stylishPortrait = cache["stylishPortrait"];
  }

  auto fineBackground = backgroundRetouch(stylishBackground, backgroundIntensity);
  auto finePortrait = portraitRetouch(stylishPortrait, portraitIntensity);

  cv::Mat result;

  auto ss = std::stringstream();
  ss << blendingIntensity;
  auto blendingKey = ss.str();

  if(inputChanged || cache.count(blendingKey) == 0) {
    result = blending(fineBackground, finePortrait, mask, blendingIntensity);
    cache[blendingKey] = result;
  } else {
    result = cache[blendingKey];
  }

  return result;
}

#endif //GRAPH_PROC_SRC_CV_SAMPLES_OPTIMIZED_H_
