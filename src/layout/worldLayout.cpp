#pragma once

#include "../render/common/renderPrimitives.cpp"
#include "../render/render.cpp"
#include "../util/deque.cpp"
#include "../util/list.cpp"
#include "hitRect.cpp"
#include "randomButtonLayout.cpp"
#include "threadAbsoluteLayout.cpp"
#include "threadLayout.cpp"
#include "threadRelativeLayout.cpp"

struct MergedThreadLayout
{
    List<LayoutedImage> images;
    List<LayoutedImage> userPics;

    void init()
    {
        this->images.init();
        this->userPics.init();
    }

    void clear()
    {
        this->images.clear();
        this->userPics.clear();
    }

    void free()
    {
        this->images.free();
        this->userPics.free();
    }
};

struct WorldLayout
{
    // Layout for all threads
    MergedThreadLayout allThreadsLayout;

    // Layouts for each thread
    Deque<ThreadLayout> threadAbsoluteLayouts;
    Deque<ThreadPortTweetIds> threadPortTweetIds;
    Deque<int64_t> threadIds;

    // Port indices to skip
    Deque<int> fromPortSkipIndices;
    Deque<int> toPortSkipIndices;

    // Arrows between threads
    Deque<LayoutedRect> arrowStraightSegments;
    Deque<LayoutedArrowCorner> arrowCorners;
    Deque<LayoutedArrowTip> arrowTips;
    Deque<int64_t> arrowSourceTweetIds;

    // Random button layout
    RandomButtonLayout *randomButtonLayout;

    // Hit rects
    List<HitRect> orderedHitRects;

    void init(RandomButtonLayout *randomButtonLayout)
    {
        this->allThreadsLayout.init();
        this->threadAbsoluteLayouts.init();
        this->threadPortTweetIds.init();
        this->threadIds.init();
        this->arrowStraightSegments.init();
        this->arrowCorners.init();
        this->arrowTips.init();
        this->arrowSourceTweetIds.init();
        this->fromPortSkipIndices.init();
        this->toPortSkipIndices.init();
        this->randomButtonLayout = randomButtonLayout;
        this->orderedHitRects.init();
    }

    void free()
    {
        this->allThreadsLayout.free();
        for (int i = 0; i < this->threadAbsoluteLayouts.size; i++)
        {
            ThreadLayout *threadLayout =
                this->threadAbsoluteLayouts.get(this->threadAbsoluteLayouts.start + i);
            threadLayout->free();
        }
        this->threadAbsoluteLayouts.free();
        this->threadPortTweetIds.free();
        this->threadIds.free();
        this->arrowStraightSegments.free();
        this->arrowCorners.free();
        this->arrowTips.free();
        this->arrowSourceTweetIds.free();
        this->fromPortSkipIndices.free();
        this->toPortSkipIndices.free();
        this->orderedHitRects.free();
    }

    void _popAll()
    {
        for (int threadSlot = this->threadAbsoluteLayouts.size - 1; threadSlot >= 0; threadSlot--)
        {
            ThreadLayout *threadLayout =
                this->threadAbsoluteLayouts.get(this->threadAbsoluteLayouts.start + threadSlot);
            threadLayout->free();
            this->threadAbsoluteLayouts.popRight();
            ThreadPortTweetIds *threadPortTweetIds =
                this->threadPortTweetIds.get(this->threadPortTweetIds.start + threadSlot);
            threadPortTweetIds->free();
            this->threadPortTweetIds.popRight();
            this->threadIds.popRight();
            this->fromPortSkipIndices.popRight();
            this->toPortSkipIndices.popRight();
        }

        for (int arrowSlot = this->arrowTips.size - 1; arrowSlot >= 0; arrowSlot--)
        {
            this->arrowStraightSegments.popRight();
            this->arrowTips.popRight();
            this->arrowSourceTweetIds.popRight();
        }
    }

