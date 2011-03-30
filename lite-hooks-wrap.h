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

#ifndef __LOCKFREE_LITE_HOOKS_WRAP_H
#define __LOCKFREE_LITE_HOOKS_WRAP_H 1

#include "lite-malloc.h"
#include <malloc.h>
#include <stdlib.h>

extern "C" {

void * __wrap_malloc (size_t size, void const *) __attribute__ ((malloc));

void * __wrap_malloc (size_t size, void const *)
{
    return lockfree::singleton <lite::Engine> ().do_malloc (size);
}

void __wrap_free (void *p, void const *)
{
    lockfree::singleton <lite::Engine> ().do_free (p);
}

void * __wrap_realloc (void *p, size_t size, void const *)
{
    return lockfree::singleton <lite::Engine> ().do_realloc (p, size);
}

void * __wrap_calloc (size_t n, size_t size)
{
    void* ret = lockfree::singleton <lite::Engine> ().do_malloc (size * n);
    ::memset(ret, 0, size * n);
    return ret;
}

//TODO: real memalign is more complex, hope nobody uses it

void * __wrap_memalign (size_t align, size_t size)
{
    ::abort();
}
void * __wrap_valloc(size_t size)
{
    ::abort();
}

int __wrap_posix_memalign(void **memptr, size_t alignment, size_t size)
{
    ::abort();
}


}


#endif //__LOCKFREE_LITE_HOOKS_WRAP_H
