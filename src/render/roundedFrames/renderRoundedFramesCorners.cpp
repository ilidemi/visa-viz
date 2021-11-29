#pragma once

#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include "renderRoundedFramesCommon.cpp"
#include <emscripten.h>
#include <math.h>

struct RoundedFrameCornerVertexAttributes
{
    float x, y;
    float centerX, centerY;
    float radius;
    float borderSize;
    GlColor insideColor, borderColor, outsideColor;

    void init(
        float cornerX, float cornerY, float centerX, float centerY, float radius, float borderSize,
        Color insideColor, Color borderColor, Color outsideColor, QuadVertex *quadVertex)
    {
        this->x = cornerX + radius * quadVertex->x;
        this->y = cornerY + radius * quadVertex->y;
        this->centerX = centerX;
        this->centerY = centerY;
        this->radius = radius;
        this->borderSize = borderSize;
        this->insideColor = GlColor::create(insideColor);
        this->borderColor = GlColor::create(borderColor);
        this->outsideColor = GlColor::create(outsideColor);
    }
};

struct RoundedFramesCornersRenderContext
{
    GLuint program;
    GLuint matPos;
    GLuint posPos, centerPosPos, radiusPos, borderSizePos, insideColorPos, borderColorPos, outsideColorPos;
    CanvasContext *canvasCtx;

    void init(CanvasContext *canvasCtx)
    {
        const char vertexShaderCode[] =
#include "roundedFramesCornersVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "roundedFramesCornersFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");

        this->posPos = glGetAttribLocation(this->program, "pos");
        this->centerPosPos = glGetAttribLocation(this->program, "centerPos");
        this->radiusPos = glGetAttribLocation(this->program, "radius");
        this->borderSizePos = glGetAttribLocation(this->program, "borderSize");
        this->insideColorPos = glGetAttribLocation(this->program, "insideColor");
        this->borderColorPos = glGetAttribLocation(this->program, "borderColor");
        this->outsideColorPos = glGetAttribLocation(this->program, "outsideColor");

        this->canvasCtx = canvasCtx;
    }
};

struct RoundedFramesCornersGroupContext
{
    RoundedFramesCornersRenderContext *renderCtx;
    List<RoundedFrameCornerVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    void init(RoundedFramesCornersRenderContext *renderCtx)
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
        float nwCornerX = roundedFrame->x;
        float nwCornerY = roundedFrame->y;
        float nwCornerCenterX = nwCornerX + roundedFrame->radius;
        float nwCornerCenterY = nwCornerY + roundedFrame->radius;
        list_for_each(quadVertex, quadVertices)
        {
            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                nwCornerX, nwCornerY, nwCornerCenterX, nwCornerCenterY, roundedFrame->radius,
                roundedFrame->borderSize, roundedFrame->insideColor, roundedFrame->borderColor,
                roundedFrame->outsideColor, quadVertex);
        }

        float neCornerX = roundedFrame->x + roundedFrame->width - roundedFrame->radius;
        float neCornerY = roundedFrame->y;
        float neCornerCenterX = neCornerX;
        float neCornerCenterY = neCornerY + roundedFrame->radius;
        list_for_each(quadVertex, quadVertices)
        {
            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                neCornerX, neCornerY, neCornerCenterX, neCornerCenterY, roundedFrame->radius,
                roundedFrame->borderSize, roundedFrame->insideColor, roundedFrame->borderColor,
                roundedFrame->outsideColor, quadVertex);
        }

        float seCornerX = roundedFrame->x + roundedFrame->width - roundedFrame->radius;
        float seCornerY = roundedFrame->y + roundedFrame->height - roundedFrame->radius;
        float seCornerCenterX = seCornerX;
        float seCornerCenterY = seCornerY;
        list_for_each(quadVertex, quadVertices)
        {
            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                seCornerX, seCornerY, seCornerCenterX, seCornerCenterY, roundedFrame->radius,
                roundedFrame->borderSize, roundedFrame->insideColor, roundedFrame->borderColor,
                roundedFrame->outsideColor, quadVertex);
        }

        float swCornerX = roundedFrame->x;
        float swCornerY = roundedFrame->y + roundedFrame->height - roundedFrame->radius;
        float swCornerCenterX = swCornerX + roundedFrame->radius;
        float swCornerCenterY = swCornerY;
        list_for_each(quadVertex, quadVertices)
        {
            auto *vertexAttributes = this->vertexAttributes.getNext();
            vertexAttributes->init(
                swCornerX, swCornerY, swCornerCenterX, swCornerCenterY, roundedFrame->radius,
                roundedFrame->borderSize, roundedFrame->insideColor, roundedFrame->borderColor,
                roundedFrame->outsideColor, quadVertex);
        }
    }

    void setAttributes()
    {
        GLsizei stride = sizeof(RoundedFrameCornerVertexAttributes);

        glEnableVertexAttribArray(this->renderCtx->posPos);
        glVertexAttribPointer(this->renderCtx->posPos, 2, GL_FLOAT, GL_FALSE, stride, 0);

        glEnableVertexAttribArray(this->renderCtx->centerPosPos);
        glVertexAttribPointer(
            this->renderCtx->centerPosPos, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(2 * sizeof(float)));

        glEnableVertexAttribArray(this->renderCtx->radiusPos);
        glVertexAttribPointer(
            this->renderCtx->radiusPos, 1, GL_FLOAT, GL_FALSE, stride, (const void *)(4 * sizeof(float)));

        glEnableVertexAttribArray(this->renderCtx->borderSizePos);
        glVertexAttribPointer(
            this->renderCtx->borderSizePos, 1, GL_FLOAT, GL_FALSE, stride, (const void *)(5 * sizeof(float)));

        glEnableVertexAttribArray(this->renderCtx->insideColorPos);
        glVertexAttribPointer(
            this->renderCtx->insideColorPos, 4, GL_FLOAT, GL_FALSE, stride, (const void *)(6 * sizeof(float)));

        glEnableVertexAttribArray(this->renderCtx->borderColorPos);
        glVertexAttribPointer(
            this->renderCtx->borderColorPos, 4, GL_FLOAT, GL_FALSE, stride,
            (const void *)(6 * sizeof(float) + 1 * sizeof(GlColor)));

        glEnableVertexAttribArray(this->renderCtx->outsideColorPos);
        glVertexAttribPointer(
            this->renderCtx->outsideColorPos, 4, GL_FLOAT, GL_FALSE, stride,
            (const void *)(6 * sizeof(float) + 2 * sizeof(GlColor)));
    }

    void unsetAttributes()
    {
        glDisableVertexAttribArray(this->renderCtx->posPos);
        glDisableVertexAttribArray(this->renderCtx->centerPosPos);
        glDisableVertexAttribArray(this->renderCtx->radiusPos);
        glDisableVertexAttribArray(this->renderCtx->borderSizePos);
        glDisableVertexAttribArray(this->renderCtx->insideColorPos);
        glDisableVertexAttribArray(this->renderCtx->borderColorPos);
        glDisableVertexAttribArray(this->renderCtx->outsideColorPos);
    }
};