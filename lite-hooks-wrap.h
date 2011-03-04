#ifndef __LOCKFREE_LITE_HOOKS_WRAP_H
#define __LOCKFREE_LITE_HOOKS_WRAP_H 1

#include "lite-malloc.h"
#include <malloc.h>

extern "C" {

void * __wrap_malloc (size_t size, void const *) __attribute__ ((malloc));

void * __wrap_malloc (size_t size, void const *)
{
    return util::singleton <lite::Engine> ().do_malloc (size);
}

void __wrap_free (void *p, void const *)
{
    util::singleton <lite::Engine> ().do_free (p);
}

void * __wrap_realloc (void *p, size_t size, void const *)
{
    return util::singleton <lite::Engine> ().do_realloc (p, size);
}

void * __wrap_calloc (size_t n, size_t size)
{
    void* ret = util::singleton <lite::Engine> ().do_malloc (size * n);
    ::memset(ret, 0, size * n);
    return ret;
}

//TODO: real memalign is more complex, hope nobody uses it

void * __wrap_memalign (size_t align, size_t size, void const *caller)
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
