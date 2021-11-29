#pragma once

#include "../util/list.cpp"
#include "threadLayout.cpp"

HitRect hitRectToAbsolute(HitRect *hitRect, int threadSlot, float threadX, float threadY)
{
    HitRect absoluteHitRect;
    float absoluteX = threadX + hitRect->semiRoundedRect.x;
    float absoluteY = threadY + hitRect->semiRoundedRect.y;
    switch (hitRect->hitAction)
    {
    case HitAction::ToggleThread:
        absoluteHitRect.initToggleThreadHitRect(
            absoluteX, absoluteY, hitRect->semiRoundedRect.width, hitRect->semiRoundedRect.height,
            hitRect->semiRoundedRect.roundedCorners, hitRect->semiRoundedRect.cornerRadius,
            hitRect->semiRoundedRect.color, hitRect->toggleThreadArgs.sourceTweetId,
            hitRect->toggleThreadArgs.targetTweetId, threadSlot + hitRect->toggleThreadArgs.sourceThreadSlot,
            threadSlot + hitRect->toggleThreadArgs.targetThreadSlot);
        break;
    case HitAction::ExternalTweet:
    {
        float absoluteIconX = threadX + hitRect->externalTweetArgs.icon.x;
        float absoluteIconY = threadY + hitRect->externalTweetArgs.icon.y;
        absoluteHitRect.initExternalTweetHitRect(
            absoluteX, absoluteY, hitRect->semiRoundedRect.width, hitRect->semiRoundedRect.height,
            hitRect->semiRoundedRect.roundedCorners, hitRect->semiRoundedRect.cornerRadius,
            hitRect->semiRoundedRect.color, hitRect->externalTweetArgs.authorScreenName,
            hitRect->externalTweetArgs.targetTweetId, absoluteIconX, absoluteIconY,
            hitRect->externalTweetArgs.icon.width, hitRect->externalTweetArgs.icon.url);
        break;
    }
    case HitAction::OpenGallery:
        absoluteHitRect.initOpenGalleryHitRect(
            absoluteX, absoluteY, hitRect->semiRoundedRect.width, hitRect->semiRoundedRect.height,
            hitRect->semiRoundedRect.roundedCorners, hitRect->semiRoundedRect.cornerRadius,
            hitRect->semiRoundedRect.color, hitRect->openGalleryArgs.photos,
            hitRect->openGalleryArgs.startPhotoIndex);
        break;
    default:
        abortWithMessage("Unsupported hit action");
    }
    return absoluteHitRect;
}

