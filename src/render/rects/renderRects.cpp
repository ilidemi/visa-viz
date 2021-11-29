#pragma once

#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include "../common/layoutedRect.cpp"
#include <emscripten.h>

struct RectVertexAttributes
{
    float x, y;
    GlColor color;

    void init(LayoutedRect *rect, QuadVertex *quadVertex)
    {
        this->x = rect->x + rect->width * quadVertex->x;
        this->y = rect->y + rect->height * quadVertex->y;
        this->color = GlColor::create(rect->color);
    }
};

struct RectsRenderContext
{

    GLuint program;
    GLuint matPos;
    GLuint posPos, colorPos;
    CanvasContext *canvasCtx;

    void init(CanvasContext *canvasCtx)
    {
        const char vertexShaderCode[] =
#include "rectsVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "rectsFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");

        this->posPos = glGetAttribLocation(this->program, "pos");
        this->colorPos = glGetAttribLocation(this->program, "color");

        this->canvasCtx = canvasCtx;
    }
};

struct RectsGroupContext
{
    RectsRenderContext *renderCtx;
    List<RectVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    void init(RectsRenderContext *renderCtx)
    {
        this->renderCtx = renderCtx;
        this->vertexAttributes.init();
        glGenBuffers(1, &this->vertexAttributesHandle);
    }

    bool isInitialized()
    {
        return true;
    }

    void setUniforms()
    {
    }

    void setAttributes()
    {
        GLsizei stride = sizeof(RectVertexAttributes);

        glEnableVertexAttribArray(this->renderCtx->posPos);
        glVertexAttribPointer(this->renderCtx->posPos, 2, GL_FLOAT, GL_FALSE, stride, 0);

        glEnableVertexAttribArray(this->renderCtx->colorPos);
        glVertexAttribPointer(
            this->renderCtx->colorPos, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(2 * sizeof(float)));
    }

    void unsetAttributes()
    {
        glDisableVertexAttribArray(this->renderCtx->posPos);
        glDisableVertexAttribArray(this->renderCtx->colorPos);
    }
};