//
// Created by soultoxik on 22.04.2021.
//

#ifndef GRAPH_PROC_FUNCHELPER_H
#define GRAPH_PROC_FUNCHELPER_H

#include <tuple>

template<typename Element, typename Tuple>
struct append;

template<typename Element, typename ...Els>
struct append<Element, std::tuple<Els...>> {
  using type = std::tuple<Element, Els...>;
};

template<typename Tuple>
struct concat;

template<typename ...Els>
struct concat<std::tuple<Els...>> {
  template<typename>
  struct with;

  template<typename ...AnotherEls>
  struct with<std::tuple<AnotherEls...>> {
    using type = std::tuple<Els..., AnotherEls...>;
  };
};

template<template<typename> typename Map, typename Tuple>
struct map;

template<template<typename> typename Map>
struct map<Map, std::tuple<>> {
  using type = std::tuple<>;
};

template<template<typename> typename Map, typename Head, typename ...Tail>
struct map<Map, std::tuple<Head, Tail...>> {
  using type = typename concat<
      std::tuple<typename Map<Head>::type>
  >::template with<
      typename map<Map, std::tuple<Tail...>>::type
  >::type;
};

template<template<typename Element, typename Buffer> typename Reduce, typename Base, typename Elements>
struct reduce;

template<template<typename Element, typename Buffer> typename Reduce, typename Base>
struct reduce<Reduce, Base, std::tuple<>> {
  using type = Base;
};

template<template<typename Element, typename Buffer> typename Reduce, typename Base, typename Head, typename ...Tail>
struct reduce<Reduce, Base, std::tuple<Head, Tail...>> {
  using newBuffer = typename reduce<Reduce, Base, std::tuple<Tail...>>::type;
  using type = typename Reduce<Head, newBuffer>::type;
};

template<template<typename> typename Predicate, typename Tuple>
struct filter;

template<template<typename> typename Predicate, typename THead, typename ...TTail>
struct filter<Predicate, std::tuple<THead, TTail...>> {
  using type = typename concat<
      std::conditional_t<
          Predicate<THead>::holds,
          std::tuple<THead>,
          std::tuple<>

      >
  >::template with<
      typename filter<Predicate, std::tuple<TTail...>>::type
  >::type;

};

template<template<typename> typename Predicate>
struct filter<Predicate, std::tuple<>> {
  using type = std::tuple<>;
};

template<template<typename> typename Predicate, typename Tuple>
struct find {
  constexpr static auto found = std::tuple_size_v<
      typename filter<Predicate, Tuple>::type
  > != 0;
};

template<typename>
struct reverse;

template<typename Head, typename ...Tail>
struct reverse<std::tuple<Head, Tail...>> {
  using type = typename concat<typename reverse<std::tuple<Tail...>>::type>::template with<std::tuple<Head>>::type;
};

template<>
struct reverse<std::tuple<>> {
  using type = std::tuple<>;
};

#endif //GRAPH_PROC_FUNCHELPER_H
