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

#ifndef __LOCKFREE_SINGLETON_H
#define __LOCKFREE_SINGLETON_H 1

#include <new>
#include <atomic>

namespace lockfree
{

struct Once
{
    Once() : l(2) {}

    std::atomic <int> l;

    bool
    lock_once ()
    {
        int c = l.load(std::memory_order_relaxed);

        for ( ; ; )
        {
            if (c == 0)
                return false;

            if (c == 2) {
                if (l.compare_exchange_weak(c, 1, std::memory_order_release, std::memory_order_acquire)) {
                    return true;
                }
            } else {
                c = l.load(std::memory_order_acquire);
            }
        }
    }

    void
    unlock ()
    {
        l.store(0, std::memory_order_release);
    }
};

template <typename T>
struct Raw_
{
    char data [sizeof (T)];
} __attribute__ ((aligned (16)));

template <typename T>
struct Holder
{
    static Raw_ <T> value;
    static Once once;
};

template <typename T> Raw_ <T> Holder <T>::value = {{}}; //TODO
template <typename T> Once Holder <T>::once;

template <typename T>
T &
singleton ()
{
    typedef Holder <T> H;
    if (H::once.lock_once ())
    {
        new (&H::value) T ();
        H::once.unlock ();
    }

    return (T &) H::value;
}

} //namespace lockfree

#endif //__LOCKFREE_SINGLETON_H
