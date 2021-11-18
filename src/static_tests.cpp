#include <iostream>

#include <spd/Graph.h>
#include "Example.h"
#include <spd/Trait.h>
#include <spd/util/Traversal.h>
#include <spd/Executor.h>
#include <spd/gc/BFSLastRecentlyUsedGCPlan.h>
#include <spd/util/Compose.h>
#include <spd/async/Pool.h>

using namespace spd;

template<typename V>
struct testPred {
  constexpr static auto holds = (V::value == 0);
};

struct HasCache {
  bool dirty;
};

struct HasntCache {

};
//

template<typename NodeId, typename InDegreeCount>
using InDegree = BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::InDegree<NodeId, InDegreeCount>;


declare_node(TestNode, Unit, int, int)
int TestNode::runImpl(int arg) {
  return arg;
}

/**
 * This execution unit contains all compile-time tests
 * @return
 */
int main() {
  {
    auto params = std::make_tuple(1, 2);
    auto otherParams = std::make_tuple(1, 2);
    std::cout << (params == otherParams) << std::endl;
  }

  {
    auto graph = withNodes<
        IndexedNode<0, ConfigurableCacheTrait<LongOp1>>, IndexedNode<1, Summer>
    >::andEdges<
        Edge<1, Deps<0, 0>>
    >{};

    auto context = graph.createContext();

    {
      auto t0 = std::chrono::system_clock::now();

      auto output = graph.execute<Inputs<0>, 1>(context, {{4}});

      auto t1 = std::chrono::system_clock::now();

      std::cout << "first configurable cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                << std::endl;
    }


    {
      auto t0 = std::chrono::system_clock::now();

      auto output = graph.execute<Inputs<0>, 1>(context, {{4}});

      auto t1 = std::chrono::system_clock::now();

      std::cout << "second configurable cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                << std::endl;
    }


    {
      auto t0 = std::chrono::system_clock::now();

      context.nodePtr<0>()->config = 4.f;


      auto output = graph.execute<Inputs<0>, 1>(context, {{4}});

      auto t1 = std::chrono::system_clock::now();

      std::cout << "third configurable cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                << std::endl;
    }


  }

  {

    auto graph = withNodes<
        IndexedNode<0, CacheTrait<LongOp1>>, IndexedNode<1, CacheTrait<LongSummer>> >::andEdges<TestCacheEdges>{};
    auto context = graph.createContext();

    {
      auto t0 = std::chrono::system_clock::now();

      auto output = graph.execute<std::tuple<Int<0>>, 1>(context, {{4}});

      auto t1 = std::chrono::system_clock::now();

      std::cout << "lazy cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                << std::endl;
    }

    {
      context.nodePtr<0>()->dirty = true;

      auto t0 = std::chrono::system_clock::now();

      auto output = graph.execute<std::tuple<Int<0>>, 1>(context, {{4}});


      auto t1 = std::chrono::system_clock::now();

      std::cout << "lazy cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                << std::endl;
    }
  }

  {
    auto graph = withNodes<TestManyCacheNodes >::andEdges<TestManyCacheEdges >{};
    auto context = graph.createContext();

    std::cout << hasCachePredicate<CacheTrait<LongOp1>>::value << std::endl;

    std::cout << hasCachePredicate<
      std::tuple_element_t<0, typename decltype(context)::AllNodes>
    >::value << std::endl;


    struct NoKeeper {
      void apply(CacheTraitBase* cacheBase) {
        std::cout << "no keep for " << cacheBase << std::endl;
        cacheBase->keep = []() {
          return false;
        };
      }
    };

    context.bulkApply<NoKeeper, hasCachePredicate>();


    auto t0 = std::chrono::system_clock::now();

    graph.execute<
        std::tuple<Int<0>, Int<1>>, 2>(context, {{4}, {5}});



    auto t1 = std::chrono::system_clock::now();

    std::cout << "many caches test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;
  }

  {
    if constexpr(has_cache<HasCache>) {
      std::cout << "hasCache passed positive" << std::endl;
    }

    if constexpr(!has_cache<HasntCache>) {
      std::cout << "hasCache passed negative" << std::endl;
    }

    using allCache = typename all<hasCachePredicate, std::tuple<HasCache, HasCache>>::type;
    using notAllCache = typename all<hasCachePredicate, std::tuple<HasCache, HasntCache>>::type;

    if constexpr(allCache::value) {
      std::cout << "all hasCache passed positive" << std::endl;
    }


    if constexpr(!notAllCache::value) {
      std::cout << "all hasCache passed negative" << std::endl;
    }
  }

  {
    auto pool = Pool<>();
    pool.start(4);
    int test = 7;

    std::function<int(void)> f = [&test]() {
      std::cout << "start async section" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      std::cout << "end async section" << std::endl;

      return test;
    };

    auto future = pool.enqueue(f);
    future.wait();
    std::cout << "pool test " << future.get() << std::endl;
  }

  {
    auto t0 = std::chrono::system_clock::now();

    auto pool = std::make_shared<Pool<>>();
    pool->start(4);

    auto graph = withNodes<TestAsyncPoolNodes >::andEdges<TestAsyncPoolEdges >{};
    auto context = graph.createContext();
    context.nodePtr<1>()->pool = pool;
    context.nodePtr<2>()->pool =pool;


    auto output = graph.execute<std::tuple<Int<0>>, 3>(context, {{4}});

    auto t1 = std::chrono::system_clock::now();

    std::cout << "async pool test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;

    pool.reset();
  }



  {
    using indexes = std::make_integer_sequence<int, 1>;
    Id id;

    auto o = id.runPack({5}, indexes{});
    std::cout << o << std::endl;

  }

  {
//    XXX: edge case for current toposort impl
//    auto g = withNodes<IndexedNode<0, Id>>::andEdges{};
//    auto o = g.execute<Inputs<0>, 0>({6});
//    std::cout << o << std::endl;
  }

  {
    std::cout << "func helper test" << std::endl;

    using test = std::tuple<int, float>;
    using another = std::tuple<std::string, double>;

    static_assert(
        std::is_same_v<
            concat<test>::with<another>::type,
            std::tuple<int, float, std::string, double>>);

    static_assert(find<testPred, std::tuple<Int<0>>>::found);
  }

  {
    std::cout << "prerequisites test" << std::endl;

    using TestEdges = std::tuple<TestGCEdges >;

    static_assert(
        std::is_same_v<
            addOrCreatePrerequisite<TestEdges, Edge<3, std::tuple<Int<7> > >>::type,
            std::tuple<
                Edge<3, std::tuple<Int<7> > >,
                Edge<2, std::tuple<Int<0>, Int<1> > >,
                Edge<1, std::tuple<Int<0> > > >
        >);


    static_assert(
        std::is_same_v<
            addOrCreatePrerequisite<TestEdges, Edge<2, std::tuple<Int<7> > >>::type,

            std::tuple<
                Edge<2, std::tuple<Int<0>, Int<1>, Int<7> > >,
                Edge<1, std::tuple<Int<0> > > >
        >);

  }

  {
    std::cout << "BFSLastRecentlyUsedGCPlan test" << std::endl;

    using TestEdges = std::tuple<TestGCEdges >;



    static_assert(
        std::is_same_v<
            addOrCreatePrerequisites<TestEdges,
                                     std::tuple<
                                         Edge<3, std::tuple<Int<7> > >,
                                         Edge<2, std::tuple<Int<7> > >
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
                    Edge<1, std::tuple<Int<0> > >,
                    Edge<2, std::tuple<Int<0>>>
                >
            >::type,

            std::tuple<Edge<0, std::tuple<Int<2>, Int<1> > > >
        >);


    static_assert(
        std::is_same_v<
            transpose<
                std::tuple<TestGCEdges >
            >::type,

            std::tuple<
                Edge<1, std::tuple<Int<2> > >,
                Edge<0, std::tuple<Int<1>, Int<2> > > >

        >);

  }

  {

    using path = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::path;

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



    using luNode = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::lastUsedNode<0,
                                                                                                          reversed_path>::id;

    static_assert(
        std::is_same_v<
            luNode,
            Int<2>
        >);

    static_assert(
        std::is_same_v<
            typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::gcMap,
            std::tuple<
                std::tuple<Int<2>, Int<0> >,
                std::tuple<Int<2>, Int<1> >,
                std::tuple<Int<2>, Int<2> > >

        >);



    static_assert(
        !BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::
        lastUsedNode<0, std::tuple<Int<0>>>::nodeContainsVAsPrereq);


    using inDegrees = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::InDegrees;

    static_assert(std::is_same_v<
        inDegrees,
        std::tuple<
            InDegree<
                Int<2>,
                Int<2> >,
            InDegree<
                Int<1>,
                Int<1> >  >>);


    using decreasedDegrees = typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>,
                                                                    TestGCEdges >::DecreaseInDegrees<inDegrees,
                                                                                                     std::tuple<
                                                                                                         Int<1>, Int<2>
                                                                                                     >
    >::type;

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
        typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::filterNonZero<decreasedDegrees>::type,
        std::tuple<Int<1> >
    >);


    static_assert(std::is_same_v<
        typename BFSLastRecentlyUsedGCPlanImpl<std::tuple<Int<0>>, TestGCEdges >::path,
        std::tuple<Int<0>, Int<1>, Int<2> >
    >);

  }

  {
    std::cout << "test GC" << std::endl;

    auto o2 = withNodes<TestGCNodes >::andEdges<TestGCEdges >{}.execute<
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
        std::tuple<Int<0>, Int<1> >, 2, BFSLastRecentlyUsedGCPlan>({{5}, {1}});

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
            Edge<1, std::tuple<Int<0>>>,
            Edge<2, std::tuple<Int<0>>>

        >{}.execute<Inputs<0>, 3, BFSLastRecentlyUsedGCPlan>({{1, 2}});

    std::cout << output << std::endl;
  }

  {
    auto t0 = std::chrono::system_clock::now();

    auto output = withNodes<TestAsyncNodes >::andEdges<TestAsyncEdges >{}.execute<std::tuple<Int<0>>, 3>({{4}});

    auto t1 = std::chrono::system_clock::now();

    std::cout << "async test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;
  }

  {
    auto t0 = std::chrono::system_clock::now();

    auto output = withNodes<TestSyncNodes >::andEdges<TestSyncEdges >{}.execute<std::tuple<Int<0>>, 3>({{4}});

    auto t1 = std::chrono::system_clock::now();

    std::cout << "sync test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;
  }

  {
    auto t0 = std::chrono::system_clock::now();

    auto output = withNodes<TestNoCacheNodes >::andEdges<TestNoCacheEdges >{}.execute<std::tuple<Int<0>>, 1>({{4}});

    auto t1 = std::chrono::system_clock::now();

    std::cout << "nocache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;
  }

  {
    auto t0 = std::chrono::system_clock::now();

    auto output = withNodes<TestCacheNodes >::andEdges<TestCacheEdges>{}.execute<std::tuple<Int<0>>, 1>({{4}});

    auto t1 = std::chrono::system_clock::now();

    std::cout << "cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;
  }


  {
    auto t0 = std::chrono::system_clock::now();

    auto executor = withNodes<TestCacheNodes >::andEdges<TestCacheEdges>{};
    auto context = executor.createContext();

//    static_cast<CacheTraitBase*>(&std::get<0>(context.allNodes))->keep = []() { return false; };
    context.nodePtr<0>()->keep = []() { return false; };


    auto output = executor.execute<std::tuple<Int<0>>, 1>(context, {{4}});

    auto t1 = std::chrono::system_clock::now();

    std::cout << "cache drop test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
              << std::endl;
  }


  return 0;
}
