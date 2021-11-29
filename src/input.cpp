#pragma once

#include "frameContext.cpp"
#include "stdio.h"

extern "C"
{
    void _jsSetCursor(int type);
    void _jsSetFocus();
    void _jsOpen(char *url);
    float _jsGetScrollLineHeight();
}

typedef void (*GoToRandomThreadCallback)(FrameContext *frameCtx);
typedef void (*ToggleThreadCallback)(WorldLayout *worldLayout);

struct InputContext
{
    bool isGrabbed;
    bool hasTouchMoved;
    long lastMouseX, lastMouseY;
    long lastTouchX, lastTouchY;
    HitRect *hoveredHitRect;
    float scrollLineHeight;
    FrameContext *frameCtx;
    GoToRandomThreadCallback goToRandomThreadCallback;
    ToggleThreadCallback toggleThreadCallback;

    void init(
        FrameContext *frameCtx, GoToRandomThreadCallback goToRandomThreadCallback,
        ToggleThreadCallback toggleThreadCallback)
    {
        this->isGrabbed = false;
        this->hasTouchMoved = false;
        this->hoveredHitRect = NULL;
        this->scrollLineHeight = 0;
        this->frameCtx = frameCtx;
        this->goToRandomThreadCallback = goToRandomThreadCallback;
        this->toggleThreadCallback = toggleThreadCallback;
    }

    void reset(LocationContext *locationCtx)
    {
        this->isGrabbed = false;
        this->hasTouchMoved = false;
        this->frameCtx->reset(locationCtx);
        this->hoveredHitRect = this->frameCtx->getHitRect(this->lastMouseX, this->lastMouseY);
    }
};

enum class CursorType
{
    Default,
    Grabbing
};

void _setCursor(CursorType type)
{
    _jsSetCursor(int(type));
}

#define MOUSE_BUTTON_LEFT 0
#define MOUSE_BUTTON_RIGHT 2

EM_BOOL mouseDownCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN && mouseEvent->button == MOUSE_BUTTON_RIGHT &&
        inputCtx->frameCtx->viewType == ViewType::Browse)
    {
        _setCursor(CursorType::Grabbing);
        inputCtx->isGrabbed = true;
        inputCtx->lastMouseX = mouseEvent->clientX;
        inputCtx->lastMouseY = mouseEvent->clientY;
    }

    return EM_TRUE;
}

void hitRectHoverAction(HitRect *hitRect, FrameContext *frameCtx)
{
    switch (hitRect->hoverAction)
    {
    case HoverAction::DoNothing:
        break;
    case HoverAction::GalleryCloseHover:
        frameCtx->galleryLayout.closeButton->color =
            frameCtx->galleryLayout.photos->get(frameCtx->galleryLayout.currentPhotoIndex)
                ->galleryButtonHoverColor;
        break;
    case HoverAction::GalleryNextHover:
        frameCtx->galleryLayout.nextPhotoButton->color =
            frameCtx->galleryLayout.photos->get(frameCtx->galleryLayout.currentPhotoIndex)
                ->galleryButtonHoverColor;
        break;
    case HoverAction::GalleryPrevHover:
        frameCtx->galleryLayout.prevPhotoButton->color =
            frameCtx->galleryLayout.photos->get(frameCtx->galleryLayout.currentPhotoIndex)
                ->galleryButtonHoverColor;
        break;
    default:
        abortWithMessage("Unsupported hover action");
    }
}

void hitRectLeaveAction(HitRect *hitRect, FrameContext *frameCtx)
{
    switch (hitRect->leaveAction)
    {
    case LeaveAction::DoNothing:
        break;
    case LeaveAction::GalleryCloseLeave:
        frameCtx->galleryLayout.closeButton->color =
            frameCtx->galleryLayout.photos->get(frameCtx->galleryLayout.currentPhotoIndex)
                ->galleryButtonColor;
        break;
    case LeaveAction::GalleryNextLeave:
        frameCtx->galleryLayout.nextPhotoButton->color =
            frameCtx->galleryLayout.photos->get(frameCtx->galleryLayout.currentPhotoIndex)
                ->galleryButtonColor;
        break;
    case LeaveAction::GalleryPrevLeave:
        frameCtx->galleryLayout.prevPhotoButton->color =
            frameCtx->galleryLayout.photos->get(frameCtx->galleryLayout.currentPhotoIndex)
                ->galleryButtonColor;
        break;
    default:
        abortWithMessage("Unsupported leave action");
    }
}

