// gl.h — select the correct OpenGL headers for the current platform.
// Desktop uses glad2 to load GL 3.3 Core function pointers at runtime.
// Mobile and web link GLES3 directly and need no loader.
#pragma once

#if defined(__EMSCRIPTEN__) || defined(__ANDROID__) || defined(GLYPH_IOS)
#  include <GLES3/gl3.h>
#else
#  include <glad/gl.h>
#endif
