#pragma once

#include "glUtil.cpp"
#include <webgl/webgl2.h>

extern "C"
{
    void _jsLoadTextureFromUrl(
        GLuint texture, const char *url, bool isWebGl2, int loadedCallbackInt, int callbackEpochInt,
        int userDataInt, int *outW, int *outH);
}

struct Texture
{
    const char *url = NULL;
    int w, h;
    GLuint texture;

    bool isInitialized()
    {
        return this->w > 0;
    }
};

typedef void (*LoadedCallbackType)(unsigned int, void *, int, int);

void loadTextureFromUrl(
    Texture *texture, const char *url, bool isWebGl2, LoadedCallbackType loadedCallback, int callbackEpoch,
    void *userData)
{
    texture->url = url;
    texture->texture = createTexture(isWebGl2);

    // Width and height will be populated asynchronously
    texture->w = 0;
    texture->h = 0;

    int loadedCallbackInt;
    int callbackEpochInt;
    int userDataInt;
    memcpy(&loadedCallbackInt, &loadedCallback, sizeof(int));
    memcpy(&callbackEpochInt, &callbackEpoch, sizeof(int));
    memcpy(&userDataInt, &userData, sizeof(int));

    _jsLoadTextureFromUrl(
        texture->texture, url, isWebGl2, loadedCallbackInt, callbackEpochInt, userDataInt, &texture->w,
        &texture->h);
}

void loadTextureFromUrl(Texture *texture, const char *url, bool isWebGl2)
{
    loadTextureFromUrl(texture, url, isWebGl2, NULL, 0, NULL);
}