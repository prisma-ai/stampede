
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
#include "spd/util/FuncHelper.h"
#include "spd/util/Compose.h"


namespace spd {

/*
 * Simple wrapper for node declaration
 * If you don't like macros -- just copy
 */
#define declare_node(name, param, out, ...) \
class name : public spd::Node<name, param, out, __VA_ARGS__> { \
  friend class Node;  \
 public:  \
  name() : spd::Node<name, param, out, __VA_ARGS__>\
                                   (#name) {}      \
 private:                            \
  out runImpl(__VA_ARGS__);                 \
                                            \
};

/*
 * Used in empty node state declaration
 */
struct Unit {};

constexpr static auto BASE_GRAPH_CALLS_LOG = false;

template<typename T>
using value_t = decltype(std::declval<T &>().value());

template<typename T>
constexpr bool has_value = std::experimental::is_detected_v<value_t, T>;

template<typename T>
using next_t = typename T::Next;

template<typename T>
constexpr bool has_next = std::experimental::is_detected_v<next_t, T>;

/**
 * Unwraps value from all side-effects from traits
 * @tparam Output
 * @param output -- wrapped value
 * @return unwrapped value
 */
template<typename Output>
auto value(Output output) {
  if constexpr (has_value<Output>) {
    return value(output.value());
  } else {
    return output;
  }
}

template<typename Param>
struct NodeBase {
  Param config;
  bool dirty = true;

};

/**
 * Base class for custom node implementation
 * You could also set "tag" for logging purposes
 * Serves as container for useful computations
 * Holds input/output types, state, and sequence point resolver (could be overrided via trait)
 *
 * @tparam Impl
 * @tparam ParamType
 * @tparam OutputT
 * @tparam InputsT
 */
template<typename Impl, typename ParamType, typename OutputT, typename ...InputsT>
struct Node : NodeBase<ParamType> {
  using Output = OutputT;
  using Inputs = std::tuple<InputsT...>;

  using Param = ParamType;


  explicit Node(const std::string &tag = "node") {
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

  template<int ...N>
  OutputT runPack(std::tuple<InputsT...> args, std::integer_sequence<int, N...>) {
    if constexpr (BASE_GRAPH_CALLS_LOG) {
      std::cout << tag_ << " executed" << std::endl;
    }
    this->dirty = true;
    return static_cast<Impl *>(this)->runImpl(std::get<N>(args)...);
  }

  void gc() {
    if constexpr (BASE_GRAPH_CALLS_LOG) {
      std::cout << tag_ << " gced" << std::endl;
    }
  }

  template<typename TraitedInputsT>
  OutputT sequencePoint(TraitedInputsT args, bool childrenChanged) {
    auto values = std::apply(
        [](auto ...x) -> Inputs { return std::make_tuple(value(x)...); }, args);
    return runPack(values, std::make_integer_sequence<int, std::tuple_size_v<TraitedInputsT>>{});
  }

  std::string tag_;

};

template<int I>
struct Int {
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
struct PrerequisitesForPack {};

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

struct NotFound {};

template<int Id, int Counter, typename IdsTuple>
struct FindInputPosition;

template<int Id, int Counter>
struct FindInputPosition<Id, Counter, std::tuple<>> {
  using type = NotFound;
};

template<int Id, int Counter, typename Head, typename ...Tail>
struct FindInputPosition<Id, Counter, std::tuple<Head, Tail...>> {
  using type = std::conditional_t<
      Head::value == Id,
      Int<Counter>,
      typename FindInputPosition<Id, Counter + 1, std::tuple<Tail...>>::type
  >;
};

template<typename Id, typename ...Nodes>
struct NodeInputArgsForMap {
  using type = typename NodeAtPack<Id::value, Nodes...>::type::Inputs;
};

template<typename IdsTuple, typename ...Nodes>
struct ArgsPackFor {
  template<typename Id>
  using nodeInputArgsForMap = NodeInputArgsForMap<Id, Nodes...>;

  using type = typename map<nodeInputArgsForMap, IdsTuple>::type;
//


};

template<int...Ids>
using Deps = std::tuple<Int<Ids>...>;


template<int...Ids>
using Inputs = Deps<Ids...>;

}

#endif //GRAPH_PROC_GRAPH_H

