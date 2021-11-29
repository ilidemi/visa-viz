#pragma once

#include "../../util/hashMap.cpp"
#include "../../util/list.cpp"
#include "../common/canvasContext.cpp"
#include "../common/glUtil.cpp"
#include <emscripten.h>
#include <limits.h>
#include <wchar.h>

// JS bindings
extern "C"
{
    void _jsMeasureChar(
        int codePoint, float fontSize, bool bold, float *outCharWidth, float *outCharMidBaseline,
        float *outCharDescentFromBaseline);
    void _jsAtlasCanvasInit(int width, int height);
    void _jsAtlasCanvasPutChar(
        int codePoint, float fontSize, bool bold, float r, float g, float b, float x, float middleBaselineY);
    void _jsAtlasCanvasUploadToTexture(bool isWebGl2);
}

struct AtlasChar
{
    float displayX, displayMidBaselineY;
    int glyphId;

    void init(float displayX, float displayMidBaselineY, int glyphId)
    {
        this->displayX = displayX;
        this->displayMidBaselineY = displayMidBaselineY;
        this->glyphId = glyphId;
    }
};

struct DisplayGlyphKey
{
    wchar_t chr;
    float displayFontSize;
    Color color;
    bool bold;

    void init(wchar_t chr, float displayFontSize, Color color, bool bold)
    {
        this->chr = chr;
        this->displayFontSize = displayFontSize;
        this->color = color;
        this->bold = bold;
    }

    explicit operator uint64_t() const
    {
        uint64_t result = ((uint64_t)(chr) << 32) | ((uint64_t)(displayFontSize * 1000)) |
            ((uint64_t)(color.r) << 48) | ((uint64_t)(color.g) << 32) | ((uint64_t)(color.b) << 16) |
            ((uint64_t)(color.a)) | ((uint64_t)(bold));
        return result;
    }

    bool operator==(const DisplayGlyphKey &other)
    {
        return this->chr == other.chr && this->displayFontSize == other.displayFontSize &&
            this->color.r == other.color.r && this->color.g == other.color.g &&
            this->color.b == other.color.b && this->bold == other.bold;
    }
};

struct DisplayGlyph
{
    int id;
    DisplayGlyphKey key;

    float displayWidth;
    float displayMidBaseline;
    float displayDescentFromMidBaseline;

    void init(
        int id, wchar_t chr, float displayFontSize, Color color, bool bold, float displayWidth,
        float displayMidBaseline, float displayDescentFromMidBaseline)
    {
        this->id = id;
        this->key.init(chr, displayFontSize, color, bold);
        this->displayWidth = displayWidth;
        this->displayMidBaseline = displayMidBaseline;
        this->displayDescentFromMidBaseline = displayDescentFromMidBaseline;
    }
};

struct DrawGlyph
{
    float drawMidBaselineOffset;
    float textureX0, textureY0, textureX1, textureY1;

    void init(float drawMidBaselineOffset, float textureX0, float textureY0, float textureX1, float textureY1)
    {
        this->drawMidBaselineOffset = drawMidBaselineOffset;
        this->textureX0 = textureX0;
        this->textureY0 = textureY0;
        this->textureX1 = textureX1;
        this->textureY1 = textureY1;
    }
};

struct TextVertexAttributes
{
    float drawX, drawY;
    float textureU, textureV;

    void init(float drawX, float drawY, float textureU, float textureV)
    {
        this->drawX = drawX;
        this->drawY = drawY;
        this->textureU = textureU;
        this->textureV = textureV;
    }
};

struct TextRenderer
{
    GLuint program;
    GLuint matPos;
    GLuint posPos, textureUvPos;

    bool isInitialized = false;
    List<AtlasChar> atlasChars;
    HashMap<DisplayGlyphKey, int> displayGlyphIndex;
    List<DisplayGlyph> displayGlyphs;
    GLuint texture;

    List<TextVertexAttributes> vertexAttributes;
    GLuint vertexAttributesHandle;

