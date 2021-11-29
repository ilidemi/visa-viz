#pragma once

#include "../render/text/renderText.cpp"
#include "../style.cpp"
#include "../tweetSchema.cpp"
#include "threadLayout.cpp"

struct LayoutedTweetLine
{
    const wchar_t *lineStart;
    int lineLength;
    List<HighlightedToken> highlightedTokens;
};

float layoutLine(
    TextRenderer *textRenderer, float lineX, float lineY, LayoutedTweetLine *line, float fontSize,
    float lineHeightFactor, Color baseColor, Color highlightColor, bool bold, ThreadLayout *relativeLayout)
{
    float tokenX = lineX;
    int preHighlightStartIndex = 0;
    list_for_each(token, line->highlightedTokens)
    {
        int preHighlightLength = token->startIndex - preHighlightStartIndex;
        tokenX += textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, tokenX, lineY, line->lineStart + preHighlightStartIndex,
            preHighlightLength, fontSize, lineHeightFactor, baseColor, bold);
        tokenX += textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, tokenX, lineY, line->lineStart + token->startIndex,
            token->endIndex - token->startIndex, fontSize, lineHeightFactor, highlightColor, bold);

        preHighlightStartIndex = token->endIndex;
    }

    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, tokenX, lineY, line->lineStart + preHighlightStartIndex,
        line->lineLength - preHighlightStartIndex, fontSize, lineHeightFactor, baseColor, bold);

    // Return line height
    return fontSize * lineHeightFactor;
}

template <typename TChar>
const TChar *advanceTillLineBreak(
    TextRenderer *textRenderer, const TChar *str, float widthLimit, int fontSize, Color color, bool bold,
    bool breakWithEllipsis)
{
    float wordX = 0;
    float spaceWidth = textRenderer->measureChar(' ', fontSize, color, bold);
    float ellipsisWidth = textRenderer->measureChar(L'…', fontSize, color, bold);
    const TChar *wordStart = str;
    while (*wordStart)
    {
        const TChar *wordEnd = wordStart;
        while (*wordEnd != ' ' && *wordEnd != '\0' && *wordEnd != '\n')
        {
            wordEnd++;
        }

        const TChar *lastFittingChar = wordStart;
        while (lastFittingChar < wordEnd)
        {
            float charWidth = textRenderer->measureChar(*lastFittingChar, fontSize, color, bold);
            if (wordX + charWidth > widthLimit)
            {
                lastFittingChar--;
                break;
            }

            wordX += charWidth;
            lastFittingChar++;
        }

        if (lastFittingChar < wordEnd - 1)
        {
            if (breakWithEllipsis)
            {
                while (wordX + ellipsisWidth > widthLimit && lastFittingChar > wordStart)
                {
                    wordX -= textRenderer->measureChar(*lastFittingChar, fontSize, color, bold);
                    lastFittingChar--;
                }
                return lastFittingChar + 1;
            }

            if (wordStart == str)
            {
                return lastFittingChar + 1;
            }

            return wordStart;
        }

        if (*wordEnd == '\n')
        {
            return wordEnd + 1;
        }

        wordStart = wordEnd;

        if (*wordEnd == ' ')
        {
            wordX += spaceWidth;
            wordStart++;
        }
    }

    return wordStart;
}

float measureHeaderDate(TextRenderer *textRenderer, StyleConfig *styleCfg, const char *date)
{
    float dateWidth = 5;
    dateWidth += textRenderer->measureText(L"·", styleCfg->mainFontSize, styleCfg->textAuxColor, false);
    dateWidth += 5;
    dateWidth += textRenderer->measureText(date, styleCfg->mainFontSize, styleCfg->textAuxColor, false);
    return dateWidth;
}

void addHeaderDateToAtlas(
    TextRenderer *textRenderer, StyleConfig *styleCfg, const char *date, float dateX, float dateY,
    ThreadLayout *relativeLayout)
{
    float textX = dateX;
    textX += 5;
    textX += textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, textX, dateY, L"·", styleCfg->mainFontSize, styleCfg->lineHeightFactor,
        styleCfg->textAuxColor, false);
    textX += 5;
    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, textX, dateY, date, styleCfg->mainFontSize, styleCfg->lineHeightFactor,
        styleCfg->textAuxColor, false);
}

float layoutHeader(
    TextRenderer *textRenderer, StyleConfig *styleCfg, User *user, const char *date, float headerX,
    float headerY, float headerMaxWidth, ThreadLayout *relativeLayout)
{
    float dateWidth = measureHeaderDate(textRenderer, styleCfg, date);
    float textX = headerX;
    float headerHeight = 20;

    const wchar_t *userNameEnd = advanceTillLineBreak(
        textRenderer, user->userName, headerMaxWidth - dateWidth, styleCfg->mainFontSize, styleCfg->textColor,
        true, true);
    int userNameDisplayLength = userNameEnd - user->userName;
    textX += textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, textX, headerY, user->userName, userNameDisplayLength,
        styleCfg->mainFontSize, styleCfg->lineHeightFactor, styleCfg->textColor, true);

    static const wchar_t *ellipsis = L"…";
    if (*userNameEnd != L'\0')
    {
        // Full username doesn't fit
        // Display partial username and align date to the right
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, textX, headerY, ellipsis, styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->textColor, true);
        addHeaderDateToAtlas(
            textRenderer, styleCfg, date, headerX + headerMaxWidth - dateWidth, headerY, relativeLayout);
        return headerHeight;
    }

    textX += 5;
    char displayScreenName[100];
    sprintf(displayScreenName, "@%s", user->screenName);
    const char *screenNameEnd = advanceTillLineBreak(
        textRenderer, displayScreenName, headerX + headerMaxWidth - dateWidth - textX, styleCfg->mainFontSize,
        styleCfg->textAuxColor, false, true);
    int screenNameLength = screenNameEnd - displayScreenName;
    if (screenNameLength == 0)
    {
        // Even a bit of screen name doesn't fit
        // Display full username and align date to the right
        addHeaderDateToAtlas(
            textRenderer, styleCfg, date, headerX + headerMaxWidth - dateWidth, headerY, relativeLayout);
        return headerHeight;
    }

    textX += textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, textX, headerY, displayScreenName, screenNameLength,
        styleCfg->mainFontSize, styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);

    if (*screenNameEnd != '\0')
    {
        // Full screen name doesn't fit
        // Display full username, partial screen name and align date to the right
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, textX, headerY, ellipsis, styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);
        addHeaderDateToAtlas(
            textRenderer, styleCfg, date, headerX + headerMaxWidth - dateWidth, headerY, relativeLayout);
        return headerHeight;
    }

    // Everything fits
    // Display full username, screen name and date aligned to the left
    addHeaderDateToAtlas(textRenderer, styleCfg, date, textX, headerY, relativeLayout);
    return headerHeight;
}

float layoutBodyText(
    TextRenderer *textRenderer, StyleConfig *styleCfg, TweetText *body, float bodyX, float bodyY,
    ThreadLayout *relativeLayout)
{
    const wchar_t *nextLineStart = body->text;
    float lineY = bodyY;
    while (*nextLineStart)
    {
        LayoutedTweetLine bodyLine;
        bodyLine.lineStart = nextLineStart;
        nextLineStart = advanceTillLineBreak(
            textRenderer, nextLineStart, styleCfg->tweetBodyWidth, styleCfg->mainFontSize,
            styleCfg->textColor, false, false);
        bodyLine.lineLength = nextLineStart - bodyLine.lineStart;

        if (body->highlightedTokens.size > 0)
        {
            bodyLine.highlightedTokens.init();

            int lineStartIndex = bodyLine.lineStart - body->text;
            int lineEndIndex = nextLineStart - body->text;
            list_for_each(sourceToken, body->highlightedTokens)
            {
                if (sourceToken->startIndex < lineEndIndex && sourceToken->endIndex > lineStartIndex + 1)
                {
                    HighlightedToken *destToken = bodyLine.highlightedTokens.getNext();
                    destToken->startIndex = sourceToken->startIndex >= lineStartIndex
                        ? sourceToken->startIndex - lineStartIndex
                        : 0;
                    destToken->endIndex = sourceToken->endIndex <= lineEndIndex
                        ? sourceToken->endIndex - lineStartIndex
                        : bodyLine.lineLength;
                }
            }
        }
        else
        {
            bodyLine.highlightedTokens.initForeverEmpty();
        }

        lineY += layoutLine(
            textRenderer, bodyX, lineY, &bodyLine, styleCfg->mainFontSize, styleCfg->lineHeightFactor,
            styleCfg->textColor, styleCfg->textHighlightColor, false, relativeLayout);
    }

    return lineY - bodyY;
}

