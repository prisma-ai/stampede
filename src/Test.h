//
// Created by soultoxik on 01.04.2021.
//

#ifndef GRAPH_PROC_TEST_H
#define GRAPH_PROC_TEST_H


#include "Graph.h"
#include <sstream>
#include <optional>


template <typename...>
class LongOp1 : public Node<LongOp1, int, int> {
    friend class Node;
public:
    LongOp1(): Node<LongOp1, int, int>("LongOp1") {}
private:
    int runImpl(int args) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        return args;
    }
};


template <typename...>
class LongOp2 : public Node<LongOp2, int, int> {
    friend class Node;

public:
    LongOp2(): Node<LongOp2, int, int>("LongOp2") {}
private:

    int runImpl(int args) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        return args;
    }
};

template <typename...>
class Id : public Node<Id, int, int> {
    friend class Node;

public:
    Id(): Node<Id, int, int>("Id") {}
private:

    int runImpl(int arg) {
        return arg;
    }
};


#define TestSyncNodes IndexedNode<0, Id<>>, IndexedNode<1, LongOp1<>>, IndexedNode<2, LongOp2<>>, IndexedNode<3, Summer<>>
#define TestSyncEdges Edge<3, std::tuple<Int<2>, Int<1>>>, Edge<1, std::tuple<Int<0>>>, Edge<2, std::tuple<Int<0>>>


#define TestAsyncNodes IndexedNode<0, Id<>>, IndexedNode<1, AsyncTrait<LongOp1<>>>, IndexedNode<2, AsyncTrait<LongOp2<>>>, IndexedNode<3, Summer<>>
#define TestAsyncEdges Edge<3, std::tuple<Int<2>, Int<1>>>, Edge<1, std::tuple<Int<0>>>, Edge<2, std::tuple<Int<0>>>


#define TestNoCacheNodes IndexedNode<0, LongOp1<>>, IndexedNode<1, Summer<>>
#define TestNoCacheEdges Edge<1, std::tuple<Int<0>, Int<0>>>

#define TestCacheNodes IndexedNode<0, CacheTrait<LongOp1<>>>, IndexedNode<1, Summer<>>
#define TestCacheEdges Edge<1, Deps<0, 1>>


#define TestGCNodes IndexedNode<0, Id<> >, IndexedNode<1, LongOp1<> >, IndexedNode<2, Summer<> >
#define TestGCEdges Edge<2, std::tuple<  Int<0>, Int<1>  >  >, Edge<1, std::tuple<  Int<0>  >  >


#endif //GRAPH_PROC_TEST_H
