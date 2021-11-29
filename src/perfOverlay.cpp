#pragma once

#include "render/common/renderPrimitives.cpp"
#include "render/rects/renderRects.cpp"

struct PerfHistogram
{
    static const int frameTimesCount = 60;
    float frameTimes[60];
    int currFrameIdx;

    void init()
    {
        for (int i = 0; i < this->frameTimesCount; i++)
        {
            this->frameTimes[i] = 0;
        }

        this->currFrameIdx = 0;
    }

    void log(float frameTime)
    {
        this->frameTimes[this->currFrameIdx] = frameTime;
    }

    void advance()
    {
        this->currFrameIdx = (this->currFrameIdx + 1) % this->frameTimesCount;
    }

    float append(
        RectsGroupContext *barsCtx, RectsGroupContext *targetsCtx, float displayHeight, float x,
        float bottomY, Color barColor, Color currBarColor)
    {
        float target = 1000.0 / 60;
        float targetHeight = 100.0;
        float barWidth = 5;

        for (int i = 0; i < this->frameTimesCount; i++)
        {
            float barX = x + i * barWidth;
            float barHeight = targetHeight / target * this->frameTimes[i];
            float barY = displayHeight - bottomY - barHeight;
            Color color = i == this->currFrameIdx ? currBarColor : barColor;
            LayoutedRect rect = LayoutedRect::create(barX, barY, barWidth, barHeight, color);
            primitivesAppend(barsCtx, &rect);
        }

        Color targetColor = Color::create(127, 0, 0, 255);
        float targetLineY = displayHeight - bottomY - targetHeight;
        float targetLineWidth = this->frameTimesCount * barWidth;
        LayoutedRect targetLineRect = LayoutedRect::create(x, targetLineY, targetLineWidth, 1, targetColor);
        primitivesAppend(targetsCtx, &targetLineRect);

        return this->frameTimesCount * barWidth;
    }
};

struct PerfOverlay
{
    PerfHistogram frameTimes;
    PerfHistogram betweenFrameTimes;
    double lastFrameEndTime;
    bool isVisible;

    void init()
    {
        this->frameTimes.init();
        this->betweenFrameTimes.init();
        this->lastFrameEndTime = -1;
        this->isVisible = false;
    }

    void render(RectsGroupContext *barsCtx, RectsGroupContext *targetsCtx, RectsRenderContext *rectsRenderCtx)
    {
        primitivesReserve(barsCtx, 2 * PerfHistogram::frameTimesCount);
        primitivesReserve(targetsCtx, 2);

        Color green = Color::create(0, 127, 0, 255);
        Color lightGreen = Color::create(127, 255, 127, 255);
        Color pink = Color::create(127, 0, 127, 255);
        Color lightPink = Color::create(255, 0, 255, 255);
        float displayHeight = rectsRenderCtx->canvasCtx->displayHeight;
        float frameTimesWidth =
            this->frameTimes.append(barsCtx, targetsCtx, displayHeight, 15, 15, green, lightGreen);
        float betweenFrameTimesX = 15 + frameTimesWidth + 15;
        this->betweenFrameTimes.append(
            barsCtx, targetsCtx, displayHeight, betweenFrameTimesX, 15, pink, lightPink);
        primitivesCommit(barsCtx);
        primitivesCommit(targetsCtx);

        primitivesRender(barsCtx, rectsRenderCtx, 0, 0);
        primitivesRender(targetsCtx, rectsRenderCtx, 0, 0);
    }
};