void layoutMedia(
    TextRenderer *textRenderer, StyleConfig *styleCfg, Media *media, float mediaX, float mediaY,
    bool isQuoted, ThreadLayout *relativeLayout, float *outMediaHeight, float *outSourceUserHeight)
{
    float mediaHeight = styleCfg->tweetBodyWidth / media->aspectRatio;
    float sourceUserHeight = 0;
    if (media->type == MediaType::Photos)
    {
        List<Photo> *photos = &media->photos;
        float hitRectX = mediaX + styleCfg->roundedFrameBorder;
        float topBorder = !isQuoted * styleCfg->roundedFrameBorder;
        float hitRectY = mediaY + topBorder;
        if (photos->size == 1)
        {
            Photo *photo0 = photos->get(0);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, mediaX, mediaY, styleCfg->photoWidth, mediaHeight,
                photo0->thumbnailUrl);

            HitRect *hitRect = relativeLayout->photoHitRects.getNext();
            float hitRectHeight = mediaHeight - topBorder - styleCfg->roundedFrameBorder;
            hitRect->initOpenGalleryHitRect(
                hitRectX, hitRectY, styleCfg->photoHitRectWidth, hitRectHeight,
                RoundedCorners::create(!isQuoted, !isQuoted, true, true), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 0);
        }
        else if (photos->size == 2)
        {
            Photo *photo0 = photos->get(0);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, mediaX, mediaY, styleCfg->photoHalfWidth, mediaHeight,
                photo0->thumbnailUrl);

            HitRect *hitRect0 = relativeLayout->photoHitRects.getNext();
            float hitRectHeight = mediaHeight - topBorder - styleCfg->roundedFrameBorder;
            hitRect0->initOpenGalleryHitRect(
                hitRectX, hitRectY, styleCfg->photoHitRectHalfWidth, hitRectHeight,
                RoundedCorners::create(!isQuoted, false, true, false), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 0);

            Photo *photo1 = photos->get(1);
            float photo1X = mediaX + styleCfg->photoHalfWidth + styleCfg->photoGap;
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, photo1X, mediaY, styleCfg->photoHalfWidth, mediaHeight,
                photo1->thumbnailUrl);

            HitRect *hitRect1 = relativeLayout->photoHitRects.getNext();
            hitRect1->initOpenGalleryHitRect(
                photo1X, hitRectY, styleCfg->photoHitRectHalfWidth, hitRectHeight,
                RoundedCorners::create(false, !isQuoted, false, true), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 1);
        }
        else if (photos->size == 3)
        {
            float photoHalfHeight = (styleCfg->photoWidth / media->aspectRatio - styleCfg->photoGap) / 2;
            float rightPhotoX = mediaX + styleCfg->photoHalfWidth + styleCfg->photoGap;

            Photo *photo0 = photos->get(0);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, mediaX, mediaY, styleCfg->photoHalfWidth, mediaHeight,
                photo0->thumbnailUrl);

            HitRect *hitRect0 = relativeLayout->photoHitRects.getNext();
            float hitRect0Height = mediaHeight - topBorder - styleCfg->roundedFrameBorder;
            hitRect0->initOpenGalleryHitRect(
                hitRectX, hitRectY, styleCfg->photoHitRectHalfWidth, hitRect0Height,
                RoundedCorners::create(!isQuoted, false, true, false), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 0);

            Photo *photo1 = photos->get(1);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, rightPhotoX, mediaY, styleCfg->photoHalfWidth, photoHalfHeight,
                photo1->thumbnailUrl);

            HitRect *hitRect1 = relativeLayout->photoHitRects.getNext();
            float hitRect1Height = photoHalfHeight - topBorder;
            hitRect1->initOpenGalleryHitRect(
                rightPhotoX, hitRectY, styleCfg->photoHitRectHalfWidth, hitRect1Height,
                RoundedCorners::create(false, !isQuoted, false, false), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 1);

            Photo *photo2 = photos->get(2);
            float photo2Y = mediaY + photoHalfHeight + styleCfg->photoGap;
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, rightPhotoX, photo2Y, styleCfg->photoHalfWidth, photoHalfHeight,
                photo2->thumbnailUrl);

            HitRect *hitRect2 = relativeLayout->photoHitRects.getNext();
            float hitRect2Height = photoHalfHeight - styleCfg->roundedFrameBorder;
            hitRect2->initOpenGalleryHitRect(
                rightPhotoX, photo2Y, styleCfg->photoHitRectHalfWidth, hitRect2Height,
                RoundedCorners::create(false, false, false, true), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 2);
        }
        else if (photos->size == 4)
        {
            float photoHalfHeight = (styleCfg->photoWidth / media->aspectRatio - styleCfg->photoGap) / 2;
            float rightPhotoX = mediaX + styleCfg->photoHalfWidth + styleCfg->photoGap;
            float bottomPhotoY = mediaY + photoHalfHeight + styleCfg->photoGap;
            float topHitRectHeight = photoHalfHeight - topBorder;
            float bottomHitRectHeight = photoHalfHeight - styleCfg->roundedFrameBorder;

            Photo *photo0 = photos->get(0);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, mediaX, mediaY, styleCfg->photoHalfWidth, photoHalfHeight,
                photo0->thumbnailUrl);

            HitRect *hitRect0 = relativeLayout->photoHitRects.getNext();
            hitRect0->initOpenGalleryHitRect(
                hitRectX, hitRectY, styleCfg->photoHitRectHalfWidth, topHitRectHeight,
                RoundedCorners::create(!isQuoted, false, false, false), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 0);

            Photo *photo1 = photos->get(1);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, rightPhotoX, mediaY, styleCfg->photoHalfWidth, photoHalfHeight,
                photo1->thumbnailUrl);

            HitRect *hitRect1 = relativeLayout->photoHitRects.getNext();
            hitRect1->initOpenGalleryHitRect(
                rightPhotoX, hitRectY, styleCfg->photoHitRectHalfWidth, topHitRectHeight,
                RoundedCorners::create(false, !isQuoted, false, false), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 1);

            Photo *photo2 = photos->get(2);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, mediaX, bottomPhotoY, styleCfg->photoHalfWidth, photoHalfHeight,
                photo2->thumbnailUrl);

            HitRect *hitRect2 = relativeLayout->photoHitRects.getNext();
            hitRect2->initOpenGalleryHitRect(
                hitRectX, bottomPhotoY, styleCfg->photoHitRectHalfWidth, bottomHitRectHeight,
                RoundedCorners::create(false, false, true, false), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 2);

            Photo *photo3 = photos->get(3);
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, rightPhotoX, bottomPhotoY, styleCfg->photoHalfWidth, photoHalfHeight,
                photo3->thumbnailUrl);

            HitRect *hitRect3 = relativeLayout->photoHitRects.getNext();
            hitRect3->initOpenGalleryHitRect(
                rightPhotoX, bottomPhotoY, styleCfg->photoHitRectHalfWidth, bottomHitRectHeight,
                RoundedCorners::create(false, false, false, true), styleCfg->photoRoundedCornerRadius,
                styleCfg->photoHoverColor, photos, 3);
        }
        else
        {
            abortWithMessage("Too many photos in a tweet");
        }
    }
    else if (media->type == MediaType::AnimatedGif)
    {
        AnimatedGif *animatedGif = &media->animatedGif;
        relativeLayout->images.getNext()->init(
            CoordinateSpace::World, mediaX, mediaY, styleCfg->tweetBodyWidth, mediaHeight,
            animatedGif->thumbnailUrl);
        float playButtonOutlineX = mediaX + (styleCfg->tweetBodyWidth - styleCfg->playButtonSize) / 2;
        float playButtonOutlineY = mediaY + (mediaHeight - styleCfg->playButtonSize) / 2;
        relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
            playButtonOutlineX, playButtonOutlineY, styleCfg->playButtonSize, styleCfg->playButtonSize,
            styleCfg->playButtonRadius, styleCfg->playButtonBorderSize, styleCfg->playButtonInsideColor,
            styleCfg->playButtonBorderColor, styleCfg->playButtonOutsideColor));
        float playTriangleX = mediaX + (styleCfg->tweetBodyWidth - styleCfg->playTriangleDisplaySize) / 2 +
            styleCfg->playTriangleOffsetX;
        float playTriangleY = mediaY + (mediaHeight - styleCfg->playTriangleDisplaySize) / 2;
        relativeLayout->icons.append(LayoutedIcon::create(
            playTriangleX, playTriangleY, styleCfg->playTriangleDisplaySize, IconType::PlayTriangle));
        float gifLabelHeight = 20;
        float gifLabelY = mediaY + mediaHeight - 11 - gifLabelHeight;
        relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
            mediaX + 11, gifLabelY, 31, gifLabelHeight, styleCfg->gifLabelBackgroundRadius, 0,
            styleCfg->gifLabelBackgroundColor, styleCfg->gifLabelBackgroundColor,
            styleCfg->gifLabelBackgroundTransparentColor));
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, mediaX + 16, gifLabelY, "GIF", styleCfg->auxFontSize,
            19.5 / styleCfg->auxFontSize, styleCfg->white, true);
    }
    else if (media->type == MediaType::Video)
    {
        Video *video = &media->video;
        relativeLayout->images.getNext()->init(
            CoordinateSpace::World, mediaX, mediaY, styleCfg->tweetBodyWidth, mediaHeight,
            video->thumbnailUrl);
        float playButtonOutlineX = mediaX + (styleCfg->tweetBodyWidth - styleCfg->playButtonSize) / 2;
        float playButtonOutlineY = mediaY + (mediaHeight - styleCfg->playButtonSize) / 2;
        relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
            playButtonOutlineX, playButtonOutlineY, styleCfg->playButtonSize, styleCfg->playButtonSize,
            styleCfg->playButtonRadius, styleCfg->playButtonBorderSize, styleCfg->playButtonInsideColor,
            styleCfg->playButtonBorderColor, styleCfg->playButtonOutsideColor));
        float playTriangleX = mediaX + (styleCfg->tweetBodyWidth - styleCfg->playTriangleDisplaySize) / 2 +
            styleCfg->playTriangleOffsetX;
        float playTriangleY = mediaY + (mediaHeight - styleCfg->playTriangleDisplaySize) / 2;
        relativeLayout->icons.append(LayoutedIcon::create(
            playTriangleX, playTriangleY, styleCfg->playTriangleDisplaySize, IconType::PlayTriangle));

        if (video->sourceUser != NULL)
        {
            float videoSourceY = mediaY + mediaHeight + 5;
            float fromWidth = textRenderer->addTextToAtlas(
                &relativeLayout->atlasChars, mediaX, videoSourceY, "From ", styleCfg->auxFontSize,
                styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);
            textRenderer->addTextToAtlas(
                &relativeLayout->atlasChars, mediaX + fromWidth, videoSourceY, video->sourceUser,
                styleCfg->auxFontSize, styleCfg->lineHeightFactor, styleCfg->textColor, true);
            sourceUserHeight = 22;
        }
    }
    else if (media->type == MediaType::DmcaVideo)
    {
        relativeLayout->miscRects.append(LayoutedRect::create(
            mediaX, mediaY, styleCfg->tweetBodyWidth, mediaHeight, styleCfg->dmcaVideoBackgroundColor));

        static const char *dmcaText =
            "This media has been disabled in response to a report by the copyright owner.";
        static const int dmcaTextLength = strlen(dmcaText);

        const char *line2Start = advanceTillLineBreak(
            textRenderer, dmcaText, styleCfg->tweetBodyWidth - 2 * styleCfg->dmcaVideoTextPadding,
            styleCfg->mainFontSize, styleCfg->dmcaVideoTextColor, false, false);
        int line1Length = line2Start - dmcaText;
        int lineCount = line1Length == dmcaTextLength ? 1 : 2;
        float line1Width = textRenderer->measureText(
            dmcaText, line1Length, styleCfg->mainFontSize, styleCfg->dmcaVideoTextColor, false);
        float line1X = mediaX + (styleCfg->tweetBodyWidth - line1Width) / 2;
        float line1Y =
            mediaY + (mediaHeight - lineCount * styleCfg->mainFontSize * styleCfg->lineHeightFactor) / 2;
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, line1X, line1Y, dmcaText, line1Length, styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->dmcaVideoTextColor, false);

        if (lineCount == 2)
        {
            float line2Length = dmcaTextLength - line1Length;
            float line2Width = textRenderer->measureText(
                line2Start, line2Length, styleCfg->mainFontSize, styleCfg->dmcaVideoTextColor, false);
            float line2X = mediaX + (styleCfg->tweetBodyWidth - line2Width) / 2;
            float line2Y = line1Y + styleCfg->mainFontSize * styleCfg->lineHeightFactor;
            textRenderer->addTextToAtlas(
                &relativeLayout->atlasChars, line2X, line2Y, line2Start, line2Length, styleCfg->mainFontSize,
                styleCfg->lineHeightFactor, styleCfg->dmcaVideoTextColor, false);
        }
    }
    else
    {
        abortWithMessage("Media type not supported");
    }

    *outMediaHeight = mediaHeight;
    *outSourceUserHeight = sourceUserHeight;
}

