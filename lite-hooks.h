#ifndef __LOCKFREE_LITE_HOOKS_H
#define __LOCKFREE_LITE_HOOKS_H 1

#include "lite-malloc.h"
#include <malloc.h>

namespace lite
{

void * malloc_hook (size_t size, void const *) __attribute__ ((malloc));

void * malloc_hook (size_t size, void const *)
{
    return util::singleton <lite::Engine> ().do_malloc (size);
}

void free_hook (void *p, void const *)
{
    util::singleton <lite::Engine> ().do_free (p);
}

void * realloc_hook (void *p, size_t size, void const *)
{
    return util::singleton <lite::Engine> ().do_realloc (p, size);
}

#if 0
//TODO: real memalign is more complex, hope nobody uses it
void * memalign_hook (size_t align, size_t size, void const *caller)
{
    return malloc_hook (size, caller);
}
#endif

static void init_hook ()
{
    __malloc_hook = malloc_hook;
    __realloc_hook = realloc_hook;
    __free_hook = free_hook;
//    __memalign_hook = memalign_hook;
}

} //namespace lite

void (*__malloc_initialize_hook) (void) = lite::init_hook;

#endif //__LOCKFREE_LITE_HOOKS_H
