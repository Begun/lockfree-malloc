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

#include "aux_.h"

namespace lockfree
{

template <typename T>
struct Stack
{
    Stack () 
    {
        m_head.ptr = 0;
        m_head.tag = 0;
    }

    T*
    head () const
    {
        return ptr (m_head);
    }

    T*
    pop ()
    {
        for ( ; ; )
        {
            Head const old = m_head;

            if (ptr (old) == NULL)
                return NULL;

            Head const h = {(long) ptr (old)->next, old.tag};

            if (cas (&m_head, old, h))
            {
                return ptr (old);
            }
        }
    }

    void
    push (T* obj)
    {
        for ( ; ; )
        {
            Head const old = m_head, h = {(long) obj, old.tag + 1};
            obj->next = ptr (old);
            if (cas (&m_head, old, h))
                return;
        }
    }

    void
    rob (Stack &r)
    {
        push_all (r.pop_all ()); 
    }

    size_t
    size () const
    {
        return 0;
    }

private:

    //TODO: support count in *all versions
    T*
    pop_all ()
    {
        for ( ; ; )
        {
            Head const old = m_head;

            if (ptr (old) == NULL)
                return NULL;

            Head const h = {0, old.tag};
            if (cas (&m_head, old, h))
                return ptr (old);
        }
    }

    void
    push_all (T *chain)
    {
        T *tail = NULL;
        for ( ; ; )
        {
            Head const old = m_head, h = {(long) chain, old.tag + 1};

            if (ptr (old) != NULL)
            {
                if (tail == NULL)
                    for ( ; tail->next != NULL; tail = tail->next);

                tail->next = ptr (old);
            }
            else
                if (tail)
                    tail->next = NULL;

            if (cas (&m_head, old, h))
                return;
        }
    }

    struct Head
    {
        long ptr : 48;
        size_t tag : 16;
    };

    Head m_head;

    static
    T*
    ptr (Head const h)
    {
        return (T*)(long) h.ptr;
    }
};

} //namespace lockfree

#endif //LOCKFREE_STACK_H