void layoutLinkCardText(
    TextRenderer *textRenderer, StyleConfig *styleCfg, const wchar_t *text, float lineWidth,
    int maxLinesCount, const wchar_t **outLine0Start, const wchar_t **outLine1Start, int *outLine0Length,
    int *outLine1Length, int *outLinesCount, float *outHeight, bool *outEndWithEllipsis)
{
    if (text == NULL)
    {
        *outLinesCount = 0;
        *outHeight = 0;
        *outEndWithEllipsis = false;
        return;
    }

    *outLine0Start = text;
    if (maxLinesCount == 1)
    {
        const wchar_t *line0End = advanceTillLineBreak(
            textRenderer, *outLine0Start, lineWidth, styleCfg->mainFontSize, styleCfg->textColor, false,
            true);
        *outLine0Length = line0End - *outLine0Start;
        *outLinesCount = 1;
        *outEndWithEllipsis = *line0End != L'\0';
    }
    else if (maxLinesCount == 2)
    {
        *outLine1Start = advanceTillLineBreak(
            textRenderer, *outLine0Start, lineWidth, styleCfg->mainFontSize, styleCfg->textColor, false,
            false);
        *outLine0Length = *outLine1Start - *outLine0Start;
        if (**outLine1Start == L'\0')
        {
            *outLinesCount = 1;
            *outEndWithEllipsis = false;
        }
        else
        {
            const wchar_t *line1End = advanceTillLineBreak(
                textRenderer, *outLine1Start, lineWidth, styleCfg->mainFontSize, styleCfg->textColor, false,
                true);
            *outLine1Length = line1End - *outLine1Start;
            *outLinesCount = 2;
            *outEndWithEllipsis = *line1End != L'\0';
        }
    }
    else
    {
        abortWithMessage("Link card text can't have more than two lines");
    }

    *outHeight = 2 + *outLinesCount * styleCfg->mainFontSize * styleCfg->lineHeightFactor;
}

