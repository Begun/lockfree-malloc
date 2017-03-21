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
#ifndef __FreeBSD__
#include <malloc.h>
#endif

#include <new>

#include <stdlib.h>

extern "C" {

void * __wrap_malloc (size_t size, void const *) __attribute__ ((malloc));

void * __wrap_malloc (size_t size, void const *)
{
    return lockfree::singleton <lite::EnginePool> ().do_malloc (size);
}

void __wrap_free (void *p, void const *)
{
    lockfree::singleton <lite::EnginePool> ().do_free (p);
}

void * __wrap_realloc (void *p, size_t size, void const *)
{
    return lockfree::singleton <lite::EnginePool> ().do_realloc (p, size);
}

void * __wrap_calloc (size_t n, size_t size)
{
    return lockfree::singleton <lite::EnginePool> ().do_calloc (n, size);
}

void * __wrap_memalign (size_t align, size_t size)
{
    return lockfree::singleton <lite::EnginePool> ().do_memalign (align, size);
}

void * __wrap_aligned_alloc(size_t align, size_t size)
{
    return __wrap_memalign (align, size);
}

int __wrap_posix_memalign(void **memptr, size_t align, size_t size) {
    *memptr = __wrap_memalign (align, size);
    return 0;
}

void * __wrap_valloc(size_t size)
{
    return __wrap_memalign (lf::base_page, size);
}



}


#endif //__LOCKFREE_LITE_HOOKS_WRAP_H
