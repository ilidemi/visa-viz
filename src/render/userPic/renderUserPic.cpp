#pragma once

#include "../../util/util.cpp"
#include "../common/canvasContext.cpp"
#include "../common/glUtil.cpp"
#include "../common/imageCache.cpp"
#include <emscripten.h>

struct UserPicRenderer
{
    GLuint program;
    GLuint posPos, sizePos, matPos;
    GLuint textureUvPos;

    ImageCache *imageCache;
    CanvasContext *canvasCtx;
    GLuint quad;

    void init(ImageCache *imageCache, CanvasContext *canvasCtx, GLuint vertexShader, GLuint quad)
    {

        const char fragmentShaderCode[] =
#include "userPicFragmentShader.glsl"
            GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderCode);
        this->program = createProgram(vertexShader, fragmentShader);

        this->posPos = glGetUniformLocation(this->program, "pos");
        this->sizePos = glGetUniformLocation(this->program, "size");
        this->matPos = glGetUniformLocation(this->program, "mat");

        this->textureUvPos = glGetAttribLocation(this->program, "textureUv");

        this->imageCache = imageCache;
        this->canvasCtx = canvasCtx;
        this->quad = quad;
    }

    void render(LayoutedImage *userPic, float displayViewportX, float displayViewportY)
    {
        assertOrAbort(userPic->width == userPic->height, "User pic should be square");
        Texture *t = this->imageCache->textureByUrl(userPic->url);
        if (!t->isInitialized())
        {
            return;
        }

        float displaySize = userPic->width;
        if (userPic->x - displayViewportX > this->canvasCtx->displayWidth ||
            (userPic->x + displaySize - displayViewportX) < 0 ||
            userPic->y - displayViewportY > this->canvasCtx->displayHeight ||
            (userPic->y + displaySize - displayViewportY) < 0)
        {
            return;
        }

        float mat[16];
        initTransformationMatrix(mat, this->canvasCtx, displayViewportX, displayViewportY);

        glUseProgram(this->program);

        glUniform2f(this->posPos, userPic->x, userPic->y);
        glUniform2f(this->sizePos, displaySize, displaySize);
        glUniformMatrix4fv(this->matPos, 1, 0, mat);

        glBindTexture(GL_TEXTURE_2D, t->texture);

        glBindBuffer(GL_ARRAY_BUFFER, this->quad);
        glEnableVertexAttribArray(this->textureUvPos);
        glVertexAttribPointer(this->textureUvPos, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glDisableVertexAttribArray(this->textureUvPos);
    }
};