float layoutLinkCard(
    TextRenderer *textRenderer, StyleConfig *styleCfg, LinkCard *linkCard, float linkCardX, float linkCardY,
    ThreadLayout *relativeLayout)
{
    assertOrAbort(
        !(linkCard->type == LinkCardType::SummaryLargeImage && linkCard->image == NULL),
        "Large link card must have an image");

    int titleMaxLines = linkCard->description == NULL ? 2 : 1;
    const wchar_t *titleLineStarts[2];
    int titleLineLengths[2];
    int titleLinesCount;
    float titleHeight;
    bool titleEndWithEllipsis;

    const wchar_t *descLineStarts[2];
    int descLineLengths[2];
    int descLinesCount;
    float descHeight;
    bool descEndWithEllipsis;

    float textX, titleY, cardHeight;

    if (linkCard->type == LinkCardType::Summary || linkCard->type == LinkCardType::Player)
    {
        if (linkCard->image != NULL)
        {
            // Image
            relativeLayout->images.getNext()->init(
                CoordinateSpace::World, linkCardX, linkCardY, styleCfg->linkCardImageSize,
                styleCfg->linkCardImageSize, linkCard->image->thumbnailUrl);
        }
        else
        {
            // Background
            relativeLayout->cardBackgrounds.append(LayoutedRect::create(
                linkCardX, linkCardY, styleCfg->linkCardImageSize, styleCfg->linkCardImageSize,
                styleCfg->cardBackgroundColor));

            if (linkCard->type == LinkCardType::Summary)
            {
                // Icon
                relativeLayout->icons.append(LayoutedIcon::create(
                    linkCardX + (styleCfg->linkCardImageSize - styleCfg->linkThumbnailIconDisplaySize) / 2,
                    linkCardY + (styleCfg->linkCardImageSize - styleCfg->linkThumbnailIconDisplaySize) / 2,
                    styleCfg->linkThumbnailIconDisplaySize, IconType::LinkThumbnail));
            }
        }

        if (linkCard->type == LinkCardType::Player)
        {
            // Play button
            float playButtonOutlineX =
                linkCardX + (styleCfg->linkCardImageSize - styleCfg->miniPlayButtonSize) / 2;
            float playButtonOutlineY =
                linkCardY + (styleCfg->linkCardImageSize - styleCfg->miniPlayButtonSize) / 2;
            relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
                playButtonOutlineX, playButtonOutlineY, styleCfg->miniPlayButtonSize,
                styleCfg->miniPlayButtonSize, styleCfg->miniPlayButtonRadius,
                styleCfg->miniPlayButtonBorderSize, styleCfg->playButtonInsideColor,
                styleCfg->playButtonBorderColor, styleCfg->playButtonOutsideColor));
            float playTriangleX = linkCardX +
                (styleCfg->linkCardImageSize - styleCfg->miniPlayTriangleDisplaySize) / 2 +
                styleCfg->miniPlayTriangleOffsetX;
            float playTriangleY =
                linkCardY + (styleCfg->linkCardImageSize - styleCfg->miniPlayTriangleDisplaySize) / 2;
            relativeLayout->icons.append(LayoutedIcon::create(
                playTriangleX, playTriangleY, styleCfg->miniPlayTriangleDisplaySize, IconType::PlayTriangle));
        }

        // Separator
        relativeLayout->miscRects.append(LayoutedRect::create(
            linkCardX + styleCfg->linkCardImageSize - 1, linkCardY, 1, styleCfg->linkCardImageSize,
            styleCfg->outlineColor));

        // Text setup
        textX = linkCardX + styleCfg->linkCardImageSize + 10;
        float lineWidth = styleCfg->tweetBodyWidth - styleCfg->linkCardImageSize - 21;
        layoutLinkCardText(
            textRenderer, styleCfg, linkCard->title, lineWidth, titleMaxLines, &titleLineStarts[0],
            &titleLineStarts[1], &titleLineLengths[0], &titleLineLengths[1], &titleLinesCount, &titleHeight,
            &titleEndWithEllipsis);
        int descMaxLines = 3 - titleLinesCount;
        layoutLinkCardText(
            textRenderer, styleCfg, linkCard->description, lineWidth, descMaxLines, &descLineStarts[0],
            &descLineStarts[1], &descLineLengths[0], &descLineLengths[1], &descLinesCount, &descHeight,
            &descEndWithEllipsis);
        float cardTextHeight = titleHeight + descHeight + 2 + 20;
        titleY = (styleCfg->linkCardImageSize - cardTextHeight) / 2;
        cardHeight = styleCfg->linkCardImageSize;
    }
    else if (linkCard->type == LinkCardType::SummaryLargeImage)
    {
        // Image
        float imageHeight = styleCfg->tweetBodyWidth / linkCard->image->aspectRatio;
        relativeLayout->images.getNext()->init(
            CoordinateSpace::World, linkCardX, linkCardY, styleCfg->tweetBodyWidth, imageHeight,
            linkCard->image->thumbnailUrl);

        // Separator
        relativeLayout->miscRects.append(LayoutedRect::create(
            linkCardX, linkCardY + imageHeight, styleCfg->tweetBodyWidth, 1, styleCfg->outlineColor));

        // Text setup
        textX = linkCardX + 11;
        float lineWidth = styleCfg->tweetBodyWidth - 22;
        layoutLinkCardText(
            textRenderer, styleCfg, linkCard->title, lineWidth, titleMaxLines, &titleLineStarts[0],
            &titleLineStarts[1], &titleLineLengths[0], &titleLineLengths[1], &titleLinesCount, &titleHeight,
            &titleEndWithEllipsis);
        int descMaxLines = 3 - titleLinesCount;
        layoutLinkCardText(
            textRenderer, styleCfg, linkCard->description, lineWidth, descMaxLines, &descLineStarts[0],
            &descLineStarts[1], &descLineLengths[0], &descLineLengths[1], &descLinesCount, &descHeight,
            &descEndWithEllipsis);
        titleY = imageHeight + 10;
        cardHeight = imageHeight + 10 + titleHeight + descHeight + 2 + 20 + 10 + 1;
    }
    else
    {
        abortWithMessage("Link card type not supported");
    }

    // Title
    float titleLastLineWidth;
    for (int lineIdx = 0; lineIdx < titleLinesCount; lineIdx++)
    {
        titleLastLineWidth = textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, textX,
            linkCardY + titleY + lineIdx * styleCfg->mainFontSize * styleCfg->lineHeightFactor,
            titleLineStarts[lineIdx], titleLineLengths[lineIdx], styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->textColor, false);
    }
    if (titleEndWithEllipsis)
    {
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, textX + titleLastLineWidth, linkCardY + titleY, L"…",
            styleCfg->mainFontSize, styleCfg->lineHeightFactor, styleCfg->textColor, false);
    }

    // Description
    float descY = titleY + titleHeight + 2;
    float descLastLineWidth;
    for (int lineIdx = 0; lineIdx < descLinesCount; lineIdx++)
    {
        descLastLineWidth = textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, textX,
            linkCardY + descY + lineIdx * styleCfg->mainFontSize * styleCfg->lineHeightFactor,
            descLineStarts[lineIdx], descLineLengths[lineIdx], styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);
    }
    if (descEndWithEllipsis)
    {
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, textX + descLastLineWidth,
            linkCardY + descY + (descLinesCount - 1) * styleCfg->mainFontSize * styleCfg->lineHeightFactor,
            L"…", styleCfg->mainFontSize, styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);
    }

    // Domain with icon
    float domainY = descY + descHeight + 2;
    relativeLayout->icons.append(
        LayoutedIcon::create(textX, linkCardY + domainY + 2, styleCfg->linkIconDisplaySize, IconType::Link));
    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, textX + 18, linkCardY + domainY, linkCard->domain,
        styleCfg->mainFontSize, styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);

    // Frame
    relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
        linkCardX, linkCardY, styleCfg->tweetBodyWidth, cardHeight, styleCfg->roundedCornerRadius,
        styleCfg->roundedFrameBorder, styleCfg->outlineTransparentColor, styleCfg->outlineColor,
        styleCfg->white));

    return cardHeight;
}

float layoutCounters(
    TextRenderer *textRenderer, StyleConfig *styleCfg, const char *replyCount, const char *retweetCount,
    const char *favoriteCount, float countersX, float countersY, ThreadLayout *relativeLayout)
{
    relativeLayout->icons.append(
        LayoutedIcon::create(countersX, countersY, styleCfg->counterDisplaySize, IconType::Reply));
    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, countersX + 29, countersY + 1, replyCount, styleCfg->auxFontSize,
        styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);
    relativeLayout->icons.append(
        LayoutedIcon::create(countersX + 153, countersY, styleCfg->counterDisplaySize, IconType::Retweet));
    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, countersX + 182, countersY + 1, retweetCount, styleCfg->auxFontSize,
        styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);
    relativeLayout->icons.append(
        LayoutedIcon::create(countersX + 306, countersY, styleCfg->counterDisplaySize, IconType::Favorite));
    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, countersX + 335, countersY + 1, favoriteCount, styleCfg->auxFontSize,
        styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);

    return 19;
}

