#pragma once

#include "../render/circle/renderCircle.cpp"
#include "../render/common/imageCache.cpp"
#include "../render/common/layoutedImage.cpp"
#include "../style.cpp"
#include "../tweetSchema.cpp"
#include "../util/list.cpp"
#include "hitRect.cpp"
#include <math.h>

struct GalleryLayout
{
    List<Photo> *photos;
    int currentPhotoIndex;
    LayoutedCircle *closeButton;
    LayoutedCircle *nextPhotoButton;
    LayoutedCircle *prevPhotoButton;
    StyleConfig *styleCfg;
    float screenWidth, screenHeight;
    unsigned int epoch;

    Color backgroundColor;
    List<LayoutedImage> layoutedImages;
    List<LayoutedCircle> buttons;
    List<LayoutedImage> icons;
    List<HitRect> hitRects;

    void init()
    {
        this->photos = NULL;
        this->layoutedImages.init();
        this->buttons.init();
        this->icons.init();
        this->hitRects.init();
        this->epoch = 0;
    }
};

void layoutCloseButton(GalleryLayout *galleryLayout, StyleConfig *styleCfg, Color color)
{
    galleryLayout->buttons.append(LayoutedCircle::createScreen(
        styleCfg->galleryButtonMargin, styleCfg->galleryButtonMargin, styleCfg->galleryButtonSize, color));
    galleryLayout->closeButton = galleryLayout->buttons.get(galleryLayout->buttons.size - 1);
    galleryLayout->icons.getNext()->init(
        CoordinateSpace::Screen, styleCfg->galleryIconMargin, styleCfg->galleryIconMargin,
        styleCfg->galleryIconSize, styleCfg->galleryIconSize, "galleryClose.7568ba97.png");
    HitRect *hitRect = galleryLayout->hitRects.getNext();
    hitRect->initCloseGalleryButtonHitRect(
        styleCfg->galleryButtonMargin, styleCfg->galleryButtonMargin, styleCfg->galleryButtonSize,
        styleCfg->galleryButtonSize, RoundedCorners::createAll(), styleCfg->galleryButtonSize / 2,
        styleCfg->transparent);
}

void layoutNextPhotoButton(
    GalleryLayout *galleryLayout, StyleConfig *styleCfg, Color color, float screenHeight)
{
    galleryLayout->buttons.append(LayoutedCircle::createScreen(
        -styleCfg->galleryButtonMargin, (screenHeight - styleCfg->galleryButtonSize) / 2,
        styleCfg->galleryButtonSize, color));
    galleryLayout->nextPhotoButton = galleryLayout->buttons.get(galleryLayout->buttons.size - 1);
    galleryLayout->icons.getNext()->init(
        CoordinateSpace::Screen, -styleCfg->galleryIconMargin, (screenHeight - styleCfg->galleryIconSize) / 2,
        styleCfg->galleryIconSize, styleCfg->galleryIconSize, "galleryNextPhoto.aea608ac.png");
    HitRect *hitRect = galleryLayout->hitRects.getNext();
    hitRect->initGalleryNextPhotoHitRect(
        -styleCfg->galleryButtonMargin, (screenHeight - styleCfg->galleryButtonSize) / 2,
        styleCfg->galleryButtonSize, styleCfg->galleryButtonSize, RoundedCorners::createAll(),
        styleCfg->galleryButtonSize / 2, styleCfg->transparent);
}

void layoutPrevPhotoButton(
    GalleryLayout *galleryLayout, StyleConfig *styleCfg, Color color, float screenHeight)
{
    galleryLayout->buttons.append(LayoutedCircle::createScreen(
        styleCfg->galleryButtonMargin, (screenHeight - styleCfg->galleryButtonSize) / 2,
        styleCfg->galleryButtonSize, color));
    galleryLayout->prevPhotoButton = galleryLayout->buttons.get(galleryLayout->buttons.size - 1);
    galleryLayout->icons.getNext()->init(
        CoordinateSpace::Screen, styleCfg->galleryIconMargin, (screenHeight - styleCfg->galleryIconSize) / 2,
        styleCfg->galleryIconSize, styleCfg->galleryIconSize, "galleryPrevPhoto.c83e1990.png");
    HitRect *hitRect = galleryLayout->hitRects.getNext();
    hitRect->initGalleryPrevPhotoHitRect(
        styleCfg->galleryButtonMargin, (screenHeight - styleCfg->galleryButtonSize) / 2,
        styleCfg->galleryButtonSize, styleCfg->galleryButtonSize, RoundedCorners::createAll(),
        styleCfg->galleryButtonSize / 2, styleCfg->transparent);
}

