#include <map>
#include <time.h>
#include <string>
#include <vector>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <functional>
#include <type_traits>
#include <random>
#include <chrono>
#include <algorithm>
#include "function_traits.hpp"
#include "tuple.hpp"
#include "singleton.hpp"
#include "red_black_tree.hpp"
#include "simple_blocking_queue.hpp"
#include "simple_lock_free_queue.hpp"
#include "timer.hpp"

using namespace simplelib;

int test(int a, double d, std::string s) {
    return -1;
}

struct aaa {
    int test(int a, double d, std::string s) {
        return -1;
    }
};

struct functor {
    int operator()(int a, double d, std::string s) {
        return -1;
    }
};

struct Inner {
    Inner(){
        printf("Default constructor called\n");
    }
    Inner(const Inner &i) {
        printf("Copy constructor called\n");
    }
    Inner(Inner &&i) {
        printf("Move constructor called\n");
    }
    ~Inner() {
        printf("Destructor called\n");
    }
    Inner& operator=(const Inner &i) {
        printf("Equal operator called\n");
        return *this;
    }
    Inner& operator=(Inner &&i) {
        printf("Move equal operator called\n");
        return *this;
    }
};

struct Defaulter {
    //Defaulter() = default;
    //~Defaulter() = default;
    Defaulter(Inner &&i) : ii(std::forward<Inner>(i)){} 
    std::string abc = "aaa";
    int def = 1;
    Inner ii;
};

class MultiHeirBase {
public:
    typedef int mytype;
    typedef int mytype2;
};

class MultiHeirA: virtual public MultiHeirBase {
public:
    typedef int64_t mytype;
};

class MultiHeirB: virtual public MultiHeirBase {
};

class MutualHeir: public MultiHeirA, public MultiHeirB {
};

//class MutualHeir: public MultiHeirB {
//};

template <typename Derived>
class Clonable {
public:
    Derived * clone() {
        return new Derived(static_cast<Derived const &>(*this));
    }
};

class CRTP: public Clonable<CRTP> {
public:
    int i;
};

template <typename Derived>
class StaticPolymorphism {
public:
    void  interface() {
        static_cast<Derived*>(this)->implementation();
    }
};

class CRTPDerived: public StaticPolymorphism<CRTPDerived> {
public:
    void implementation() {
        printf("In CRTPDerived implementation\n");
    }
};

template <typename T>
class ZeroInitialization {
public:
    ZeroInitialization() {
        //T x = T();
        T x;
        printf("x = %d\n", x);
    }
};

class LazyResource {
public:
    LazyResource(){
        printf("Resource inited\n");
    }
};

template <bool B>
class LazyInitialization {
public:
    LazyInitialization() {
        printf("Do nothing\n");
    }
    const static LazyResource *m_resource;
};

//template <>
//class LazyInitialization<true> {
//public:
//    LazyInitialization() {
//        printf("Lazy initialization\n");
//    }
//};

template <bool B>
const LazyResource * LazyInitialization<B>::m_resource = new LazyResource();

template<bool B>
void do_lazy() {
    LazyInitialization<B>::m_resource;
}

class DerivedSingleton: public Singleton<DerivedSingleton> {
public:
    DerivedSingleton(){}
    int32_t init(std::string path_to_conf);
    void sayHello() {
        printf("Hello from singleton\n");
    }
};

void simple_dir_walker(const std::string& path, std::function<void(struct dirent*)> process) {
    DIR *d = opendir(path.c_str());
    struct dirent *dir;
    while ((dir = readdir(d)) != NULL) {
        process(dir);
    }
    closedir(d);
}

enum AAA {
    A,
    B
};

enum class AAA1 {
    A,
    B
};

int main(int argc, char *argv[]) {
    //normal functions
    printf("Type is %s\n", typeid(function_traits<decltype(test)>::RType).name());
    printf("Type is %s\n", typeid(function_traits<decltype(test)>::PType<2>::Type).name());

    //member functions
    printf("Type is %s\n", typeid(function_traits<decltype(&aaa::test)>::RType).name());
    printf("Type is %s\n", typeid(function_traits<decltype(&aaa::test)>::PType<2>::Type).name());

    //functor
    functor f;
    printf("Type is %s\n", typeid(function_traits<decltype(f)>::RType).name());
    printf("Type is %s\n", typeid(function_traits<decltype(f)>::PType<2>::Type).name());

    //lambda
    auto lambda = [](int a, double d, std::string s){return -1;};
    printf("Type is %s\n", typeid(function_traits<decltype(lambda)>::RType).name());
    printf("Type is %s\n", typeid(function_traits<decltype(lambda)>::PType<2>::Type).name());

    Inner i;
    Defaulter d(std::move(i));
    //d.abc = "bbb";
    //d.def = 2;
    //Defaulter d2(std::move(d));
    std::map<std::string, std::string> m;
    auto &astr = m["abc"];
    printf("%s\n", astr.c_str());
    printf("size is %d\n", sizeof(MutualHeir::mytype));
    CRTP *crtp = new CRTP();
    crtp->i = 10;
    CRTP *crtp_clone = crtp->clone();
    printf("%d vs %d\n", crtp->i, crtp_clone->i);
    delete crtp;
    delete crtp_clone;
    CRTPDerived crtpd;
    crtpd.interface();
    Tuple<int, long, float, double, bool> my_tuple(1, 2L, 3.0, 4.5, true);
    printf("size=%d\n", decltype(my_tuple)::size);
    printf("value=%d\n", get_from_tuple<4>(my_tuple));
    ZeroInitialization<int> z1;
    do_lazy<true>();
    DerivedSingleton * ds = DerivedSingleton::getInstance();
    DerivedSingleton ds_;
    ds->sayHello();

    return 0;
}
