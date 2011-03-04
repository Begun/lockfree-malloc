#ifndef __UTIL_SINGLETON_H
#define __UTIL_SINGLETON_H

#include <limits>
#include <boost/mpl/if.hpp>

namespace util {

// ��������.
// gcc ����������� ������ �� ������������� ������������� ������� � ������
// �������, �������������� �������� ��� ����������. ��� �����������
// ������������� �������, ����� �������������� ������������ � ���
// ������������������.
//
// ������� ���� ���������� ����������� ���������� ����������
// -fno-threadsafe-statics
template <typename T>
T& singleton() {
    static T obj;
    return obj;
}

// ��������, ��������� ��� ������� ������
// �������� ������ ��� ����� �� ����������� ��������������:
// POD-����, ��������� ��� �������������.
// http://gcc.gnu.org/onlinedocs/gcc-4.3.5/gcc/Thread_002dLocal.html
// ������ ����� ����� ��������������� boost::thread_specific_ptr
template <typename T>
T& threaded_singleton() {
    static __thread T obj;
    return obj;
}

// ������, ��� ��������� �������������� ���� �� ������������ ���������.
// ��� �������������� ���������, ��� ����� ��� ��, ��� � ��������
template<typename T>
struct type_of_const {
    typedef T type;
};

template<typename T>
struct type_of_const<const T> {
    typedef T type;
};

// ���� �������� ������� T - �������� ���, �� if_numeric::result - ����� ���� RES1, ����� RES2
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

