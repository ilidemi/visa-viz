#pragma once

#include "layout/galleryLayout.cpp"
#include "layout/worldLayout.cpp"
#include "perfOverlay.cpp"
#include "render/render.cpp"

struct Viewport
{
    float x, y;

    void init(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
};

struct LocationContext
{
    bool isSingleTweet;
    int64_t singleTweetId;
    List<int64_t> sourceTweetIds;
};

enum class ViewType
{
    Browse,
    Gallery
};

struct FrameContext
{
    RenderContext renderCtx;
    Viewport viewport;
    StyleConfig styleCfg;
    RandomButtonLayout randomButtonLayout;
    WorldLayout worldLayout;
    PerfOverlay perfOverlay;
    GalleryLayout galleryLayout;
    TweetData *tweetData;
    HashMap<int64_t, Thread *> *tweetIdToThread;
    HashMap<int64_t, Tweet *> *tweetIdToTweet;
    User *visa;
    ViewType viewType;
    bool shouldAlwaysRerender;
    bool shouldRerenderNextFrame;

    void _initLayoutAndViewport(LocationContext *locationCtx)
    {
        this->worldLayout.init(&this->randomButtonLayout);
        float targetTweetMidY;
        bool isTargetTweetFirst;
        if (locationCtx->isSingleTweet)
        {
            isTargetTweetFirst = this->worldLayout.layoutFirstThread(
                &this->renderCtx, &this->styleCfg, this->tweetData, this->tweetIdToThread, this->visa,
                locationCtx->singleTweetId, &targetTweetMidY);
        }
        else
        {
            isTargetTweetFirst = true;
            this->worldLayout.layoutMultipleThreads(
                &this->renderCtx, &this->styleCfg, this->tweetData, this->tweetIdToThread,
                this->tweetIdToTweet, this->visa, &locationCtx->sourceTweetIds, &targetTweetMidY);
        }

        float viewportX = -(this->renderCtx.canvasCtx.displayWidth - this->styleCfg.threadWidth) / 2;
        float viewportY = isTargetTweetFirst ? -this->renderCtx.canvasCtx.displayHeight / 3
                                             : targetTweetMidY - this->renderCtx.canvasCtx.displayHeight / 2;
        this->viewport.init(viewportX, viewportY);
    }

    static void
    canvasResizedCallback(float oldWidth, float oldHeight, float newWidth, float newHeight, void *userData)
    {
        FrameContext *frameCtx = (FrameContext *)userData;
        frameCtx->viewport.x -= (newWidth - oldWidth) / 2;
        frameCtx->viewport.y -= (newHeight - oldHeight) / 2;
        if (frameCtx->viewType == ViewType::Gallery)
        {
            layoutGallery(
                &frameCtx->galleryLayout, &frameCtx->styleCfg, &frameCtx->renderCtx.imageCache,
                frameCtx->galleryLayout.photos, frameCtx->galleryLayout.currentPhotoIndex, newWidth,
                newHeight);
        }
        frameCtx->shouldRerenderNextFrame = true;
    }

    void init(
        TweetData *tweetData, HashMap<int64_t, Thread *> *tweetIdToThread,
        HashMap<int64_t, Tweet *> *tweetIdToTweet, User *visa, LocationContext *locationCtx)
    {
        double start = emscripten_get_now();
        this->tweetData = tweetData;
        this->tweetIdToThread = tweetIdToThread;
        this->tweetIdToTweet = tweetIdToTweet;
        this->visa = visa;
        this->renderCtx.init(&(canvasResizedCallback), this, &this->styleCfg, visa->profileImageUrl);
        this->randomButtonLayout.init(&this->styleCfg);
        this->_initLayoutAndViewport(locationCtx);
        this->perfOverlay.init();
        this->galleryLayout.init();
        this->viewType = ViewType::Browse;
        this->shouldAlwaysRerender = false;
        this->shouldRerenderNextFrame = true;
        double finish = emscripten_get_now();
        printf("Frame context init: %lfms\n", finish - start);
    }

    HitRect *getHitRect(float hitX, float hitY)
    {
        List<HitRect> *hitRects;

        switch (this->viewType)
        {
        case ViewType::Browse:
            hitRects = &this->worldLayout.orderedHitRects;
            break;
        case ViewType::Gallery:
            hitRects = &this->galleryLayout.hitRects;
            break;
        default:
            abortWithMessage("Unsupported view type");
        }

        list_for_each(hitRect, *hitRects)
        {
            if (hitRect->test(
                    hitX, hitY, this->viewport.x, this->viewport.y, this->renderCtx.canvasCtx.displayWidth,
                    this->renderCtx.canvasCtx.displayHeight))
            {
                return hitRect;
            }
        }

        return NULL;
    }

    void reset(LocationContext *locationCtx)
    {
        double start = emscripten_get_now();
        this->worldLayout.free();
        this->_initLayoutAndViewport(locationCtx);
        // Skip perf overlay
        this->viewType = ViewType::Browse;
        this->shouldAlwaysRerender = false;
        this->shouldRerenderNextFrame = true;
        double finish = emscripten_get_now();
        printf("Frame context reset: %lfms\n", finish - start);
    }
};