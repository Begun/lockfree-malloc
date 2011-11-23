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

#ifndef __LOCKFREE_LITE_MALLOC_H
#define __LOCKFREE_LITE_MALLOC_H 1


#include <stddef.h>
#include <string.h>
#include <cstdarg>

//#include "u-singleton.h"
#include "l-singleton.h"
#include "stack.h"

#ifdef __FreeBSD__
#define __MAP_ANONYMOUS MAP_ANON
#else
#define __MAP_ANONYMOUS MAP_ANONYMOUS
#endif


namespace lite
{

struct Sb
{
    struct Pool *home;
    size_t data_size;
    size_t mmap_size;
    void* mmap_addr;

    char *raw () {return &this->last_one;}
    // 32 byte aligned
    char last_one;
};
size_t const header_size = offsetof(Sb, last_one);

size_t const grid_step_log2 = 18;
size_t const grid_step = 1 << grid_step_log2;
size_t const offset_mask = grid_step - 1;
size_t const grid_mask = ~offset_mask;
size_t const sb_size = grid_step - lf::base_page;

inline
size_t grid_align (size_t s)
{
    return ((s - 1) & grid_mask) + grid_step;
}


struct Sb_cache
{
    Sb_cache () /*: cur ((char*) grid_align (0x2aeacd6a6000ul))*/ {}

    //char *cur;

    Sb *pop (Pool *home, size_t size)
    {
        size_t sz = align_up (size, lf::base_page);

        size_t mmap_size = 0;
        void* mmap_addr = NULL;
        Sb *sb = (Sb*) aux_pop (sz, mmap_addr, mmap_size);

        if (sb)
        {
            sb->home = home;
            sb->data_size = sz;
            sb->mmap_size = mmap_size;
            sb->mmap_addr = mmap_addr;
        }

        return sb;
    }

    void push (Sb *sb)
    {
        size_t off = cache_offset (sb->data_size);
        if (off < caches_count)
            caches [off].push ((lf::Link *) sb);
        else
            munmap (sb->mmap_addr, sb->mmap_size);
    }


private:

    static size_t const caches_count = 32;
    lf::Stack <lf::Link> caches [caches_count];

    static size_t cache_offset (size_t size) {return size / lf::base_page - 1;}

    void *aux_pop (size_t size, void*& mmap_addr, size_t& mmap_size)
    {
        size_t off = cache_offset (size);
        if (off < caches_count)
            if (void *p = caches [off].pop ())
                return p;

        size_t sx = size + grid_step;

        char *p = map (0, sx);
        if (!p)
            return 0;

        size_t pp = (size_t) p, ga = grid_align (pp); //, head = ga - pp, tail = sx - size - head;

        //if (head)
        //    munmap (p, head);

        //if (tail)
        //    munmap (p + size + head, tail);

        char *np = (char *) ga;

        mmap_addr = p;
        mmap_size = sx;

        return (Sb*) np;

        
        /*
        size_t s = grid_align (size), sx = size + grid_step;
        for ( ; ; )
        {
            char *c = cur;
            if (lf::cas (&cur, c, c + s))
            {
                char *p = map (c, size);

                if (p)
                    return (Sb *) p;

                p = map (0, sx);
                if (!p)
                    return 0;

                size_t pp = (size_t) p, ga = grid_align (pp), head = ga - pp, tail = sx - size - head;
                munmap (p, head);
                munmap (p + size + head, tail);
                char *np = (char *) ga;
                lf::cas (&cur, c, np);
                return (Sb*) np;
            }
        }
        */
    }

    static
    char *map (void *hint, size_t size)
    {
        void * p = mmap (hint,
                         size,
                         PROT_WRITE | PROT_READ,
                         MAP_PRIVATE | __MAP_ANONYMOUS | (hint ? MAP_FIXED : 0),
                         -1 /*fd*/,
                         0);
        return (char *) ((long) p == -1 ? 0 : p);
    }

};

//TODO: split exact/inexact pool
struct __attribute__ ((aligned(32))) Pool
{
    Pool () : raw (0), elem_size (0) {}

    size_t push (void *p)
    {
        freed.push ((lf::Link*) p);
        return elem_size;
    }

    void *pop (Sb_cache &cache, size_t size)
    {
        (void) size;
        return aux_pop (cache);
    }


private:

    char *aux_pop (Sb_cache &cache)
    {
        Sb *sb = 0;
        char *result = 0;
        for ( ; ; )
        {
            result = (char *) freed.pop ();
            if (result)
                break;

            char *r = raw, *next = r + elem_size;
            size_t off = (size_t) r & offset_mask;
            if (r == 0 || off == 0 || off + elem_size > sb_size)
            {
                if (!sb)
                    sb = cache.pop (this, sb_size);

                if (lf::cas (&raw, r, sb->raw () + elem_size))
                {
                    result = sb->raw ();
                    sb = 0;
                    break;
                }
            }
            else if (lf::cas (&raw, r, next))
            {
                result = r;
                break;
            }
        }

        if (sb)
            cache.push (sb);

        return result;
    }

public:

    lf::Stack <lf::Link> freed; 
    char *raw;
    size_t elem_size;
};


inline
size_t block_size (void *p)
{
    if (p)
    {
        Sb *sb = (Sb *) ((size_t) p & grid_mask);
        return sb->home ? sb->home->elem_size : sb->data_size - header_size;
    }

    return 0;
}

class Engine
{
    Sb_cache sb_cache;

    static size_t const pools_count = 1023; //up to 8Kb
    Pool pools [pools_count];

    // boost::noncopyable replacement
    Engine(Engine&);
    Engine(const Engine&);
    Engine& operator=(Engine&);
    Engine& operator=(const Engine&);

public:

    Engine ()
    {
        for (size_t i = 0; i < pools_count; ++i)
            pools [i].elem_size = (i + 1) * lf::ptr_size;

        //TODO: run GC thread
    }

    void *do_malloc (size_t size)
    {
        if (!size)
            return 0;

        size_t words = (size + lf::ptr_size - 1) / lf::ptr_size, off = words - 1;
        if (off < pools_count)
            return pools [off].pop (sb_cache, words * lf::ptr_size);

        Sb *sb = sb_cache.pop (0, size + header_size);
        if (!sb)
            return 0;
        return sb->raw ();
    }

    void do_free (void *p)
    {
        if (p)
        {
            Sb *sb = (Sb *) ((size_t) p & grid_mask);
            if (sb->home)
                sb->home->push (p);
            else
                sb_cache.push (sb);
        }
    }

    void *do_calloc (size_t n, size_t m)
    {
        if (size_t size = n * m)
        {
            void *p = do_malloc (size);
            bzero (p, size);
            return p;
        }

        return 0;
    }

    void *do_realloc (void *p, size_t size)
    {
        if (!p)
            return do_malloc (size);

        void *r = 0;
        if (size)
        {
            size_t old_size = block_size (p);

            if (size <= old_size)
                return p;

            r = do_malloc (size);
            if (!r)
                return 0;

            memcpy (r, p, old_size < size ? old_size : size);
        }

        do_free (p);
        return r;
    }

};



} //namespace lite

#endif //__LOCKFREE_LITE_MALLOC_H
