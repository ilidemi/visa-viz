#pragma once

#include "../../util/util.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/coordinateSpace.cpp"
#include "../common/glUtil.cpp"
#include <emscripten.h>

struct LayoutedCircle
{
    CoordinateSpace coordinateSpace;
    float x, y;
    float size;
    Color color;

    static LayoutedCircle createScreen(float x, float y, float size, Color color)
    {
        return { CoordinateSpace::Screen, x, y, size, color };
    }
};

struct CircleRenderer
{
    GLuint program;
    GLuint posPos, sizePos, matPos, colorPos;
    GLuint textureUvPos;

    CanvasContext *canvasCtx;
    GLuint quad;

    void init(CanvasContext *canvasCtx, GLuint vertexShader, GLuint quad)
    {
        const char fragmentShaderCode[] =
#include "circleFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->posPos = glGetUniformLocation(this->program, "pos");
        this->sizePos = glGetUniformLocation(this->program, "size");
        this->matPos = glGetUniformLocation(this->program, "mat");
        this->colorPos = glGetUniformLocation(this->program, "color");

        this->textureUvPos = glGetAttribLocation(this->program, "textureUv");

        this->canvasCtx = canvasCtx;
        this->quad = quad;
    }

    void render(LayoutedCircle *circle, float displayViewportX, float displayViewportY)
    {
        float displayX = circle->x;
        float displayY = circle->y;

        if (circle->coordinateSpace == CoordinateSpace::Screen)
        {
            assertOrAbort(displayViewportX == 0, "Viewport is not applicable in screen space");
            assertOrAbort(displayViewportY == 0, "Viewport is not applicable in screen space");

            if (circle->x < 0)
            {
                displayX = this->canvasCtx->displayWidth + circle->x - circle->size;
            }

            if (circle->y < 0)
            {
                displayY = this->canvasCtx->displayHeight + circle->y - circle->size;
            }
        }

        float mat[16];
        initTransformationMatrix(mat, this->canvasCtx, displayViewportX, displayViewportY);

        glUseProgram(this->program);

        glUniform2f(this->posPos, displayX, displayY);
        glUniform2f(this->sizePos, circle->size, circle->size);
        glUniformMatrix4fv(this->matPos, 1, 0, mat);
        glUniformColor4f(this->colorPos, circle->color);

        glBindBuffer(GL_ARRAY_BUFFER, this->quad);
        glEnableVertexAttribArray(this->textureUvPos);
        glVertexAttribPointer(this->textureUvPos, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(this->textureUvPos);
    }
};
