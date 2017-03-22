

## About ##

This is a drop-in replacement for malloc/free that is designed from
the ground up to be used in scalable server software.

Why another memory allocator?


Here are the advantages of our allocator:

* It's thread-friendly. It supports a practically-unlimited number of concurrent threads, without locking or performance degradation.

* It's efficient, especially in a multi-threaded environment. Compared to a stock libc allocator, we see a significant performance boost.

* It does NOT fragment or leak memory, unlike a stock libc allocator.

* It wastes less memory. For small objects (less than 8kb in size), the overhead is around 0 bytes. (!)

* It is designed from the ground-up for 64-bit architectures.

* It is elegant. The whole codebase is only around 700 lines of fairly clean C++. (!)

* It fully stand-alone; it does not rely on pthreads or libc at runtime.


The disadvantages:

* Works only with a x86_64 CPU.

* It does not try to economise on virtual memory mapping space.

* Memory used for small objects (those less than 128kb in size) is cached within the allocator and is never released back to the OS. In practice, this means that your app will stabilize at some peak usage of memory and will, from that moment on, rarely ask the OS for memory allocation or deallocation.

_Note_: there is only limited support for the functions memalign, posix_memalign and valloc.

## Usage ##

Build requirements:

For building the library, you will need:

* g++ or clang with C++11 support.


Usage requirements:

* An x86_64 CPU.
* An OS with sane mmap/munmap support. We have production-tested only on Linux. (But it should work with FreeBSD too.)


### Compiling ###

Edit Makefile to set your build options; type `make` to build a static and a shared version of the library.

_Note_: we recommend integrating the allocator into your own project(s). The enclosed Makefile is intended only as a demonstration, not a complete build solution.

### Using the library ###

There are five options:

1. Compile into a statically-linked application. For this, you must: 
    * Compile your application's object files as normal. 
    * Compile `lite-malloc.cpp`, or use the `liblite-malloc-static.a` generated with the enclosed Makefile.
    * Link your app with the `-static` flag, and these linker flags for wrapping:
    ```
   -Wl,--wrap,malloc -Wl,--wrap,free -Wl,--wrap,calloc -Wl,--wrap,realloc
   -Wl,--wrap,memalign -Wl,--wrap,valloc -Wl,--wrap,posix_memalign
   ```

2. Link with a shared library. Linking with `liblite-malloc-shared.so` should work.

3. Via standard glibc malloc hooks. (Deprecated.) To enable, just include `lite-hooks.h` as the first line of your program.

4. Via the magic `LD_PRELOAD` environment variable:
    ```
    $ LD_PRELOAD=<full pathname>/liblite-malloc-shared.so <your application>
    ```

5. Compile a malloc hook static library into your dynamically-linked application. Follow the instructions in option 1, except omit the `-static` flag, and add these linker flags:
    ```
    -Wl,--defsym=malloc=__wrap_malloc -Wl,--defsym=free=__wrap_free -Wl,--defsym=calloc=__wrap_calloc
    -Wl,--defsym=realloc=__wrap_realloc -Wl,--defsym=memalign=__wrap_memalign -Wl,--defsym=valloc=__wrap_valloc
    -Wl,--defsym=posix_memalign=__wrap_posix_memalign
    ```

*Important notes*:

Linux has an OS-imposed limit on the number of virtual memory mappings.
If you want to allocate large amounts of memory, you may need to adjust this 
limit via `/proc/sys/vm/max_map_count`.


### Configuration ###

The allocator can be tuned with compile-time defines. Here are the supported options:

    `LITE_MALLOC_SUPERBLOCK_CACHE_SIZE`
     (default is 32)
     Mmap blocks of size `LITE_MALLOC_SUPERBLOCK_CACHE_SIZE` * 4096 bytes and less will be cached.
     (With a separate cache per size / 4096 bytes.)

    `LITE_MALLOC_MINIBLOCK_CACHE_SIZE`
     (default is 1024)
     Allocations of size `LITE_MALLOC_MINIBLOCK_CACHE_SIZE` * 8 bytes and less will be cached.
     (With a separate cache per size / 8 bytes.)

    `LITE_MALLOC_ENGINES_COUNT`
     (default is 32)
     This many independent allocator engines will be used.

    `LITE_MALLOC_USE_PERTHREAD_ENGINES`
     (default is 1)
     If 1, pin a specific allocator engine to each thread. If 0, choose an allocator engine
     round-robin for each allocation.
     (Use 1 unless you have threads with highly unbalanced allocation workloads and are seeing
     dramatic overhead.)


## Credits ##

https://github.com/Begun/lockfree-malloc

This library is licensed under the GNU LGPL. Please see the file LICENSE.