void updateHoveredHitRect(InputContext *inputCtx)
{
    HitRect *newHoveredHitRect = inputCtx->frameCtx->getHitRect(inputCtx->lastMouseX, inputCtx->lastMouseY);

    if (newHoveredHitRect != NULL)
    {
        hitRectHoverAction(newHoveredHitRect, inputCtx->frameCtx);
    }

    inputCtx->hoveredHitRect = newHoveredHitRect;
}

void hitRectHitAction(HitRect *hitRect, InputContext *inputCtx)
{
    FrameContext *frameCtx = inputCtx->frameCtx;

    if (inputCtx->hoveredHitRect != NULL)
    {
        hitRectLeaveAction(inputCtx->hoveredHitRect, frameCtx);
    }

    switch (hitRect->hitAction)
    {
    case HitAction::ToggleThread:
    {
        float hitRectMidY = hitRect->semiRoundedRect.y + hitRect->semiRoundedRect.height / 2;
        if (hitRect->toggleThreadArgs.targetThreadSlot > hitRect->toggleThreadArgs.sourceThreadSlot)
        {
            frameCtx->worldLayout.toggleThreadToRight(
                &frameCtx->renderCtx, &frameCtx->styleCfg, frameCtx->tweetData, frameCtx->tweetIdToThread,
                frameCtx->visa, &hitRect->toggleThreadArgs, hitRectMidY);
        }
        else
        {
            frameCtx->worldLayout.toggleThreadToLeft(
                &frameCtx->renderCtx, &frameCtx->styleCfg, frameCtx->tweetData, frameCtx->tweetIdToThread,
                frameCtx->visa, &hitRect->toggleThreadArgs, hitRectMidY);
        }
        inputCtx->toggleThreadCallback(&frameCtx->worldLayout);
        break;
    }
    case HitAction::ExternalTweet:
        char url[100];
        sprintf(
            url, "https://twitter.com/%s/status/%lld", hitRect->externalTweetArgs.authorScreenName,
            hitRect->externalTweetArgs.targetTweetId);
        _jsOpen(url);
        inputCtx->hoveredHitRect = NULL;
        break;
    case HitAction::GoToRandomThread:
        inputCtx->goToRandomThreadCallback(frameCtx);
        break;
    case HitAction::OpenGallery:
        layoutGallery(
            &frameCtx->galleryLayout, &frameCtx->styleCfg, &frameCtx->renderCtx.imageCache,
            hitRect->openGalleryArgs.photos, hitRect->openGalleryArgs.startPhotoIndex,
            frameCtx->renderCtx.canvasCtx.displayWidth, frameCtx->renderCtx.canvasCtx.displayHeight);
        frameCtx->viewType = ViewType::Gallery;
        break;
    case HitAction::CloseGallery:
        frameCtx->viewType = ViewType::Browse;
        break;
    case HitAction::GalleryNextPhoto:
        layoutGallery(
            &frameCtx->galleryLayout, &frameCtx->styleCfg, &frameCtx->renderCtx.imageCache,
            frameCtx->galleryLayout.photos, frameCtx->galleryLayout.currentPhotoIndex + 1,
            frameCtx->renderCtx.canvasCtx.displayWidth, frameCtx->renderCtx.canvasCtx.displayHeight);
        break;
    case HitAction::GalleryPrevPhoto:
        layoutGallery(
            &frameCtx->galleryLayout, &frameCtx->styleCfg, &frameCtx->renderCtx.imageCache,
            frameCtx->galleryLayout.photos, frameCtx->galleryLayout.currentPhotoIndex - 1,
            frameCtx->renderCtx.canvasCtx.displayWidth, frameCtx->renderCtx.canvasCtx.displayHeight);
        break;
    case HitAction::GalleryDoNothing:
        break;
    default:
        abortWithMessage("Unsupported hit action");
    }

    updateHoveredHitRect(inputCtx);
    frameCtx->shouldRerenderNextFrame = true;
}

