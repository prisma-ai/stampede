//
// Created by soultoxik on 02.04.2021.
//

#ifndef GRAPH_PROC_TRAIT_H
#define GRAPH_PROC_TRAIT_H

#include <future>
#include <optional>
#include <iostream>

constexpr static auto TRAIT_LOG = false;


template<typename NextT>
struct CacheTrait : NextT {
    using Next = NextT;

    struct Cache {
        std::optional<typename Next::Output> data;

        typename Next::Output value() {
            return data.value();
        }
    };

    using Output = Cache;

    Cache cache;

    Cache runPack(typename Next::Inputs args) {
        if (!cache.data.has_value()) {
            if constexpr (TRAIT_LOG) {
                std::cout << "renewing cache" << std::endl;
            }
            cache.data = static_cast<Next *>(this)->runPack(args);
        } else {
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


    Future runPack(typename Next::Inputs args) {
        if constexpr (TRAIT_LOG) {
            std::cout << "running async" << std::endl;
        }
        auto task = Future{
                .data = std::async(std::launch::async,
                                   &Next::runPack, static_cast<Next *>(this), args)};

        return task;
    }

    void gc() {
        static_cast<Next *>(this)->gc();
    }
};



#endif //GRAPH_PROC_TRAIT_H
