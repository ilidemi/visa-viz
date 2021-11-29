#pragma once

#include "color.cpp"

struct LayoutedRect
{
    float x, y;
    float width, height;
    Color color;

    static LayoutedRect create(float x, float y, float width, float height, Color color)
    {
        return { x, y, width, height, color };
    }
};