void layoutPhoto(unsigned int callbackEpoch, void *userData, int photoWidth, int photoHeight)
{
    GalleryLayout *galleryLayout = (GalleryLayout *)userData;
    if (callbackEpoch != galleryLayout->epoch)
    {
        return;
    }

    float layoutedWidth, layoutedHeight;
    int minSize = 600;

    if (photoWidth < minSize && photoHeight < minSize)
    {
        if (photoWidth > photoHeight)
        {
            layoutedWidth = minSize;
            layoutedHeight = float(layoutedWidth) / photoWidth * photoHeight;
        }
        else
        {
            layoutedHeight = minSize;
            layoutedWidth = float(layoutedHeight) / photoHeight * photoWidth;
        }
    }
    else if (photoWidth <= galleryLayout->screenWidth && photoHeight <= galleryLayout->screenHeight)
    {
        layoutedWidth = photoWidth;
        layoutedHeight = photoHeight;
    }
    else
    {
        float screenAspectRatio = float(galleryLayout->screenWidth) / galleryLayout->screenHeight;
        float photoAspectRatio = float(photoWidth) / photoHeight;
        if (photoAspectRatio > screenAspectRatio)
        {
            layoutedWidth = galleryLayout->screenWidth;
            layoutedHeight = layoutedWidth / photoAspectRatio;
        }
        else
        {
            layoutedHeight = galleryLayout->screenHeight;
            layoutedWidth = layoutedHeight * photoAspectRatio;
        }
    }

    float layoutedX = (galleryLayout->screenWidth - layoutedWidth) / 2;
    float layoutedY = (galleryLayout->screenHeight - layoutedHeight) / 2;
    Photo *photo = galleryLayout->photos->get(galleryLayout->currentPhotoIndex);
    galleryLayout->layoutedImages.getNext()->init(
        CoordinateSpace::Screen, layoutedX, layoutedY, layoutedWidth, layoutedHeight, photo->fullUrl);

    HitRect *hitRect = galleryLayout->hitRects.getNext();
    hitRect->initGalleryDoNothingHitRect(
        layoutedX, layoutedY, layoutedWidth, layoutedHeight, RoundedCorners::createNone(), 0,
        galleryLayout->styleCfg->transparent);
}

void loadPhoto(GalleryLayout *galleryLayout, Photo *photo, ImageCache *imageCache)
{
    Texture *texture =
        imageCache->textureByUrl(photo->fullUrl, &layoutPhoto, galleryLayout->epoch, (void *)galleryLayout);
    if (texture->isInitialized())
    {
        // Layout inline, callback won't be called
        layoutPhoto(galleryLayout->epoch, (void *)galleryLayout, texture->w, texture->h);
    }
}

void layoutGallery(
    GalleryLayout *galleryLayout, StyleConfig *styleCfg, ImageCache *imageCache, List<Photo> *photos,
    int startPhotoIndex, float screenWidth, float screenHeight)
{
    Photo *photo = photos->get(startPhotoIndex);

    galleryLayout->photos = photos;
    galleryLayout->currentPhotoIndex = startPhotoIndex;
    galleryLayout->backgroundColor = photo->galleryBackgroundColor;
    galleryLayout->layoutedImages.clear();
    galleryLayout->buttons.clear();
    galleryLayout->icons.clear();
    galleryLayout->hitRects.clear();
    galleryLayout->styleCfg = styleCfg;
    galleryLayout->screenWidth = screenWidth;
    galleryLayout->screenHeight = screenHeight;
    galleryLayout->epoch++;

    layoutCloseButton(galleryLayout, styleCfg, photo->galleryButtonColor);
    if (startPhotoIndex < photos->size - 1)
    {
        layoutNextPhotoButton(galleryLayout, styleCfg, photo->galleryButtonColor, screenHeight);
    }
    if (startPhotoIndex > 0)
    {
        layoutPrevPhotoButton(galleryLayout, styleCfg, photo->galleryButtonColor, screenHeight);
    }

    loadPhoto(galleryLayout, photo, imageCache);

    HitRect *backgroundHitRect = galleryLayout->hitRects.getNext();
    backgroundHitRect->initCloseGalleryBackgroundHitRect(
        0, 0, screenWidth, screenHeight, RoundedCorners::createNone(), 0, styleCfg->transparent);
}
