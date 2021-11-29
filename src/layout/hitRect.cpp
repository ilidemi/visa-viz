#pragma once

#include "../render/common/color.cpp"
#include "../render/common/layoutedImage.cpp"
#include "../render/semiRoundedRect/renderSemiRoundedRect.cpp"
#include "../tweetSchema.cpp"
#include "../util/util.cpp"
#include <stdint.h>

enum class HoverAction
{
    DoNothing,
    GalleryCloseHover,
    GalleryNextHover,
    GalleryPrevHover
};

enum class LeaveAction
{
    DoNothing,
    GalleryCloseLeave,
    GalleryNextLeave,
    GalleryPrevLeave
};

enum class HitAction
{
    ToggleThread,
    ExternalTweet,
    GoToRandomThread,
    OpenGallery,
    CloseGallery,
    GalleryNextPhoto,
    GalleryPrevPhoto,
    GalleryDoNothing
};

struct ToggleThreadArgs
{
    int64_t sourceTweetId;
    int64_t targetTweetId;
    int sourceThreadSlot;
    int targetThreadSlot;

    void init(int64_t sourceTweetId, int64_t targetTweetId, int sourceThreadSlot, int targetThreadSlot)
    {
        assertOrAbort(abs(targetThreadSlot - sourceThreadSlot) == 1, "Slots have to be near each other");
        this->sourceTweetId = sourceTweetId;
        this->targetTweetId = targetTweetId;
        this->sourceThreadSlot = sourceThreadSlot;
        this->targetThreadSlot = targetThreadSlot;
    }
};

struct ExternalTweetArgs
{
    const char *authorScreenName;
    int64_t targetTweetId;
    LayoutedImage icon;

    void init(
        const char *authorScreenName, int64_t targetTweetId, float iconX, float iconY, float iconSize,
        const char *iconUrl)
    {
        this->authorScreenName = authorScreenName;
        this->targetTweetId = targetTweetId;
        this->icon.init(CoordinateSpace::World, iconX, iconY, iconSize, iconSize, iconUrl);
    }
};

struct OpenGalleryArgs
{
    List<Photo> *photos;
    int startPhotoIndex;

    void init(List<Photo> *photos, int startPhotoIndex)
    {
        this->photos = photos;
        this->startPhotoIndex = startPhotoIndex;
    }
};

struct HitRect
{
    LayoutedSemiRoundedRect semiRoundedRect;
    HoverAction hoverAction;
    LeaveAction leaveAction;
    HitAction hitAction;
    union
    {
        ToggleThreadArgs toggleThreadArgs;
        ExternalTweetArgs externalTweetArgs;
        OpenGalleryArgs openGalleryArgs;
    };

