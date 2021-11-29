#pragma once

#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/glUtil.cpp"
#include "../common/texture.cpp"
#include <emscripten.h>

static const int64_t VisaUserId = 16884623;

struct LayoutedVisaPic
{
    float x, y;
    float size;

    static LayoutedVisaPic create(float x, float y, float size)
    {
        return { x, y, size };
    }
};

struct VisaPicVertexAttributes
{
    float x, y;
    float vertexX, vertexY;
    float size;

    void init(LayoutedVisaPic *visaPic, QuadVertex *quadVertex)
    {
        this->vertexX = visaPic->size * quadVertex->x;
        this->vertexY = visaPic->size * quadVertex->y;

        // Android really wants me to do this addition here and not in the vertex shader
        this->x = visaPic->x + this->vertexX;
        this->y = visaPic->y + this->vertexY;
        this->size = visaPic->size;
    }
};

struct VisaPicsRenderContext
{
    GLuint program;
    GLuint matPos;
    GLuint posPos, vertexPosPos, sizePos;

    Texture texture;
    CanvasContext *canvasCtx;

    List<VisaPicVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    void init(CanvasContext *canvasCtx, bool isWebGl2, const char *visaPicUrl)
    {
        const char vertexShaderCode[] =
#include "visaPicsVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "visaPicsFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");

        this->posPos = glGetAttribLocation(this->program, "pos");
        this->vertexPosPos = glGetAttribLocation(this->program, "vertexPos");
        this->sizePos = glGetAttribLocation(this->program, "size");

        loadTextureFromUrl(&this->texture, visaPicUrl, isWebGl2);
        this->canvasCtx = canvasCtx;

        this->vertexAttributes.init();
        glGenBuffers(1, &this->vertexAttributesHandle);
    }

    bool isInitialized()
    {
        return this->texture.isInitialized();
    }

    void setUniforms()
    {
        glBindTexture(GL_TEXTURE_2D, this->texture.texture);
    }

    void setAttributes()
    {
        GLsizei stride = sizeof(VisaPicVertexAttributes);

        glEnableVertexAttribArray(this->posPos);
        glVertexAttribPointer(this->posPos, 2, GL_FLOAT, GL_FALSE, stride, 0);

        glEnableVertexAttribArray(this->vertexPosPos);
        glVertexAttribPointer(
            this->vertexPosPos, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(2 * sizeof(float)));

        glEnableVertexAttribArray(this->sizePos);
        glVertexAttribPointer(
            this->sizePos, 1, GL_FLOAT, GL_FALSE, stride, (const void *)(4 * sizeof(float)));
    }

    void unsetAttributes()
    {
        glDisableVertexAttribArray(this->posPos);
        glDisableVertexAttribArray(this->vertexPosPos);
        glDisableVertexAttribArray(this->sizePos);
    }
};