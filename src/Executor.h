//
// Created by soultoxik on 08.04.2021.
//

#ifndef GRAPH_PROC_EXECUTOR_H
#define GRAPH_PROC_EXECUTOR_H

#include "Graph.h"
#include "Traversal.h"
#include "GCPlan.h"


template<typename...Nodes>
struct Context {
    using AllNodes = std::tuple<typename Nodes::type...>;
    AllNodes allNodes{};
};

// effectfull map
template<typename Plan, typename Applier, typename Input, typename Output, typename Ids>
struct SequenceMap {

    template<typename Context, int ...Ts>
    constexpr auto apply(Context &context, Input input, std::integer_sequence<int, Ts...>) -> Output {
        Applier applier;

        return { applier.template topDown<std::tuple_element_t<Ts, Ids>::value, Plan>(context, input)... };
    }
};


// effectfull map
template<int id, typename Plan>
struct planApplier;

template<int id, typename PHead, typename ...PTail>
struct planApplier<id, std::tuple<PHead, PTail...>> {
    template <typename Context>
    constexpr auto apply(Context &context) {
        if constexpr (id == PlanLast<PHead>::value) {
            constexpr auto depId = PlanDep<PHead>::value;
            (&std::get<depId>(context.allNodes))->gc();
        }
        planApplier<id, std::tuple<PTail...>> {}.template apply<Context>(context);
    }
};

template<int id>
struct planApplier<id, std::tuple<>> {
    template <typename Context>
    constexpr auto apply(Context &context) { }
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


        template<int SourceId>
        struct fixedInput;

        template <int SourceId, int DestId, typename GCPlan = NoPlan>
        typename NodeAtPack<DestId, Nodes...>::type::Output execute(typename NodeAtPack<SourceId, Nodes...>::type::Inputs args) {
            if constexpr (std::is_same_v<GCPlan, BFSLastRecentlyUsedGCPlan>) {
                using gcPlan = typename BFSLastRecentlyUsedGCPlanImpl<SourceId, Edges...>::gcMap ;
                return topDown<SourceId, DestId, gcPlan>(args);
            } else {
                return topDown<SourceId, DestId, NoPlan>(args);
            }
        };


        template<int SourceId, int DestId, typename Plan = NoPlan>
        typename NodeAtPack<DestId, Nodes...>::type::Output
        topDown(typename NodeAtPack<SourceId, Nodes...>::type::Inputs args) {
            Context<Nodes...> context;
            return topDown<SourceId, DestId, Plan>(context, args);
        }

        template<int SourceId, int DestId, typename Plan = NoPlan>
        typename NodeAtPack<DestId, Nodes...>::type::Output
        topDown(Context<Nodes...> &context, typename NodeAtPack<SourceId, Nodes...>::type::Inputs args) {
            using Source = typename NodeAtPack<SourceId, Nodes...>::type;
            using Dest = typename NodeAtPack<DestId, Nodes...>::type;


            auto dest = &std::get<DestId>(context.allNodes);
//            auto dest = Dest{};

            if constexpr(SourceId == DestId) {
                return dest->runPack(args);
            } else {
                using Prerequisites = typename PrerequisitesForPack<DestId, Edges...>::type;
                using PrerequisitesOutputs = typename withNodes<Nodes...>::template outputsFor<Prerequisites>::type;

                auto indexes = std::make_integer_sequence<int, std::tuple_size_v<typename Dest::Inputs>>{};

                auto inputsTemp = SequenceMap<
                        Plan,
                        withNodes<Nodes...>::andEdges<Edges...>::fixedInput<SourceId>,
                        typename NodeAtPack<SourceId, Nodes...>::type::Inputs,
                        PrerequisitesOutputs,
                        Prerequisites>{}.apply(context, args, indexes);


                // sequence point
                auto values = std::apply(
                        [](auto ...x) -> typename Dest::Inputs { return std::make_tuple(value(x)...); }, inputsTemp);


                auto output = dest->runPack(values);

                // GC:
                if constexpr (!std::is_same_v<Plan, NoPlan>) {
                    planApplier<DestId, Plan>{}.apply(context);
                }



                return output;

            }
        }

        template<int SourceId>
        struct fixedInput {
            template<int DestId, typename Plan>
            typename NodeAtPack<DestId, Nodes...>::type::Output
            topDown(Context<Nodes...> &context, typename NodeAtPack<SourceId, Nodes...>::type::Inputs args) {
                return withNodes<Nodes...>::andEdges<Edges...>{}.topDown<SourceId, DestId, Plan>(context, args);
            }
        };

    };
};


#endif //GRAPH_PROC_EXECUTOR_H
