#ifndef __LOCKFREE_SINGLETON_H
#define __LOCKFREE_SINGLETON_H 1

#include "aux_.h"

namespace lockfree
{

struct Once
{
    int l;

    bool
    lock_once ()
    {
        for ( ; ; )
        {
            int c = l;
            if (c == 0)
                return false;

            if (c == 2 && cas (&l, 2, 1))
                return true;

            mfence ();
        }
    }

    void
    unlock ()
    {
        l = 0;
        mfence ();
    }
};

template <typename T>
struct Raw_
{
    char data [sizeof (T)];
} __attribute__ ((aligned (16)));

template <typename T>
struct Holder
{
    static Raw_ <T> value;
    static Once once;
};

template <typename T> Raw_ <T> Holder <T>::value = {{}}; //TODO
template <typename T> Once Holder <T>::once = {2};

template <typename T>
T &
singleton ()
{
    typedef Holder <T> H;
    if (H::once.lock_once ())
    {
        new (&H::value) T ();
        H::once.unlock ();
    }

    return (T &) H::value;
}

} //namespace lockfree

#endif //__LOCKFREE_SINGLETON_H
