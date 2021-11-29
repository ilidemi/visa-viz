#pragma once

#include "../../util/util.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/coordinateSpace.cpp"
#include "../common/glUtil.cpp"
#include <emscripten.h>

struct RoundedCorners
{
    bool topLeft;
    bool topRight;
    bool bottomLeft;
    bool bottomRight;

    static RoundedCorners createAll()
    {
        return { true, true, true, true };
    }
    static RoundedCorners createNone()
    {
        return { false, false, false, false };
    }

    static RoundedCorners create(bool topLeft, bool topRight, bool bottomLeft, bool bottomRight)
    {
        return { topLeft, topRight, bottomLeft, bottomRight };
    }
};

struct LayoutedSemiRoundedRect
{
    CoordinateSpace coordinateSpace;
    float x, y;
    float width, height;
    RoundedCorners roundedCorners;
    float cornerRadius;
    Color color;

    void init(
        CoordinateSpace coordinateSpace, float x, float y, float width, float height,
        RoundedCorners roundedCorners, float cornerRadius, Color color)
    {
        this->coordinateSpace = coordinateSpace;
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->roundedCorners = roundedCorners;
        this->cornerRadius = cornerRadius;
        this->color = color;
    }
};

struct SemiRoundedRectRenderer
{
    GLuint program;
    GLuint posPos, sizePos, matPos, colorPos, topLeftRadiusPos, topRightRadiusPos, bottomLeftRadiusPos,
        bottomRightRadiusPos;
    GLuint uvPos;

    CanvasContext *canvasCtx;
    GLuint quad;

    void init(CanvasContext *canvasCtx, GLuint quad)
    {
        const char vertexShaderCode[] =
#include "semiRoundedRectVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        const char fragmentShaderCode[] =
#include "semiRoundedRectFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->posPos = glGetUniformLocation(this->program, "pos");
        this->sizePos = glGetUniformLocation(this->program, "size");
        this->matPos = glGetUniformLocation(this->program, "mat");
        this->colorPos = glGetUniformLocation(this->program, "color");
        this->topLeftRadiusPos = glGetUniformLocation(this->program, "topLeftRadius");
        this->topRightRadiusPos = glGetUniformLocation(this->program, "topRightRadius");
        this->bottomLeftRadiusPos = glGetUniformLocation(this->program, "bottomLeftRadius");
        this->bottomRightRadiusPos = glGetUniformLocation(this->program, "bottomRightRadius");

        this->uvPos = glGetAttribLocation(this->program, "uv");

        this->canvasCtx = canvasCtx;
        this->quad = quad;
    }

    void render(LayoutedSemiRoundedRect *semiRoundedRect, float displayViewportX, float displayViewportY)
    {
        float displayX = semiRoundedRect->x;
        float displayY = semiRoundedRect->y;

        if (semiRoundedRect->coordinateSpace == CoordinateSpace::Screen)
        {
            assertOrAbort(displayViewportX == 0, "Viewport is not applicable in screen space");
            assertOrAbort(displayViewportY == 0, "Viewport is not applicable in screen space");

            if (semiRoundedRect->x < 0)
            {
                displayX = this->canvasCtx->displayWidth + semiRoundedRect->x - semiRoundedRect->width;
            }

            if (semiRoundedRect->y < 0)
            {
                displayY = this->canvasCtx->displayHeight + semiRoundedRect->y - semiRoundedRect->height;
            }
        }

        float mat[16];
        initTransformationMatrix(mat, this->canvasCtx, displayViewportX, displayViewportY);

        glUseProgram(this->program);

        glUniform2f(this->posPos, displayX, displayY);
        glUniform2f(this->sizePos, semiRoundedRect->width, semiRoundedRect->height);
        glUniformMatrix4fv(this->matPos, 1, 0, mat);
        glUniformColor4f(this->colorPos, semiRoundedRect->color);
        glUniform1f(
            this->topLeftRadiusPos, semiRoundedRect->roundedCorners.topLeft * semiRoundedRect->cornerRadius);
        glUniform1f(
            this->topRightRadiusPos,
            semiRoundedRect->roundedCorners.topRight * semiRoundedRect->cornerRadius);
        glUniform1f(
            this->bottomLeftRadiusPos,
            semiRoundedRect->roundedCorners.bottomLeft * semiRoundedRect->cornerRadius);
        glUniform1f(
            this->bottomRightRadiusPos,
            semiRoundedRect->roundedCorners.bottomRight * semiRoundedRect->cornerRadius);

        glBindBuffer(GL_ARRAY_BUFFER, this->quad);
        glEnableVertexAttribArray(this->uvPos);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(this->uvPos);
    }
};
