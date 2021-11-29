#pragma once

#include "../../style.cpp"
#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include <emscripten.h>

struct LayoutedArrowTip
{
    float x, midY;
    float width, height;

    static LayoutedArrowTip create(float x, float midY, float width, float height)
    {
        return { x, midY, width, height };
    }

    void init(float x, float midY, float width, float height)
    {
        this->x = x;
        this->midY = midY;
        this->width = width;
        this->height = height;
    }
};

struct ArrowTipVertexAttributes
{
    float x, y;
    float vertexX, vertexY;

    void init(LayoutedArrowTip *arrowTip, QuadVertex *quadVertex)
    {
        this->vertexX = arrowTip->width * quadVertex->x;
        this->vertexY = arrowTip->height * quadVertex->y;

        // Android really wants me to do this addition here and not in the vertex shader
        this->x = arrowTip->x + this->vertexX;
        this->y = arrowTip->midY - 0.5 * arrowTip->height + this->vertexY;
    }
};

struct ArrowTipsRenderContext
{
    GLuint program;
    GLuint matPos, widthPos, heightPos, colorPos;
    GLuint posPos, vertexPosPos;

    List<ArrowTipVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    float displayWidth, displayHeight;
    Color color;

    CanvasContext *canvasCtx;

    void init(CanvasContext *canvasCtx, StyleConfig *styleCfg)
    {
        const char vertexShaderCode[] =
#include "arrowTipsVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "arrowTipsFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");
        this->widthPos = glGetUniformLocation(this->program, "width");
        this->heightPos = glGetUniformLocation(this->program, "height");
        this->colorPos = glGetUniformLocation(this->program, "color");

        this->posPos = glGetAttribLocation(this->program, "pos");
        this->vertexPosPos = glGetAttribLocation(this->program, "vertexPos");

        this->vertexAttributes.init();
        glGenBuffers(1, &this->vertexAttributesHandle);

        this->displayWidth = styleCfg->arrowTipSize;
        this->displayHeight = styleCfg->arrowTipHeight;
        this->color = styleCfg->arrowColor;

        this->canvasCtx = canvasCtx;
    }

    bool isInitialized()
    {
        return true;
    }

    void setUniforms()
    {
        glUniform1f(this->widthPos, this->displayWidth);
        glUniform1f(this->heightPos, this->displayHeight);
        glUniformColor4f(this->colorPos, this->color);
    }

    void setAttributes()
    {
        GLsizei stride = sizeof(ArrowTipVertexAttributes);

        glEnableVertexAttribArray(this->posPos);
        glVertexAttribPointer(this->posPos, 2, GL_FLOAT, GL_FALSE, stride, 0);

        glEnableVertexAttribArray(this->vertexPosPos);
        glVertexAttribPointer(
            this->vertexPosPos, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(2 * sizeof(float)));
    }

    void unsetAttributes()
    {
        glDisableVertexAttribArray(this->posPos);
        glDisableVertexAttribArray(this->vertexPosPos);
    }
};