    void _popRightTillSlot(int targetThreadSlot)
    {
        for (int threadSlot = this->threadAbsoluteLayouts.start + this->threadAbsoluteLayouts.size - 1;
             threadSlot >= targetThreadSlot; threadSlot--)
        {
            ThreadLayout *threadLayout = this->threadAbsoluteLayouts.get(threadSlot);
            threadLayout->free();
            this->threadAbsoluteLayouts.popRight();
            this->threadPortTweetIds.popRight();
            this->threadIds.popRight();
            this->fromPortSkipIndices.popRight();
            this->toPortSkipIndices.popRight();
        }

        *this->fromPortSkipIndices.get(targetThreadSlot - 1) = -1;

        // Removing thread N should also remove arrow from N-1 to N
        for (int arrowSlot = this->arrowTips.start + this->arrowTips.size - 1;
             arrowSlot >= targetThreadSlot - 1; arrowSlot--)
        {
            this->arrowStraightSegments.popRight();
            this->arrowTips.popRight();
            this->arrowSourceTweetIds.popRight();
        }
    }

    void _popLeftTillSlot(int targetThreadSlot)
    {
        for (int threadSlot = this->threadAbsoluteLayouts.start; threadSlot <= targetThreadSlot; threadSlot++)
        {
            ThreadLayout *threadLayout = this->threadAbsoluteLayouts.get(threadSlot);
            threadLayout->free();
            this->threadAbsoluteLayouts.popLeft();
            this->threadPortTweetIds.popLeft();
            this->threadIds.popLeft();
            this->fromPortSkipIndices.popLeft();
            this->toPortSkipIndices.popLeft();
        }

        *this->toPortSkipIndices.get(targetThreadSlot + 1) = -1;

        // Removing thread N should also remove arrow from N to N + 1
        for (int arrowSlot = this->arrowTips.start; arrowSlot <= targetThreadSlot; arrowSlot++)
        {
            this->arrowStraightSegments.popLeft();
            this->arrowTips.popLeft();
            this->arrowSourceTweetIds.popLeft();
        }
    }

