//
// Created by soultoxik on 02.04.2021.
//

#ifndef GRAPH_PROC_TRAIT_H
#define GRAPH_PROC_TRAIT_H

#include <future>
#include <optional>
#include <iostream>
#include "async/Pool.h"

namespace spd {

constexpr static auto TRAIT_LOG = false;

template <typename NextT>
struct TraitBase : NextT {
  void gc() {
    static_cast<NextT *>(this)->gc();
  }

};

template<typename T>
struct Cache {
  std::optional<T> data;


  T value() {
    return data.value();
  }
};

struct CacheTraitBase {
  std::function<bool()> keep = []() { return true; };
};


template<typename T>
using cache_t = decltype(std::declval<T &>().data);

template<typename T>
static constexpr bool has_cache = std::experimental::is_detected_v<cache_t, T>;

template<typename T>
struct hasCachePredicate {
  constexpr static auto value = has_cache<T>;
};


template<typename NextT>
struct CacheTrait : CacheTraitBase, TraitBase<NextT> {
  using Next = NextT;
  using Param = typename Next::Param;

  using Output = Cache<typename Next::Output>;

  Output cache;

  bool force = false;

  template<int ...N>
  Output runPack(typename Next::Inputs args, std::integer_sequence<int, N...> ids) {
    // no cache -- recompute
    // cache and no keep -- recompute
    if (!cache.data.has_value() || force) {
      if constexpr (TRAIT_LOG) {
        std::cout << "renewing cache" << std::endl;
      }
      cache.data = static_cast<Next *>(this)->runPack(args, ids);

      force = false;

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
//    cache.dirty = true;

    static_cast<Next *>(this)->gc();

    if constexpr (TRAIT_LOG) {
      std::cout << "cache gced" << std::endl;
    }
  }


  template<typename TraitedInputsT>
  Output sequencePoint(TraitedInputsT args, bool childrenChanged) {
    if(childrenChanged) {
      force = true;
      auto values = std::apply(
          [](auto ...x) -> typename Next::Inputs { return std::make_tuple(value(x)...); }, args);
      return runPack(values, std::make_integer_sequence<int, std::tuple_size_v<TraitedInputsT>>{});
    } else {
      return cache;
    }
  }
};




template <typename NextT>
struct ConfigurableCacheTrait : CacheTrait<NextT> {
 public:
  using Param = typename NextT::Param;
  ConfigurableCacheTrait() {
   static_cast<CacheTraitBase*>(this)->keep = [&]() {
     if(!lastConfig.has_value()) {
       lastConfig = static_cast<NodeBase<Param>*>(this)->config;
       return false;
     }
     return static_cast<NodeBase<Param>*>(this)->config == *lastConfig;
   };
  }
 private:
  std::optional<Param> lastConfig;
};

template <typename T>
struct Future {
  std::shared_future<T> data;

  T value() {
    return data.get();
  }
};

struct AsyncPoolBase {
 public:
  void pool(std::shared_ptr<PoolBase> pool) {
    pool_ = std::move(pool);
  }

 protected:
  std::shared_ptr<PoolBase> pool_;

};

template<typename NextT>
struct AsyncPoolTrait : AsyncPoolBase, TraitBase<NextT> {
  using Next = NextT;
  using Param = typename Next::Param;

  using Output = Future<typename NextT::Output>;

  template<int ...N>
  Output runPack(typename Next::Inputs args, std::integer_sequence<int, N...> ids) {
    if constexpr (TRAIT_LOG) {
      std::cout << "running async" << std::endl;
    }

    auto task = Output {
        .data = pool_->enqueue<typename Next::Output>([args, ids, this]() {
          return static_cast<Next *>(this)->runPack(args, ids);
        })
    };

    return task;
  }


  template<typename TraitedInputsT>
  Output sequencePoint(TraitedInputsT args, bool childrenChanged) {
    auto values = std::apply(
        [](auto ...x) -> typename Next::Inputs { return std::make_tuple(value(x)...); }, args);
    return runPack(values, std::make_integer_sequence<int, std::tuple_size_v<TraitedInputsT>>{});
  }

};

template<typename NextT>
struct AsyncTrait : TraitBase<NextT> {
  using Next = NextT;
  using Param = typename Next::Param;



  using Output = Future<typename NextT::Output>;

  template<int ...N>
  Output runPack(typename Next::Inputs args, std::integer_sequence<int, N...> ids) {
    if constexpr (TRAIT_LOG) {
      std::cout << "running async" << std::endl;
    }
    auto task = Output {
        .data = std::async(std::launch::async,
                           &Next::template runPack<N...>,
                           static_cast<Next *>(this), args, ids)
    };

    return task;
  }


  template<typename TraitedInputsT>
  Output sequencePoint(TraitedInputsT args, bool childrenChanged) {
    auto values = std::apply(
        [](auto ...x) -> typename Next::Inputs { return std::make_tuple(value(x)...); }, args);
    return runPack(values, std::make_integer_sequence<int, std::tuple_size_v<TraitedInputsT>>{});
  }
};

}

#endif //GRAPH_PROC_TRAIT_H
