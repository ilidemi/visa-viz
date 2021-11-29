#pragma once

#include "../../style.cpp"
#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/color.cpp"
#include "../common/glUtil.cpp"
#include <emscripten.h>

enum class IconType
{
    Reply,
    Retweet,
    Favorite,
    Link,
    LinkThumbnail,
    PlayTriangle,
    ExternalTweet
};

struct AtlasIcon
{
    int x, y;
    int size;
};

static const int _iconAtlasWidth = 468;
static const int _iconAtlasHeight = 150;
static const AtlasIcon _iconTypeToAtlasIcon[] = {
    { 0, 0, 75 }, // Reply
    { 0, 75, 75 }, // Retweet
    { 75, 0, 75 }, // Favorite
    { 75, 75, 65 }, // Link
    { 150, 0, 120 }, // Link thumbnail
    { 270, 0, 134 }, // Play triangle
    { 404, 0, 64 } // External tweet
};

struct LayoutedIcon
{
    float x, y;
    float size;
    IconType type;

    static LayoutedIcon create(float x, float y, float size, IconType type)
    {
        return { x, y, size, type };
    }
};

struct IconVertexAttributes
{
    float x, y;
    float textureU, textureV;

    void init(LayoutedIcon *icon, QuadVertex *quadVertex)
    {
        this->x = icon->x + icon->size * quadVertex->x;
        this->y = icon->y + icon->size * quadVertex->y;
        AtlasIcon atlasIcon = _iconTypeToAtlasIcon[(int)(icon->type)];
        this->textureU = float(atlasIcon.x + atlasIcon.size * quadVertex->x) / _iconAtlasWidth;
        this->textureV = float(atlasIcon.y + atlasIcon.size * quadVertex->y) / _iconAtlasHeight;
    }
};

struct IconsRenderContext
{
    GLuint program;
    GLuint matPos;
    GLuint posPos, textureUvPos;

    Texture texture;
    CanvasContext *canvasCtx;

    List<IconVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    void init(CanvasContext *canvasCtx, bool isWebGl2, GLuint texturedRectFragmentShader)
    {
        const char vertexShaderCode[] =
#include "iconsVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        this->program = createProgram(vertexShader, texturedRectFragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");

        this->posPos = glGetAttribLocation(this->program, "pos");
        this->textureUvPos = glGetAttribLocation(this->program, "textureUv");

        loadTextureFromUrl(&this->texture, "icons.93877d28.png", isWebGl2);
        this->canvasCtx = canvasCtx;

        this->vertexAttributes.init();
        glGenBuffers(1, &this->vertexAttributesHandle);
    }

    bool isInitialized()
    {
        return this->texture.isInitialized();
    }

    void setUniforms()
    {
        glBindTexture(GL_TEXTURE_2D, this->texture.texture);
    }

    void setAttributes()
    {
        GLsizei stride = sizeof(IconVertexAttributes);

        glEnableVertexAttribArray(this->posPos);
        glVertexAttribPointer(this->posPos, 2, GL_FLOAT, GL_FALSE, stride, 0);

        glEnableVertexAttribArray(this->textureUvPos);
        glVertexAttribPointer(
            this->textureUvPos, 2, GL_FLOAT, GL_FALSE, stride, (const void *)(2 * sizeof(float)));
    }

    void unsetAttributes()
    {
        glDisableVertexAttribArray(this->posPos);
        glDisableVertexAttribArray(this->textureUvPos);
    }
};