
CXX = g++
CFLAGS = -std=c++11 -O3 -finline-functions -Wno-inline -Wall -pthread
LFLAGS =

IDEPS = aux_.h lite-hooks-wrap.h lite-malloc.h stack.h l-singleton.h 

STATIC_LIB = liblite-malloc-static.a
SHARED_LIB = liblite-malloc-shared.so

all: $(STATIC_LIB) $(SHARED_LIB)

lite-malloc.o: $(IDEPS)
	$(CXX) $(CFLAGS) lite-malloc.cpp -fPIC -c -o lite-malloc.o

$(STATIC_LIB): lite-malloc.o
	-rm -f $(STATIC_LIB)
	ar rc $(STATIC_LIB) lite-malloc.o 

$(SHARED_LIB): lite-malloc.o
	objcopy --redefine-sym __wrap_malloc=malloc \
		--redefine-sym __wrap_free=free \
		--redefine-sym __wrap_calloc=calloc \
		--redefine-sym __wrap_realloc=realloc \
		--redefine-sym __wrap_memalign=memalign \
		--redefine-sym __wrap_posix_memalign=posix_memalign \
		--redefine-sym __wrap_valloc=valloc \
		--redefine-sym __wrap_aligned_alloc=aligned_alloc \
		lite-malloc.o lite-malloc-shared.o
	$(CXX) $(LFLAGS) lite-malloc-shared.o -shared -o $(SHARED_LIB)

clean:
	-rm -f lite-malloc.o lite-malloc-shared.o $(STATIC_LIB) $(SHARED_LIB) 


