#ifndef SIMPLELIB_FUNCTION_TRAITS_HPP_
#define SIMPLELIB_FUNCTION_TRAITS_HPP_

#include "common.h"

BEGIN_NAMESPACE_SIMPLELIB

//Helper template to get the Nth param type
template <std::size_t N, typename First, typename ...Rest>
struct param_traits {
    static_assert(sizeof...(Rest) != 0, "Param index out of range ");
    typedef typename param_traits<N-1, Rest...>::Type Type;
};

//Partial specialization to end the recursion
template <typename First, typename ...Rest>
struct param_traits<0, First, Rest...> {
    typedef First Type;
};

//Basic template
//Inherit to match functor and lambda:
//    1.Functor will inherit from partial specialization 2
//    2.Lambda will inherit from partial specialization 3
template <typename T>
class function_traits : public function_traits<decltype(&T::operator())>{
};

//Partial specialization 1 to match normal function
template <typename ReturnType, typename ...ParamTypes>
class function_traits<ReturnType(ParamTypes ...)>{
public:
    typedef ReturnType RType;

    template <std::size_t N>
    struct PType: public param_traits<N, ParamTypes...> {
    };

    enum {ParamSize = sizeof...(ParamTypes)};
};

//Partial specialization 2 to match member function
template <typename ReturnType, typename ClassType, typename ...ParamTypes>
class function_traits<ReturnType(ClassType::*)(ParamTypes ...)> 
            : public function_traits<ReturnType(ParamTypes ...)>{
public:
    typedef ClassType CType;
};

//Partial specialization 3 to match const member function and lambda
//The operator() in lambda is a const member function
template <typename ReturnType, typename ClassType, typename ...ParamTypes>
class function_traits<ReturnType(ClassType::*)(ParamTypes ...) const> 
    : public function_traits<ReturnType(ClassType::*)(ParamTypes ...)>{
};

END_NAMESPACE_SIMPLELIB

#endif  //SIMPLELIB_FUNCTION_TRAITS_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
