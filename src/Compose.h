//
// Created by soultoxik on 26.04.2021.
//

#ifndef GRAPH_PROC_COMPOSE_H
#define GRAPH_PROC_COMPOSE_H

struct Nil {};

template <typename Base, template <typename > class ...Types>
struct Compose;

template <typename Base, template <typename> class Head, template <typename > class ...Types>
struct Compose<Base, Head, Types...> {
    using type = Head<typename Compose<Base, Types...>::type >;
};

template <typename Base>
struct Compose<Base> {
    using type = Base;
};

template<typename Base, template <typename > class ...Types>
using Apply = typename Compose<Base, Types...>::type;



#endif //GRAPH_PROC_COMPOSE_H
