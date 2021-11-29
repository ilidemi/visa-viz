#pragma once

#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include "renderRoundedFramesCommon.cpp"
#include <emscripten.h>
#include <math.h>

struct RoundedFrameSideVertexAttributes
{
    float x, y;
    float borderDistance;
    float borderSize;
    GlColor insideColor, borderColor;

    void init(float x, float y, float borderDistance, float borderSize, Color insideColor, Color borderColor)
    {
        this->x = x;
        this->y = y;
        this->borderDistance = borderDistance;
        this->borderSize = borderSize;
        this->insideColor = GlColor::create(insideColor);
        this->borderColor = GlColor::create(borderColor);
    }
};

struct RoundedFramesSidesRenderContext
{
    GLuint program;
    GLuint matPos;
    GLuint posPos, borderDistancePos, borderSizePos, insideColorPos, borderColorPos;
    CanvasContext *canvasCtx;

    void init(CanvasContext *canvasCtx)
    {
        const char vertexShaderCode[] =
#include "roundedFramesSidesVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "roundedFramesSidesFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");

        this->borderDistancePos = glGetAttribLocation(this->program, "borderDistance");
        this->borderSizePos = glGetAttribLocation(this->program, "borderSize");
        this->insideColorPos = glGetAttribLocation(this->program, "insideColor");
        this->borderColorPos = glGetAttribLocation(this->program, "borderColor");

        this->canvasCtx = canvasCtx;
    }
};

struct RoundedFramesSidesGroupContext
{
    RoundedFramesSidesRenderContext *renderCtx;
    List<RoundedFrameSideVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    void init(RoundedFramesSidesRenderContext *renderCtx)
    {
        this->renderCtx = renderCtx;
        this->vertexAttributes.init();
        glGenBuffers(1, &this->vertexAttributesHandle);
    }

    void reserve(int roundedFramesCount)
    {
        this->vertexAttributes.clear();
        this->vertexAttributes.reserve(roundedFramesCount * 4 * quadVertices.size);
    }

    void append(LayoutedRoundedFrame *roundedFrame)
    {
        float leftSideX = roundedFrame->x;
        float leftSideY = roundedFrame->y + roundedFrame->radius;
        list_for_each(quadVertex, quadVertices)
        {
            float leftVertexX = leftSideX + roundedFrame->radius * quadVertex->x;
            float leftVertexY = leftSideY + (roundedFrame->height - 2 * roundedFrame->radius) * quadVertex->y;
            float leftBorderDistance = roundedFrame->radius * quadVertex->x;

            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                leftVertexX, leftVertexY, leftBorderDistance, roundedFrame->borderSize,
                roundedFrame->insideColor, roundedFrame->borderColor);
        }

        float rightSideX = roundedFrame->x + roundedFrame->width - roundedFrame->radius;
        float rightSideY = roundedFrame->y + roundedFrame->radius;
        list_for_each(quadVertex, quadVertices)
        {
            float rightVertexX = rightSideX + roundedFrame->radius * quadVertex->x;
            float rightVertexY =
                rightSideY + (roundedFrame->height - 2 * roundedFrame->radius) * quadVertex->y;
            float rightBorderDistance = roundedFrame->radius * (1 - quadVertex->x);

            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                rightVertexX, rightVertexY, rightBorderDistance, roundedFrame->borderSize,
                roundedFrame->insideColor, roundedFrame->borderColor);
        }

        float topSideX = roundedFrame->x + roundedFrame->radius;
        float topSideY = roundedFrame->y;
        list_for_each(quadVertex, quadVertices)
        {
            float topVertexX = topSideX + (roundedFrame->width - 2 * roundedFrame->radius) * quadVertex->x;
            float topVertexY = topSideY + roundedFrame->radius * quadVertex->y;
            float topBorderDistance = roundedFrame->radius * quadVertex->y;

            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                topVertexX, topVertexY, topBorderDistance, roundedFrame->borderSize,
                roundedFrame->insideColor, roundedFrame->borderColor);
        }

        float bottomSideX = roundedFrame->x + roundedFrame->radius;
        float bottomSideY = roundedFrame->y + roundedFrame->height - roundedFrame->radius;
        list_for_each(quadVertex, quadVertices)
        {
            float bottomVertexX =
                bottomSideX + (roundedFrame->width - 2 * roundedFrame->radius) * quadVertex->x;
            float bottomVertexY = bottomSideY + roundedFrame->radius * quadVertex->y;
            float bottomBorderDistance = roundedFrame->radius * (1 - quadVertex->y);

            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                bottomVertexX, bottomVertexY, bottomBorderDistance, roundedFrame->borderSize,
                roundedFrame->insideColor, roundedFrame->borderColor);
        }
    }

    void setAttributes()
    {
        GLsizei stride = sizeof(RoundedFrameSideVertexAttributes);

        glEnableVertexAttribArray(this->renderCtx->posPos);
        glVertexAttribPointer(this->renderCtx->posPos, 2, GL_FLOAT, GL_FALSE, stride, 0);

        glEnableVertexAttribArray(this->renderCtx->borderDistancePos);
        glVertexAttribPointer(
            this->renderCtx->borderDistancePos, 1, GL_FLOAT, GL_FALSE, stride,
            (const void *)(2 * sizeof(float)));

        glEnableVertexAttribArray(this->renderCtx->borderSizePos);
        glVertexAttribPointer(
            this->renderCtx->borderSizePos, 1, GL_FLOAT, GL_FALSE, stride, (const void *)(3 * sizeof(float)));

        glEnableVertexAttribArray(this->renderCtx->insideColorPos);
        glVertexAttribPointer(
            this->renderCtx->insideColorPos, 4, GL_FLOAT, GL_FALSE, stride,
            (const void *)(4 * sizeof(float)));

        glEnableVertexAttribArray(this->renderCtx->borderColorPos);
        glVertexAttribPointer(
            this->renderCtx->borderColorPos, 4, GL_FLOAT, GL_FALSE, stride,
            (const void *)(4 * sizeof(float) + 1 * sizeof(GlColor)));
    }

    void unsetAttributes()
    {
        glDisableVertexAttribArray(this->renderCtx->posPos);
        glDisableVertexAttribArray(this->renderCtx->borderDistancePos);
        glDisableVertexAttribArray(this->renderCtx->borderSizePos);
        glDisableVertexAttribArray(this->renderCtx->insideColorPos);
        glDisableVertexAttribArray(this->renderCtx->borderColorPos);
    }
};