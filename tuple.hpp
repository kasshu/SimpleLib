#ifndef SIMPLELIB_TUPLE_HPP_
#define SIMPLELIB_TUPLE_HPP_

#include "common.h"
#include <type_traits>

BEGIN_NAMESPACE_SIMPLELIB

//Recursive definition of template tuple
template <typename First, typename ...Rest>
struct Tuple {
    //type define
    typedef First T1;
    typedef Tuple<Rest...> T2;

    //size
    enum { size = T2::size + 1};

    //values
    T1 v1;
    T2 v2;

    //constructors
    Tuple() : v1(), v2() {
    }

    Tuple(First const& a, Rest... rest) : v1(a), v2(rest...) {
    }
};

//Partial specialization to end the recursion
template <typename First>
struct Tuple<First> {
    //type define
    typedef First T1;

    //size
    enum { size = 1};

    //values
    T1 v1;

    //constructors
    Tuple() : v1(){
    }

    Tuple(First const& a) : v1(a){
    }
};

//Helper template to get the Nth param type
template <std::size_t N, typename First, typename ...Rest>
struct TupleType {
    static_assert(sizeof...(Rest) != 0, "Element index out of range ");
    typedef typename TupleType<N-1, Rest...>::RetType RetType;
};

//Partial specialization to end the recursion
template <typename First, typename ...Rest>
struct TupleType <0, First, Rest...> {
    typedef First RetType;
};

//funtion utility to get the Nth element from tuple
template <std::size_t N, typename ...Types>
typename std::enable_if<N != 0, typename TupleType<N, Types...>::RetType>::type
get_from_tuple(Tuple<Types...> const& t) {
    return get_from_tuple<N-1>(t.v2);
}

//Overloaded function to end the recursion
template <std::size_t N, typename ...Types>
typename std::enable_if<N == 0, typename TupleType<N, Types...>::RetType>::type
get_from_tuple(Tuple<Types...> const& t) {
    return t.v1;
}

END_NAMESPACE_SIMPLELIB

#endif  //SIMPLELIB_TUPLE_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
