#pragma once

#include "../render/render.cpp"
#include "../util/list.cpp"
#include "hitRect.cpp"

struct ThreadLayout
{
    List<LayoutedRoundedFrame> threadOutlines;
    List<LayoutedImage> images;
    List<LayoutedVisaPic> visaPics;
    List<LayoutedImage> userPics;
    List<LayoutedRect> cardBackgrounds;
    List<LayoutedRect> miscRects;
    List<LayoutedRoundedFrame> miscRoundedFrames;
    List<LayoutedIcon> icons;
    List<AtlasChar> atlasChars;

    List<HitRect> tweetHitRects;
    List<HitRect> quotedTweetHitRects;
    List<HitRect> photoHitRects;
    List<HitRect> fromPortHitRects;
    List<HitRect> toPortHitRects;

    List<LayoutedArrowTip> fromPortArrowTips;
    List<LayoutedArrowTip> toPortArrowTips;

    void init()
    {
        this->threadOutlines.init();
        this->images.init();
        this->visaPics.init();
        this->userPics.init();
        this->cardBackgrounds.init();
        this->miscRects.init();
        this->miscRoundedFrames.init();
        this->icons.init();
        this->atlasChars.init();
        this->tweetHitRects.init();
        this->quotedTweetHitRects.init();
        this->photoHitRects.init();
        this->fromPortHitRects.init();
        this->toPortHitRects.init();
        this->fromPortArrowTips.init();
        this->toPortArrowTips.init();
    }

    void free()
    {
        this->threadOutlines.free();
        this->images.free();
        this->visaPics.free();
        this->userPics.free();
        this->cardBackgrounds.free();
        this->miscRects.free();
        this->miscRoundedFrames.free();
        this->icons.free();
        this->atlasChars.free();
        this->tweetHitRects.free();
        this->quotedTweetHitRects.free();
        this->photoHitRects.free();
        this->fromPortHitRects.free();
        this->toPortHitRects.free();
        this->fromPortArrowTips.free();
        this->toPortArrowTips.free();
    }
};

struct ThreadPortTweetIds
{
    List<int64_t> fromPortTweetIds;
    List<int64_t> toPortSourceTweetIds;
    List<int64_t> toPortTargetTweetIds;

    void init()
    {
        this->fromPortTweetIds.init();
        this->toPortSourceTweetIds.init();
        this->toPortTargetTweetIds.init();
    }

    void free()
    {
        this->fromPortTweetIds.free();
        this->toPortSourceTweetIds.free();
        this->toPortTargetTweetIds.free();
    }
};