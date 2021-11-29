#pragma once

#include "../common/color.cpp"

struct LayoutedRoundedFrame
{
    float x, y;
    float width, height;
    float radius;
    float borderSize;
    Color insideColor, borderColor, outsideColor;

    static LayoutedRoundedFrame create(
        float x, float y, float width, float height, float radius, float borderSize, Color insideColor,
        Color borderColor, Color outsideColor)
    {
        return { x, y, width, height, radius, borderSize, insideColor, borderColor, outsideColor };
    }
};