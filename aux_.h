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

namespace lockfree
{

static size_t const ptr_size = sizeof (void *);

template <size_t s>
struct cas_type { typedef typename cas_type<s-1>::type type; };
template<> struct cas_type<0> { typedef void type; };
template<> struct cas_type<sizeof(char)> { typedef char type; };
template<> struct cas_type<sizeof(char)+1> { typedef short type; };
template<> struct cas_type<sizeof(short)+1> { typedef int type; };
template<> struct cas_type<sizeof(int)+1> { typedef long type; };
template<> struct cas_type<sizeof(long)+1> { typedef void type; };


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
    typedef typename cas_type<sizeof(T)>::type Raw;

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
