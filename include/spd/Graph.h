
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
 public:  \
  name() : spd::Node<name, param, out, __VA_ARGS__>\
                                   (#name) {}      \
                                            \
  out runImpl(__VA_ARGS__);                 \
                                            \
};

#define declare_tnode(name, param, out, in) \
class name : public spd::Node<name, param, out, in> { \
 public:  \
  name() : spd::Node<name, param, out, in>\
                                   (#name) {}      \
                                            \
  out runImpl(in);                 \
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
template<typename Impl, typename ParamType, typename OutputT, typename InputsT>
struct TNode : NodeBase<ParamType> {
  using Output = OutputT;

  using Inputs = InputsT;
  using Param = ParamType;


  explicit TNode(const std::string &tag = "node") {
    tag_ = tag;
    if constexpr (BASE_GRAPH_CALLS_LOG) {
      std::cout << tag_ << " created" << std::endl;
    }
  }

  ~TNode() {
    if constexpr (BASE_GRAPH_CALLS_LOG) {
      std::cout << tag_ << " destroyed" << std::endl;
    }
  }

  template<int ...N>
  OutputT runPack(Inputs args, std::integer_sequence<int, N...>) {
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

template<typename Impl, typename ParamType, typename OutputT, typename ...InputsT>
using Node = TNode<Impl, ParamType, OutputT, std::tuple<InputsT...>>;

template<typename WrappedGraph, typename SourcesIds, int DestId>
struct GraphNode {
  template<typename SourceId>
  struct InputMap {
    using type = typename NodeAt<SourceId::value, typename WrappedGraph::GNodes>::type::Inputs;
  };

  template<typename Node>
  struct ParamMap {
    using type = typename Node::type::Param;
  };



  using GNInputs = typename map<InputMap, SourcesIds>::type;
  using GNOutput = typename NodeAt<DestId, typename WrappedGraph::GNodes>::type::Output;
  using GNState = typename map<ParamMap, typename WrappedGraph::GNodes>::type;


  template<typename Context, typename ContextParamsT>
  struct RefParam {
    Context* context;


    template<std::size_t I = 0, typename ...Ts>
    inline typename std::enable_if<I == sizeof...(Ts), void>::type
    assign(std::tuple<Ts...>) { }


    template<std::size_t I = 0, typename ...Ts>
    inline typename std::enable_if<I < sizeof...(Ts), void>::type
    assign(std::tuple<Ts...> ts) {
      context->template nodePtr<I>()->config = std::get<I>(ts);
      assign<I + 1>(ts);
    }

    RefParam& operator=(ContextParamsT other) {
      assign(other);
      return *this;
    }

    template<int...N>
    bool equals(RefParam<Context, ContextParamsT> other, std::integer_sequence<int, N...>) {
      return ((context->template nodePtr<N>()->config == other.context.template nodePtr<N>()->config) && ...);
    }

    bool operator==(RefParam<Context, ContextParamsT> other) {
      return equals(other, std::make_integer_sequence<int, std::tuple_size_v<ContextParamsT>>{});
    }

  };

  struct T : TNode<T, RefParam<typename WrappedGraph::GContext, GNState>, GNOutput, GNInputs > {
    WrappedGraph graph {};
    typename WrappedGraph::GContext context {};

    T(): TNode<T, RefParam<typename WrappedGraph::GContext, GNState>, GNOutput, GNInputs>("graph") {
      this->config.context = &context;
    }

    GNOutput runImpl(GNInputs inputs) {
      return graph.template execute<SourcesIds, DestId>(context, inputs);
    }

    template<std::size_t I = 0, std::size_t N>
    inline typename std::enable_if<I == N, void>::type
    innerGC() { }


    template<std::size_t I = 0, std::size_t N>
    inline typename std::enable_if<I < N, void>::type
    innerGC() {
      context->template nodePtr<I>()->gc();
      innerGC<I + 1>();
    }


    void gc() {
      innerGC();

//      XXX: tbd
    }
  };
};


}

#endif //GRAPH_PROC_GRAPH_H

