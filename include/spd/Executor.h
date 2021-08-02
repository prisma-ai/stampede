//
// Created by soultoxik on 08.04.2021.
//

#ifndef GRAPH_PROC_EXECUTOR_H
#define GRAPH_PROC_EXECUTOR_H

#include "Graph.h"
#include "spd/util/Traversal.h"
#include "spd/gc/GCPlan.h"

namespace spd {

template<typename...Nodes>
struct Context {
  using AllNodes = std::tuple<typename Nodes::type...>;
  AllNodes allNodes{};

  template<int Idx, typename TraitBaseType = std::tuple_element_t<Idx, AllNodes>>
  TraitBaseType *nodePtr() {
    return static_cast<TraitBaseType *>(&std::get<Idx>(allNodes));
  }
};

// effectful map
template<typename Plan, typename Applier, typename Input, typename Output, typename Ids>
struct SequenceMap {

  template<typename Context, int ...Ts>
  constexpr auto apply(Context &context, Input input, std::integer_sequence<int, Ts...>) -> Output {
    Applier applier;

    return {applier.template topDown<std::tuple_element_t<Ts, Ids>::value, Plan>(context, input)...};
  }
};

// effectful map
template<int id, typename Plan>
struct planApplier;

template<int id, typename PHead, typename ...PTail>
struct planApplier<id, std::tuple<PHead, PTail...>> {
  template<typename Context>
  constexpr auto apply(Context &context) {
    if constexpr (id == PlanLast<PHead>::value) {
      constexpr auto depId = PlanDep<PHead>::value;
      (&std::get<depId>(context.allNodes))->gc();
    }
    planApplier<id, std::tuple<PTail...>>{}.template apply<Context>(context);
  }
};

template<int id>
struct planApplier<id, std::tuple<>> {
  template<typename Context>
  constexpr auto apply(Context &context) {}
};

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

    template<typename SourcesIds, int DestId, typename GCPlan = NoPlan>
    typename NodeAtPack<DestId, Nodes...>::type::Output
    execute(typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
      Context<Nodes...> context;
      return execute<SourcesIds, DestId, GCPlan>(context, args);
    };

    template<typename SourcesIds, int DestId, typename GCPlan = NoPlan>
    typename NodeAtPack<DestId, Nodes...>::type::Output
    execute(Context<Nodes...> &context, typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
      if constexpr (std::is_same_v<GCPlan, BFSLastRecentlyUsedGCPlan>) {
        using gcPlan = typename BFSLastRecentlyUsedGCPlanImpl<SourcesIds, Edges...>::gcMap;
        return topDown<SourcesIds, DestId, gcPlan>(context, args);
      } else {
        return topDown<SourcesIds, DestId, NoPlan>(context, args);
      }
    };

    auto createContext() {
      return Context<Nodes...>{};
    }

    template<typename SourcesIds, int DestId, typename Plan = NoPlan>
    typename NodeAtPack<DestId, Nodes...>::type::Output
    topDown(typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
      Context<Nodes...> context;
      return topDown<SourcesIds, DestId, Plan>(context, args);
    }

    template<typename SourcesIds, int DestId, typename Plan = NoPlan>
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

        constexpr auto SourceId = std::tuple_element_t<0, SourcesIds>::value;

        using Prerequisites = typename PrerequisitesForPack<DestId, Edges...>::type;
        using PrerequisitesOutputs = typename withNodes<Nodes...>::template outputsFor<Prerequisites>::type;

        auto indexes = std::make_integer_sequence<int, std::tuple_size_v<typename Dest::Inputs>>{};

        auto inputsTemp = SequenceMap<
            Plan,
            withNodes<Nodes...>::andEdges<Edges...>::fixedInput<SourcesIds>,
            typename ArgsPackFor<SourcesIds, Nodes...>::type,
            PrerequisitesOutputs,
            Prerequisites>{}.apply(context, args, indexes);


        // sequence point
        auto values = std::apply(
            [](auto ...x) -> typename Dest::Inputs { return std::make_tuple(value(x)...); }, inputsTemp);

        auto output = dest->runPack(values,
                                    std::make_integer_sequence<int, std::tuple_size_v<typename Dest::Inputs>>{});

        // GC:
        if constexpr (!std::is_same_v<Plan, NoPlan>) {
          planApplier<DestId, Plan>{}.apply(context);
        }

        return output;

      }
    }

    template<typename SourcesIds>
    struct fixedInput {
      template<int DestId, typename Plan>
      typename NodeAtPack<DestId, Nodes...>::type::Output
      topDown(Context<Nodes...> &context, typename ArgsPackFor<SourcesIds, Nodes...>::type args) {
        return withNodes<Nodes...>::andEdges<Edges...>{}.topDown<SourcesIds, DestId, Plan>(context, args);
      }
    };

  };
};

}

#endif //GRAPH_PROC_EXECUTOR_H
