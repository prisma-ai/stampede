
#ifndef GRAPH_PROC_GRAPH_H
#define GRAPH_PROC_GRAPH_H

#include <utility>
#include <vector>
#include <memory>
#include <unordered_map>
#include <deque>
#include <iostream>
#include <experimental/type_traits>
#include <future>
#include <unordered_map>

constexpr static auto BASE_GRAPH_CALLS_LOG = false;


template<typename T>
using value_t = decltype(std::declval<T &>().value());

template<typename T>
constexpr bool has_value = std::experimental::is_detected_v<value_t, T>;

template<typename T>
using next_t = typename T::Next;

template<typename T>
constexpr bool has_next = std::experimental::is_detected_v<next_t , T>;


template<typename Output>
auto value(Output output) {
    if constexpr (has_value<Output>) {
        return value(output.value());
    } else {
        return output;
    }
}

template<template<typename...> typename Impl, typename OutputT, typename ...InputsT>
struct Node {
    using Output = OutputT;
    using Inputs = std::tuple<InputsT...>;


    explicit Node(const std::string &tag) {
        tag_ = tag;
        if constexpr (BASE_GRAPH_CALLS_LOG) {
            std::cout << tag_ << " created" << std::endl;
        }
    }

    ~Node() {
        if constexpr (BASE_GRAPH_CALLS_LOG) {
            std::cout << tag_ << " destroyed" << std::endl;
        }
    }


    OutputT runPack(std::tuple<InputsT...> args) {
        if constexpr (BASE_GRAPH_CALLS_LOG) {
            std::cout << tag_ << " executed" << std::endl;
        }
        return static_cast<Impl<OutputT, InputsT...> *>(this)->runImpl(args);
    }

    void gc() {
        if constexpr (BASE_GRAPH_CALLS_LOG) {
            std::cout << tag_ << " gced" << std::endl;
        }
    }


    std::string tag_;


};

template<int I>
struct IntType {
    constexpr static int value = I;
};


template<int NodeIdx, typename NodeT>
struct IndexedNode {
    constexpr static int idx = NodeIdx;
    using type = NodeT;
};

template<int Id, typename IdsTuple>
struct Edge {
    constexpr static int source = Id;
    using ids = IdsTuple;
};


// Id
template<typename T>
struct type_t {
    using type = T;
};


template<int NodeIdx, typename ...Nodes>
struct NodeAtPack {
};

template<int NodeIdx, typename Head, typename ...Tail>
struct NodeAtPack<NodeIdx, Head, Tail...> {
    static auto find() {
        if constexpr(Head::idx == NodeIdx) {
            return type_t<typename Head::type>{};
        } else {
            return type_t<typename NodeAtPack<NodeIdx, Tail...>::type>{};
        }
    }

    using type = typename decltype(find())::type;
};


template<int NodeIdx, typename NodesTuple, int Counter = 0>
struct NodeAt {
    static auto find() {
        if constexpr(std::tuple_element_t<Counter, NodesTuple>::idx == NodeIdx) {
            return type_t<typename std::tuple_element_t<Counter, NodesTuple>::type>{};
        } else {
            return type_t<typename NodeAt<NodeIdx, NodesTuple, Counter + 1>::type>{};
        }
    }

    using type = typename decltype(find())::type;
};


template<int NodeIdx, typename ...Edges>
struct PrerequisitesForPack {
};


template<int NodeIdx, typename Head, typename...Tail>
struct PrerequisitesForPack<NodeIdx, Head, Tail...> {
    using type = std::conditional_t<
            Head::source == NodeIdx,
            typename Head::ids,
            typename PrerequisitesForPack<NodeIdx, Tail...>::type>;
};


template<int NodeIdx, typename Head>
struct PrerequisitesForPack<NodeIdx, Head> {
    using type = std::conditional_t<Head::source == NodeIdx, typename Head::ids, std::tuple<>>;
};



template<int NodeIdx, typename PrerequisitesTuple>
struct PrerequisitesForTuple;


template<int NodeIdx, typename ...Preqs>
struct PrerequisitesForTuple<NodeIdx, std::tuple<Preqs...>> {
    using type = typename PrerequisitesForPack<NodeIdx, Preqs...>::type;
};






#endif //GRAPH_PROC_GRAPH_H

