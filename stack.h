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

#ifndef LOCKFREE_STACK_H
#define LOCKFREE_STACK_H 1

#include <atomic>

namespace lockfree
{

template <typename T>
struct Stack
{
    Stack () 
    {
        Head const h = { 0, 0 };
        m_head.store(h, std::memory_order_relaxed);
    }

    T*
    pop ()
    {
        Head old = m_head.load(std::memory_order_relaxed);

        for ( ; ; )
        {
            if (ptr (old) == NULL)
                return NULL;

            Head const h = {(long) ptr (old)->next, old.tag};

            if (m_head.compare_exchange_weak(old, h, std::memory_order_release, std::memory_order_relaxed))
            {
                return ptr (old);
            }
        }
    }

    void
    push (T* obj)
    {
        Head old = m_head.load(std::memory_order_relaxed);
        for ( ; ; )
        {
            Head const h = {(long) obj, old.tag + (size_t)1};
            obj->next = ptr (old);

            if (m_head.compare_exchange_weak(old, h, std::memory_order_release, std::memory_order_relaxed))
                return;
        }
    }

private:

    struct Head
    {
        long ptr : 48;
        size_t tag : 16;
    };

    std::atomic <Head> m_head;

    static
    T*
    ptr (Head const h)
    {
        return (T*)(long) h.ptr;
    }
};

} //namespace lockfree

#endif //LOCKFREE_STACK_H
