#pragma once

#include "../../util/deque.cpp"
#include "../../util/list.cpp"
#include "glUtil.cpp"

template <typename TPrimitivesGroupContext>
void primitivesReserve(TPrimitivesGroupContext *groupCtx, int primitivesCount)
{
    groupCtx->vertexAttributes.clear();
    groupCtx->vertexAttributes.reserve(primitivesCount * quadVertices.size);
}

template <typename TPrimitivesGroupContext, typename TLayoutedPrimitive>
void primitivesAppend(TPrimitivesGroupContext *groupCtx, TLayoutedPrimitive *primitive)
{
    list_for_each(quadVertex, quadVertices)
    {
        auto *vertexAttributes = groupCtx->vertexAttributes.getNext();
        vertexAttributes->init(primitive, quadVertex);
    }
}

template <typename TPrimitivesGroupContext, typename TLayoutedPrimitive>
void primitivesAppendRange(TPrimitivesGroupContext *groupCtx, List<TLayoutedPrimitive> *primitives)
{
    list_for_each(primitive, *primitives)
    {
        primitivesAppend(groupCtx, primitive);
    }
}

template <typename TPrimitivesGroupContext, typename TLayoutedPrimitive>
void primitivesAppendRange(TPrimitivesGroupContext *groupCtx, Deque<TLayoutedPrimitive> *primitives)
{
    deque_for_each(primitive, *primitives)
    {
        primitivesAppend(groupCtx, primitive);
    }
}

template <typename TPrimitivesGroupContext>
void primitivesCommit(TPrimitivesGroupContext *groupCtx)
{
    glBindBuffer(GL_ARRAY_BUFFER, groupCtx->vertexAttributesHandle);
    glBufferData(
        GL_ARRAY_BUFFER,
        groupCtx->vertexAttributes.size * sizeof(decltype(*groupCtx->vertexAttributes.values)),
        groupCtx->vertexAttributes.values, GL_STATIC_DRAW);
}

template <typename TPrimitivesGroupContext, typename TPrimitivesRenderContext>
void primitivesRender(
    TPrimitivesGroupContext *groupCtx, TPrimitivesRenderContext *renderCtx, float displayViewportX,
    float displayViewportY)
{
    if (groupCtx->vertexAttributes.size == 0 || !groupCtx->isInitialized())
    {
        return;
    }

    float mat[16];
    initTransformationMatrix(mat, renderCtx->canvasCtx, displayViewportX, displayViewportY);

    glUseProgram(renderCtx->program);
    glUniformMatrix4fv(renderCtx->matPos, 1, 0, mat);
    groupCtx->setUniforms();

    glBindBuffer(GL_ARRAY_BUFFER, groupCtx->vertexAttributesHandle);
    groupCtx->setAttributes();
    glDrawArrays(GL_TRIANGLES, 0, groupCtx->vertexAttributes.size);
    
    groupCtx->unsetAttributes();
}

template <typename TPrimitivesRenderContext>
void primitivesRender(TPrimitivesRenderContext *renderCtx, float displayViewportX, float displayViewportY)
{
    primitivesRender(renderCtx, renderCtx, displayViewportX, displayViewportY);
}