EM_BOOL mouseMoveCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE)
    {
        if (inputCtx->isGrabbed && frameCtx->viewType == ViewType::Browse)
        {
            frameCtx->viewport.x -= (mouseEvent->clientX - inputCtx->lastMouseX);
            frameCtx->viewport.y -= (mouseEvent->clientY - inputCtx->lastMouseY);
            frameCtx->shouldRerenderNextFrame = true;
        }

        inputCtx->lastMouseX = mouseEvent->clientX;
        inputCtx->lastMouseY = mouseEvent->clientY;

        HitRect *newHoveredHitRect = frameCtx->getHitRect(inputCtx->lastMouseX, inputCtx->lastMouseY);
        if (newHoveredHitRect != inputCtx->hoveredHitRect)
        {
            if (inputCtx->hoveredHitRect != NULL)
            {
                hitRectLeaveAction(inputCtx->hoveredHitRect, frameCtx);
            }

            if (newHoveredHitRect != NULL)
            {
                hitRectHoverAction(newHoveredHitRect, frameCtx);
            }

            inputCtx->hoveredHitRect = newHoveredHitRect;
            frameCtx->shouldRerenderNextFrame = true;
        }
    }

    return EM_TRUE;
}

EM_BOOL mouseLeaveCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    if ((eventType == EMSCRIPTEN_EVENT_MOUSEUP && mouseEvent->button == MOUSE_BUTTON_RIGHT) ||
        eventType == EMSCRIPTEN_EVENT_MOUSEOUT)
    {
        _setCursor(CursorType::Default);
        inputCtx->isGrabbed = false;
    }

    return EM_TRUE;
}

EM_BOOL mouseClickCallback(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    if (eventType == EMSCRIPTEN_EVENT_CLICK && mouseEvent->button == MOUSE_BUTTON_LEFT &&
        inputCtx->hoveredHitRect != NULL)
    {
        hitRectHitAction(inputCtx->hoveredHitRect, inputCtx);
    }

    return EM_TRUE;
}

EM_BOOL mouseWheelCallback(int eventType, const EmscriptenWheelEvent *wheelEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    if (eventType == EMSCRIPTEN_EVENT_WHEEL && frameCtx->viewType == ViewType::Browse)
    {
        if (wheelEvent->mouse.ctrlKey == EM_TRUE || wheelEvent->mouse.metaKey == EM_TRUE)
        {
            // Prevent browser zoom
            // LayoutViewport zoom is ok but VisualViewport zoom is bad and its events are indistinguishable
            return EM_TRUE;
        }

        // Line and screen units are translated to pixels according to
        // https://stackoverflow.com/questions/20110224/what-is-the-height-of-a-line-in-a-wheel-event-deltamode-dom-delta-line/37474225#37474225
        float deltaMultiplierX, deltaMultiplierY;
        switch (wheelEvent->deltaMode)
        {
        case DOM_DELTA_PIXEL:
            deltaMultiplierX = deltaMultiplierY = 1;
            break;
        case DOM_DELTA_LINE:
            if (inputCtx->scrollLineHeight == 0)
            {
                inputCtx->scrollLineHeight = _jsGetScrollLineHeight();
            }
            deltaMultiplierX = deltaMultiplierY = inputCtx->scrollLineHeight;
            break;
        case DOM_DELTA_PAGE:
        {
            if (inputCtx->scrollLineHeight == 0)
            {
                inputCtx->scrollLineHeight = _jsGetScrollLineHeight();
            }

            // Used for both x and y scrolling. Firefox would use avg character width for horizontal,
            // but it's roughly similar so let's keep it simple.
            float lineOffset = 2 * inputCtx->scrollLineHeight;

            float displayOffsetX = 0.1 * frameCtx->renderCtx.canvasCtx.displayWidth;
            float offsetX = displayOffsetX < lineOffset ? displayOffsetX : lineOffset;
            deltaMultiplierX = frameCtx->renderCtx.canvasCtx.displayWidth - offsetX;

            float displayOffsetY = 0.1 * frameCtx->renderCtx.canvasCtx.displayHeight;
            float offsetY = displayOffsetY < lineOffset ? displayOffsetY : lineOffset;
            deltaMultiplierY = frameCtx->renderCtx.canvasCtx.displayHeight - offsetY;

            break;
        }
        default:
            printf("Don't know scroll unit: %lu\n", wheelEvent->deltaMode);
            return EM_TRUE;
        }

        if (wheelEvent->mouse.shiftKey == EM_TRUE && wheelEvent->deltaX == 0)
        {
            // Shift + real mouse wheel = horizontal scroll
            frameCtx->viewport.x += wheelEvent->deltaY * deltaMultiplierX;
        }
        else
        {
            frameCtx->viewport.x += wheelEvent->deltaX * deltaMultiplierX;
            frameCtx->viewport.y += wheelEvent->deltaY * deltaMultiplierY;
        }
        frameCtx->shouldRerenderNextFrame = true;

        mouseMoveCallback(EMSCRIPTEN_EVENT_MOUSEMOVE, &wheelEvent->mouse, userData);
    }

    return EM_TRUE;
}

