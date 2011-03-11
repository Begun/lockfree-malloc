/* Copyright 2011 ZAO "Begun".
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

// $Id$

#ifndef LOCKFREE_AUX_H
#define LOCKFREE_AUX_H 1

#include <unistd.h>
#include <sys/mman.h>

#include <boost/cstdint.hpp>
#include <boost/mpl/if.hpp>

namespace lockfree
{

static size_t const ptr_size = sizeof (void *);



inline
bool
cas_aux (__uint128_t *f, __uint128_t o, __uint128_t n)
{   
    bool res;

    asm
    (
        "rex64 lock cmpxchg8b (%1)\n\t"
        "setz %0\n\t"
        :   "=r" (res)
        :
        "r" (f),
        "d" ((size_t) (o >> 64)),
        "a" ((size_t) (o >> 0)),
        "c" ((size_t) (n >> 64)),
        "b" ((size_t) (n >> 0))
    );

    return res;
}

template <typename T>
bool
cas_aux (T *p, T c, T s)
{
    return __sync_bool_compare_and_swap (p, c, s);
}

template <typename T>
bool
cas (T *p, T const &c, T const &s)
{
    typedef typename boost::mpl::if_c <sizeof (T) <= 1, char,
            typename boost::mpl::if_c <sizeof (T) <= 2, short,
            typename boost::mpl::if_c <sizeof (T) <= 4, int,
            typename boost::mpl::if_c <sizeof (T) <= 8, long,
            typename boost::mpl::if_c <sizeof (T) <= 16, __uint128_t,
                                       void>::type>::type>::type>::type>::type Raw;

    union
    {
        T  t;
        Raw r;
    } uc = {c}, us = {s};

    return cas_aux (reinterpret_cast <Raw *> (p), uc.r, us.r);
}

template <typename T>
T
atomic_add (T *v, T t)
{
    return __sync_add_and_fetch (v, t);
}

template <typename T>
T
atomic_inc (T *v)
{
    return atomic_add (v, (T) 1);
}

template <typename T>
T
atomic_dec (T *v)
{
    return atomic_add (v, (T) -1);
}



template <typename T>
struct Linked
{
    Linked () : next (NULL) {}
    T *next;
};

struct Link : public Linked <Link> {};

inline void mfence () {__asm__ __volatile__ ("mfence": : :"memory");}
inline void sfence () {__asm__ __volatile__ ("sfence": : :"memory");}
inline void lfence () {__asm__ __volatile__ ("lfence": : :"memory");}



//we have page size hardcoded; it is not quite correct, but still eases things
//TODO: would it be any slower with get_page_size ()?
size_t const base_page = 4 * 1024;



} //namespace lockfree

inline
size_t
align_up (size_t v, size_t alignment)
{
    return (v + alignment - 1) / alignment * alignment;
}

namespace lf = lockfree;

#endif //LOCKFREE_AUX_H