float layoutPoll(
    TextRenderer *textRenderer, StyleConfig *styleCfg, Poll *poll, float pollX, float pollY,
    ThreadLayout *relativeLayout)
{
    for (int choiceIdx = 0; choiceIdx < poll->choicesCount; choiceIdx++)
    {
        bool isWinningChoice = choiceIdx == poll->winningChoiceIndex;
        Color barColor = isWinningChoice ? styleCfg->pollWinningBarColor : styleCfg->pollBarColor;
        Color transparentBarColor =
            isWinningChoice ? styleCfg->pollWinningBarTransparentColor : styleCfg->pollBarTransparentColor;

        // Bar
        float barY = pollY + 35 * choiceIdx;
        float barWidth = fmaxf(poll->voteFractions[choiceIdx] * styleCfg->tweetBodyWidth, 7);
        float cornerRadius = fminf(styleCfg->pollRoundedCornerRadius, barWidth / 2);
        relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
            pollX, barY, barWidth, 30, cornerRadius, styleCfg->roundedFrameBorder, barColor, barColor,
            transparentBarColor));

        // Choice text
        float textY = barY + 5;
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, pollX + 10, textY, poll->choices[choiceIdx], styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->textColor, isWinningChoice);

        // Percentage text
        float percentageTextWidth = textRenderer->measureText(
            poll->votePercentagesText[choiceIdx], styleCfg->mainFontSize, styleCfg->textColor,
            choiceIdx == poll->winningChoiceIndex);
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, pollX + styleCfg->tweetBodyWidth - percentageTextWidth, textY,
            poll->votePercentagesText[choiceIdx], styleCfg->mainFontSize, styleCfg->lineHeightFactor,
            styleCfg->textColor, isWinningChoice);
    }

    // Totals text
    float totalVotesY = pollY + 30 + (poll->choicesCount - 1) * 35 + 10;
    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, pollX, totalVotesY, poll->totalVotes, styleCfg->auxFontSize,
        styleCfg->lineHeightFactor * styleCfg->mainFontSize / styleCfg->auxFontSize, styleCfg->textAuxColor,
        false);

    return totalVotesY + 17 - pollY;
}

float layoutQuotedBodyText(
    TextRenderer *textRenderer, StyleConfig *styleCfg, const wchar_t *body, float bodyX, float bodyY,
    ThreadLayout *relativeLayout)
{
    const wchar_t *nextLineStart = body;
    float lineY = bodyY;
    while (*nextLineStart)
    {
        const wchar_t *lineStart = nextLineStart;
        nextLineStart = advanceTillLineBreak(
            textRenderer, nextLineStart, styleCfg->quotedTweetBodyWidth, styleCfg->mainFontSize,
            styleCfg->textColor, false, false);
        int lineLength = nextLineStart - lineStart;

        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, bodyX, lineY, lineStart, lineLength, styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->textColor, false);

        lineY += styleCfg->mainFontSize * styleCfg->lineHeightFactor;
    }

    return lineY - bodyY;
}

float layoutRegularQuotedTweet(
    TextRenderer *textRenderer, StyleConfig *styleCfg, User *visa, QuotedTweet *quotedTweet,
    float quotedTweetX, float quotedTweetY, bool isQuotedTweetExternal, ThreadLayout *relativeLayout)
{
    assertOrAbort(quotedTweet->type == TweetType::Regular, "Can only render regular quoted tweets");

    float bodyX = quotedTweetX + 11;
    float avatarY = quotedTweetY + 11;

    // Avatar
    if (quotedTweet->user == visa)
    {
        relativeLayout->visaPics.append(
            LayoutedVisaPic::create(bodyX, avatarY, styleCfg->quotedAvatarDisplaySize));
    }
    else
    {
        relativeLayout->userPics.getNext()->init(
            CoordinateSpace::World, bodyX, avatarY, styleCfg->quotedAvatarDisplaySize,
            styleCfg->quotedAvatarDisplaySize, quotedTweet->user->profileImageUrl);
    }

    // Header
    float headerX = bodyX + styleCfg->quotedAvatarDisplaySize + 5;
    float headerY = quotedTweetY + 9;
    float headerMaxWidth = styleCfg->quotedTweetBodyWidth - 5 - styleCfg->quotedAvatarDisplaySize - 5 -
        styleCfg->externalTweetIconSize * isQuotedTweetExternal;
    float headerHeight = layoutHeader(
        textRenderer, styleCfg, quotedTweet->user, quotedTweet->date, headerX, headerY, headerMaxWidth,
        relativeLayout);
    float headerEndY = headerY + headerHeight;

    // External tweet icon
    float externalTweetIconX = bodyX + styleCfg->quotedTweetBodyWidth - styleCfg->externalTweetIconSize;
    float externalTweetIconY = headerY;
    if (isQuotedTweetExternal)
    {
        relativeLayout->icons.append(LayoutedIcon::create(
            externalTweetIconX, externalTweetIconY, styleCfg->externalTweetIconSize,
            IconType::ExternalTweet));
    }

    // Mentions
    float mentionsY = headerEndY;
    float mentionsEndY = mentionsY;
    if (quotedTweet->mentions != NULL)
    {
        float mentionsTopMargin = 5;
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, bodyX, mentionsY + mentionsTopMargin, quotedTweet->mentions,
            styleCfg->mainFontSize, styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);
        mentionsEndY = mentionsY + mentionsTopMargin + 20;
    }

    // Body
    float bodyY = mentionsEndY;
    float bodyTopMargin = 5;
    float bodyHeight = layoutQuotedBodyText(
        textRenderer, styleCfg, quotedTweet->body, bodyX, bodyY + bodyTopMargin, relativeLayout);
    float bodyBottomY = bodyHeight > 0 ? bodyY + bodyTopMargin + bodyHeight : bodyY;

    // Expand
    float expandY = bodyBottomY;
    float expandBottomY = expandY;
    if (quotedTweet->expandType == QuotedTweetExpandType::Poll ||
        quotedTweet->expandType == QuotedTweetExpandType::Thread)
    {
        float expandTopMargin = 5;
        const char *expandText =
            quotedTweet->expandType == QuotedTweetExpandType::Poll ? "Show this poll" : "Show this thread";
        textRenderer->addTextToAtlas(
            &relativeLayout->atlasChars, bodyX, expandY + expandTopMargin, expandText, styleCfg->mainFontSize,
            styleCfg->lineHeightFactor, styleCfg->textHighlightColor, false);
        expandBottomY = expandY + expandTopMargin + 20;
    }
    else if (quotedTweet->expandType != QuotedTweetExpandType::None)
    {
        abortWithMessage("Quoted tweet expand type not supported");
    }

    // Media
    float mediaY = expandBottomY;
    float mediaEndY = mediaY;
    if (quotedTweet->media != NULL)
    {
        Media *media = quotedTweet->media;
        assertOrAbort(
            !(media->type == MediaType::Video && media->video.sourceUser != NULL),
            "Video source user is not supported in quoted tweets");
        float mediaTopMargin = 15;
        float mediaHeight, sourceUserHeight;
        layoutMedia(
            textRenderer, styleCfg, quotedTweet->media, quotedTweetX, mediaY + mediaTopMargin, true,
            relativeLayout, &mediaHeight, &sourceUserHeight);
        mediaEndY = mediaY + mediaTopMargin + mediaHeight;
    }

    // Outline
    float quotedTweetHeight = (mediaEndY != mediaY ? mediaEndY : mediaY + 11) - quotedTweetY;
    relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
        quotedTweetX, quotedTweetY, styleCfg->tweetBodyWidth, quotedTweetHeight,
        styleCfg->roundedCornerRadius, styleCfg->roundedFrameBorder, styleCfg->outlineTransparentColor,
        styleCfg->outlineColor, styleCfg->white));

    // Hit rect
    if (isQuotedTweetExternal)
    {
        HitRect *quotedTweetHitRect = relativeLayout->quotedTweetHitRects.getNext();
        quotedTweetHitRect->initExternalTweetHitRect(
            quotedTweetX, quotedTweetY, styleCfg->tweetBodyWidth, quotedTweetHeight,
            RoundedCorners::createAll(), styleCfg->roundedCornerRadius, styleCfg->tweetHoverColor,
            quotedTweet->user->screenName, quotedTweet->id, externalTweetIconX, externalTweetIconY,
            styleCfg->externalTweetIconSize, "externalTweetRegularHover.d1d0ce06.png");
    }

    return quotedTweetHeight;
}

