#pragma once

#include "coordinateSpace.cpp"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct LayoutedImage
{
    CoordinateSpace coordinateSpace;
    float x, y;
    float width, height;
    const char *url;

    void init(CoordinateSpace coordinateSpace, float x, float y, float width, float height, const char *url)
    {
        this->coordinateSpace = coordinateSpace;
        this->x = x;
        this->y = y;
        this->width = width;
        this->height = height;
        this->url = url;
    }
};