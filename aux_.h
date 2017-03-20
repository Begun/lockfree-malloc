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

template <typename T>
struct Linked
{
    Linked () : next (NULL) {}
    T *next;
};

struct Link : public Linked <Link> {};


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
