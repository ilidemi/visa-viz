#pragma once

#include "../../style.cpp"
#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include <emscripten.h>

struct LayoutedArrowCorner
{
    float x, y;
    float size;
    float centerU, centerV;

    static LayoutedArrowCorner create(float x, float y, float size, float centerU, float centerV)
    {
        assertOrAbort(centerU == 0 || centerU == 1, "centerU has to be 0 or 1");
        assertOrAbort(centerV == 0 || centerV == 1, "centerV has to be 0 or 1");
        return { x, y, size, centerU, centerV };
    }
};

struct ArrowCornerVertexAttributes
{
    float arrowCornerX, arrowCornerY;
    float vertexX, vertexY;
    float centerU, centerV;

    void init(LayoutedArrowCorner *arrowCorner, QuadVertex *quadVertex)
    {
        this->arrowCornerX = arrowCorner->x;
        this->arrowCornerY = arrowCorner->y;
        this->vertexX = arrowCorner->size * quadVertex->x;
        this->vertexY = arrowCorner->size * quadVertex->y;
        this->centerU = arrowCorner->centerU;
        this->centerV = arrowCorner->centerV;
    }
};

struct ArrowCornersRenderContext
{
    GLuint program;
    GLuint matPos, colorPos, sizePos, radiusPos, thicknessPos;
    GLuint arrowCornerPosPos, vertexPosPos, centerUvPos;

    List<ArrowCornerVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    float size, radius, thickness;
    Color color;

    CanvasContext *canvasCtx;

    void init(CanvasContext *canvasCtx, StyleConfig *styleCfg)
    {
        const char vertexShaderCode[] =
#include "arrowCornersVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "arrowCornersFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");
        this->colorPos = glGetUniformLocation(this->program, "color");
        this->sizePos = glGetUniformLocation(this->program, "size");
        this->radiusPos = glGetUniformLocation(this->program, "radius");
        this->thicknessPos = glGetUniformLocation(this->program, "thickness");

        this->arrowCornerPosPos = glGetAttribLocation(this->program, "arrowCornerPos");
        this->vertexPosPos = glGetAttribLocation(this->program, "vertexPos");
        this->centerUvPos = glGetAttribLocation(this->program, "centerUv");

        this->vertexAttributes.init();
        glGenBuffers(1, &this->vertexAttributesHandle);

        this->size = styleCfg->arrowCornerSize;
        this->radius = styleCfg->arrowCornerRadius;
        this->thickness = styleCfg->arrowThickness;
        this->color = styleCfg->arrowColor;

        this->canvasCtx = canvasCtx;
    }

    bool isInitialized()
    {
        return true;
    }

    void setUniforms()
    {
        glUniformColor4f(this->colorPos, this->color);
        glUniform1f(this->sizePos, this->size);
        glUniform1f(this->radiusPos, this->radius);
        glUniform1f(this->thicknessPos, this->thickness);
    }

    void setAttributes()
    {
        GLsizei stride = sizeof(ArrowCornerVertexAttributes);

        glEnableVertexAttribArray(this->arrowCornerPosPos);
        glVertexAttribPointer(this->arrowCornerPosPos, 2, GL_FLOAT, GL_FALSE, stride, 0);

        glEnableVertexAttribArray(this->vertexPosPos);
        glVertexAttribPointer(
            this->vertexPosPos, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(2 * sizeof(float)));

        glEnableVertexAttribArray(this->centerUvPos);
        glVertexAttribPointer(
            this->centerUvPos, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(4 * sizeof(float)));
    }

    void unsetAttributes()
    {
        glDisableVertexAttribArray(this->arrowCornerPosPos);
        glDisableVertexAttribArray(this->vertexPosPos);
        glDisableVertexAttribArray(this->centerUvPos);
    }
};