    void _commit(RenderContext *renderCtx)
    {
        this->allThreadsLayout.clear();
        this->orderedHitRects.clear();

        int threadOutlinesCount = 0;
        int imagesCount = 0;
        int visaPicsCount = 0;
        int userPicsCount = 0;
        int cardBackgroundsCount = 0;
        int rectsCount = 0;
        int roundedFramesCount = 0;
        int iconsCount = 0;
        int atlasCharsCount = 0;
        int hitRectsCount = 1;
        int arrowCornersCount = 0;
        int arrowTipsCount = 0;

        deque_for_each(threadLayout, this->threadAbsoluteLayouts)
        {
            threadOutlinesCount += threadLayout->threadOutlines.size;
            imagesCount += threadLayout->images.size;
            visaPicsCount += threadLayout->visaPics.size;
            userPicsCount += threadLayout->userPics.size;
            cardBackgroundsCount += threadLayout->cardBackgrounds.size;
            rectsCount += threadLayout->miscRects.size;
            roundedFramesCount += threadLayout->miscRoundedFrames.size;
            iconsCount += threadLayout->icons.size;
            atlasCharsCount += threadLayout->atlasChars.size;
            hitRectsCount += threadLayout->tweetHitRects.size + threadLayout->quotedTweetHitRects.size +
                threadLayout->photoHitRects.size + threadLayout->fromPortHitRects.size +
                threadLayout->toPortHitRects.size;
            arrowTipsCount += threadLayout->fromPortArrowTips.size + threadLayout->toPortArrowTips.size;
        }

        rectsCount += this->arrowStraightSegments.size;
        arrowCornersCount += this->arrowCorners.size;
        arrowTipsCount += this->arrowTips.size;

        MergedThreadLayout *allThreadsLayout = &this->allThreadsLayout;
        renderCtx->threadOutlinesCtx.reserve(threadOutlinesCount);
        allThreadsLayout->images.reserve(imagesCount);
        primitivesReserve(&renderCtx->visaPicsCtx, visaPicsCount);
        allThreadsLayout->userPics.reserve(userPicsCount);
        primitivesReserve(&renderCtx->cardBackgroundsCtx, cardBackgroundsCount);
        primitivesReserve(&renderCtx->miscRectsCtx, rectsCount);
        renderCtx->miscRoundedFramesCtx.reserve(roundedFramesCount);
        primitivesReserve(&renderCtx->iconsCtx, iconsCount);
        renderCtx->textRenderer.reserve(atlasCharsCount);
        primitivesReserve(&renderCtx->arrowCornersCtx, arrowCornersCount);
        primitivesReserve(&renderCtx->arrowTipsCtx, arrowTipsCount);
        this->orderedHitRects.reserve(hitRectsCount);

        this->orderedHitRects.append(this->randomButtonLayout->hitRect);

        for (int i = 0; i < this->threadAbsoluteLayouts.size; i++)
        {
            ThreadLayout *threadLayout =
                this->threadAbsoluteLayouts.get(this->threadAbsoluteLayouts.start + i);

            renderCtx->threadOutlinesCtx.appendRange(&threadLayout->threadOutlines);
            allThreadsLayout->images.appendRange(&threadLayout->images);
            primitivesAppendRange(&renderCtx->visaPicsCtx, &threadLayout->visaPics);
            allThreadsLayout->userPics.appendRange(&threadLayout->userPics);
            primitivesAppendRange(&renderCtx->cardBackgroundsCtx, &threadLayout->cardBackgrounds);
            primitivesAppendRange(&renderCtx->miscRectsCtx, &threadLayout->miscRects);
            renderCtx->miscRoundedFramesCtx.appendRange(&threadLayout->miscRoundedFrames);
            primitivesAppendRange(&renderCtx->iconsCtx, &threadLayout->icons);
            renderCtx->textRenderer.appendRange(&threadLayout->atlasChars);
            primitivesAppendRange(&renderCtx->arrowCornersCtx, &this->arrowCorners);

            this->orderedHitRects.appendRange(&threadLayout->photoHitRects);
            this->orderedHitRects.appendRange(&threadLayout->quotedTweetHitRects);

            int fromPortSkipIndex = *this->fromPortSkipIndices.get(this->fromPortSkipIndices.start + i);
            for (int fromPortIdx = 0; fromPortIdx < threadLayout->fromPortArrowTips.size; fromPortIdx++)
            {
                HitRect *fromPortHitRect = threadLayout->fromPortHitRects.get(fromPortIdx);
                this->orderedHitRects.append(*fromPortHitRect);

                if (fromPortIdx == fromPortSkipIndex)
                {
                    continue;
                }

                LayoutedArrowTip *fromPortArrowTip = threadLayout->fromPortArrowTips.get(fromPortIdx);
                primitivesAppend(&renderCtx->arrowTipsCtx, fromPortArrowTip);
            }

            int toPortSkipIndex = *this->toPortSkipIndices.get(this->toPortSkipIndices.start + i);
            for (int toPortIdx = 0; toPortIdx < threadLayout->toPortArrowTips.size; toPortIdx++)
            {
                HitRect *toPortHitRect = threadLayout->toPortHitRects.get(toPortIdx);
                this->orderedHitRects.append(*toPortHitRect);

                if (toPortIdx == toPortSkipIndex)
                {
                    continue;
                }

                LayoutedArrowTip *toPortArrowTip = threadLayout->toPortArrowTips.get(toPortIdx);
                primitivesAppend(&renderCtx->arrowTipsCtx, toPortArrowTip);
            }
        }

        deque_for_each(threadLayout, this->threadAbsoluteLayouts)
        {
            this->orderedHitRects.appendRange(&threadLayout->tweetHitRects);
        }

        primitivesAppendRange(&renderCtx->miscRectsCtx, &this->arrowStraightSegments);
        primitivesAppendRange(&renderCtx->arrowTipsCtx, &this->arrowTips);

        renderCtx->threadOutlinesCtx.commit();
        primitivesCommit(&renderCtx->visaPicsCtx);
        primitivesCommit(&renderCtx->cardBackgroundsCtx);
        primitivesCommit(&renderCtx->miscRectsCtx);
        renderCtx->textRenderer.commit();
        renderCtx->miscRoundedFramesCtx.commit();
        primitivesCommit(&renderCtx->iconsCtx);
        primitivesCommit(&renderCtx->arrowCornersCtx);
        primitivesCommit(&renderCtx->arrowTipsCtx);
    }

