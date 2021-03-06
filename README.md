![](https://user-images.githubusercontent.com/7226886/113434932-8781e780-93ea-11eb-81ed-7576ca090bcc.png)

## What is this
Graph-level abstraction over heavy computations.
1. Graph built at compile-time (zero overhead)
2. Full typesafety
3. Separate independent computational traits (i.e. caching)

## Table of contents
1. [How to use](#how-to-use)
2. [Building](#building)

## How to use

Suppose we want to add two number, the calculation of which is a heavy operation.

`f(x) = Add( Heavy_0(Id(x)) , Heavy_1(Id(x)) )`

1. Implement `Node`'s with necessary computations

Prototype:
```
declare_node({UserDefined}, {StateType}, {OutputType}, {InputTypesWithComma})
{OutputType} {UserDefined}::runImpl({InputTypesWithComma} args...) {
    ...
}

```
that expands to:
```
template <typename...>
class {UserDefined} : public Node<{UserDefined}, {StateType}, {OutputType}, {InputTypesWithComma}> {
    friend class Node;

public:
    {UserDefined}(): Node<{UserDefined}, {StateType}, {OutputType}, {InputTypesWithComma}>("{user_defined}") {}
private:
    {OutputType} runImpl({InputTypesWithComma} args...) {
      ...
    }
};

```

Example nodes:
```
#include <Graph.h>

template <typename...>
class Id : public Node<Sum, int, int> {
    friend class Node;

public:
    Id(): Node<Id, int, int>("id") {}
private:
    int runImpl(int arg) {
        return arg;
    }
};


template <typename...>
class Sum : public Node<Sum, int, int, int> {
    friend class Node;

public:
    Summer(): Node<Summer, int, int, int>("sum") {}
private:
    int runImpl(int arg0, int arg1) {
        return arg0 + arg1;
    }
};

template <typename...>
class Heavy_0 : public Heavy_0<Heavy_0, int, int> {
    friend class Node;

public:
    Heavy_0(): Node<Heavy_0, int, int>("Heavy_0") {}
private:
    int runImpl(int arg) {
        return someHeavyOperation(arg);
    }
};

template <typename...>
class Heavy_1 : public Heavy_1<Heavy_1, int, int> {
    friend class Node;

public:
    Heavy_1(): Node<Heavy_1, int, int>("Heavy_1") {}
private:
    int runImpl(int arg) {
        return anotherHeavyOperation(arg);
    }
};
```

2. Extends your `Node`'s with necessary traits

In example we want to parallelize our heavy operations.
```
#include <Trait.h>

using AsyncHeavy_0 = AsyncTrait<Heavy_0<>>;
using AsyncHeavy_1 = AsyncTrait<Heavy_1<>>;
```

3. Enumerate `Node`'s with indicies

In example we assigning:
* `Id<>` with id `0`
* `AsyncHeavy_0<>` with id `1`
* `AsyncHeavy_1<>` with id `2`
* `Sum<>` with id `3`

```
#define NODES IndexedNode <0, Id<>> , IndexedNode <1, AsyncHeavy_0<>>, IndexedNode <2, AsyncHeavy_1<>>, IndexedNode <3, Sum<>>
```

4. Declare edges that forms ` node_id <- dependecies_ids `

In example we assuming that:
* inputs for `Sum<>` (id `3`) is `AsyncHeavy_0<>` (id `1`) and `AsyncHeavy_1<>` (id `2`)
* input for `AsyncHeavy_0<>` (id `1`) is `Id<>` (id `0`)
* input for `AsyncHeavy_0<>` (id `2`) is `Id<>` (id `0`)
```
#define EDGES Edge<3, std::tuple< Int<1>, Int<2> >> , Edge<2, std::tuple< Int<0> >>, Edge<1, std::tuple< Int<0> >>
```

5. Select garbage-collection policy

By default all nodes instances has same lifecycle as `Context` (`GCPlan == NoPlan`).

If you need clear intermediate data (such as a cache) as soon as possible (then there are no other nodes remains thats uses this data ) you can select this behaviour.
```
#define PLAN BFSLastRecentlyUsedGCPlanImpl
```


6. Run computations

In example we're assuming that `Id<>` (id `0`) will be source, `Sum<>` (id `3`) will be target node
```
std::tuple<int> input = ...;
auto output = withNodes<NODES>::andEdges<EDGES>{}.execute<Inputs<Int<0>>, 3, PLAN>( { input } );
```

See example of semipractical case of migrating at `stampede` in `src/cv_samples`

## Building

For building example / test:
```
mkdir build
cd build
cmake ..
make
./graph_proc
```
For using in another CMake project:

CMakeLists.txt
```
...
include_directory({path/to/stampede/src})
...
```

## Special thanks

@evsluzh for early-stage review

@opedge for early-stage review

@ashagraev for help in YaTalks report

## Used in




