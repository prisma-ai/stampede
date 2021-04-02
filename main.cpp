#include <iostream>

#include "src/Graph.h"
#include "src/Example.h"
#include "src/Test.h"
#include "src/Trait.h"


int main() {
    {
        std::cout << "node test" << std::endl;

        Summer<> summer;
        std::cout << value(summer.runPack({1, 2})) << std::endl;
    }

    {
        std::cout << "complex graph test" << std::endl;

        auto output =
                withNodes<
                        IndexedNode<0, AsyncTrait<CacheTrait<Summer<>>>>, IndexedNode<1, Stringify<>>, IndexedNode<2, AsyncTrait<Mul2<>>>, IndexedNode<3, Dummy<>>>
                ::andEdges<
                        Edge<3, std::tuple<IntType<2>, IntType<1>>>, Edge<1, std::tuple<IntType<0>>>, Edge<2, std::tuple<IntType<0>>>>{}
                        .topDown<0, 3>({1, 2});


        std::cout << output << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestAsyncNodes>::andEdges<TestAsyncEdges >{}.topDown<0, 3>({4});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "async test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestSyncNodes>::andEdges<TestSyncEdges >{}.topDown<0, 3>({4});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "sync test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
        << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestNoCacheNodes>::andEdges<TestNoCacheEdges >{}.topDown<0, 1>({4});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "nocache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << std::endl;
    }

    {
        auto t0 = std::chrono::system_clock::now();

        auto output = withNodes<TestCacheNodes>::andEdges<TestCacheEdges >{}.topDown<0, 1>({4});

        auto t1 = std::chrono::system_clock::now();

        std::cout << "cache test " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()
                  << std::endl;
    }


    return 0;
}
