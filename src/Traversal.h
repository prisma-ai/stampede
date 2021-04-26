//
// Created by soultoxik on 08.04.2021.
//

#ifndef GRAPH_PROC_TRAVERSAL_H
#define GRAPH_PROC_TRAVERSAL_H

#include <tuple>
#include "Graph.h"
#include "FuncHelper.h"

struct Endl { };

template<int NodeIdx, int Counter, typename EdgesTuple>
struct PrerequisitesIdFor {
    static auto find() {
        if constexpr (Counter < std::tuple_size_v<EdgesTuple> ) {
            if constexpr(std::tuple_element_t<Counter, EdgesTuple>::source == NodeIdx) {
                return type_t<Int<Counter>>{};
            } else {
                return type_t<typename PrerequisitesIdFor<NodeIdx, Counter + 1, EdgesTuple>::type>{};
            }
        } else {
            return type_t<Endl> {};
        }
    }

    using type = typename decltype(find())::type;
};


template<typename EdgesTuple, typename NewEdge>
struct appendEdge {
    using type = typename append<NewEdge, EdgesTuple>::type;
};

template<typename Edge, typename PrereqsTuple>
struct appendPrerequisitesToEdge;

template<typename PrereqsTuple, int E, typename ...Prereqs>
struct appendPrerequisitesToEdge<Edge<E, std::tuple<Prereqs...>>, PrereqsTuple> {
    using type = Edge<E, typename concat<std::tuple<Prereqs...>>::template with<PrereqsTuple>::type>;
};


template<typename EdgesTuple, int who, typename IdsTuple>
struct appendPrerequisites;

template<int who, typename IdsTuple, typename ...Edges>
struct appendPrerequisites<std::tuple<Edges...>, who, IdsTuple> {
    using type = std::tuple<std::conditional_t<
            Edges::source == who,
            typename appendPrerequisitesToEdge<Edges, IdsTuple>::type,
            Edges
    >...>;
};

template<typename EdgesTuple, typename NewEdge>
struct addOrCreatePrerequisite {
    using found = typename PrerequisitesIdFor<NewEdge::source, 0, EdgesTuple>::type;

    using type = std::conditional_t<
            std::is_same_v<found, Endl>,
//
            typename appendEdge<EdgesTuple, NewEdge>::type,
//            float,
            typename appendPrerequisites<EdgesTuple, NewEdge::source, typename NewEdge::ids>::type
    >;
};

template<typename, typename>
struct addOrCreatePrerequisites;


template<typename EdgesTuple, typename NewEdge, typename ...NewEdges>
struct addOrCreatePrerequisites<EdgesTuple, std::tuple<NewEdge, NewEdges...>> {
    using type = typename addOrCreatePrerequisite<typename addOrCreatePrerequisites<EdgesTuple, std::tuple<NewEdges...>>::type, NewEdge>::type;
};

template<typename EdgesTuple, typename NewEdge>
struct addOrCreatePrerequisites<EdgesTuple, std::tuple<NewEdge>> {
    using type = typename addOrCreatePrerequisite<EdgesTuple, NewEdge>::type;
};

template <typename EdgesTuple>
struct transpose;


template <typename HEdge>
struct transpose<std::tuple<HEdge>> {
    template< typename, typename >
    struct inner;

    template<typename Collector, int V, typename ...Ids>
    struct inner<Collector, Edge<V, std::tuple<Ids...>>> {
        using type = typename addOrCreatePrerequisites<Collector,
                std::tuple<
                        Edge<Ids::value, std::tuple<Int<V>>>...
                        >
                >::type;
    };

    using type = typename inner< std::tuple<>, HEdge >::type;
};


template <typename HEdge, typename ...TEdges>
struct transpose<std::tuple<HEdge, TEdges...>> {

    template<typename, typename>
    struct inner;

    template<typename Collector, int V, typename ...Ids>
    struct inner<Collector, Edge<V, std::tuple<Ids...>>> {
        using type = typename addOrCreatePrerequisites<
                typename transpose< std::tuple<TEdges...> >::type,

                std::tuple<
                        Edge<
                          Ids::value,
                          std::tuple<Int<V>>
                          >...
                        >
                >::type;

    };

    using type = typename inner< std::tuple<>, HEdge >::type;

};



#endif //GRAPH_PROC_TRAVERSAL_H