float layoutDeletedQuotedTweet(
    TextRenderer *textRenderer, StyleConfig *styleCfg, QuotedTweet *quotedTweet, float quotedTweetX,
    float quotedTweetY, ThreadLayout *relativeLayout)
{
    assertOrAbort(quotedTweet->type == TweetType::Deleted, "Can only render deleted quoted tweets");

    float quotedTweetHeight = 42;
    relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
        quotedTweetX, quotedTweetY, styleCfg->tweetBodyWidth, quotedTweetHeight,
        styleCfg->roundedCornerRadius, styleCfg->roundedFrameBorder, styleCfg->cardBackgroundColor,
        styleCfg->unavailableTweetOutlineColor, styleCfg->unavailableTweetOutlineTransparentColor));

    LayoutedTweetLine deletedLine;
    deletedLine.lineStart = L"This Tweet is unavailable.";
    deletedLine.lineLength = wcslen(deletedLine.lineStart);
    deletedLine.highlightedTokens.initForeverEmpty();

    textRenderer->addTextToAtlas(
        &relativeLayout->atlasChars, quotedTweetX + 16, quotedTweetY + 11, "This Tweet is unavailable.",
        styleCfg->mainFontSize, styleCfg->lineHeightFactor, styleCfg->textAuxColor, false);

    return quotedTweetHeight;
}

void layoutCopyrightedText(
    TextRenderer *textRenderer, StyleConfig *styleCfg, const char *screenName, float textX, float textY,
    float lineWidth, ThreadLayout *relativeLayout)
{
    wchar_t *copyrightedText = (wchar_t *)malloc(sizeof(wchar_t) * 120);
    int textLength = swprintf(
        copyrightedText, 120,
        L"This Tweet from %s has been withheld in response to a report from the copyright holder. Learn more",
        screenName);

    const wchar_t *line1Start = advanceTillLineBreak(
        textRenderer, copyrightedText, lineWidth, styleCfg->mainFontSize, styleCfg->textAuxColor, false,
        false);

    LayoutedTweetLine line0;
    line0.lineStart = copyrightedText;
    line0.lineLength = line1Start - copyrightedText;
    line0.highlightedTokens.initForeverEmpty();

    LayoutedTweetLine line1;
    line1.lineStart = line1Start;
    line1.lineLength = copyrightedText + textLength - line1Start;
    line1.highlightedTokens.init();
    line1.highlightedTokens.append({ line1.lineLength - 10, line1.lineLength });

    float lineY = textY;
    lineY += layoutLine(
        textRenderer, textX, lineY, &line0, styleCfg->mainFontSize, styleCfg->lineHeightFactor,
        styleCfg->textAuxColor, styleCfg->textHighlightColor, false, relativeLayout);
    layoutLine(
        textRenderer, textX, lineY, &line1, styleCfg->mainFontSize, styleCfg->lineHeightFactor,
        styleCfg->textAuxColor, styleCfg->textHighlightColor, false, relativeLayout);
}

float layoutCopyrightedQuotedTweet(
    TextRenderer *textRenderer, StyleConfig *styleCfg, QuotedTweet *quotedTweet, float quotedTweetX,
    float quotedTweetY, ThreadLayout *relativeLayout)
{
    assertOrAbort(quotedTweet->type == TweetType::Copyrighted, "Can only render copyrighted quoted tweets");

    float quotedTweetHeight = 62;
    relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
        quotedTweetX, quotedTweetY, styleCfg->tweetBodyWidth, quotedTweetHeight,
        styleCfg->roundedCornerRadius, styleCfg->roundedFrameBorder, styleCfg->cardBackgroundColor,
        styleCfg->unavailableTweetOutlineColor, styleCfg->unavailableTweetOutlineTransparentColor));

    layoutCopyrightedText(
        textRenderer, styleCfg, quotedTweet->copyrightedUserScreenName, quotedTweetX + 16, quotedTweetY + 11,
        styleCfg->tweetBodyWidth - 32, relativeLayout);

    return quotedTweetHeight;
}

void layoutFromPort(
    StyleConfig *styleCfg, List<int64_t> *fromPortTweetIds, Tweet *tweet, float quotedTweetMidY,
    ThreadLayout *relativeLayout)
{
    for (int fromPortIdx = 0; fromPortIdx < fromPortTweetIds->size; fromPortIdx++)
    {
        if (tweet->id == *fromPortTweetIds->get(fromPortIdx))
        {
            // Hit rect
            HitRect *fromPortHitRect = relativeLayout->fromPortHitRects.get(fromPortIdx);
            float hitRectMidX = styleCfg->threadWidth + styleCfg->arrowTipSize / 3;
            float hitRectX = hitRectMidX - styleCfg->arrowTipHitRadius;
            float hitRectY = quotedTweetMidY - styleCfg->arrowTipHitRadius;
            fromPortHitRect->initToggleThreadHitRect(
                hitRectX, hitRectY, styleCfg->arrowTipHitDiameter, styleCfg->arrowTipHitDiameter,
                RoundedCorners::createAll(), styleCfg->arrowTipHitRadius, styleCfg->arrowHoverColor,
                tweet->id, tweet->quotedTweet->id, 0, 1);

            // Arrow tip
            LayoutedArrowTip *fromArrowTip = relativeLayout->fromPortArrowTips.get(fromPortIdx);
            fromArrowTip->init(
                styleCfg->threadWidth, quotedTweetMidY, styleCfg->arrowTipSize, styleCfg->arrowTipHeight);

            break;
        }
    }
}

