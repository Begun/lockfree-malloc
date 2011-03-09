#ifndef __UTIL_SINGLETON_H
#define __UTIL_SINGLETON_H

#include <limits>
#include <boost/mpl/if.hpp>

namespace util {

// Синглтон.
// gcc гарантирует защиту от одновременной инициализации объекта в разных
// потоках, самостоятельно вставляя код блокировки. Для дальнейшего
// использования объекта, нужно самостоятельно позаботиться о его
// потокобезопасности.
//
// Вставка кода блокировки отключается параметром компиляции
// -fno-threadsafe-statics
template <typename T>
T& singleton() {
    static T obj;
    return obj;
}

// Синглтон, локальный для данного потока
// Работает только для типов со статической инициализацией:
// POD-типы, структуры без конструкторов.
// http://gcc.gnu.org/onlinedocs/gcc-4.3.5/gcc/Thread_002dLocal.html
// Вместо этого можно воспользоваться boost::thread_specific_ptr
template <typename T>
T& threaded_singleton() {
    static __thread T obj;
    return obj;
}

// Шаблон, для получение неконстантного типа по константному аргументу.
// Для неконстантного аргумента, тип будет тем же, что и аргумент
template<typename T>
struct type_of_const {
    typedef T type;
};

template<typename T>
struct type_of_const<const T> {
    typedef T type;
};

// Если параметр шаблона T - числовой тип, то if_numeric::result - будет типа RES1, иначе RES2
template<typename T, typename RES1, typename RES2>
struct if_numeric {
    typedef typename util::type_of_const<T>::type varT;
    typedef typename boost::mpl::if_c<
            std::numeric_limits<varT>::is_specialized,
            RES1,
            RES2
        >::type result;
};


} // namespace util

#endif