EM_BOOL touchStartCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART)
    {
        if (frameCtx->viewType == ViewType::Browse)
        {
            _setCursor(CursorType::Grabbing);
            inputCtx->isGrabbed = true;
            inputCtx->hasTouchMoved = false;
        }
        inputCtx->lastTouchX = touchEvent->touches[0].clientX;
        inputCtx->lastTouchY = touchEvent->touches[0].clientY;
    }

    return EM_FALSE; // Needed so that the browser fires a click event on tap
}

EM_BOOL touchEndCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    if ((eventType == EMSCRIPTEN_EVENT_TOUCHEND || eventType == EMSCRIPTEN_EVENT_TOUCHCANCEL) &&
        touchEvent->numTouches == 1)
    {
        if (frameCtx->viewType == ViewType::Browse)
        {
            _setCursor(CursorType::Default);
            inputCtx->isGrabbed = false;
        }

        if (inputCtx->hasTouchMoved)
        {
            // Scroll, not tap
            return EM_TRUE;
        }

        // Handle tap
        HitRect *tappedHitRect =
            frameCtx->getHitRect(touchEvent->touches[0].clientX, touchEvent->touches[0].clientY);
        if (tappedHitRect == NULL)
        {
            // Didn't hit any rect
            return EM_TRUE;
        }

        hitRectHitAction(tappedHitRect, inputCtx);
    }

    return EM_TRUE;
}

EM_BOOL touchMoveCallback(int eventType, const EmscriptenTouchEvent *touchEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE)
    {
        if (frameCtx->viewType == ViewType::Browse)
        {
            frameCtx->viewport.x -= (touchEvent->touches[0].clientX - inputCtx->lastTouchX);
            frameCtx->viewport.y -= (touchEvent->touches[0].clientY - inputCtx->lastTouchY);
            inputCtx->hasTouchMoved = true;
            frameCtx->shouldRerenderNextFrame = true;
        }

        inputCtx->lastTouchX = touchEvent->touches[0].clientX;
        inputCtx->lastTouchY = touchEvent->touches[0].clientY;
    }

    return EM_TRUE;
}

EM_BOOL visibilityChangeCallback(
    int eventType, const EmscriptenVisibilityChangeEvent *visibilityChangeEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;

    if (eventType == EMSCRIPTEN_EVENT_VISIBILITYCHANGE && visibilityChangeEvent->hidden)
    {
        _setCursor(CursorType::Default);
        inputCtx->isGrabbed = false;
    }

    return EM_TRUE;
}

EM_BOOL keyPressCallback(int eventType, const EmscriptenKeyboardEvent *keyboardEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    if (eventType == EMSCRIPTEN_EVENT_KEYPRESS)
    {
        if (strcmp(keyboardEvent->code, "KeyD") == 0)
        {
            frameCtx->perfOverlay.isVisible = !frameCtx->perfOverlay.isVisible;
            frameCtx->shouldRerenderNextFrame = true;
            return EM_TRUE;
        }

        if (strcmp(keyboardEvent->code, "KeyR") == 0)
        {
            inputCtx->goToRandomThreadCallback(frameCtx);
            return EM_TRUE;
        }

        if (strcmp(keyboardEvent->code, "KeyU") == 0)
        {
            frameCtx->shouldAlwaysRerender = !frameCtx->shouldAlwaysRerender;
            return EM_TRUE;
        }
    }

    return EM_FALSE;
}

