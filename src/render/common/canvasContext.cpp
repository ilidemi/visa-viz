#pragma once

#include <emscripten.h>
#include <emscripten/html5.h>

const char *canvasSelector = "#canvas";

struct CanvasContext
{
    int drawWidth, drawHeight;
    float drawPixelWidth, drawPixelHeight;
    float displayWidth, displayHeight;
    float displayPixelWidth, displayPixelHeight;
    float devicePixelRatio;

    void init()
    {
        emscripten_get_canvas_element_size(canvasSelector, &this->drawWidth, &this->drawHeight);
        this->drawPixelWidth = 2.f / this->drawWidth;
        this->drawPixelHeight = 2.f / this->drawHeight;
        this->devicePixelRatio = emscripten_get_device_pixel_ratio();
        this->displayWidth = this->drawWidth / this->devicePixelRatio;
        this->displayHeight = this->drawHeight / this->devicePixelRatio;
        this->displayPixelWidth = 2.f / this->displayWidth;
        this->displayPixelHeight = 2.f / this->displayHeight;
        emscripten_set_element_css_size(canvasSelector, this->displayWidth, this->displayHeight);
    }
};