    void _filterThreadPorts(
        List<FromPort> *fromPorts, List<ToPort> *toPorts, int64_t targetThreadId,
        ThreadPortTweetIds *outThreadPortTweetIds)
    {
        outThreadPortTweetIds->init();

        list_for_each(fromPort, *fromPorts)
        {
            if (fromPort->threadId == targetThreadId)
            {
                outThreadPortTweetIds->fromPortTweetIds.append(fromPort->tweetId);
            }
        }

        for (int i = 0; i < toPorts->size; i++)
        {
            ToPort *toPort = toPorts->get(i);
            if (toPort->threadId == targetThreadId)
            {
                FromPort *fromPort = fromPorts->get(i);
                outThreadPortTweetIds->toPortSourceTweetIds.append(fromPort->tweetId);
                outThreadPortTweetIds->toPortTargetTweetIds.append(toPort->tweetId);
            }
        }
    }

    bool _layoutFirstThread(
        RenderContext *renderCtx, StyleConfig *styleCfg, TweetData *tweetData,
        HashMap<int64_t, Thread *> *tweetIdToThread, User *visa, int64_t targetTweet1Id,
        int64_t targetTweet2Id, float *outTargetTweet1MidY, float *outTargetTweet2QuotedMidY)
    {
        // Assuming target tweets belong to the same thread
        Thread *targetThread = tweetIdToThread->get(targetTweet1Id);

        this->_popAll();

        ThreadPortTweetIds *threadPortTweetIds = this->threadPortTweetIds.getNextRight();
        _filterThreadPorts(&tweetData->fromPorts, &tweetData->toPorts, targetThread->id, threadPortTweetIds);

        ThreadLayout relativeLayout;
        relativeLayoutThread(
            &renderCtx->textRenderer, styleCfg, tweetIdToThread, visa, targetThread, threadPortTweetIds,
            targetTweet1Id, targetTweet2Id, &relativeLayout, outTargetTweet1MidY, outTargetTweet2QuotedMidY);

        ThreadLayout *absoluteLayout = this->threadAbsoluteLayouts.getNextRight();
        relativeLayoutToAbsolute(&relativeLayout, 0, 0, 0, absoluteLayout);
        relativeLayout.free();

        this->threadIds.appendRight(targetThread->id);

        this->fromPortSkipIndices.appendRight(-1);
        this->toPortSkipIndices.appendRight(-1);

        return targetThread->tweets.get(0)->id == targetTweet1Id;
    }

    bool layoutFirstThread(
        RenderContext *renderCtx, StyleConfig *styleCfg, TweetData *tweetData,
        HashMap<int64_t, Thread *> *tweetIdToThread, User *visa, int64_t targetTweetId,
        float *outTargetTweetMidY)
    {
        int64_t dummyTargetTweet2Id = 0;
        bool isTargetTweetFirst = _layoutFirstThread(
            renderCtx, styleCfg, tweetData, tweetIdToThread, visa, targetTweetId, dummyTargetTweet2Id,
            outTargetTweetMidY, NULL);

        this->_commit(renderCtx);
        return isTargetTweetFirst;
    }

    void _layoutStraightArrowFromRight(
        StyleConfig *styleCfg, float fromX, float toX, float y, int64_t sourceTweetId)
    {
        assertOrAbort(toX > fromX, "Arrow must point right");

        this->arrowStraightSegments.appendRight(LayoutedRect::create(
            fromX, y - styleCfg->arrowHalfThickness, toX - fromX - styleCfg->arrowTipSize,
            styleCfg->arrowThickness, styleCfg->arrowColor));
        this->arrowTips.appendRight(LayoutedArrowTip::create(
            toX - styleCfg->arrowTipSize, y, styleCfg->arrowTipSize, styleCfg->arrowTipHeight));
        this->arrowSourceTweetIds.appendRight(sourceTweetId);
    }

    void _layoutStraightArrowFromLeft(
        StyleConfig *styleCfg, float fromX, float toX, float y, int64_t sourceTweetId)
    {
        assertOrAbort(toX > fromX, "Arrow must point right");

        this->arrowStraightSegments.appendLeft(LayoutedRect::create(
            fromX, y - styleCfg->arrowHalfThickness, toX - fromX - styleCfg->arrowTipSize,
            styleCfg->arrowThickness, styleCfg->arrowColor));
        this->arrowTips.appendLeft(LayoutedArrowTip::create(
            toX - styleCfg->arrowTipSize, y, styleCfg->arrowTipSize, styleCfg->arrowTipHeight));
        this->arrowSourceTweetIds.appendLeft(sourceTweetId);
    }

