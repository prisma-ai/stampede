//
// Created by soultoxik on 11.10.2021.
//


#include "Optimized.h"
#include "Vanilla.h"
#include "Stampede.h"


#include <random>

using namespace spd;
constexpr auto SAMPLES = 5;

bool randomBool() {
  return rand() > (RAND_MAX / 2);
}

int main() {
  {
    cv::Mat image;
    auto allTime = 0.f;
    for (auto i = 0; i < SAMPLES; ++i) {
      auto portraitIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto backgroundIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto blendingIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

      auto t0 = std::chrono::system_clock::now();
      vanilla(image, portraitIntensity, backgroundIntensity, blendingIntensity);

      auto t1 = std::chrono::system_clock::now();

      allTime += std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    }
    std::cout << allTime / SAMPLES << std::endl;
  }

  {
    cv::Mat image;
    auto allTime = 0.f;
    std::unordered_map<std::string, cv::Mat> cache;
    for (auto i = 0; i < SAMPLES; ++i) {
      auto portraitIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto backgroundIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto blendingIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto inputChanged = randomBool();

      auto t0 = std::chrono::system_clock::now();
      optimized(image, portraitIntensity, backgroundIntensity, blendingIntensity, inputChanged, cache);

      auto t1 = std::chrono::system_clock::now();

      allTime += std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    }
    std::cout << allTime / SAMPLES << std::endl;
  }

  {
    cv::Mat image;
    auto allTime = 0.f;

    auto graph = withNodes<
        IndexedNode<0, CacheTrait<PortraitSegmentationNode>>,
        IndexedNode<1, ConfigurableCacheTrait<StyleTransferNode>>,
        IndexedNode<2, ConfigurableCacheTrait<StyleTransferNode>>,
        IndexedNode<3, ConfigurableCacheTrait<PortraitRetouchNode>>,
        IndexedNode<4, ConfigurableCacheTrait<BackgroundRetouchNode>>,
        IndexedNode<5, BlendingNode>
    >::andEdges<
        Edge<1, Deps<0>>,
        Edge<2, Deps<0>>,
        Edge<3, Deps<1>>,
        Edge<4, Deps<2>>,
        Edge<5, Deps<0, 3, 4>>
    >{};
    auto context = graph.createContext();


    for (auto i = 0; i < SAMPLES; ++i) {
      auto portraitIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto backgroundIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto blendingIntensity = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
      auto inputChanged = randomBool();

      auto t0 = std::chrono::system_clock::now();

      context.nodePtr<3>()->config = portraitIntensity;
      context.nodePtr<4>()->config = backgroundIntensity;
      context.nodePtr<5>()->config = blendingIntensity;

      context.nodePtr<0>()->dirty = inputChanged;

      graph.execute<Inputs<0>, 5>(context, {{image}});

      auto t1 = std::chrono::system_clock::now();

      allTime += std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    }
    std::cout << allTime / SAMPLES << std::endl;
  }


  return 0;
}