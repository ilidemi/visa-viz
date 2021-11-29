#pragma once

#include "../style.cpp"
#include "../util/list.cpp"
#include "../util/util.cpp"
#include "arrows/renderArrowCorners.cpp"
#include "arrows/renderArrowTips.cpp"
#include "circle/renderCircle.cpp"
#include "common/canvasContext.cpp"
#include "common/glUtil.cpp"
#include "common/imageCache.cpp"
#include "fullScreenFill/renderFullScreenFill.cpp"
#include "icons/renderIcons.cpp"
#include "image/renderImage.cpp"
#include "rects/renderRects.cpp"
#include "roundedFrames/renderRoundedFrames.cpp"
#include "semiRoundedRect/renderSemiRoundedRect.cpp"
#include "text/renderText.cpp"
#include "userPic/renderUserPic.cpp"
#include "visaPics/renderVisaPics.cpp"
#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <webgl/webgl2.h>

extern "C"
{
    void _jsWebGlFailedFallback();
}

typedef void (*CanvasResizedCallback)(float, float, float, float, void *);

struct RenderContext
{
    CanvasContext canvasCtx;
    CanvasResizedCallback canvasResizedCallback;
    void *canvasResizedCallbackUserData;

    RoundedFramesRenderContext roundedFramesRenderCtx;
    RectsRenderContext rectsRenderCtx;

    // In the order of drawing
    RoundedFramesGroupContextV2 threadOutlinesCtx;
    ImageRenderer imageRenderer;
    VisaPicsRenderContext visaPicsCtx;
    UserPicRenderer userPicRenderer;
    RectsGroupContext cardBackgroundsCtx;
    RectsGroupContext miscRectsCtx;
    RoundedFramesGroupContextV2 miscRoundedFramesCtx;
    IconsRenderContext iconsCtx;
    TextRenderer textRenderer;
    SemiRoundedRectRenderer semiRoundedRectRenderer;
    ArrowCornersRenderContext arrowCornersCtx;
    ArrowTipsRenderContext arrowTipsCtx;
    FullScreenFillRenderer fullScreenFillRenderer;
    CircleRenderer circleRenderer;
    RectsGroupContext perfOverlayBarsCtx;
    RectsGroupContext perfOverlayTargetsCtx;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE glContext;
    bool isWebGl2;
    GLuint quad;

    ImageCache imageCache;

    static EM_BOOL _canvasResizedCallback(int eventType, const void *reserved, void *userData)
    {
        RenderContext *renderCtx = (RenderContext *)userData;
        float oldDisplayWidth = renderCtx->canvasCtx.displayWidth;
        float oldDisplayHeight = renderCtx->canvasCtx.displayHeight;
        renderCtx->canvasCtx.init();
        renderCtx->textRenderer.update();

        if (renderCtx->canvasResizedCallback != NULL)
        {
            renderCtx->canvasResizedCallback(
                oldDisplayWidth, oldDisplayHeight, renderCtx->canvasCtx.displayWidth,
                renderCtx->canvasCtx.displayHeight, renderCtx->canvasResizedCallbackUserData);
        }

        return EM_TRUE;
    }

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE _initGlContext(bool *outIsWebGl2)
    {
        EmscriptenWebGLContextAttributes attrs;
        emscripten_webgl_init_context_attributes(&attrs);
        attrs.antialias = EM_FALSE;
        attrs.alpha = EM_TRUE;
        attrs.depth = EM_TRUE;
        attrs.premultipliedAlpha = EM_TRUE;
        attrs.preserveDrawingBuffer = EM_TRUE;
        attrs.majorVersion = 2;
        *outIsWebGl2 = true;
        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE glContext = emscripten_webgl_create_context(canvasSelector, &attrs);
        if (glContext == NULL)
        {
            printf("WebGL 2 initialization failed, falling back to WebGL 1\n");
            attrs.majorVersion = 1;
            *outIsWebGl2 = false;
            glContext = emscripten_webgl_create_context(canvasSelector, &attrs);
        }
        else
        {
            printf("WebGL 2 initialized\n");
        }
        return glContext;
    }