EM_BOOL keyDownCallback(int eventType, const EmscriptenKeyboardEvent *keyboardEvent, void *userData)
{
    InputContext *inputCtx = (InputContext *)userData;
    FrameContext *frameCtx = inputCtx->frameCtx;
    if (eventType == EMSCRIPTEN_EVENT_KEYDOWN)
    {
        if (frameCtx->viewType == ViewType::Gallery)
        {
            if (strcmp(keyboardEvent->code, "ArrowLeft") == 0 &&
                frameCtx->galleryLayout.currentPhotoIndex > 0)
            {
                if (inputCtx->hoveredHitRect != NULL)
                {
                    hitRectLeaveAction(inputCtx->hoveredHitRect, frameCtx);
                }

                layoutGallery(
                    &frameCtx->galleryLayout, &frameCtx->styleCfg, &frameCtx->renderCtx.imageCache,
                    frameCtx->galleryLayout.photos, frameCtx->galleryLayout.currentPhotoIndex - 1,
                    frameCtx->renderCtx.canvasCtx.displayWidth, frameCtx->renderCtx.canvasCtx.displayHeight);
                updateHoveredHitRect(inputCtx);
                frameCtx->shouldRerenderNextFrame = true;
                return EM_TRUE;
            }

            if (strcmp(keyboardEvent->code, "ArrowRight") == 0 &&
                frameCtx->galleryLayout.currentPhotoIndex < frameCtx->galleryLayout.photos->size - 1)
            {
                if (inputCtx->hoveredHitRect != NULL)
                {
                    hitRectLeaveAction(inputCtx->hoveredHitRect, frameCtx);
                }

                layoutGallery(
                    &frameCtx->galleryLayout, &frameCtx->styleCfg, &frameCtx->renderCtx.imageCache,
                    frameCtx->galleryLayout.photos, frameCtx->galleryLayout.currentPhotoIndex + 1,
                    frameCtx->renderCtx.canvasCtx.displayWidth, frameCtx->renderCtx.canvasCtx.displayHeight);
                updateHoveredHitRect(inputCtx);
                frameCtx->shouldRerenderNextFrame = true;
                return EM_TRUE;
            }

            if (strcmp(keyboardEvent->code, "Escape") == 0)
            {
                if (inputCtx->hoveredHitRect != NULL)
                {
                    hitRectLeaveAction(inputCtx->hoveredHitRect, frameCtx);
                }

                frameCtx->viewType = ViewType::Browse;
                updateHoveredHitRect(inputCtx);
                frameCtx->shouldRerenderNextFrame = true;
                return EM_TRUE;
            }
        }
    }

    return EM_FALSE;
}

void setInputCallbacks(InputContext *inputCtx)
{
    _setCursor(CursorType::Default);
    emscripten_set_mousedown_callback(canvasSelector, inputCtx, false, mouseDownCallback);
    emscripten_set_mouseup_callback(canvasSelector, inputCtx, false, mouseLeaveCallback);
    emscripten_set_mouseout_callback(canvasSelector, inputCtx, false, mouseLeaveCallback);
    emscripten_set_mousemove_callback(canvasSelector, inputCtx, false, mouseMoveCallback);
    emscripten_set_click_callback(canvasSelector, inputCtx, false, mouseClickCallback);
    emscripten_set_wheel_callback(canvasSelector, inputCtx, false, mouseWheelCallback);

    emscripten_set_touchstart_callback(canvasSelector, inputCtx, false, touchStartCallback);
    emscripten_set_touchend_callback(canvasSelector, inputCtx, false, touchEndCallback);
    emscripten_set_touchcancel_callback(canvasSelector, inputCtx, false, touchEndCallback);
    emscripten_set_touchmove_callback(canvasSelector, inputCtx, false, touchMoveCallback);

    emscripten_set_visibilitychange_callback(inputCtx, false, visibilityChangeCallback);

    _jsSetFocus();
    emscripten_set_keypress_callback(canvasSelector, inputCtx, false, keyPressCallback);
    emscripten_set_keydown_callback(canvasSelector, inputCtx, false, keyDownCallback);
}