//
// Created by soultoxik on 08.04.2021.
//

#ifndef GRAPH_PROC_EXECUTOR_H
#define GRAPH_PROC_EXECUTOR_H

#include "Graph.h"
#include "spd/util/Traversal.h"
#include "spd/gc/GCPlan.h"

namespace spd {

template<typename NodesT, typename Applier, template<typename> typename Condition, int Id, typename TypesT>
struct BulkApplier;

template<typename NodesT, typename Applier, template<typename> typename Condition, int Id, typename Head, typename ...Tail>
struct BulkApplier<NodesT, Applier, Condition, Id, std::tuple<Head, Tail...>> {
  void apply(NodesT nodes) {
    if constexpr(Condition<std::tuple_element_t<Id, NodesT>>::value) {
      Applier{}.apply(&std::get<Id>(nodes));
    }

    BulkApplier<NodesT, Applier, Condition, Id + 1, std::tuple<Tail...>>{}.apply(nodes);
  }
};

template<typename NodesT, typename Applier, template<typename> typename Condition, int Id>
struct BulkApplier<NodesT, Applier, Condition, Id, std::tuple<>> {
  void apply(NodesT _) {

  }
};


template<typename...Nodes>
struct Context {
  using AllNodes = std::tuple<typename Nodes::type...>;
  AllNodes allNodes{};

  template<int Idx, typename TraitBaseType = std::tuple_element_t<Idx, AllNodes>>
  TraitBaseType *nodePtr() {
    return static_cast<TraitBaseType *>(&std::get<Idx>(allNodes));
  }


  template<typename Applier, template<typename> typename Condition>
  void bulkApply() {
    BulkApplier<AllNodes, Applier, Condition, 0, AllNodes>{}.apply(allNodes);
  }

};


// effectful map
template<typename Plan, typename Path, typename Applier, typename Input, typename Output, typename Ids>
struct SequenceMap {

  template<typename Context, int ...Ts>
  constexpr auto apply(Context &context, Input input, std::integer_sequence<int, Ts...>) -> Output {
    Applier applier;

    return {applier.template topDown<std::tuple_element_t<Ts, Ids>::value, Path, Plan>(context, input)...};
  }
};

// effectful map
template<int id, typename Plan>
struct GCPlanApplier;

template<int id, typename PHead, typename ...PTail>
struct GCPlanApplier<id, std::tuple<PHead, PTail...>> {
  template<typename Context>
  constexpr auto apply(Context &context) {
    if constexpr (id == PlanLast<PHead>::value) {
      constexpr auto depId = PlanDep<PHead>::value;
      (&std::get<depId>(context.allNodes))->gc();
    }
    GCPlanApplier<id, std::tuple<PTail...>>{}.template apply<Context>(context);
  }
};

template<int id>
struct GCPlanApplier<id, std::tuple<>> {
  template<typename Context>
  constexpr auto apply(Context &context) {}
};

// effectful map
template<int id, typename Plan>
struct CleanApplier;

template<int id, typename PHead, typename ...PTail>
struct CleanApplier<id, std::tuple<PHead, PTail...>> {
  template<typename Context>
  constexpr auto apply(Context &context) {
    if constexpr (id == PlanLast<PHead>::value) {
      constexpr auto depId = PlanDep<PHead>::value;
      (&std::get<depId>(context.allNodes))->dirty = false;
    }
    CleanApplier<id, std::tuple<PTail...>>{}.template apply<Context>(context);
  }
};

template<int id>
struct CleanApplier<id, std::tuple<>> {
  template<typename Context>
  constexpr auto apply(Context &context) {}
};

template<int id, typename Plan>
struct SomethingChangedBefore;

template<int id, typename PHead, typename ...PTail>
struct SomethingChangedBefore<id, std::tuple<PHead, PTail...>> {
  template<typename Context>
  constexpr auto apply(Context &context) {
    auto changed = false;
    if constexpr (id == PlanLast<PHead>::value) {
      constexpr auto depId = PlanDep<PHead>::value;
      changed = (&std::get<depId>(context.allNodes))->dirty;
    }
    return changed || SomethingChangedBefore<id, std::tuple<PTail...>>{}.template apply<Context>(context);
  }
};

template<int id>
struct SomethingChangedBefore<id, std::tuple<>> {
  template<typename Context>
  constexpr auto apply(Context &context) {
    return false;
  }
};

/**
 * Graph generator
 * Provide "IndexedNodes", "Edges" via variable typenames and compile via object creation
 * Also generates toposorted graph of links
 * @tparam Nodes
 * @tparam Edges
 */
template<typename...Nodes>
struct withNodes {