void layoutRegularTweet(
    TextRenderer *textRenderer, StyleConfig *styleCfg, HashMap<int64_t, Thread *> *tweetIdToThread,
    User *visa, List<int64_t> *fromPortTweetIds, Tweet *tweet, float tweetX, float tweetY,
    RoundedCorners hitRectRoundedCorners, ThreadLayout *relativeLayout, float *outTweetHeight,
    float *outThreadLineStartY, float *outQuotedTweetMidY)
{
    assertOrAbort(tweet->type == TweetType::Regular, "Can only layout regular tweets");

    // Avatar
    float headerY = tweetY + 10;
    float avatarX = tweetX + 15;
    if (tweet->user == visa)
    {
        relativeLayout->visaPics.append(
            LayoutedVisaPic::create(avatarX, headerY, styleCfg->avatarDisplaySize));
    }
    else
    {
        relativeLayout->userPics.getNext()->init(
            CoordinateSpace::World, avatarX, headerY, styleCfg->avatarDisplaySize,
            styleCfg->avatarDisplaySize, tweet->user->profileImageUrl);
    }

    // Header
    float bodyX = avatarX + styleCfg->avatarDisplaySize + 10;
    float headerMaxWidth = styleCfg->tweetBodyWidth - styleCfg->externalTweetIconSize;
    float headerHeight = layoutHeader(
        textRenderer, styleCfg, tweet->user, tweet->date, bodyX, headerY, headerMaxWidth, relativeLayout);
    float headerEndY = headerY + headerHeight;

    // External tweet icon
    float externalTweetIconX = bodyX + headerMaxWidth;
    float externalTweetIconY = headerY;
    relativeLayout->icons.append(LayoutedIcon::create(
        externalTweetIconX, externalTweetIconY, styleCfg->externalTweetIconSize, IconType::ExternalTweet));

    // Body
    float bodyY = headerEndY;
    float bodyTopMargin = 2;
    float bodyHeight =
        layoutBodyText(textRenderer, styleCfg, &tweet->body, bodyX, bodyY + bodyTopMargin, relativeLayout);
    float bodyEndY = bodyY + (bodyHeight > 0 ? bodyTopMargin + bodyHeight : 0);

    // Media
    float mediaY = bodyEndY;
    float mediaEndY = mediaY;
    if (tweet->media != NULL)
    {
        float mediaTopMargin = 10;
        float mediaHeight, sourceUserHeight;
        layoutMedia(
            textRenderer, styleCfg, tweet->media, bodyX, mediaY + mediaTopMargin, false, relativeLayout,
            &mediaHeight, &sourceUserHeight);
        relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
            bodyX, mediaY + mediaTopMargin, styleCfg->tweetBodyWidth, mediaHeight,
            styleCfg->roundedCornerRadius, styleCfg->roundedFrameBorder, styleCfg->outlineTransparentColor,
            styleCfg->outlineColor, styleCfg->white));
        mediaEndY = mediaY + mediaTopMargin + mediaHeight + sourceUserHeight;
    }

    // Link card
    float linkCardY = mediaEndY;
    float linkCardEndY = linkCardY;
    if (tweet->linkCard != NULL)
    {
        float linkCardTopMargin = 10;
        float linkCardHeight = layoutLinkCard(
            textRenderer, styleCfg, tweet->linkCard, bodyX, linkCardY + linkCardTopMargin, relativeLayout);
        linkCardEndY = linkCardY + linkCardTopMargin + linkCardHeight;
    }

    // Poll
    float pollY = linkCardEndY;
    float pollEndY = pollY;
    if (tweet->poll != NULL)
    {
        float pollTopMargin = 10;
        float pollHeight =
            layoutPoll(textRenderer, styleCfg, tweet->poll, bodyX, pollY + pollTopMargin, relativeLayout);
        pollEndY = pollY + pollTopMargin + pollHeight;
    }

    // Quoted tweet
    float quotedTweetY = pollEndY;
    float quotedTweetEndY = quotedTweetY;
    if (tweet->quotedTweet != NULL)
    {
        bool isQuotedTweetInThreadNetwork = tweetIdToThread->contains(tweet->quotedTweet->id);
        bool isQuotedTweetExternal =
            !isQuotedTweetInThreadNetwork && tweet->quotedTweet->type == TweetType::Regular;

        float quotedTweetTopMargin = 10;
        float quotedTweetBodyY = quotedTweetY + quotedTweetTopMargin;
        float quotedTweetHeight;
        if (tweet->quotedTweet->type == TweetType::Regular)
        {
            quotedTweetHeight = layoutRegularQuotedTweet(
                textRenderer, styleCfg, visa, tweet->quotedTweet, bodyX, quotedTweetBodyY,
                isQuotedTweetExternal, relativeLayout);
        }
        else if (tweet->quotedTweet->type == TweetType::Deleted)
        {
            quotedTweetHeight = layoutDeletedQuotedTweet(
                textRenderer, styleCfg, tweet->quotedTweet, bodyX, quotedTweetBodyY, relativeLayout);
        }
        else if (tweet->quotedTweet->type == TweetType::Copyrighted)
        {
            quotedTweetHeight = layoutCopyrightedQuotedTweet(
                textRenderer, styleCfg, tweet->quotedTweet, bodyX, quotedTweetBodyY, relativeLayout);
        }
        else
        {
            abortWithMessage("Quoted tweet type not supported");
        }

        // Hit rect
        if (isQuotedTweetInThreadNetwork)
        {
            HitRect *quotedTweetHitRect = relativeLayout->quotedTweetHitRects.getNext();
            quotedTweetHitRect->initToggleThreadHitRect(
                bodyX, quotedTweetBodyY, styleCfg->tweetBodyWidth, quotedTweetHeight,
                RoundedCorners::createAll(), styleCfg->roundedCornerRadius, styleCfg->tweetHoverColor,
                tweet->id, tweet->quotedTweet->id, 0, 1);
        }

        // From arrow tip
        float quotedTweetMidY = quotedTweetBodyY + quotedTweetHeight / 2;
        layoutFromPort(styleCfg, fromPortTweetIds, tweet, quotedTweetMidY, relativeLayout);

        if (outQuotedTweetMidY != NULL)
        {
            *outQuotedTweetMidY = quotedTweetMidY;
        }

        quotedTweetEndY = quotedTweetBodyY + quotedTweetHeight;
    }

    // Counters
    float countersY = quotedTweetEndY;
    float countersTopMargin = 10;
    float countersHeight = layoutCounters(
        textRenderer, styleCfg, tweet->replyCount, tweet->retweetCount, tweet->favoriteCount, bodyX,
        countersY + countersTopMargin, relativeLayout);
    float countersEndY = countersY + countersTopMargin + countersHeight;

    float tweetBottomMargin = 10;
    float tweetHeight = countersEndY + tweetBottomMargin - tweetY;

    // Hit rect
    HitRect *tweetHitRect = relativeLayout->tweetHitRects.getNext();
    tweetHitRect->initExternalTweetHitRect(
        tweetX, tweetY, styleCfg->tweetWidth, tweetHeight, hitRectRoundedCorners,
        styleCfg->roundedCornerRadius, styleCfg->tweetHoverColor, tweet->user->screenName, tweet->id,
        externalTweetIconX, externalTweetIconY, styleCfg->externalTweetIconSize,
        "externalTweetRegularHover.d1d0ce06.png");

    *outTweetHeight = tweetHeight;
    *outThreadLineStartY = headerY + styleCfg->avatarDisplaySize + 5;
}

void layoutUnavailableExternalTweet(
    StyleConfig *styleCfg, Tweet *tweet, float tweetX, float tweetY, float tweetHeight, float outlineX,
    float outlineY, const char *authorScreenName, RoundedCorners hitRectRoundedCorners,
    ThreadLayout *relativeLayout)
{
    // External tweet icon
    float externalTweetIconX = outlineX + styleCfg->unavailableTweetBodyWidth - styleCfg->roundedFrameBorder -
        10 - styleCfg->externalTweetIconSize;
    float externalTweetIconY = outlineY + styleCfg->roundedFrameBorder + 11;
    relativeLayout->icons.append(LayoutedIcon::create(
        externalTweetIconX, externalTweetIconY, styleCfg->externalTweetIconSize, IconType::ExternalTweet));

    // Hit rect
    HitRect *tweetHitRect = relativeLayout->tweetHitRects.getNext();
    tweetHitRect->initExternalTweetHitRect(
        tweetX, tweetY, styleCfg->tweetWidth, tweetHeight, hitRectRoundedCorners,
        styleCfg->roundedCornerRadius, styleCfg->tweetHoverColor, authorScreenName, tweet->id,
        externalTweetIconX, externalTweetIconY, styleCfg->externalTweetIconSize,
        "externalTweetUnavailableHover.e9a8c1da.png");
}

void layoutDeletedTweet(
    TextRenderer *textRenderer, StyleConfig *styleCfg, Tweet *tweet, float tweetX, float tweetY,
    RoundedCorners hitRectRoundedCorners, const char *visaScreenName, ThreadLayout *relativeLayout,
    float *outTweetHeight, float *outThreadLineStartY)
{
    assertOrAbort(tweet->type == TweetType::Deleted, "Can only layout deleted tweets");

    // Outline
    float outlineX = tweetX + 15;
    float outlineY = tweetY + 10;
    relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
        outlineX, outlineY, styleCfg->unavailableTweetBodyWidth, 42, styleCfg->roundedCornerRadius,
        styleCfg->roundedFrameBorder, styleCfg->cardBackgroundColor, styleCfg->unavailableTweetOutlineColor,
        styleCfg->unavailableTweetOutlineTransparentColor));

    // Text
    LayoutedTweetLine deletedLine;
    deletedLine.lineStart = L"This Tweet was deleted by the Tweet author. Learn more";
    deletedLine.lineLength = wcslen(deletedLine.lineStart);
    deletedLine.highlightedTokens.init();
    deletedLine.highlightedTokens.append({ deletedLine.lineLength - 10, deletedLine.lineLength });
    layoutLine(
        textRenderer, tweetX + 31, tweetY + 21, &deletedLine, styleCfg->mainFontSize,
        styleCfg->lineHeightFactor, styleCfg->textAuxColor, styleCfg->textHighlightColor, false,
        relativeLayout);

    float tweetHeight = 62;

    // Hit rect
    layoutUnavailableExternalTweet(
        styleCfg, tweet, tweetX, tweetY, tweetHeight, outlineX, outlineY, visaScreenName,
        hitRectRoundedCorners, relativeLayout);

    *outTweetHeight = tweetHeight;
    *outThreadLineStartY = tweetY + *outTweetHeight - 5;
}

