// shader.cpp — GL shader program compilation and uniform helpers.
#include "shader.h"

#include "gl.h"
#include "shaders.h"

#include <cstdio>
#include <cstring>

namespace glyph {

static unsigned int compile_stage(GLenum type, const char* src) {
    // Pass the platform #version header as a separate source string so the
    // shader source itself stays version-agnostic.
    const char* sources[] = {glyph::shaders::kVersionHeader, src};
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 2, sources, nullptr);
    glCompileShader(s);

    int ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[glyph] shader compile error: %s\n", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

bool Shader::compile(const char* vert_src, const char* frag_src) {
    unsigned int vert = compile_stage(GL_VERTEX_SHADER,   vert_src);
    unsigned int frag = compile_stage(GL_FRAGMENT_SHADER, frag_src);
    if (!vert || !frag) {
        glDeleteShader(vert);
        glDeleteShader(frag);
        return false;
    }

    id_ = glCreateProgram();
    glAttachShader(id_, vert);
    glAttachShader(id_, frag);
    glLinkProgram(id_);
    glDeleteShader(vert);
    glDeleteShader(frag);

    int ok;
    glGetProgramiv(id_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(id_, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[glyph] shader link error: %s\n", log);
        glDeleteProgram(id_);
        id_ = 0;
        return false;
    }
    return true;
}

void Shader::destroy() {
    if (id_) {
        glDeleteProgram(id_);
        id_ = 0;
    }
}

void Shader::bind() const {
    glUseProgram(id_);
}

void Shader::set_mat4(const char* name, const float* m) const {
    glUniformMatrix4fv(glGetUniformLocation(id_, name), 1, GL_FALSE, m);
}

void Shader::set_int(const char* name, int v) const {
    glUniform1i(glGetUniformLocation(id_, name), v);
}

} // namespace glyph
