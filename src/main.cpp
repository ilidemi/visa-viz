#include "frameContext.cpp"
#include "input.cpp"
#include "location.cpp"
#include "style.cpp"
#include "tweetDataLoad.cpp"
#include "util/list.cpp"
#include "util/util.cpp"
#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

EM_BOOL drawFrame(double t, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    RenderContext *renderCtx = &frameCtx->renderCtx;
    StyleConfig *styleCfg = &frameCtx->styleCfg;
    WorldLayout *worldLayout = &frameCtx->worldLayout;
    float vX = frameCtx->viewport.x;
    float vY = frameCtx->viewport.y;

    double frameStart = emscripten_get_now();

    frameCtx->shouldRerenderNextFrame |= frameCtx->shouldAlwaysRerender;
    if (frameCtx->shouldRerenderNextFrame)
    {
        clearScreen(styleCfg->backgroundColor);

        // Thread outlines
        renderCtx->threadOutlinesCtx.render(vX, vY);

        // Images
        list_for_each(image, worldLayout->allThreadsLayout.images)
        {
            renderCtx->imageRenderer.render(image, vX, vY);
        }

        // Visa pics
        primitivesRender(&renderCtx->visaPicsCtx, vX, vY);

        // User pics
        list_for_each(userPic, worldLayout->allThreadsLayout.userPics)
        {
            renderCtx->userPicRenderer.render(userPic, vX, vY);
        }

        // Card backgrounds
        primitivesRender(&renderCtx->cardBackgroundsCtx, &renderCtx->rectsRenderCtx, vX, vY);

        // Misc rects
        primitivesRender(&renderCtx->miscRectsCtx, &renderCtx->rectsRenderCtx, vX, vY);

        // Misc rounded frames
        renderCtx->miscRoundedFramesCtx.render(vX, vY);

        // Icons
        primitivesRender(&renderCtx->iconsCtx, vX, vY);

        // All text
        renderCtx->textRenderer.render(vX, vY);

        // Hover in world space
        if (inputCtx->hoveredHitRect != NULL)
        {
            HitRect *hitRect = inputCtx->hoveredHitRect;
            if (hitRect->semiRoundedRect.coordinateSpace == CoordinateSpace::World)
            {
                renderCtx->semiRoundedRectRenderer.render(&hitRect->semiRoundedRect, vX, vY);

                if (hitRect->hitAction == HitAction::ExternalTweet)
                {
                    renderCtx->imageRenderer.render(&hitRect->externalTweetArgs.icon, vX, vY);
                }
            }
        }

        // Arrow corners
        primitivesRender(&renderCtx->arrowCornersCtx, vX, vY);

        // Arrow tips
        primitivesRender(&renderCtx->arrowTipsCtx, vX, vY);

        // Random button
        renderCtx->imageRenderer.render(&worldLayout->randomButtonLayout->icon, 0, 0);

        // Gallery view
        if (frameCtx->viewType == ViewType::Gallery)
        {
            // Background
            renderCtx->fullScreenFillRenderer.render(frameCtx->galleryLayout.backgroundColor);

            // Photo
            list_for_each(layoutedImage, frameCtx->galleryLayout.layoutedImages)
            {
                renderCtx->imageRenderer.render(layoutedImage, 0, 0);
            }

            // Buttons
            list_for_each(button, frameCtx->galleryLayout.buttons)
            {
                renderCtx->circleRenderer.render(button, 0, 0);
            }

            // Icons
            list_for_each(icon, frameCtx->galleryLayout.icons)
            {
                renderCtx->imageRenderer.render(icon, 0, 0);
            }
        }

        // Hover in screen space
        if (inputCtx->hoveredHitRect != NULL)
        {
            HitRect *hitRect = inputCtx->hoveredHitRect;
            if (hitRect->semiRoundedRect.coordinateSpace == CoordinateSpace::Screen)
            {
                renderCtx->semiRoundedRectRenderer.render(&hitRect->semiRoundedRect, 0, 0);
            }
        }
    }

    double frameEnd = emscripten_get_now();

    PerfOverlay *perfOverlay = &frameCtx->perfOverlay;
    perfOverlay->frameTimes.log(frameEnd - frameStart);
    if (perfOverlay->lastFrameEndTime >= 0)
    {
        perfOverlay->betweenFrameTimes.log(frameStart - perfOverlay->lastFrameEndTime);
    }

    if (frameCtx->shouldRerenderNextFrame && perfOverlay->isVisible)
    {
        perfOverlay->render(
            &renderCtx->perfOverlayBarsCtx, &renderCtx->perfOverlayTargetsCtx, &renderCtx->rectsRenderCtx);
    }

    perfOverlay->frameTimes.advance();
    perfOverlay->betweenFrameTimes.advance();
    perfOverlay->lastFrameEndTime = frameEnd;

    frameCtx->shouldRerenderNextFrame = false;

    return EM_TRUE;
}

static FrameContext frameCtx;

extern "C"
{
    void
    imageLoadedCallback(int loadedCallbackInt, int callbackEpochInt, int userDataInt, int width, int height)
    {
        if (loadedCallbackInt != 0)
        {
            auto loadedCallback = (LoadedCallbackType)loadedCallbackInt;
            unsigned int callbackEpoch = (unsigned int)callbackEpochInt;
            void *userData = (void *)userDataInt;
            loadedCallback(callbackEpoch, userData, width, height);
        }

        frameCtx.shouldRerenderNextFrame = true;
    }
}

static InputContext inputCtx;
void inputContextReset(LocationContext *locationCtx)
{
    inputCtx.reset(locationCtx);
}

static Location location;
void locationGoToRandomThread(FrameContext *frameCtx)
{
    location.goToRandomThread(frameCtx);
}

void locationUpdateFromWorldLayout(WorldLayout *worldLayout)
{
    location.updateFromWorldLayout(worldLayout);
}

int main()
{
    setlocale(LC_ALL, "");
    srand(time(0));

    static TweetData *tweetData = getTweetData();

    static HashMap<int64_t, Thread *> tweetIdToThread;
    tweetIdToThread.init();
    static HashMap<int64_t, Tweet *> tweetIdToTweet;
    tweetIdToTweet.init();
    list_for_each(thread, tweetData->threads)
    {
        list_for_each(tweet, thread->tweets)
        {
            tweetIdToThread.insert(tweet->id, thread);
            tweetIdToTweet.insert(tweet->id, tweet);
        }
    }

    location.init(&tweetIdToThread, &tweetIdToTweet);

    User *visa;
    list_for_each(user, tweetData->users)
    {
        if (strcmp(user->screenName, "visakanv") == 0)
        {
            visa = user;
        }
    }

    frameCtx.init(tweetData, &tweetIdToThread, &tweetIdToTweet, visa, &location.ctx);

    inputCtx.init(&frameCtx, &locationGoToRandomThread, &locationUpdateFromWorldLayout);
    setInputCallbacks(&inputCtx);
    location.setLocationHashChangeCallback(&inputContextReset);

    emscripten_request_animation_frame_loop(&drawFrame, &inputCtx);
}
