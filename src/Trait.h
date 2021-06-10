//
// Created by soultoxik on 02.04.2021.
//

#ifndef GRAPH_PROC_TRAIT_H
#define GRAPH_PROC_TRAIT_H

#include <future>
#include <optional>
#include <iostream>

constexpr static auto TRAIT_LOG = false;

struct CacheTraitBase {
  std::function<bool()> keep = []() { return true; };
};

template<typename NextT>
struct CacheTrait : CacheTraitBase, NextT {
  using Next = NextT;

  struct Cache {
    std::optional<typename Next::Output> data;

    typename Next::Output value() {
      return data.value();
    }
  };

  using Output = Cache;

  Cache cache;

  template<int ...N>
  Cache runPack(typename Next::Inputs args, std::integer_sequence<int, N...> ids) {
    // no cache -- recompute
    // cache and no keep -- recompute
    if (!cache.data.has_value()) {
      if constexpr (TRAIT_LOG) {
        std::cout << "renewing cache" << std::endl;
      }
      cache.data = static_cast<Next *>(this)->runPack(args, ids);
    } else {
      if (!static_cast<CacheTraitBase *>(this)->keep()) {
        cache.data = {};
        cache.data = static_cast<Next *>(this)->runPack(args, ids);
      }
      if constexpr (TRAIT_LOG) {
        std::cout << "using cached" << std::endl;
      }
    }

    return cache;
  }

  void gc() {
    cache.data = {};
    static_cast<Next *>(this)->gc();

    if constexpr (TRAIT_LOG) {
      std::cout << "cache gced" << std::endl;
    }
  }
};

template<typename NextT>
struct AsyncTrait : NextT {
  using Next = NextT;

  struct Future {
    std::shared_future<typename Next::Output> data;

    typename Next::Output value() {
      return data.get();
    }
  };

  using Output = Future;

  template<int ...N>
  Future runPack(typename Next::Inputs args, std::integer_sequence<int, N...> ids) {
    if constexpr (TRAIT_LOG) {
      std::cout << "running async" << std::endl;
    }
    auto task = Future{
        .data = std::async(std::launch::async,
                           &Next::template runPack<N...>,
                           static_cast<Next *>(this), args, ids)
    };

    return task;
  }

  void gc() {
    static_cast<Next *>(this)->gc();
  }
};

#endif //GRAPH_PROC_TRAIT_H
