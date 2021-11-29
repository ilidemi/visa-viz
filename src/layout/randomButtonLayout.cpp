#pragma once

#include "../render/common/layoutedImage.cpp"
#include "../render/semiRoundedRect/renderSemiRoundedRect.cpp"
#include "../style.cpp"
#include "hitRect.cpp"

struct RandomButtonLayout
{
    LayoutedImage icon;
    HitRect hitRect;

    void init(StyleConfig *styleCfg)
    {
        this->icon.init(
            CoordinateSpace::Screen, styleCfg->randomButtonIconX, styleCfg->randomButtonIconY,
            styleCfg->randomButtonIconSize, styleCfg->randomButtonIconSize, "random.28f964ea.png");
        this->hitRect.initGoToRandomThreadHitRect(
            styleCfg->randomButtonHitRectX, styleCfg->randomButtonHitRectY, styleCfg->randomButtonHitRectSize,
            styleCfg->randomButtonHitRectSize, RoundedCorners::createAll(),
            styleCfg->randomButtonHitRectRadius, styleCfg->randomButtonHoverColor);
    }
};