void relativeLayoutToAbsolute(
    ThreadLayout *relativeLayout, float threadX, float threadY, int threadSlot,
    ThreadLayout *outAbsoluteLayout)
{
    outAbsoluteLayout->init();
    outAbsoluteLayout->threadOutlines.reserve(relativeLayout->threadOutlines.size);
    outAbsoluteLayout->images.reserve(relativeLayout->images.size);
    outAbsoluteLayout->userPics.reserve(relativeLayout->userPics.size);
    outAbsoluteLayout->cardBackgrounds.reserve(relativeLayout->cardBackgrounds.size);
    outAbsoluteLayout->miscRects.reserve(relativeLayout->miscRects.size);
    outAbsoluteLayout->miscRoundedFrames.reserve(relativeLayout->miscRoundedFrames.size);
    outAbsoluteLayout->icons.reserve(relativeLayout->icons.size);
    outAbsoluteLayout->atlasChars.reserve(relativeLayout->atlasChars.size);
    outAbsoluteLayout->quotedTweetHitRects.reserve(relativeLayout->quotedTweetHitRects.size);
    outAbsoluteLayout->fromPortHitRects.reserve(relativeLayout->fromPortHitRects.size);
    outAbsoluteLayout->toPortHitRects.reserve(relativeLayout->toPortHitRects.size);
    outAbsoluteLayout->fromPortArrowTips.reserve(relativeLayout->fromPortArrowTips.size);
    outAbsoluteLayout->toPortArrowTips.reserve(relativeLayout->toPortArrowTips.size);

    list_for_each(threadOutline, relativeLayout->threadOutlines)
    {
        outAbsoluteLayout->threadOutlines.append(LayoutedRoundedFrame::create(
            threadX + threadOutline->x, threadY + threadOutline->y, threadOutline->width,
            threadOutline->height, threadOutline->radius, threadOutline->borderSize,
            threadOutline->insideColor, threadOutline->borderColor, threadOutline->outsideColor));
    }

    list_for_each(image, relativeLayout->images)
    {
        outAbsoluteLayout->images.getNext()->init(
            CoordinateSpace::World, threadX + image->x, threadY + image->y, image->width, image->height,
            image->url);
    }

    list_for_each(visaPic, relativeLayout->visaPics)
    {
        outAbsoluteLayout->visaPics.append(
            LayoutedVisaPic::create(threadX + visaPic->x, threadY + visaPic->y, visaPic->size));
    }

    list_for_each(userPic, relativeLayout->userPics)
    {
        outAbsoluteLayout->userPics.getNext()->init(
            CoordinateSpace::World, threadX + userPic->x, threadY + userPic->y, userPic->width,
            userPic->height, userPic->url);
    }

    list_for_each(cardBackground, relativeLayout->cardBackgrounds)
    {
        outAbsoluteLayout->cardBackgrounds.append(LayoutedRect::create(
            threadX + cardBackground->x, threadY + cardBackground->y, cardBackground->width,
            cardBackground->height, cardBackground->color));
    }

    list_for_each(rect, relativeLayout->miscRects)
    {
        outAbsoluteLayout->miscRects.append(LayoutedRect::create(
            threadX + rect->x, threadY + rect->y, rect->width, rect->height, rect->color));
    }

    list_for_each(roundedFrame, relativeLayout->miscRoundedFrames)
    {
        outAbsoluteLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
            threadX + roundedFrame->x, threadY + roundedFrame->y, roundedFrame->width, roundedFrame->height,
            roundedFrame->radius, roundedFrame->borderSize, roundedFrame->insideColor,
            roundedFrame->borderColor, roundedFrame->outsideColor));
    }

    list_for_each(icon, relativeLayout->icons)
    {
        outAbsoluteLayout->icons.append(
            LayoutedIcon::create(threadX + icon->x, threadY + icon->y, icon->size, icon->type));
    }

    list_for_each(atlasChar, relativeLayout->atlasChars)
    {
        AtlasChar *absoluteAtlasChar = outAbsoluteLayout->atlasChars.getNext();
        absoluteAtlasChar->init(
            threadX + atlasChar->displayX, threadY + atlasChar->displayMidBaselineY, atlasChar->glyphId);
    }

    list_for_each(tweetHitRect, relativeLayout->tweetHitRects)
    {
        outAbsoluteLayout->tweetHitRects.append(
            hitRectToAbsolute(tweetHitRect, threadSlot, threadX, threadY));
    }

    list_for_each(quotedTweetHitRect, relativeLayout->quotedTweetHitRects)
    {
        outAbsoluteLayout->quotedTweetHitRects.append(
            hitRectToAbsolute(quotedTweetHitRect, threadSlot, threadX, threadY));
    }

    list_for_each(photoHitRect, relativeLayout->photoHitRects)
    {
        outAbsoluteLayout->photoHitRects.append(
            hitRectToAbsolute(photoHitRect, threadSlot, threadX, threadY));
    }

    list_for_each(fromPortHitRect, relativeLayout->fromPortHitRects)
    {
        outAbsoluteLayout->fromPortHitRects.append(
            hitRectToAbsolute(fromPortHitRect, threadSlot, threadX, threadY));
    }

    list_for_each(toPortHitRect, relativeLayout->toPortHitRects)
    {
        outAbsoluteLayout->toPortHitRects.append(
            hitRectToAbsolute(toPortHitRect, threadSlot, threadX, threadY));
    }

    list_for_each(arrowTip, relativeLayout->fromPortArrowTips)
    {
        outAbsoluteLayout->fromPortArrowTips.append(LayoutedArrowTip::create(
            threadX + arrowTip->x, threadY + arrowTip->midY, arrowTip->width, arrowTip->height));
    }

    list_for_each(arrowTip, relativeLayout->toPortArrowTips)
    {
        outAbsoluteLayout->toPortArrowTips.append(LayoutedArrowTip::create(
            threadX + arrowTip->x, threadY + arrowTip->midY, arrowTip->width, arrowTip->height));
    }
}