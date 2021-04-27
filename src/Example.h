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
    int runImpl(int arg0, int arg1) {
        return arg0 + arg1;
    }
};


template <typename...>
class Mul2 : public Node<Mul2, int, int> {
    friend class Node;

public:
    Mul2(): Node<Mul2, int, int>("mul2") {}
private:
    int runImpl(int arg) {
        return arg * 2;
    }
};

template <typename...>
class Stringify : public Node<Stringify, std::string, int> {
    friend class Node;
public:

    Stringify(): Node<Stringify, std::string, int>("stringify") {}
private:
    std::string runImpl(int arg) {
        return std::to_string(arg);
    }
};

template <typename...>
class Dummy : public Node<Dummy, std::string, int, std::string> {
    friend class Node;
public:
    Dummy(): Node<Dummy, std::string, int, std::string>("dummy") {}
private:
    std::string runImpl(int arg0, std::string arg1) {
    	std::stringstream os;
    	os << arg0;
    	os << " ";
    	os << arg1;

        return os.str();
    }
};



#define ExampleNodes IndexedNode<0, Summer<>>, IndexedNode<1, Stringify<>>, IndexedNode<2, Mul2<>>, IndexedNode<3, Dummy<>>
#define ExampleEdges Edge<3, std::tuple<IntType<2>, IntType<1>>>, Edge<1, std::tuple<IntType<0>>>, Edge<2, std::tuple<IntType<0>>>


#endif