void layoutCopyrightedTweet(
    TextRenderer *textRenderer, StyleConfig *styleCfg, Tweet *tweet, float tweetX, float tweetY,
    RoundedCorners hitRectRoundedCorners, ThreadLayout *relativeLayout, float *outTweetHeight,
    float *outThreadLineStartY)
{
    assertOrAbort(tweet->type == TweetType::Copyrighted, "Can only layout copyrighted tweets");

    // Outline
    float outlineX = tweetX + 15;
    float outlineY = tweetY + 10;
    relativeLayout->miscRoundedFrames.append(LayoutedRoundedFrame::create(
        outlineX, outlineY, styleCfg->unavailableTweetBodyWidth, 62, styleCfg->roundedCornerRadius,
        styleCfg->roundedFrameBorder, styleCfg->cardBackgroundColor, styleCfg->unavailableTweetOutlineColor,
        styleCfg->unavailableTweetOutlineTransparentColor));

    // Text
    layoutCopyrightedText(
        textRenderer, styleCfg, tweet->copyrightedUserScreenName, tweetX + 31, tweetY + 21,
        styleCfg->unavailableTweetBodyWidth - 32 - styleCfg->externalTweetIconSize, relativeLayout);

    float tweetHeight = 82;

    // Hit rect
    layoutUnavailableExternalTweet(
        styleCfg, tweet, tweetX, tweetY, tweetHeight, outlineX, outlineY, tweet->copyrightedUserScreenName,
        hitRectRoundedCorners, relativeLayout);

    *outTweetHeight = tweetHeight;
    *outThreadLineStartY = tweetY + *outTweetHeight - 5;
}

void layoutToPort(
    StyleConfig *styleCfg, List<int64_t> *toPortSourceTweetIds, List<int64_t> *toPortTargetTweetIds,
    Tweet *tweet, float tweetHeight, float tweetMidY, ThreadLayout *relativeLayout)
{
    int toPortsMatchCount = 0;
    list_for_each(toPortTargetTweetId, *toPortTargetTweetIds)
    {
        if (tweet->id == *toPortTargetTweetId)
        {
            toPortsMatchCount++;
        }
    }

    float arrowGap = fminf(
        styleCfg->arrowGap,
        (tweetHeight - styleCfg->arrowTipSize - styleCfg->roundedCornerRadius * 2) / (toPortsMatchCount - 1));

    if (toPortsMatchCount > 0)
    {
        float firstToPortMidY =
            tweetMidY - (toPortsMatchCount - 1) * arrowGap / 2 - styleCfg->arrowTipHalfSize;
        int toPortMatchIdx = 0;
        for (int toPortIdx = 0; toPortIdx < toPortTargetTweetIds->size; toPortIdx++)
        {
            if (tweet->id != *toPortTargetTweetIds->get(toPortIdx))
            {
                continue;
            }

            int64_t sourceTweetId = *toPortSourceTweetIds->get(toPortIdx);
            float toPortMidY = firstToPortMidY + toPortMatchIdx * arrowGap;

            // Hit rect
            HitRect *toPortHitRect = relativeLayout->toPortHitRects.get(toPortIdx);
            float hitRectMidX = -styleCfg->arrowTipBoundingRadius;
            float hitRectX = hitRectMidX - styleCfg->arrowTipHitRadius;
            float hitRectY = toPortMidY - styleCfg->arrowTipHitRadius;
            // Reverse link so source and target are swapped
            toPortHitRect->initToggleThreadHitRect(
                hitRectX, hitRectY, styleCfg->arrowTipHitDiameter, styleCfg->arrowTipHitDiameter,
                RoundedCorners::createAll(), styleCfg->arrowTipHitRadius, styleCfg->arrowHoverColor,
                tweet->id, sourceTweetId, 0, -1);

            // Arrow tip
            LayoutedArrowTip *toArrowTip = relativeLayout->toPortArrowTips.get(toPortIdx);
            toArrowTip->init(
                -styleCfg->arrowTipSize, toPortMidY, styleCfg->arrowTipSize, styleCfg->arrowTipHeight);

            toPortMatchIdx++;
        }
    }
}

void relativeLayoutThread(
    TextRenderer *textRenderer, StyleConfig *styleCfg, HashMap<int64_t, Thread *> *tweetIdToThread,
    User *visa, Thread *thread, ThreadPortTweetIds *threadPortTweetIds, int64_t targetTweet1Id,
    int64_t targetTweet2Id, ThreadLayout *outRelativeLayout, float *outTargetTweet1MidY,
    float *outTargetTweet2QuotedMidY)
{
    List<int64_t> *fromPortTweetIds = &threadPortTweetIds->fromPortTweetIds;
    List<int64_t> *toPortSourceTweetIds = &threadPortTweetIds->toPortSourceTweetIds;
    List<int64_t> *toPortTargetTweetIds = &threadPortTweetIds->toPortTargetTweetIds;

    outRelativeLayout->init();
    outRelativeLayout->fromPortHitRects.resize(fromPortTweetIds->size);
    outRelativeLayout->toPortHitRects.resize(toPortTargetTweetIds->size);
    outRelativeLayout->fromPortArrowTips.resize(fromPortTweetIds->size);
    outRelativeLayout->toPortArrowTips.resize(toPortTargetTweetIds->size);

    float tweetX = 1;
    float nextTweetY = 1;
    float threadLineStartY;
    for (int tweetIdx = 0; tweetIdx < thread->tweets.size; tweetIdx++)
    {
        Tweet *tweet = thread->tweets.get(tweetIdx);
        float tweetHeight;
        float tweetY = nextTweetY;

        if (tweetIdx > 0)
        {
            if (tweet->continuesThread)
            {
                outRelativeLayout->miscRects.append(LayoutedRect::create(
                    tweetX + 38.5, threadLineStartY, 2, tweetY + 5 - threadLineStartY,
                    styleCfg->threadOutlineColor));
            }
            else
            {
                outRelativeLayout->miscRects.append(
                    LayoutedRect::create(0, tweetY, styleCfg->threadWidth, 1, styleCfg->threadOutlineColor));
                tweetY++;
            }
        }

        RoundedCorners hitRectRoundedCorners;
        if (thread->tweets.size == 1) hitRectRoundedCorners = RoundedCorners::createAll();
        else if (tweetIdx == 0)
            hitRectRoundedCorners = RoundedCorners::create(true, true, false, false);
        else if (tweetIdx == thread->tweets.size - 1)
            hitRectRoundedCorners = RoundedCorners::create(false, false, true, true);
        else
            hitRectRoundedCorners = RoundedCorners::createNone();

        if (tweet->type == TweetType::Regular)
        {
            float *quotedTweetMidY = tweet->id == targetTweet2Id ? outTargetTweet2QuotedMidY : NULL;
            layoutRegularTweet(
                textRenderer, styleCfg, tweetIdToThread, visa, fromPortTweetIds, tweet, tweetX, tweetY,
                hitRectRoundedCorners, outRelativeLayout, &tweetHeight, &threadLineStartY, quotedTweetMidY);
        }
        else if (tweet->type == TweetType::Deleted)
        {
            layoutDeletedTweet(
                textRenderer, styleCfg, tweet, tweetX, tweetY, hitRectRoundedCorners, visa->screenName,
                outRelativeLayout, &tweetHeight, &threadLineStartY);
        }
        else if (tweet->type == TweetType::Copyrighted)
        {
            layoutCopyrightedTweet(
                textRenderer, styleCfg, tweet, tweetX, tweetY, hitRectRoundedCorners, outRelativeLayout,
                &tweetHeight, &threadLineStartY);
        }
        else
        {
            abortWithMessage("Tweet type not supported");
        }

        float tweetMidY = nextTweetY + tweetHeight / 2;
        layoutToPort(
            styleCfg, toPortSourceTweetIds, toPortTargetTweetIds, tweet, tweetHeight, tweetMidY,
            outRelativeLayout);

        if (outTargetTweet1MidY != NULL && tweet->id == targetTweet1Id)
        {
            *outTargetTweet1MidY = tweetMidY;
        }

        nextTweetY += tweetHeight;
    }

    float threadHeight = nextTweetY + 1;
    outRelativeLayout->threadOutlines.append(LayoutedRoundedFrame::create(
        0, 0, styleCfg->threadWidth, threadHeight, styleCfg->roundedCornerRadius,
        styleCfg->roundedFrameBorder, styleCfg->white, styleCfg->threadOutlineColor,
        styleCfg->threadOutlineTransparentColor));
}