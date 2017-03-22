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


// Mmap blocks less then LITE_MALLOC_SUPERBLOCK_CACHE_SIZE * 4096 bytes in size
// will be cached.
 
#ifndef LITE_MALLOC_SUPERBLOCK_CACHE_SIZE
#define LITE_MALLOC_SUPERBLOCK_CACHE_SIZE 32
#endif
 
// Blocks less then LITE_MALLOC_MINIBLOCK_CACHE_SIZE * 8 bytes in size 
// will be cached in pool allocator.

#ifndef LITE_MALLOC_MINIBLOCK_CACHE_SIZE 
#define LITE_MALLOC_MINIBLOCK_CACHE_SIZE 1024
#endif

// Use LITE_MALLOC_ENGINES_COUNT independent allocator engines.

#ifndef LITE_MALLOC_ENGINES_COUNT
#define LITE_MALLOC_ENGINES_COUNT 32
#endif

// Use per-thread engines instead of picking engines uniformly round-robin.

#ifndef LITE_MALLOC_USE_PERTHREAD_ENGINES
#define LITE_MALLOC_USE_PERTHREAD_ENGINES 1
#endif


#include <atomic>
#include <stddef.h>
#include <string.h>
#include <cstdarg>

#include "aux_.h"
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

    size_t engine_id;
    char* raw_addr;
    size_t _pad2;
    size_t _pad3;

    char *raw ()
    {
        return raw_addr;
    }

    void set_raw_addr (size_t alignment)
    {
        char* r = &this->last_one;
        size_t n = (size_t)r & alignment;

        if (n) {
            r += alignment + 1 - n;
        }

        raw_addr = r;
    }

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
    Sb_cache () {}

    Sb *pop (Pool *home, size_t size, size_t alignment, size_t engine_id)
    {
        size_t sz = align_up (size, lf::base_page);

        size_t mmap_size = 0;
        void* mmap_addr = NULL;
        Sb *sb = (Sb*) aux_pop (sz, mmap_addr, mmap_size);

        if (sb)
        {
            sb->engine_id = engine_id;
            sb->home = home;
            sb->data_size = sz;
            sb->mmap_size = mmap_size;
            sb->mmap_addr = mmap_addr;
            sb->set_raw_addr (alignment);
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

    static size_t const caches_count = LITE_MALLOC_SUPERBLOCK_CACHE_SIZE;
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

        char *np = (char *) ga;

        mmap_addr = p;
        mmap_size = sx;

        return (Sb*) np;
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

    void *pop (Sb_cache &cache, size_t alignment, size_t engine_id)
    {
        return aux_pop (cache, alignment, engine_id);
    }


private:

    char *aux_pop (Sb_cache &cache, size_t alignment, size_t engine_id)
    {
        Sb *sb = 0;
        char *result = 0;
        char *r = raw.load(std::memory_order_relaxed);

        for ( ; ; )
        {
            result = (char *) freed.pop ();
            if (result)
                break;

            char *next = r + elem_size;
            size_t off = (size_t) r & offset_mask;
            if (r == 0 || off == 0 || off + elem_size > sb_size)
            {
                if (!sb)
                    sb = cache.pop (this, sb_size, alignment, engine_id);

                if (raw.compare_exchange_weak(r, sb->raw () + elem_size, std::memory_order_release, std::memory_order_relaxed))
                {
                    result = sb->raw ();
                    sb = 0;
                    break;
                }
            }
            else if (raw.compare_exchange_weak(r, next, std::memory_order_release, std::memory_order_relaxed))
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
    std::atomic <char*> raw;
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

inline
size_t block_engine_n (void *p)
{
    if (p)
    {
        Sb *sb = (Sb *) ((size_t) p & grid_mask);
        return sb->engine_id;
    }

    return 0;
}

class Engine
{
    Sb_cache sb_cache;

    static size_t const pools_count = LITE_MALLOC_MINIBLOCK_CACHE_SIZE - 1; //up to 8Kb
    Pool pools [pools_count];

    Engine(Engine&) = delete;
    Engine(const Engine&) = delete;
    Engine& operator=(Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

public:

    size_t engine_id;
    size_t alignment;

    Engine () : engine_id(0), alignment(0)
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
            return pools [off].pop (sb_cache, alignment, engine_id);

        Sb *sb = sb_cache.pop (0, size + header_size + alignment, alignment, engine_id);
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

constexpr size_t log2(size_t n)
{
    return (n <= 1 ? 0 : 1 + log2(n / 2));
}

#if LITE_MALLOC_USE_PERTHREAD_ENGINES
#define STATIC_THREAD_LOCAL static thread_local
#else
#define STATIC_THREAD_LOCAL
#endif

class EnginePool
{
 
    static size_t const plain_engines_count   = LITE_MALLOC_ENGINES_COUNT;
    static size_t const aligned_engines_count = log2(lf::base_page) - 3;
    static size_t const total_engines_count   = plain_engines_count + aligned_engines_count;
    Engine engines [total_engines_count];

    std::atomic<size_t> alloc_count;

    EnginePool(EnginePool&) = delete;
    EnginePool(const EnginePool&) = delete;
    EnginePool& operator=(EnginePool&) = delete;
    EnginePool& operator=(const EnginePool&) = delete;

public:

    EnginePool () : alloc_count(0)
    {
        for (size_t i = 0; i < total_engines_count; ++i)
            engines [i].engine_id = i + 1;

        for (size_t i = plain_engines_count, a = 16; i < total_engines_count; ++i, a *= 2)
            engines [i].alignment = a - 1;
    }

    void *do_malloc (size_t size)
    {
        STATIC_THREAD_LOCAL size_t en{++alloc_count % plain_engines_count};

        return engines [en].do_malloc(size);
    }

    void do_free (void *p) 
    {
        size_t engine = block_engine_n(p);

        if (engine)
            engines [engine-1].do_free(p);
    }    

    void *do_calloc (size_t n, size_t m)
    {
        STATIC_THREAD_LOCAL size_t en{++alloc_count % plain_engines_count};

        return engines [en].do_calloc(n, m);
    }

    void *do_realloc (void *p, size_t size)
    {
        STATIC_THREAD_LOCAL size_t en{++alloc_count % plain_engines_count};
        size_t engine = block_engine_n(p);

        if (!engine)
            engine = en;
        else
            engine = engine - 1;

        return engines [engine].do_realloc(p, size);
    }

    void *do_memalign (size_t align, size_t size)
    {
        if (align <= 8)
            return do_malloc (size);

        size = align_up (size, align);
        align--;

        for (size_t i = plain_engines_count; i < total_engines_count; ++i)
        {
            if (engines [i].alignment == align)
            {
                return engines [i].do_malloc(size);
            }
        }

        return 0;
    }
};

#undef STATIC_THREAD_LOCAL

} //namespace lite

#endif //__LOCKFREE_LITE_MALLOC_H
