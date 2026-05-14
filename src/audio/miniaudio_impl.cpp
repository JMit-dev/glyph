// miniaudio_impl.cpp — defines the miniaudio implementation.
// This translation unit must be compiled exactly once.

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#endif

#ifdef _MSC_VER
#  pragma warning(push, 0)
#endif

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