    float _setSkipPortIndicesToRight(
        RenderContext *renderCtx, ThreadPortTweetIds *threadPortTweetIds, int64_t sourceTweetId,
        int64_t targetTweetId, ThreadLayout *relativeLayout)
    {
        int targetThreadToPortSkipIndex = -1;
        float *targetToPortY = NULL;
        for (int toPortIdx = 0; toPortIdx < threadPortTweetIds->toPortTargetTweetIds.size; toPortIdx++)
        {
            int64_t toPortSourceTweetId = *threadPortTweetIds->toPortSourceTweetIds.get(toPortIdx);
            if (toPortSourceTweetId == sourceTweetId)
            {
                targetThreadToPortSkipIndex = toPortIdx;
                targetToPortY = &relativeLayout->toPortArrowTips.get(toPortIdx)->midY;
                break;
            }
        }
        assertOrAbort(targetToPortY != NULL, "Couldn't find target to port");

        this->toPortSkipIndices.appendRight(targetThreadToPortSkipIndex);
        this->fromPortSkipIndices.appendRight(-1);

        int *sourceThreadFromPortSkipIndex = this->fromPortSkipIndices.get(
            this->fromPortSkipIndices.start + this->fromPortSkipIndices.size - 2);
        ThreadPortTweetIds *sourceThreadPortTweetIds =
            this->threadPortTweetIds.get(this->threadPortTweetIds.start + this->threadPortTweetIds.size - 2);
        List<int64_t> *sourceThreadFromPortTweetIds = &sourceThreadPortTweetIds->fromPortTweetIds;
        for (int fromPortIdx = 0; fromPortIdx < sourceThreadFromPortTweetIds->size; fromPortIdx++)
        {
            int64_t fromPortTweetId = *sourceThreadFromPortTweetIds->get(fromPortIdx);
            if (fromPortTweetId == sourceTweetId)
            {
                *sourceThreadFromPortSkipIndex = fromPortIdx;
                break;
            }
        }

        return *targetToPortY;
    }

    void _layoutNextThreadToRight(
        RenderContext *renderCtx, StyleConfig *styleCfg, TweetData *tweetData,
        HashMap<int64_t, Thread *> *tweetIdToThread, User *visa, int64_t sourceTweetId, int64_t targetTweetId,
        int sourceThreadSlot, int targetThreadSlot, float sourcePortMidY, int64_t targetTweet1Id,
        float *outTargetTweet1QuotedMidY)
    {
        Thread *targetThread = tweetIdToThread->get(targetTweetId);

        if (targetThreadSlot < this->threadAbsoluteLayouts.size)
        {
            this->_popRightTillSlot(targetThreadSlot);
        }

        ThreadPortTweetIds *threadPortTweetIds = this->threadPortTweetIds.getNextRight();
        this->_filterThreadPorts(
            &tweetData->fromPorts, &tweetData->toPorts, targetThread->id, threadPortTweetIds);

        ThreadLayout relativeLayout;
        int64_t dummyTargetTweet1Id = 0;
        float targetTweet1QuotedRelativeMidY;
        relativeLayoutThread(
            &renderCtx->textRenderer, styleCfg, tweetIdToThread, visa, targetThread, threadPortTweetIds,
            dummyTargetTweet1Id, targetTweet1Id, &relativeLayout, NULL, &targetTweet1QuotedRelativeMidY);

        float targetToPortY = _setSkipPortIndicesToRight(
            renderCtx, threadPortTweetIds, sourceTweetId, targetTweetId, &relativeLayout);

        float targetThreadX = targetThreadSlot * (styleCfg->threadWidth + styleCfg->threadGap);
        float targetThreadY = sourcePortMidY - targetToPortY;
        *outTargetTweet1QuotedMidY = targetTweet1QuotedRelativeMidY + targetThreadY;

        ThreadLayout *absoluteLayout = this->threadAbsoluteLayouts.getNextRight();
        relativeLayoutToAbsolute(
            &relativeLayout, targetThreadX, targetThreadY, targetThreadSlot, absoluteLayout);
        relativeLayout.free();

        this->threadIds.appendRight(targetThread->id);

        this->_layoutStraightArrowFromRight(
            styleCfg, targetThreadX - styleCfg->threadGap, targetThreadX, sourcePortMidY, sourceTweetId);
    }

