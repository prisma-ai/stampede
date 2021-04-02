
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

constexpr static auto CTOR_DTOR_LOG = false;


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
        if constexpr (CTOR_DTOR_LOG) {
            std::cout << tag_ << " created" << std::endl;
        }
    }

    ~Node() {
        if constexpr (CTOR_DTOR_LOG) {
            std::cout << tag_ << " destroyed" << std::endl;
        }
    }


    OutputT runPack(std::tuple<InputsT...> args) {
        return static_cast<Impl<OutputT, InputsT...> *>(this)->runImpl(args);
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
    using type = std::conditional_t<Head::source == NodeIdx, typename Head::ids, void>;
};


template<typename...Nodes>
struct Context {
    using AllNodes = std::tuple<typename Nodes::type...>;
    AllNodes allNodes {};
};

template<typename Applier, typename Input, typename Output, typename Ids>
struct SequenceMap {

    template<typename Context, int ...Ts>
    auto apply(Context& context, Input input, std::integer_sequence<int, Ts...>) -> Output {
        Applier applier;

        return {applier.template topDown<std::tuple_element_t<Ts, Ids>::value>(context, input)...};
    }
};




template<typename ...Nodes>
struct extractWith {
    template<typename TypesTuple>
    struct outputsFor;

    template<typename... Ts>
    struct outputsFor<std::tuple<Ts...>> {
        using type = std::tuple<typename NodeAtPack<Ts::value ,Nodes...>::type::Output ...>;
    };
};





template<typename...Nodes>
struct withNodes {
    template<typename...Edges>
    struct andEdges {
        template<int SourceId>
        struct fixedInput;


        template<int SourceId, int DestId>
        typename NodeAtPack<DestId, Nodes...>::type::Output
        topDown(typename NodeAtPack<SourceId, Nodes...>::type::Inputs args) {
            Context<Nodes...> context;
            return topDown<SourceId, DestId>(context, args);
        }

        template<int SourceId, int DestId>
        typename NodeAtPack<DestId, Nodes...>::type::Output
        topDown(Context<Nodes...> &context, typename NodeAtPack<SourceId, Nodes...>::type::Inputs args) {
            using Source = typename NodeAtPack<SourceId, Nodes...>::type;
            using Dest = typename NodeAtPack<DestId, Nodes...>::type;


            auto dest = &std::get<DestId>(context.allNodes);

            if constexpr(SourceId == DestId) {
                return dest->runPack(args);
            } else {
                using Prerequisites = typename PrerequisitesForPack<DestId, Edges...>::type;
                using PrerequisitesOutputs = typename extractWith<Nodes...>::template outputsFor<Prerequisites >::type ;

                auto indexes = std::make_integer_sequence<int, std::tuple_size_v<typename Dest::Inputs>>{};

                auto inputsTemp = SequenceMap<
                        withNodes<Nodes...>::andEdges<Edges...>::fixedInput<SourceId>,
                        typename NodeAtPack<SourceId, Nodes...>::type::Inputs,
                        PrerequisitesOutputs ,
                        Prerequisites>{}.apply(context, args, indexes);


                // sequence point
                auto values = std::apply([](auto ...x) -> typename Dest::Inputs { return std::make_tuple(value(x)...); }, inputsTemp);

                return dest->runPack(values);

            }
        }

        template<int SourceId>
        struct fixedInput {
            template<int DestId>
            typename NodeAtPack<DestId, Nodes...>::type::Output
            topDown(Context<Nodes...> &context, typename NodeAtPack<SourceId, Nodes...>::type::Inputs args) {
                return withNodes<Nodes...>::andEdges<Edges...>{}.topDown<SourceId, DestId>(context, args);
            }
        };

    };
};

#endif //GRAPH_PROC_GRAPH_H