    void initToggleThreadHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color, int64_t sourceTweetId, int64_t targetTweetId, int sourceThreadSlot, int targetThreadSlot)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::World, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::DoNothing;
        this->leaveAction = LeaveAction::DoNothing;
        this->hitAction = HitAction::ToggleThread;
        this->toggleThreadArgs.init(sourceTweetId, targetTweetId, sourceThreadSlot, targetThreadSlot);
    }

    void initExternalTweetHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color, const char *authorScreenName, int64_t targetTweetId, float iconX, float iconY,
        float iconSize, const char *iconUrl)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::World, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::DoNothing;
        this->leaveAction = LeaveAction::DoNothing;
        this->hitAction = HitAction::ExternalTweet;
        this->externalTweetArgs.init(authorScreenName, targetTweetId, iconX, iconY, iconSize, iconUrl);
    }

    void initGoToRandomThreadHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::Screen, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::DoNothing;
        this->leaveAction = LeaveAction::DoNothing;
        this->hitAction = HitAction::GoToRandomThread;
    }

    void initOpenGalleryHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color, List<Photo> *photos, int startPhotoIndex)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::World, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::DoNothing;
        this->leaveAction = LeaveAction::DoNothing;
        this->hitAction = HitAction::OpenGallery;
        this->openGalleryArgs.init(photos, startPhotoIndex);
    }

    void initCloseGalleryBackgroundHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::Screen, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::DoNothing;
        this->leaveAction = LeaveAction::DoNothing;
        this->hitAction = HitAction::CloseGallery;
    }

    void initCloseGalleryButtonHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::Screen, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::GalleryCloseHover;
        this->leaveAction = LeaveAction::GalleryCloseLeave;
        this->hitAction = HitAction::CloseGallery;
    }

    void initGalleryNextPhotoHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::Screen, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::GalleryNextHover;
        this->leaveAction = LeaveAction::GalleryNextLeave;
        this->hitAction = HitAction::GalleryNextPhoto;
    }

    void initGalleryPrevPhotoHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::Screen, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::GalleryPrevHover;
        this->leaveAction = LeaveAction::GalleryPrevLeave;
        this->hitAction = HitAction::GalleryPrevPhoto;
    }

    void initGalleryDoNothingHitRect(
        float x, float y, float width, float height, RoundedCorners roundedCorners, float cornerRadius,
        Color color)
    {
        this->semiRoundedRect.init(
            CoordinateSpace::Screen, x, y, width, height, roundedCorners, cornerRadius, color);
        this->hoverAction = HoverAction::DoNothing;
        this->leaveAction = LeaveAction::DoNothing;
        this->hitAction = HitAction::GalleryDoNothing;
    }

    bool test(
        float hitScreenX, float hitScreenY, float viewportX, float viewportY, float viewportWidth,
        float viewportHeight)
    {
        LayoutedSemiRoundedRect *semiRoundedRect = &this->semiRoundedRect;

        float hitX, hitY;
        float rectX, rectY;
        switch (semiRoundedRect->coordinateSpace)
        {
        case CoordinateSpace::World:
            hitX = hitScreenX + viewportX;
            hitY = hitScreenY + viewportY;
            rectX = semiRoundedRect->x;
            rectY = semiRoundedRect->y;
            break;
        case CoordinateSpace::Screen:
            hitX = hitScreenX;
            hitY = hitScreenY;
            rectX = semiRoundedRect->x >= 0 ? semiRoundedRect->x
                                            : viewportWidth + semiRoundedRect->x - semiRoundedRect->width;
            rectY = semiRoundedRect->y >= 0 ? semiRoundedRect->y
                                            : viewportHeight + semiRoundedRect->y - semiRoundedRect->height;
            break;
        default:
            abortWithMessage("Unsupported coordinate space");
        }

        if (!(hitX >= rectX && hitX < rectX + semiRoundedRect->width && hitY >= rectY &&
              hitY < rectY + semiRoundedRect->height))
        {
            return false;
        }

        float cornerCenterX;
        float leftCornerCenterX = rectX + semiRoundedRect->cornerRadius;
        float rightCornerCenterX = rectX + semiRoundedRect->width - semiRoundedRect->cornerRadius;
        if (hitX <= leftCornerCenterX &&
            (semiRoundedRect->roundedCorners.topLeft || semiRoundedRect->roundedCorners.bottomLeft))
        {
            cornerCenterX = leftCornerCenterX;
        }
        else if (
            hitX >= rightCornerCenterX &&
            (semiRoundedRect->roundedCorners.topRight || semiRoundedRect->roundedCorners.bottomRight))
        {
            cornerCenterX = rightCornerCenterX;
        }
        else
        {
            return true;
        }

        float cornerCenterY;
        float topCornerCenterY = rectY + semiRoundedRect->cornerRadius;
        float bottomCornerCenterY = rectY + semiRoundedRect->height - semiRoundedRect->cornerRadius;
        if (hitY <= topCornerCenterY &&
            (semiRoundedRect->roundedCorners.topLeft || semiRoundedRect->roundedCorners.topRight))
        {
            cornerCenterY = topCornerCenterY;
        }
        else if (
            hitY >= bottomCornerCenterY &&
            (semiRoundedRect->roundedCorners.bottomLeft || semiRoundedRect->roundedCorners.bottomRight))
        {
            cornerCenterY = bottomCornerCenterY;
        }
        else
        {
            return true;
        }

        float distanceSqr =
            (hitX - cornerCenterX) * (hitX - cornerCenterX) + (hitY - cornerCenterY) * (hitY - cornerCenterY);
        return distanceSqr <= semiRoundedRect->cornerRadius * semiRoundedRect->cornerRadius;
    }
};