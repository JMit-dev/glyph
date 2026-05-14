// stb_impl.cpp — defines all stb single-header implementations.
// This translation unit must be compiled exactly once.
// Other files include stb headers without the IMPLEMENTATION define.

#ifdef _MSC_VER
#  pragma warning(push, 0)   // suppress stb's MSVC warnings
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#ifdef _MSC_VER
#  pragma warning(pop)
#endif
