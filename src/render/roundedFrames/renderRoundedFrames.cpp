#pragma once

#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include "../common/renderPrimitives.cpp"
#include "../rects/renderRects.cpp"
#include "renderRoundedFramesCommon.cpp"
#include "renderRoundedFramesCorners.cpp"
#include "renderRoundedFramesSides.cpp"
#include <emscripten.h>
#include <math.h>

struct RoundedFramesRenderContext
{
    RoundedFramesCornersRenderContext cornersRenderCtx;
    RoundedFramesSidesRenderContext sidesRenderCtx;
    RectsRenderContext *insidesRenderCtx;

    void init(RectsRenderContext *rectsRenderCtx, CanvasContext *canvasCtx)
    {
        cornersRenderCtx.init(canvasCtx);
        sidesRenderCtx.init(canvasCtx);
        this->insidesRenderCtx = rectsRenderCtx;
    }
};

struct RoundedFramesGroupContextV2
{
    RoundedFramesRenderContext *renderCtx;
    RoundedFramesCornersGroupContext cornersGroupCtx;
    RoundedFramesSidesGroupContext sidesGroupCtx;
    RectsGroupContext insidesGroupCtx;

    void init(RoundedFramesRenderContext *renderCtx)
    {
        this->renderCtx = renderCtx;
        this->cornersGroupCtx.init(&renderCtx->cornersRenderCtx);
        this->sidesGroupCtx.init(&renderCtx->sidesRenderCtx);
        this->insidesGroupCtx.init(renderCtx->insidesRenderCtx);
    }

    void reserve(int roundedFramesCount)
    {
        this->cornersGroupCtx.reserve(roundedFramesCount);
        this->sidesGroupCtx.reserve(roundedFramesCount);
        primitivesReserve(&this->insidesGroupCtx, roundedFramesCount);
    }

    void appendRange(List<LayoutedRoundedFrame> *roundedFrames)
    {
        list_for_each(roundedFrame, *roundedFrames)
        {
            this->cornersGroupCtx.append(roundedFrame);
            this->sidesGroupCtx.append(roundedFrame);

            if (roundedFrame->insideColor.a > 0)
            {
                LayoutedRect inside = LayoutedRect::create(
                    roundedFrame->x + roundedFrame->radius, roundedFrame->y + roundedFrame->radius,
                    roundedFrame->width - roundedFrame->radius * 2,
                    roundedFrame->height - roundedFrame->radius * 2, roundedFrame->insideColor);
                primitivesAppend(&this->insidesGroupCtx, &inside);
            }
        }
    }

    void commit()
    {
        primitivesCommit(&this->cornersGroupCtx);
        primitivesCommit(&this->sidesGroupCtx);
        primitivesCommit(&this->insidesGroupCtx);
    }

    template <typename TGroupCtx, typename TRenderCtx>
    static void
    _render(TGroupCtx *groupCtx, TRenderCtx *renderCtx, float displayViewportX, float displayViewportY)
    {
        if (groupCtx->vertexAttributes.size == 0)
        {
            return;
        }

        float mat[16];
        initTransformationMatrix(mat, renderCtx->canvasCtx, displayViewportX, displayViewportY);

        glUseProgram(renderCtx->program);
        glUniformMatrix4fv(renderCtx->matPos, 1, 0, mat);

        glBindBuffer(GL_ARRAY_BUFFER, groupCtx->vertexAttributesHandle);
        groupCtx->setAttributes();
        glDrawArrays(GL_TRIANGLES, 0, groupCtx->vertexAttributes.size);
        groupCtx->unsetAttributes();
    }

    void render(float displayViewportX, float displayViewportY)
    {
        _render(
            &this->cornersGroupCtx, &this->renderCtx->cornersRenderCtx, displayViewportX, displayViewportY);
        _render(&this->sidesGroupCtx, &this->renderCtx->sidesRenderCtx, displayViewportX, displayViewportY);
        primitivesRender(
            &this->insidesGroupCtx, this->renderCtx->insidesRenderCtx, displayViewportX, displayViewportY);
    }
};