  template<typename TypesTuple>
  struct outputsFor;

  template<typename... Ts>
  struct outputsFor<std::tuple<Ts...>> {
    using type = std::tuple<typename NodeAtPack<Ts::value, Nodes...>::type::Output ...>;
  };

  template<typename...Edges>
  struct andEdges {
    /**
     * Executes compiled graph
     * Also generates default context
     *
     * @tparam SourcesIds -- input nodes ids
     * @tparam DestId -- output node id
     * @tparam GCPlan
     * @param args -- tuple of inputs
     * @return
     */
    template<typename SourcesIds, int DestId, typename GCPlan = NoPlan>
    typename NodeAtPack<DestId, Nodes...>::type::Output
    execute(typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
      Context<Nodes...> context;
      return execute<SourcesIds, DestId, GCPlan>(context, args);
    };

    /**
     * Executes compiled graph with provided context
     * See method before
     *
     * @tparam SourcesIds
     * @tparam DestId
     * @tparam GCPlan
     * @param context
     * @param args
     * @return
     */
    template<typename SourcesIds, int DestId, typename GCPlan = NoPlan>
    typename NodeAtPack<DestId, Nodes...>::type::Output
    execute(Context<Nodes...> &context, typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
      using path = typename BFSLastRecentlyUsedGCPlanImpl<SourcesIds, Edges...>::gcMap;

      return topDown<SourcesIds, DestId, path, GCPlan>(context, args);
    };

    /**
     * Generates execution context thats holds all runtime info / state
     * @return
     */
    auto createContext() {
      return Context<Nodes...>{};
    }

   private:
    template<typename SourcesIds, int DestId, typename Path, typename Plan = NoPlan>
    typename NodeAtPack<DestId, Nodes...>::type::Output
    topDown(typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
      Context<Nodes...> context;
      return topDown<SourcesIds, DestId, Path, Plan>(context, args);
    }

    /**
     * Inner executor
     * Performs top down deduction via simple recursion
     *
     * @tparam SourcesIds
     * @tparam DestId
     * @tparam Path
     * @tparam Plan
     * @param context
     * @param args
     * @return
     */
    template<typename SourcesIds, int DestId, typename Path, typename Plan = NoPlan>
    typename NodeAtPack<DestId, Nodes...>::type::Output
    topDown(Context<Nodes...> &context, typename ArgsPackFor<SourcesIds, Nodes...>::type args) {

      using Dest = typename NodeAtPack<DestId, Nodes...>::type;


      auto dest = &std::get<DestId>(context.allNodes);

      using sourceInDest = typename FindInputPosition<DestId, 0, SourcesIds>::type;

      if constexpr(!std::is_same_v<sourceInDest, NotFound>) {
        using indexes = std::make_integer_sequence<int, std::tuple_size_v<
            std::tuple_element_t<sourceInDest::value, typename ArgsPackFor<SourcesIds, Nodes...>::type>
        >>;


        return dest->runPack(std::get<sourceInDest::value>(args), indexes{});
      } else {
        using Prerequisites = typename PrerequisitesForPack<DestId, Edges...>::type;
        using PrerequisitesOutputs = typename withNodes<Nodes...>::template outputsFor<Prerequisites>::type;

        auto indexes = std::make_integer_sequence<int, std::tuple_size_v<typename Dest::Inputs>>{};

        auto inputsTemp = SequenceMap<
            Plan,
            Path,
            withNodes<Nodes...>::andEdges<Edges...>::fixedInput<SourcesIds>,
            typename ArgsPackFor<SourcesIds, Nodes...>::type,
            PrerequisitesOutputs,
            Prerequisites>{}.apply(context, args, indexes);

        auto output = dest->sequencePoint(inputsTemp, SomethingChangedBefore<DestId, Path>{}.apply(context));

        // GC:
        if constexpr (!std::is_same_v<Plan, NoPlan>) {
          GCPlanApplier<DestId, Path>{}.apply(context);
        }

        CleanApplier<DestId, Path>{}.apply(context);

        return output;

      }
    }

    template<typename SourcesIds>
    struct fixedInput {
      template<int DestId, typename Path, typename Plan>
      typename NodeAtPack<DestId, Nodes...>::type::Output
      topDown(Context<Nodes...> &context, typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
        return withNodes<Nodes...>::andEdges<Edges...>{}.topDown<SourcesIds, DestId, Path, Plan>(context, args);
      }
    };

  };
};

}

#endif //GRAPH_PROC_EXECUTOR_H