    void toggleThreadToRight(
        RenderContext *renderCtx, StyleConfig *styleCfg, TweetData *tweetData,
        HashMap<int64_t, Thread *> *tweetIdToThread, User *visa, ToggleThreadArgs *toggleThreadArgs,
        float sourcePortMidY)
    {
        if (toggleThreadArgs->targetThreadSlot <
                this->threadAbsoluteLayouts.start + this->threadAbsoluteLayouts.size &&
            *this->arrowSourceTweetIds.get(toggleThreadArgs->sourceThreadSlot) ==
                toggleThreadArgs->sourceTweetId)
        {
            this->_popRightTillSlot(toggleThreadArgs->targetThreadSlot);
        }
        else
        {
            int64_t dummyTargetTweetId1 = 0;
            this->_layoutNextThreadToRight(
                renderCtx, styleCfg, tweetData, tweetIdToThread, visa, toggleThreadArgs->sourceTweetId,
                toggleThreadArgs->targetTweetId, toggleThreadArgs->sourceThreadSlot,
                toggleThreadArgs->targetThreadSlot, sourcePortMidY, dummyTargetTweetId1, NULL);
        }

        this->_commit(renderCtx);
    }

    void layoutMultipleThreads(
        RenderContext *renderCtx, StyleConfig *styleCfg, TweetData *tweetData,
        HashMap<int64_t, Thread *> *tweetIdToThread, HashMap<int64_t, Tweet *> *tweetIdToTweet, User *visa,
        List<int64_t> *sourceTweetIds, float *outFirstTweetMidY)
    {
        int64_t firstSourceTweetId = *sourceTweetIds->get(0);
        int64_t firstThreadTweetId = tweetIdToThread->get(firstSourceTweetId)->tweets.get(0)->id;
        float sourceTweetQuotedMidY;
        this->_layoutFirstThread(
            renderCtx, styleCfg, tweetData, tweetIdToThread, visa, firstThreadTweetId, firstSourceTweetId,
            outFirstTweetMidY, &sourceTweetQuotedMidY);

        for (int i = 0; i < sourceTweetIds->size; i++)
        {
            int64_t sourceTweetId = *sourceTweetIds->get(i);
            Tweet *sourceTweet = tweetIdToTweet->get(sourceTweetId);
            int64_t nextSourceTweetId = (i + 1) < sourceTweetIds->size ? *sourceTweetIds->get(i + 1) : 0;
            float nextSourceTweetQuotedMidY;
            this->_layoutNextThreadToRight(
                renderCtx, styleCfg, tweetData, tweetIdToThread, visa, sourceTweetId,
                sourceTweet->quotedTweet->id, i, i + 1, sourceTweetQuotedMidY, nextSourceTweetId,
                &nextSourceTweetQuotedMidY);
            sourceTweetQuotedMidY = nextSourceTweetQuotedMidY;
        }

        this->_commit(renderCtx);
    }

