#pragma once

#include "render/common/color.cpp"
#include <math.h>

struct StyleConfig
{
    float lineHeightFactor = 1.3125;
    float mainFontSize = 15;
    float auxFontSize = 13;

    Color white = Color::create(255, 255, 255, 255);
    Color transparent = Color::create(0, 0, 0, 0);
    Color backgroundColor = Color::create(239, 239, 239, 255);
    Color threadOutlineColor = Color::create(204, 214, 221, 255);
    Color threadOutlineTransparentColor = threadOutlineColor.makeTransparent();
    Color outlineColor = Color::create(196, 207, 214, 255);
    Color outlineTransparentColor = outlineColor.makeTransparent();
    Color textColor = Color::create(20, 23, 26, 255);
    Color textAuxColor = Color::create(101, 119, 134, 255);
    Color textHighlightColor = Color::create(27, 149, 224, 255);
    Color gifLabelBackgroundColor = Color::create(0, 0, 0, 0.7 * 255);
    Color gifLabelBackgroundTransparentColor = gifLabelBackgroundColor.makeTransparent();
    Color dmcaVideoBackgroundColor = Color::create(247, 249, 250, 255);
    Color dmcaVideoTextColor = Color::create(91, 112, 131, 255);
    Color playButtonInsideColor = Color::create(29.0, 161, 242, 255);
    Color playButtonBorderColor = white;
    Color playButtonOutsideColor = playButtonBorderColor.makeTransparent();
    Color cardBackgroundColor = Color::create(245, 248, 250, 255);
    Color pollWinningBarColor = Color::create(113, 201, 248, 255);
    Color pollWinningBarTransparentColor = pollWinningBarColor.makeTransparent();
    Color pollBarColor = threadOutlineColor;
    Color pollBarTransparentColor = threadOutlineTransparentColor;
    Color unavailableTweetOutlineColor = Color::create(230, 236, 240, 255);
    Color unavailableTweetOutlineTransparentColor = unavailableTweetOutlineColor.makeTransparent();
    // NOTE: externalTweetHover icon background is hacked to match this over white
    Color tweetHoverColor = Color::create(0, 0, 0, 0.03 * 255);
    Color arrowHoverColor = Color::create(0, 0, 0, 0.12 * 255);
    Color photoHoverColor = arrowHoverColor;
    Color quotedTweetOutlineColor = threadOutlineColor;
    Color arrowColor = textAuxColor;

    float avatarDisplaySize = 49;
    float quotedAvatarDisplaySize = 18;
    float counterDisplaySize = 18.75;
    float gifLabelBackgroundRadius = 5;
    float dmcaVideoTextPadding = 30;
    float playButtonSize = 67;
    float playButtonRadius = playButtonSize / 2;
    float playButtonBorderSize = 4;
    float playTriangleDisplaySize = 33.5;
    float playTriangleOffsetX = 1.25;
    float miniPlayButtonSize = 35;
    float miniPlayButtonRadius = miniPlayButtonSize / 2;
    float miniPlayButtonBorderSize = 4;
    float miniPlayTriangleDisplaySize = 17.5;
    float miniPlayTriangleOffsetX = 1;
    float linkCardImageSize = 123;
    float linkThumbnailIconDisplaySize = 30;
    float linkIconDisplaySize = 16.25;
    float pollRoundedCornerRadius = 5;
    float roundedCornerRadius = 15;
    float roundedFrameBorder = 1;
    float externalTweetIconSize = 16;

    float threadWidth = 550;
    float threadGap = 135;
    float tweetWidth = threadWidth - 2;
    float tweetBodyWidth = threadWidth - 16 - 10 - avatarDisplaySize - 16;
    float quotedTweetBodyWidth = tweetBodyWidth - 22;
    float unavailableTweetBodyWidth = threadWidth - 32;

    float photoRoundedCornerRadius = roundedCornerRadius - roundedFrameBorder;
    float photoWidth = tweetBodyWidth;
    float photoGap = 2;
    float photoHalfWidth = (photoWidth - photoGap) / 2;
    float photoHitRectWidth = photoWidth - roundedFrameBorder;
    float photoHitRectHalfWidth = photoHalfWidth - roundedFrameBorder;

    float arrowThickness = 3;
    float arrowHalfThickness = arrowThickness / 2;
    float arrowOffset = 45;
    float arrowCornerRadius = 15;
    float arrowCornerSize = arrowCornerRadius + arrowHalfThickness;
    float arrowTipSize = 15;
    float arrowTipHeight = arrowTipSize / sqrtf(3) * 2;
    float arrowTipHalfSize = arrowTipSize / 2;
    float arrowTipBoundingRadius = arrowTipSize * 2 / 3;
    float arrowTipHitRadius = arrowTipSize;
    float arrowTipHitDiameter = arrowTipHitRadius * 2;
    float arrowGap = arrowTipHitDiameter;

    float randomButtonIconX = 20;
    float randomButtonIconY = -18;
    float randomButtonIconSize = 134;
    float randomButtonHitRectX = randomButtonIconX + 18;
    float randomButtonHitRectY = randomButtonIconY - 19;
    float randomButtonHitRectSize = 98;
    float randomButtonHitRectRadius = randomButtonHitRectSize / 2;
    Color randomButtonHoverColor = tweetHoverColor;

    float galleryButtonMargin = 11;
    float galleryButtonSize = 37;
    float galleryIconMargin = galleryButtonMargin + 7.25;
    float galleryIconSize = 22.5;
};