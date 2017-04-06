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

#ifndef __LOCKFREE_LITE_HOOKS_H
#define __LOCKFREE_LITE_HOOKS_H 1

#include "lite-malloc.h"
#include <malloc.h>

namespace lite
{

void * malloc_hook (size_t size, void const *) __attribute__ ((malloc));

void * malloc_hook (size_t size, void const *)
{
    return lockfree::singleton <lite::EnginePool> ().do_malloc (size);
}

void free_hook (void *p, void const *)
{
    lockfree::singleton <lite::EnginePool> ().do_free (p);
}

void * realloc_hook (void *p, size_t size, void const *)
{
    return lockfree::singleton <lite::EnginePool> ().do_realloc (p, size);
}

void * memalign_hook (size_t align, size_t size, void const *caller)
{
    return lockfree::singleton <lite::EnginePool> ().do_memalign (align, size);
}

static void init_hook ()
{
    __malloc_hook = malloc_hook;
    __realloc_hook = realloc_hook;
    __free_hook = free_hook;
    __memalign_hook = memalign_hook;
}

} //namespace lite

void (*__malloc_initialize_hook) (void) = lite::init_hook;

#endif //__LOCKFREE_LITE_HOOKS_H
