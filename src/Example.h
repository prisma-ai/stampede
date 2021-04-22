#ifndef GRAPH_PROC_EXAMPLE_H
#define GRAPH_PROC_EXAMPLE_H

#include "Graph.h"
#include <string>
#include <sstream>
#include <optional>


template <typename...>
class Summer : public Node<Summer, int, int, int> {
    friend class Node;

public:
    Summer(): Node<Summer, int, int, int>("summer") {}
private:
    int runImpl(std::tuple<int, int> args) {
        return std::get<0>(args) + std::get<1>(args);
    }
};


template <typename...>
class Mul2 : public Node<Mul2, int, int> {
    friend class Node;

public:
    Mul2(): Node<Mul2, int, int>("mul2") {}
private:
    int runImpl(std::tuple<int> args) {
        return std::get<0>(args) * 2;
    }
};

template <typename...>
class Stringify : public Node<Stringify, std::string, int> {
    friend class Node;
public:
    Stringify(): Node<Stringify, std::string, int>("stringify") {}
private:
    std::string runImpl(std::tuple<int> args) {
        return std::to_string(std::get<0>(args));
    }
};

template <typename...>
class Dummy : public Node<Dummy, std::string, int, std::string> {
    friend class Node;
public:
    Dummy(): Node<Dummy, std::string, int, std::string>("dummy") {}
private:
    std::string runImpl(std::tuple<int, std::string> args) {
    	std::stringstream os;
    	os << std::get<0>(args);
    	os << " ";
    	os << std::get<1>(args);

        return os.str();
    }
};
//
//template <typename...>
//class A : public Node<A, Unit, Unit> {
//    friend class Node;
//private:
//    Unit runImpl(std::tuple<Unit> _) {
//        std::cout << "A" << std::endl;
//        return {};
//    }
//};
//
//template <typename...>
//class B : public Node<B, Unit, Unit > {
//    friend class Node;
//private:
//    Unit runImpl(std::tuple<Unit> _) {
//        std::cout << "B" << std::endl;
//        return {};
//    }
//};



#define ExampleNodes IndexedNode<0, Summer<>>, IndexedNode<1, Stringify<>>, IndexedNode<2, Mul2<>>, IndexedNode<3, Dummy<>>
#define ExampleEdges Edge<3, std::tuple<IntType<2>, IntType<1>>>, Edge<1, std::tuple<IntType<0>>>, Edge<2, std::tuple<IntType<0>>>


#endif