#pragma once

#include "../../util/util.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include "../common/layoutedRect.cpp"
#include <emscripten.h>

struct FullScreenFillRenderer
{
    GLuint program;
    GLuint colorPos;
    GLuint uvPos;
    GLuint quad;

    void init(GLuint quad)
    {
        const char vertexShaderCode[] =
#include "fullScreenFillVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "fullScreenFillFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);
        
        this->colorPos = glGetUniformLocation(this->program, "color");
        this->uvPos = glGetAttribLocation(this->program, "uv");
        this->quad = quad;
    }

    void render(Color color)
    {
        glUseProgram(this->program);

        glUniformColor4f(this->colorPos, color);

        glBindBuffer(GL_ARRAY_BUFFER, this->quad);
        glEnableVertexAttribArray(this->uvPos);
        glVertexAttribPointer(this->uvPos, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(this->uvPos);
    }
};