    float _setSkipPortIndicesToLeft(
        RenderContext *renderCtx, ThreadPortTweetIds *threadPortTweetIds, int64_t sourceTweetId,
        int64_t targetTweetId, ThreadLayout *relativeLayout)
    {
        int targetThreadFromPortSkipIndex = -1;
        float *targetFromPortY = NULL;
        for (int fromPortIdx = 0; fromPortIdx < threadPortTweetIds->fromPortTweetIds.size; fromPortIdx++)
        {
            int64_t fromPortTweetId = *threadPortTweetIds->fromPortTweetIds.get(fromPortIdx);
            if (fromPortTweetId == targetTweetId)
            {
                targetThreadFromPortSkipIndex = fromPortIdx;
                targetFromPortY = &relativeLayout->fromPortArrowTips.get(fromPortIdx)->midY;
                break;
            }
        }
        assertOrAbort(targetFromPortY != NULL, "Couldn't find target to port");

        this->toPortSkipIndices.appendLeft(-1);
        this->fromPortSkipIndices.appendLeft(targetThreadFromPortSkipIndex);

        int *sourceThreadToPortSkipIndex = this->toPortSkipIndices.get(this->toPortSkipIndices.start + 1);
        ThreadPortTweetIds *sourceThreadPortTweetIds =
            this->threadPortTweetIds.get(this->threadPortTweetIds.start + 1);
        List<int64_t> *sourceThreadToPortTweetIds = &sourceThreadPortTweetIds->toPortSourceTweetIds;
        for (int toPortIdx = 0; toPortIdx < sourceThreadToPortTweetIds->size; toPortIdx++)
        {
            int64_t toPortSourceTweetId = *sourceThreadToPortTweetIds->get(toPortIdx);
            if (toPortSourceTweetId == targetTweetId)
            {
                *sourceThreadToPortSkipIndex = toPortIdx;
                break;
            }
        }

        return *targetFromPortY;
    }

    void _layoutNextThreadToLeft(
        RenderContext *renderCtx, StyleConfig *styleCfg, TweetData *tweetData,
        HashMap<int64_t, Thread *> *tweetIdToThread, User *visa, int64_t sourceTweetId, int64_t targetTweetId,
        int sourceThreadSlot, int targetThreadSlot, float sourcePortMidY)
    {
        Thread *targetThread = tweetIdToThread->get(targetTweetId);

        if (targetThreadSlot < this->threadAbsoluteLayouts.size)
        {
            this->_popLeftTillSlot(targetThreadSlot);
        }

        ThreadPortTweetIds *threadPortTweetIds = this->threadPortTweetIds.getNextLeft();
        _filterThreadPorts(&tweetData->fromPorts, &tweetData->toPorts, targetThread->id, threadPortTweetIds);

        ThreadLayout relativeLayout;
        int64_t dummyTargetTweet1Id = 0;
        int64_t dummyTargetTweet2Id = 0;
        relativeLayoutThread(
            &renderCtx->textRenderer, styleCfg, tweetIdToThread, visa, targetThread, threadPortTweetIds,
            dummyTargetTweet1Id, dummyTargetTweet2Id, &relativeLayout, NULL, NULL);

        float targetFromPortY = this->_setSkipPortIndicesToLeft(
            renderCtx, threadPortTweetIds, sourceTweetId, targetTweetId, &relativeLayout);

        float targetThreadX = targetThreadSlot * (styleCfg->threadWidth + styleCfg->threadGap);
        float targetThreadY = sourcePortMidY - targetFromPortY;

        ThreadLayout *absoluteLayout = this->threadAbsoluteLayouts.getNextLeft();
        relativeLayoutToAbsolute(
            &relativeLayout, targetThreadX, targetThreadY, targetThreadSlot, absoluteLayout);
        relativeLayout.free();

        this->threadIds.appendLeft(targetThread->id);

        _layoutStraightArrowFromLeft(
            styleCfg, targetThreadX + styleCfg->threadWidth,
            targetThreadX + styleCfg->threadWidth + styleCfg->threadGap, sourcePortMidY, targetTweetId);
    }

    void toggleThreadToLeft(
        RenderContext *renderCtx, StyleConfig *styleCfg, TweetData *tweetData,
        HashMap<int64_t, Thread *> *tweetIdToThread, User *visa, ToggleThreadArgs *toggleThreadArgs,
        float sourcePortMidY)
    {
        if (toggleThreadArgs->targetThreadSlot >= this->threadAbsoluteLayouts.start &&
            *this->arrowSourceTweetIds.get(toggleThreadArgs->targetThreadSlot) ==
                toggleThreadArgs->targetTweetId)
        {
            this->_popLeftTillSlot(toggleThreadArgs->targetThreadSlot);
        }
        else
        {
            this->_layoutNextThreadToLeft(
                renderCtx, styleCfg, tweetData, tweetIdToThread, visa, toggleThreadArgs->sourceTweetId,
                toggleThreadArgs->targetTweetId, toggleThreadArgs->sourceThreadSlot,
                toggleThreadArgs->targetThreadSlot, sourcePortMidY);
        }

        this->_commit(renderCtx);
    }
};
