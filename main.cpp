#include <iostream>

#include "src/Graph.h"
#include "src/Example.h"
#include "src/Test.h"
#include "src/Trait.h"
#include "src/Traversal.h"
#include "src/Executor.h"
#include "src/BFSLastRecentlyUsedGCPlan.h"
#include "src/Compose.h"


template<typename V>
struct testPred {
    constexpr static auto holds = (V::value == 0);
};


template<typename NodeId, typename InDegreeCount>
using InDegree = BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::InDegree<NodeId, InDegreeCount>;


int main() {
    {
        using indexes = std::make_integer_sequence<int, 1>;
        Id id;

        auto o = id.runPack({5}, indexes{});
        std::cout << o << std::endl;

    }

    {
        auto g = withNodes<IndexedNode<0, Id>>::andEdges{};
        auto o = g.execute<Inputs<0>, 0>({6});
        std::cout << o << std::endl;
    }

    {
        std::cout << "func helper test" << std::endl;

        using test = std::tuple<int, float>;
        using another = std::tuple<std::string, double>;

        static_assert(
                std::is_same_v<
                        concat<test>::with<another >::type,
                        std::tuple<int, float, std::string, double>>);

        static_assert(find<testPred, std::tuple<Int<0>>>::found);
    }

    {
        std::cout << "prerequisites test" << std::endl;

        using TestEdges = std::tuple<TestGCEdges>;

        static_assert(
                std::is_same_v<
                        addOrCreatePrerequisite<TestEdges, Edge< 3, std::tuple< Int<7> > >>::type,
                        std::tuple<
                                Edge<3, std::tuple<Int<7> > >,
                                Edge<2, std::tuple<Int<0>, Int<1> > >,
                                Edge<1, std::tuple<Int<0> > > >
                >);


        static_assert(
                std::is_same_v<
                        addOrCreatePrerequisite<TestEdges, Edge< 2, std::tuple< Int<7> > >>::type,

                        std::tuple<
                                Edge<2, std::tuple<Int<0>, Int<1>, Int<7> > >,
                                Edge<1, std::tuple<Int<0> > > >
                        >);

    }

    {
        std::cout << "BFSLastRecentlyUsedGCPlan test" << std::endl;

        using TestEdges = std::tuple<TestGCEdges>;



        static_assert(
                std::is_same_v<
                        addOrCreatePrerequisites<TestEdges,
                                std::tuple<
                                        Edge< 3, std::tuple< Int<7> > >,
                                        Edge< 2, std::tuple< Int<7> > >
                                >>::type,

                        std::tuple<
                                Edge<3, std::tuple<Int<7> > >,
                                Edge<2, std::tuple<Int<0>, Int<1>, Int<7> > >,
                                Edge<1, std::tuple<Int<0> > > >
                >);

    }

    {
        static_assert(
                std::is_same_v<
                        transpose<
                                std::tuple<
                                        Edge<1, std::tuple< Int<0> > >,
                                        Edge<2, std::tuple<Int<0>>  >
                                >
                        >::type,

                        std::tuple<Edge<0, std::tuple<Int<2>, Int<1> > > >
                >);


        static_assert(
                std::is_same_v<
                        transpose<
                                std::tuple<TestGCEdges>
                        >::type,


                        std::tuple<
                                Edge<1, std::tuple<Int<2> > >,
                                Edge<0, std::tuple<Int<1>, Int<2> > > >

                        >);

    }


    {

        using path = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::path ;

        static_assert(
                std::is_same_v<
                        path,
                        std::tuple<Int<0>, Int<1>, Int<2> >
                        >);

        using reversed_path = typename reverse<path>::type;

        static_assert(
                std::is_same_v<
                        reversed_path,
                        std::tuple<Int<2>, Int<1>, Int<0> >
                >);



        using luNode = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::lastUsedNode<0, reversed_path>::id;

        static_assert(
                std::is_same_v<
                        luNode,
                        Int<2>
                >);

        static_assert(
                std::is_same_v<
                        typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::gcMap,
                        std::tuple<
                                std::tuple<Int<2>, Int<0> >,
                                std::tuple<Int<2>, Int<1> >,
                                std::tuple<Int<2>, Int<2> > >

                >);



        static_assert(
                !BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::
                        lastUsedNode<0, std::tuple<Int<0>>>::nodeContainsVAsPrereq);


        using inDegrees = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::InDegrees ;

        static_assert(std::is_same_v<
                inDegrees,
                std::tuple<
                        InDegree<
                                Int<2>,
                                Int<2> >,
                        InDegree<
                                Int<1>,
                                Int<1> >  >>);


        using decreasedDegrees = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::DecreaseInDegrees<inDegrees,
                std::tuple<
                        Int<1>, Int<2>
                >
        >::type ;

        static_assert(std::is_same_v<
                decreasedDegrees,
                std::tuple<
                        InDegree<
                                Int<2>,
                                Int<1> >,
                        InDegree<
                                Int<1>,
                                Int<0> >  >>);


        static_assert(std::is_same_v<
                typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::filterNonZero<decreasedDegrees>::type,
                std::tuple<Int<1> >
        >);


        static_assert(std::is_same_v<
                typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges>::path,
                std::tuple<Int<0>, Int<1>, Int<2> >
        >);


    }

    {
        std::cout << "test GC" << std::endl;

        auto o2 = withNodes<TestGCNodes>::andEdges<TestGCEdges >{}.execute<
                std::tuple<Int<0>>, 2, BFSLastRecentlyUsedGCPlan
        >({{4}});

        std::cout << o2 << std::endl;
    }


    {
        std::cout << "node test" << std::endl;

        Summer summer;
        std::cout << value(summer.runPack({1, 2}, std::make_integer_sequence<int, 2>{})) << std::endl;
    }

    {
        std::cout << "multiple input test" << std::endl;

        auto output = withNodes<
                IndexedNode<0, Id>,
                IndexedNode<1, Id>,
                IndexedNode<2, Summer>
            >::andEdges<
                Edge<2, std::tuple<
                            Int<0>,
                            Int<1>
                        >>
            >{}.execute<
                    std::tuple< Int<0>, Int<1> >, 2, BFSLastRecentlyUsedGCPlan>({{5}, {1} });

        std::cout << output << std::endl;

    }
//
    {
        std::cout << "complex graph test" << std::endl;

        auto output =
                withNodes<
                        IndexedNode<0, AsyncTrait<CacheTrait<Summer>>>,
                        IndexedNode<1, Stringify>,
                        IndexedNode<2, AsyncTrait<Mul2>>,
                        IndexedNode<3, Dummy>>
                ::andEdges<
                        Edge<3, std::tuple<Int<2>, Int<1>>>,
                        Edge<1, std::tuple<Int<0>> >,
                        Edge<2, std::tuple<Int<0>> >

                        >{}.execute<Inputs<0>, 3, BFSLastRecentlyUsedGCPlan>({{1, 2}});



        std::cout << output << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestAsyncNodes>::andEdges<TestAsyncEdges >{}.topDown<std::tuple<Int<0>>, 3>({{4}});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "async test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestSyncNodes>::andEdges<TestSyncEdges >{}.topDown<std::tuple<Int<0>>, 3>({{4}});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "sync test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
        << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestNoCacheNodes>::andEdges<TestNoCacheEdges >{}.topDown<std::tuple<Int<0>>, 1>({{4}});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "nocache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestCacheNodes>::andEdges<TestCacheEdges >{}.topDown<std::tuple<Int<0>>, 1>({{4}});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << std::endl;
    }


    return 0;
}
