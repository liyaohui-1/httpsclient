#ifndef MACROS_H_
#define MACROS_H_

#include <cstdlib>
#include <new>

#if __GNUC__ >= 3
#    define iros_likely(x) (__builtin_expect((x), 1))
#    define iros_unlikely(x) (__builtin_expect((x), 0))
#else
#    define iros_likely(x) (x)
#    define iros_unlikely(x) (x)
#endif

#define CACHELINE_SIZE 64

#define DEFINE_TYPE_TRAIT(name, func)                       \
    template<typename T>                                    \
    struct name                                             \
    {                                                       \
        template<typename Class>                            \
        static constexpr bool Test(decltype(&Class::func)*) \
        {                                                   \
            return true;                                    \
        }                                                   \
        template<typename>                                  \
        static constexpr bool Test(...)                     \
        {                                                   \
            return false;                                   \
        }                                                   \
                                                            \
        static constexpr bool value = Test<T>(nullptr);     \
    };                                                      \
                                                            \
    template<typename T>                                    \
    constexpr bool name<T>::value;

#ifdef _WIN32
#    include <intrin.h>
inline void cpu_relax()
{
#    if defined(__aarch64__)
    __yield();
#    else
    _mm_pause();
#    endif
}
#else
inline void cpu_relax()
{
#    if defined(__aarch64__)
    asm volatile("yield" ::: "memory");
#    else
    asm volatile("rep; nop" ::: "memory");
#    endif
}
#endif   // _WIN32

inline void* CheckedMalloc(size_t size)
{
    void* ptr = std::malloc(size);
    if (!ptr) { throw std::bad_alloc(); }
    return ptr;
}

inline void* CheckedCalloc(size_t num, size_t size)
{
    void* ptr = std::calloc(num, size);
    if (!ptr) { throw std::bad_alloc(); }
    return ptr;
}

#endif   // MACROS_H_
