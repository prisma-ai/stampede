//
// Created by soultoxik on 19.04.2021.
//

#ifndef GRAPH_PROC_BFSLASTRECENTLYUSEDGCPLAN_H
#define GRAPH_PROC_BFSLASTRECENTLYUSEDGCPLAN_H

#include "spd/util/Traversal.h"
#include "spd/util/FuncHelper.h"

namespace spd {

struct BFSLastRecentlyUsedGCPlan {};

/**
 * Forms a map like:
 *  std::tuple<Children>
 * where Child:
 *  std::tuple<NodeId, ChildId>
 * via BFS toposort
 *
 * This used not only for GC so XXX: rename
 * Also use this map anytime when you need some child dependencies such as cache checking
 *
 * @tparam FromTuple
 * @tparam Edges
 *
 * @field gcMap -- toposorted map
 */
template<typename FromTuple, typename ...Edges>
struct BFSLastRecentlyUsedGCPlanImpl {

  template<typename NodeId, typename InDegreeCount>
  struct InDegree {
    using nodeid_t = NodeId;
    constexpr static auto nodeid_v = NodeId::value;

    using count_t = InDegreeCount;
    constexpr static auto count_v = InDegreeCount::value;

  };


  using TEdges = std::tuple<Edges...>;
  using Transposed = typename transpose<TEdges>::type;


  using InDegrees = std::tuple<
      InDegree<Int < Edges::source>, Int < std::tuple_size_v<typename Edges::ids>>>...
  >;

  template<int source, typename ID>
  struct FindAndDecrease {
    using type = std::conditional_t<
        source == ID::nodeid_v,
        InDegree<typename ID::nodeid_t, Int < ID::count_v - 1>>,
    ID>;
  };

  template<typename NewIndegrees, typename PrereqsTuple>
  struct DecreaseInDegrees;

  template<typename NewIndegrees, typename PHead, typename ...PTail>
  struct DecreaseInDegrees<NewIndegrees, std::tuple<PHead, PTail...>> {
    template<typename ID>
    using findAndDecreaseMap = FindAndDecrease<PHead::value, ID>;
    using type = typename map<findAndDecreaseMap, typename DecreaseInDegrees<
        NewIndegrees,
        std::tuple<PTail...> >::type>::type;

  };

  template<typename NewIndegrees>
  struct DecreaseInDegrees<NewIndegrees, std::tuple<>> {
    using type = NewIndegrees;
  };

//    const foldl
  template<typename InDegreesTuple>
  struct filterNonZero;

  template<typename IDHead, typename ...IDTail>
  struct filterNonZero<std::tuple<IDHead, IDTail...>> {
    using type = typename concat<
        std::conditional_t<
            IDHead::count_v == 0,
            std::tuple<typename IDHead::nodeid_t>,
            std::tuple<>

        >
    >::template with<
        typename filterNonZero<std::tuple<IDTail...>>::type
    >::type;

  };

  template<typename ID>
  struct filterNonZero<std::tuple<ID>> {
    using type = std::conditional_t<
        ID::count_v == 0,
        std::tuple<typename ID::nodeid_t>,
        std::tuple<>
    >;
  };

  template<typename Neighbours, typename V>
  struct NeighboursPredicate {
    template<typename NeighboursTuple>
    struct inner;

    template<typename NHead, typename ...NTail>
    struct inner<std::tuple<NHead, NTail...>> {
      constexpr static auto has = (NHead::value == V::value) || inner<std::tuple<NTail...>>::has;
    };

    template<>
    struct inner<std::tuple<>> {
      constexpr static auto has = false;
    };

    constexpr static auto holds = inner<Neighbours>::has;
  };

  template<typename Path, typename InDegreesInfo, typename IntQueue>
  struct Kahns;

  template<typename Path, typename InDegreesInfo>
  struct Kahns<Path, InDegreesInfo, std::tuple<>> {
    using type = Path;
  };

  template<typename Path, typename InDegreesInfo, typename IQHead, typename ...IQTail>
  struct Kahns<Path, InDegreesInfo, std::tuple<IQHead, IQTail...>> {
    using path = typename concat<Path>::template with<std::tuple<Int < IQHead::value>>>::type;
    using neighbours = typename PrerequisitesForTuple<IQHead::value, Transposed>::type;

    using newInDegrees = typename DecreaseInDegrees<InDegreesInfo, neighbours>::type;

    using newHead = typename filterNonZero<newInDegrees>::type;

    template<typename V>
    using neighboursPredicate = NeighboursPredicate<neighbours, V>;

    using localHead = typename filter<neighboursPredicate, newHead>::type;

    using newQueue = typename concat<localHead>::template with<std::tuple<IQTail...>>::type;

    using type = typename Kahns<path, newInDegrees, newQueue>::type;
  };

  using path = typename Kahns<std::tuple<>, InDegrees, FromTuple>::type;


//    const foldl
  template<int, typename>
  struct lastUsedNode;

  template<int V, typename PHead, typename ...PTail>
  struct lastUsedNode<V, std::tuple<PHead, PTail...>> {

    template<typename WV>
    struct findPredicate {
      constexpr static auto holds = (WV::value == V);
    };

    using nodePrereqs = typename PrerequisitesForPack<PHead::value, Edges...>::type;

    constexpr static auto nodeContainsVAsPrereq = find<findPredicate, nodePrereqs>::found;


    using id = std::conditional_t<
        V == PHead::value,
        Int < V>,
    std::conditional_t<
        nodeContainsVAsPrereq,
        PHead,
        typename lastUsedNode<V, std::tuple<PTail...>>::id
    >
    >;
  };

  struct error_invalid_path;

  template<int V>
  struct lastUsedNode<V, std::tuple<>> {
    using id = error_invalid_path;
  };

//    const map
  template<typename RPath, typename Path>
  struct GCMap;

  template<typename RPath, typename PHead, typename ...PTail>
  struct GCMap<RPath, std::tuple<PHead, PTail...>> {
    using lastUsed = typename lastUsedNode<PHead::value, RPath>::id;
    using newMap = std::tuple<lastUsed, Int < PHead::value>>;
    using tail = typename GCMap<RPath, std::tuple<PTail...>>::type;
    using type = typename concat<std::tuple<newMap>>::template with<tail>::type;
  };

  template<typename RPath>
  struct GCMap<RPath, std::tuple<>> {
    using type = std::tuple<>;
  };

  using gcMap = typename GCMap<typename reverse<path>::type, path>::type;
};

template<typename PlanItem>
using PlanLast = std::tuple_element_t<0, PlanItem>;

template<typename PlanItem>
using PlanDep = std::tuple_element_t<1, PlanItem>;

}

#endif //GRAPH_PROC_BFSLASTRECENTLYUSEDGCPLAN_H
