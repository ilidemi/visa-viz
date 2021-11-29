#pragma once

#include "../common/canvasContext.cpp"
#include "../common/glUtil.cpp"
#include "../common/imageCache.cpp"
#include "../common/layoutedImage.cpp"
#include <emscripten.h>

struct ImageRenderer
{
    GLuint program;
    GLuint posPos, sizePos, matPos;
    GLuint textureUvPos;

    ImageCache *imageCache;
    CanvasContext *canvasCtx;
    GLuint quad;

    void init(
        ImageCache *imageCache, CanvasContext *canvasCtx, GLuint vertexShader, GLuint fragmentShader,
        GLuint quad)
    {
        this->program = createProgram(vertexShader, fragmentShader);

        this->posPos = glGetUniformLocation(this->program, "pos");
        this->sizePos = glGetUniformLocation(this->program, "size");
        this->matPos = glGetUniformLocation(this->program, "mat");

        this->textureUvPos = glGetAttribLocation(this->program, "textureUv");

        this->imageCache = imageCache;
        this->canvasCtx = canvasCtx;
        this->quad = quad;
    }

    void render(LayoutedImage *image, float displayViewportX, float displayViewportY)
    {
        Texture *t = this->imageCache->textureByUrl(image->url);
        if (!t->isInitialized())
        {
            return;
        }

        float displayX = image->x;
        float displayY = image->y;

        if (image->coordinateSpace == CoordinateSpace::Screen)
        {
            assertOrAbort(displayViewportX == 0, "Viewport is not applicable in screen space");
            assertOrAbort(displayViewportY == 0, "Viewport is not applicable in screen space");

            if (image->x < 0)
            {
                displayX = this->canvasCtx->displayWidth + image->x - image->width;
            }

            if (image->y < 0)
            {
                displayY = this->canvasCtx->displayHeight + image->y - image->height;
            }
        }

        if (displayX - displayViewportX > this->canvasCtx->displayWidth ||
            (displayX + image->width - displayViewportX) < 0 ||
            displayY - displayViewportY > this->canvasCtx->displayHeight ||
            (displayY + image->height - displayViewportY) < 0)
        {
            return;
        }

        float mat[16];
        initTransformationMatrix(mat, this->canvasCtx, displayViewportX, displayViewportY);

        glUseProgram(this->program);

        glUniform2f(this->posPos, displayX, displayY);
        glUniform2f(this->sizePos, image->width, image->height);
        glUniformMatrix4fv(this->matPos, 1, 0, mat);
        
        glBindTexture(GL_TEXTURE_2D, t->texture);
        
        glBindBuffer(GL_ARRAY_BUFFER, this->quad);
        glEnableVertexAttribArray(this->textureUvPos);
        glVertexAttribPointer(this->textureUvPos, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(this->textureUvPos);
    }
};