    void init(
        CanvasResizedCallback canvasResizedCallback, void *canvasResizedCallbackUserData,
        StyleConfig *styleCfg, const char *visaPicUrl)
    {
        EmscriptenFullscreenStrategy fullscreenStrategy;
        memset(&fullscreenStrategy, 0, sizeof(fullscreenStrategy));
        fullscreenStrategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
        fullscreenStrategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_HIDEF;
        fullscreenStrategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_NEAREST;
        fullscreenStrategy.canvasResizedCallback = &_canvasResizedCallback;
        fullscreenStrategy.canvasResizedCallbackUserData = this;
        EMSCRIPTEN_RESULT softFullscreenResult =
            emscripten_enter_soft_fullscreen(canvasSelector, &fullscreenStrategy);
        if (softFullscreenResult != EMSCRIPTEN_RESULT_SUCCESS)
        {
            printf("Soft fullscreen request failed: %i\n", softFullscreenResult);
        }

        this->canvasCtx.init();
        this->canvasResizedCallback = canvasResizedCallback;
        this->canvasResizedCallbackUserData = canvasResizedCallbackUserData;

        this->glContext = _initGlContext(&this->isWebGl2);
        if (this->glContext == NULL)
        {
            _jsWebGlFailedFallback();
            abortWithMessage("Couldn't initialize GL context");
        }
        emscripten_webgl_make_context_current(this->glContext);

        this->imageCache.init(this->isWebGl2);

        const char vertexShaderCode[] =
#include "vertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);

        const char texturedRectFragmentShaderCode[] =
#include "texturedRectFragmentShader.glsl"
            GLuint texturedRectFragmentShader =
                compileShader(GL_FRAGMENT_SHADER, texturedRectFragmentShaderCode);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // 4 vertex quad
        glGenBuffers(1, &this->quad);
        glBindBuffer(GL_ARRAY_BUFFER, this->quad);
        const float quadValues[] = { 0, 0, 1, 0, 0, 1, 1, 1 };
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadValues), quadValues, GL_STATIC_DRAW);

        this->rectsRenderCtx.init(&this->canvasCtx);
        this->roundedFramesRenderCtx.init(&this->rectsRenderCtx, &this->canvasCtx);
        this->circleRenderer.init(&this->canvasCtx, vertexShader, this->quad);
        this->threadOutlinesCtx.init(&this->roundedFramesRenderCtx);
        this->imageRenderer.init(
            &this->imageCache, &this->canvasCtx, vertexShader, texturedRectFragmentShader, this->quad);
        this->visaPicsCtx.init(&this->canvasCtx, this->isWebGl2, visaPicUrl);
        this->userPicRenderer.init(&this->imageCache, &this->canvasCtx, vertexShader, this->quad);
        this->cardBackgroundsCtx.init(&this->rectsRenderCtx);
        this->miscRectsCtx.init(&this->rectsRenderCtx);
        this->miscRoundedFramesCtx.init(&this->roundedFramesRenderCtx);
        this->iconsCtx.init(&this->canvasCtx, this->isWebGl2, texturedRectFragmentShader);
        this->textRenderer.init(&this->canvasCtx, this->isWebGl2, texturedRectFragmentShader);
        this->semiRoundedRectRenderer.init(&this->canvasCtx, this->quad);
        this->arrowCornersCtx.init(&this->canvasCtx, styleCfg);
        this->arrowTipsCtx.init(&this->canvasCtx, styleCfg);
        this->fullScreenFillRenderer.init(quad);
        this->perfOverlayBarsCtx.init(&this->rectsRenderCtx);
        this->perfOverlayTargetsCtx.init(&this->rectsRenderCtx);

        glEnableVertexAttribArray(0);
    }
};

void clearScreen(Color color)
{
    GlColor glColor = GlColor::create(color);
    glClearColor(glColor.r, glColor.g, glColor.b, glColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}