    CanvasContext *canvasCtx;
    bool isWebGl2;

    void init(CanvasContext *canvasCtx, bool isWebGl2, GLuint texturedRectFragmentShader)
    {
        const char vertexShaderCode[] =
#include "textVertexShader.glsl"
            GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderCode);
        this->program = createProgram(vertexShader, texturedRectFragmentShader);

        this->matPos = glGetUniformLocation(this->program, "mat");

        this->posPos = glGetAttribLocation(this->program, "pos");
        this->textureUvPos = glGetAttribLocation(this->program, "textureUv");

        this->atlasChars.init();
        this->displayGlyphIndex.init();
        this->displayGlyphs.init();
        this->texture = 0;
        this->vertexAttributes.init();
        glGenBuffers(1, &this->vertexAttributesHandle);

        this->canvasCtx = canvasCtx;
        this->isWebGl2 = isWebGl2;
        this->isInitialized = true;
    }

    void update()
    {
        if (!this->isInitialized)
        {
            return;
        }

        this->vertexAttributes.clear();
        float devicePixelRatio = this->canvasCtx->devicePixelRatio;

        if (this->texture != 0)
        {
            glDeleteTextures(1, &this->texture);
        }

        int maxDrawGlyphWidth = 0;
        int maxDrawGlyphHeight = 0;
        list_for_each(displayGlyph, this->displayGlyphs)
        {
            float drawGlyphWidth = ceilf(displayGlyph->displayWidth * devicePixelRatio);
            if (drawGlyphWidth > maxDrawGlyphWidth)
            {
                maxDrawGlyphWidth = drawGlyphWidth;
            }
            // +1 to counteract clipping bottom row of low hanging characters like g, y
            float drawGlyphHeight = ceilf(displayGlyph->displayMidBaseline * devicePixelRatio) +
                ceilf(displayGlyph->displayDescentFromMidBaseline * devicePixelRatio) + 1;
            if (drawGlyphHeight > maxDrawGlyphHeight)
            {
                maxDrawGlyphHeight = drawGlyphHeight;
            }
        }

        int glyphsPerSide = ceilf(sqrtf(this->displayGlyphs.size));
        int drawTextureWidth = maxDrawGlyphWidth * glyphsPerSide;
        int drawTextureHeight = maxDrawGlyphHeight * glyphsPerSide;

        List<DrawGlyph> drawGlyphs;
        drawGlyphs.init();
        drawGlyphs.reserve(this->displayGlyphs.size);

        _jsAtlasCanvasInit(drawTextureWidth, drawTextureHeight);
        for (int i = 0; i < this->displayGlyphs.size; i++)
        {
            DisplayGlyph *displayGlyph = this->displayGlyphs.get(i);
            float drawFontSize = displayGlyph->key.displayFontSize * devicePixelRatio;
            int drawX = (i % glyphsPerSide) * maxDrawGlyphWidth;
            int drawMidBaselineOffset = ceilf(displayGlyph->displayMidBaseline * devicePixelRatio);
            int drawY = (i / glyphsPerSide) * maxDrawGlyphHeight;
            int drawMidBaselineY = drawY + drawMidBaselineOffset;
            _jsAtlasCanvasPutChar(
                displayGlyph->key.chr, drawFontSize, displayGlyph->key.bold, displayGlyph->key.color.r,
                displayGlyph->key.color.g, displayGlyph->key.color.b, drawX, drawMidBaselineY);

            int drawWidth = ceilf(displayGlyph->displayWidth * devicePixelRatio);
            // +1 to counteract clipping bottom row of low hanging characters like g, y
            int drawHeight = drawMidBaselineOffset +
                ceilf(displayGlyph->displayDescentFromMidBaseline * devicePixelRatio) + 1;
            DrawGlyph *drawGlyph = drawGlyphs.getNext();
            drawGlyph->init(drawMidBaselineOffset, drawX, drawY, drawX + drawWidth, drawY + drawHeight);
        }
        this->texture = createTexture(this->isWebGl2);
        _jsAtlasCanvasUploadToTexture(this->isWebGl2);

        list_for_each(atlasChar, this->atlasChars)
        {
            DisplayGlyph *displayGlyph = this->displayGlyphs.get(atlasChar->glyphId);
            float drawX = roundf(atlasChar->displayX * devicePixelRatio);
            float drawY = roundf(atlasChar->displayMidBaselineY * devicePixelRatio) -
                ceilf(displayGlyph->displayMidBaseline * devicePixelRatio);
            DrawGlyph *drawGlyph = drawGlyphs.get(atlasChar->glyphId);
            float drawWidth = drawGlyph->textureX1 - drawGlyph->textureX0;
            float drawHeight = drawGlyph->textureY1 - drawGlyph->textureY0;

            list_for_each(quadVertex, quadVertices)
            {
                TextVertexAttributes *vertexAttributes = this->vertexAttributes.getNext();
                float drawVertexX = drawX + drawWidth * quadVertex->x;
                float drawVertexY = drawY + drawHeight * quadVertex->y;
                float textureU = (drawGlyph->textureX0 + drawWidth * quadVertex->x) / drawTextureWidth;
                float textureV =
                    1.0 - (drawGlyph->textureY0 + drawHeight * quadVertex->y) / drawTextureHeight;
                vertexAttributes->init(drawVertexX, drawVertexY, textureU, textureV);
            }
        }

        drawGlyphs.free();

        glBindBuffer(GL_ARRAY_BUFFER, this->vertexAttributesHandle);
        glBufferData(
            GL_ARRAY_BUFFER, this->vertexAttributes.size * sizeof(TextVertexAttributes),
            this->vertexAttributes.values, GL_STATIC_DRAW);
    }

    void reserve(int charCount)
    {
        this->atlasChars.clear();
        this->atlasChars.reserve(charCount);

        this->vertexAttributes.clear();
        this->vertexAttributes.reserve(6 * charCount);
    }

    void appendRange(List<AtlasChar> *atlasChars)
    {
        this->atlasChars.appendRange(atlasChars);
    }

    void commit()
    {
        this->update();
    }

    void _initTransformationMatrix(float *mat, float displayViewportX, float displayViewportY)
    {
        float drawViewportX = displayViewportX * canvasCtx->devicePixelRatio;
        float drawViewportY = displayViewportY * canvasCtx->devicePixelRatio;

        float glViewportX = roundf(-drawViewportX) * canvasCtx->drawPixelWidth - 1;
        float glViewportY = roundf(drawViewportY) * canvasCtx->drawPixelHeight + 1;

        mat[0] = canvasCtx->drawPixelWidth;
        mat[1] = 0;
        mat[2] = 0;
        mat[3] = 0;

        mat[4] = 0;
        mat[5] = -canvasCtx->drawPixelHeight;
        mat[6] = 0;
        mat[7] = 0;

        mat[8] = 0;
        mat[9] = 0;
        mat[10] = 1;
        mat[11] = 0;

        mat[12] = glViewportX;
        mat[13] = glViewportY;
        mat[14] = 0;
        mat[15] = 1;
    }

    void render(float displayViewportX, float displayViewportY)
    {
        if (this->vertexAttributes.size == 0)
        {
            return;
        }

        float mat[16];
        this->_initTransformationMatrix(mat, displayViewportX, displayViewportY);

        glUseProgram(this->program);

        glUniformMatrix4fv(this->matPos, 1, 0, mat);

        glBindTexture(GL_TEXTURE_2D, this->texture);

        glBindBuffer(GL_ARRAY_BUFFER, this->vertexAttributesHandle);

        glEnableVertexAttribArray(this->posPos);
        glVertexAttribPointer(this->posPos, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertexAttributes), 0);

        glEnableVertexAttribArray(this->textureUvPos);
        glVertexAttribPointer(
            this->textureUvPos, 2, GL_FLOAT, GL_FALSE, sizeof(TextVertexAttributes),
            (const void *)(2 * sizeof(float)));
        
        glDrawArrays(GL_TRIANGLES, 0, this->vertexAttributes.size);
        
        glDisableVertexAttribArray(this->posPos);
        glDisableVertexAttribArray(this->textureUvPos);
    }

    DisplayGlyph *_findOrCacheDisplayGlyph(wchar_t chr, float displayFontSize, Color color, bool bold)
    {
        int displayGlyphIdx;
        DisplayGlyphKey key;
        key.init(chr, displayFontSize, color, bold);
        if (this->displayGlyphIndex.tryGet(key, &displayGlyphIdx))
        {
            DisplayGlyph *displayGlyph = this->displayGlyphs.get(displayGlyphIdx);
            return displayGlyph;
        }

        float displayCharWidth, displayCharMidBaseline, displayCharDescentFromMidBaseline;
        _jsMeasureChar(
            chr, displayFontSize, bold, &displayCharWidth, &displayCharMidBaseline,
            &displayCharDescentFromMidBaseline);

        int glyphId = this->displayGlyphs.size;
        DisplayGlyph *displayGlyph = this->displayGlyphs.getNext();
        displayGlyph->init(
            glyphId, chr, displayFontSize, color, bold, displayCharWidth, displayCharMidBaseline,
            displayCharDescentFromMidBaseline);
        this->displayGlyphIndex.insert(displayGlyph->key, glyphId);
        return displayGlyph;
    }

    template <typename TChar>
    float measureChar(TChar chr, float displayFontSize, Color color, bool bold)
    {
        DisplayGlyph *displayGlyph = _findOrCacheDisplayGlyph(chr, displayFontSize, color, bold);
        return displayGlyph->displayWidth;
    }

    template <typename TChar>
    float measureText(const TChar *str, float displayFontSize, Color color, bool bold)
    {
        float drawWidth = 0;
        while (*str)
        {
            drawWidth += measureChar(*str, displayFontSize, color, bold);
            str++;
        }
        return drawWidth;
    }

    template <typename TChar>
    float measureText(const TChar *str, int textLength, float displayFontSize, Color color, bool bold)
    {
        float drawWidth = 0;
        for (int i = 0; i < textLength; i++)
        {
            drawWidth += measureChar(str[i], displayFontSize, color, bold);
        }
        return drawWidth;
    }

    template <typename TChar>
    float _addTextToAtlas(
        List<AtlasChar> *atlas, float displayX, float displayY, const TChar *text, int textLength,
        float displayFontSize, float lineHeightFactor, Color color, bool bold)
    {
        float textDisplayWidth = 0.0;
        for (int i = 0; i < textLength && text[i]; i++)
        {
            DisplayGlyph *displayGlyph = _findOrCacheDisplayGlyph(text[i], displayFontSize, color, bold);

            float displayCharX = displayX + textDisplayWidth;
            float displayCharMidBaselineY = displayY + displayFontSize * lineHeightFactor / 2;
            AtlasChar *atlasChar = atlas->getNext();
            atlasChar->init(displayCharX, displayCharMidBaselineY, displayGlyph->id);

            textDisplayWidth += displayGlyph->displayWidth;
        }

        return textDisplayWidth;
    }

    template <typename TChar>
    float addTextToAtlas(
        List<AtlasChar> *atlas, float displayX, float displayY, const TChar *text, int textLength,
        float displayFontSize, float lineHeightFactor, Color color, bool bold)
    {
        return _addTextToAtlas(
            atlas, displayX, displayY, text, textLength, displayFontSize, lineHeightFactor, color, bold);
    }

    template <typename TChar>
    float addTextToAtlas(
        List<AtlasChar> *atlas, float displayX, float displayY, const TChar *text, float displayFontSize,
        float lineHeightFactor, Color color, bool bold)
    {
        return _addTextToAtlas(
            atlas, displayX, displayY, text, INT_MAX, displayFontSize, lineHeightFactor, color, bold);
    }
};