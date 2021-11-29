#pragma once

#include "glUtil.cpp"
#include "texture.cpp"
#include <string.h>
#include <webgl/webgl2.h>

#define _MAX_TEXTURES 256

struct ImageCache
{
    Texture textures[_MAX_TEXTURES] = {};
    unsigned int lastUsed[_MAX_TEXTURES] = {};
    unsigned int epoch = 0;
    bool isWebGl2;

    void init(bool isWebGl2)
    {
        this->isWebGl2 = isWebGl2;
    }

    Texture *textureByUrl(
        const char *url, LoadedCallbackType loadedCallback, unsigned int callbackEpoch, void *userData)
    {
        this->epoch++;
        for (int i = 0; i < _MAX_TEXTURES; ++i) // Naive O(n) lookup for tiny code size
        {
            Texture *texture = this->textures + i;
            if (this->textures[i].url && !strcmp(this->textures[i].url, url))
            {
                this->lastUsed[i] = this->epoch;
                return texture;
            }
            else if (!this->textures[i].url)
            {
                loadTextureFromUrl(texture, url, this->isWebGl2, loadedCallback, callbackEpoch, userData);
                this->lastUsed[i] = epoch;
                return texture;
            }
        }

        // Evict last recently used
        int oldestTextureIdx = 0;
        for (int i = 0; i < _MAX_TEXTURES; ++i) // Naive O(n) lookup for tiny code size
        {
            if (this->lastUsed[i] < this->lastUsed[oldestTextureIdx])
            {
                oldestTextureIdx = i;
            }
        }

        Texture *oldestTexture = &this->textures[oldestTextureIdx];
        deleteTexture(oldestTexture->texture);
        loadTextureFromUrl(oldestTexture, url, this->isWebGl2, loadedCallback, callbackEpoch, userData);
        this->lastUsed[oldestTextureIdx] = epoch;
        return oldestTexture;
    }

    Texture *textureByUrl(const char *url)
    {
        return this->textureByUrl(url, NULL, 0, NULL);
    }
};