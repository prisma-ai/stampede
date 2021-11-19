#ifndef GRAPH_PROC_EXAMPLE_H
#define GRAPH_PROC_EXAMPLE_H

#include "spd/Graph.h"
#include <string>
#include <sstream>
#include <optional>

namespace spd {

class Summer : public Node<Summer, Unit, int, int, int> {
//  friend class Node;

 public:

  Summer() : Node<Summer, Unit, int, int, int>("summer") {}
// private:
  int runImpl(int arg0, int arg1) {
    return arg0 + arg1;
  }
};

class LongSummer : public Node<LongSummer, Unit, int, int, int> {
//  friend class Node;

 public:
  LongSummer() : Node<LongSummer, Unit, int, int, int>("long summer") {}
// private:
  int runImpl(int arg0, int arg1) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    return arg0 + arg1;
  }
};

class Mul2 : public Node<Mul2, Unit, int, int> {
//  friend class Node;

 public:
  Mul2() : Node<Mul2, Unit, int, int>("mul2") {}
// private:
  int runImpl(int arg) {
    return arg * 2;
  }
};

class Stringify : public Node<Stringify, Unit, std::string, int> {
//  friend class Node;
 public:

  Stringify() : Node<Stringify, Unit, std::string, int>("stringify") {}
// private:
  std::string runImpl(int arg) {
    return std::to_string(arg);
  }
};

class Dummy : public Node<Dummy, Unit, std::string, int, std::string> {
//  friend class Node;
 public:
  Dummy() : Node<Dummy, Unit, std::string, int, std::string>("dummy") {}
// private:
  std::string runImpl(int arg0, std::string arg1) {
    std::stringstream os;
    os << arg0;
    os << " ";
    os << arg1;

    return os.str();
  }
};


class LongOp1 : public Node<LongOp1, float, int, int> {
//  friend class Node;
 public:
  LongOp1() : Node<LongOp1, float, int, int>("LongOp1") {}
// private:
  int runImpl(int args) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    return args;
  }
};


class LongOp2 : public Node<LongOp2, Unit, int, int> {
//  friend class Node;

 public:
  LongOp2() : Node<LongOp2, Unit, int, int>("LongOp2") {}
// private:

  int runImpl(int args) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    return args;
  }
};

class Id : public Node<Id, Unit, int, int> {
//  friend class Node;

 public:
  Id() : Node<Id, Unit, int, int>("Id") {}
// private:

  int runImpl(int arg) {
    return arg;
  }
};

}

#define ExampleNodes IndexedNode<0, Summer<>>, IndexedNode<1, Stringify<>>, IndexedNode<2, Mul2<>>, IndexedNode<3, Dummy<>>
#define ExampleEdges Edge<3, std::tuple<IntType<2>, IntType<1>>>, Edge<1, std::tuple<IntType<0>>>, Edge<2, std::tuple<IntType<0>>>

#define TestSyncNodes IndexedNode<0, Id>, IndexedNode<1, LongOp1>, IndexedNode<2, LongOp2>, IndexedNode<3, Summer>
#define TestSyncEdges Edge<3, std::tuple<Int<2>, Int<1>>>, Edge<1, std::tuple<Int<0>>>, Edge<2, std::tuple<Int<0>>>

#define TestAsyncNodes IndexedNode<0, Id>, IndexedNode<1, AsyncTrait<LongOp1>>, IndexedNode<2, AsyncTrait<LongOp2>>, IndexedNode<3, Summer>
#define TestAsyncEdges Edge<3, std::tuple<Int<2>, Int<1>>>, Edge<1, std::tuple<Int<0>>>, Edge<2, std::tuple<Int<0>>>

#define TestAsyncPoolNodes IndexedNode<0, Id>, IndexedNode<1, AsyncPoolTrait<LongOp1>>, IndexedNode<2, AsyncPoolTrait<LongOp2>>, IndexedNode<3, Summer>
#define TestAsyncPoolEdges Edge<3, std::tuple<Int<2>, Int<1>>>, Edge<1, std::tuple<Int<0>>>, Edge<2, std::tuple<Int<0>>>

#define TestNoCacheNodes IndexedNode<0, LongOp1>, IndexedNode<1, Summer>
#define TestNoCacheEdges Edge<1, std::tuple<Int<0>, Int<0>>>

#define TestCacheNodes IndexedNode<0, CacheTrait<LongOp1>>, IndexedNode<1, Summer>
#define TestCacheEdges Edge<1, Deps<0, 0>>


#define TestManyCacheNodes IndexedNode<0, CacheTrait<LongOp1>>, IndexedNode<1, CacheTrait<LongOp2>>, IndexedNode<2, Summer>
#define TestManyCacheEdges Edge<2, Deps<0, 1>>

#define TestGCNodes IndexedNode<0, Id >, IndexedNode<1, LongOp1 >, IndexedNode<2, Summer >
#define TestGCEdges Edge<2, std::tuple<  Int<0>, Int<1>  >  >, Edge<1, std::tuple<  Int<0>  >  >


#endif
