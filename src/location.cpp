#pragma once

#include "frameContext.cpp"
#include "util/base64.cpp"
#include "util/list.cpp"
#include <ctype.h>

extern "C"
{
    void _jsGetLocationHash(int maxLength, char *outHash);
    void _jsSetLocationHash(const char *hash);
    void _jsSetLocationHashChangeCallback(const char *cCallbackName, int userDataInt);
    void _jsAlert(const char *message);
}

typedef void (*ResetCallback)(LocationContext *locationCtx);

struct Location
{
    bool isInitialized;
    char hash[2000];
    LocationContext ctx;
    HashMap<int64_t, Thread *> *tweetIdToThread;
    HashMap<int64_t, Tweet *> *tweetIdToTweet;
    ResetCallback resetCallback;

    void _setToSafeTweet()
    {
#ifdef DATA_MOCK
        static const int64_t safeTweetId = 1;
#else
        static const int64_t safeTweetId = 939810687991234560;
#endif
        this->ctx.isSingleTweet = true;
        this->ctx.singleTweetId = safeTweetId;
        sprintf(this->hash, "%lld", this->ctx.singleTweetId);
        _jsSetLocationHash(this->hash);
    }

    bool _updateFromSingleTweet(char *newHash)
    {
        int64_t targetTweetId;
        int tokensScanned = sscanf(newHash, "%lld", &targetTweetId);
        if (tokensScanned == 1 && tweetIdToTweet->contains(targetTweetId))
        {
            this->ctx.isSingleTweet = true;
            this->ctx.singleTweetId = targetTweetId;
            return true;
        }
        else
        {
            return false;
        }
    }

    bool _updateFromMultipleTweets(char *base64, int base64Length)
    {
        uint8_t bytes[3000];
        int bytesLength;
        if (!base64Decode(base64, base64Length, bytes, &bytesLength))
        {
            return false;
        }

        if (bytesLength <= 0 || bytesLength % 8 != 0)
        {
            return false;
        }

        List<int64_t> newSourceTweetIds;
        newSourceTweetIds.init();
        newSourceTweetIds.reserve(bytesLength / 8);
        int64_t expectedThreadId;
        for (int i = 0; i < bytesLength; i += 8)
        {
            int64_t sourceTweetId = 0;
            for (int j = 7; j >= 0; j--)
            {
                // Decode in little endian
                sourceTweetId <<= 8;
                sourceTweetId |= bytes[i + j];
            }

            if (!this->tweetIdToTweet->contains(sourceTweetId))
            {
                newSourceTweetIds.free();
                return false;
            }

            if (i > 0)
            {
                int64_t threadId = this->tweetIdToThread->get(sourceTweetId)->id;
                if (threadId != expectedThreadId)
                {
                    return false;
                }
            }

            Tweet *sourceTweet = this->tweetIdToTweet->get(sourceTweetId);
            if (!(sourceTweet->type == TweetType::Regular && sourceTweet->quotedTweet != NULL))
            {
                return false;
            }

            if (!tweetIdToTweet->contains(sourceTweet->quotedTweet->id))
            {
                return false;
            }

            expectedThreadId = tweetIdToThread->get(sourceTweet->quotedTweet->id)->id;
            newSourceTweetIds.append(sourceTweetId);
        }

        this->ctx.isSingleTweet = false;
        this->ctx.sourceTweetIds.free();
        this->ctx.sourceTweetIds = newSourceTweetIds;
        return true;
    }

    void _genericFallback(char *newHash)
    {
        char alertMessage[2100];
        sprintf(alertMessage, "The link #%s is invalid", newHash);
        _jsAlert(alertMessage);
        if (!this->isInitialized)
        {
            this->_setToSafeTweet();
        }
        // Otherwise keep as is
    }

    void _updateFromHash(char *newHash)
    {
        int newHashLength = strlen(newHash);
        if (newHashLength == 0)
        {
            this->_setToSafeTweet();
        }
        else if (isdigit(newHash[0]))
        {
            if (!this->_updateFromSingleTweet(newHash))
            {
                char alertMessage[200];
                sprintf(alertMessage, "Tweet %s not found in thread network", newHash);
                _jsAlert(alertMessage);
                if (!this->isInitialized)
                {
                    this->_setToSafeTweet();
                }
                // Otherwise keep as is
            }
        }
        else if (newHash[0] == 'A')
        {
            if (!this->_updateFromMultipleTweets(newHash + 1, newHashLength - 1))
            {
                this->_genericFallback(newHash);
            }
        }
        else
        {
            this->_genericFallback(newHash);
        }
    }

    void init(HashMap<int64_t, Thread *> *tweetIdToThread, HashMap<int64_t, Tweet *> *tweetIdToTweet)
    {
        this->isInitialized = false;
        this->hash[0] = '\0';
        this->ctx.sourceTweetIds.init();
        this->tweetIdToThread = tweetIdToThread;
        this->tweetIdToTweet = tweetIdToTweet;
        this->resetCallback = NULL;

        char newHash[2000];
        _jsGetLocationHash(sizeof(newHash), newHash);
        _updateFromHash(newHash);
        this->isInitialized = true;
    }

    void updateFromHash(char *newHash)
    {
        if (this->isInitialized && strcmp(newHash, this->hash) == 0)
        {
            return;
        }

        _updateFromHash(newHash);
        if (this->resetCallback != NULL)
        {
            this->resetCallback(&this->ctx);
        }
        else
        {
            printf("Reset callback is not set!\n");
        }
    }

    void updateFromWorldLayout(WorldLayout *worldLayout)
    {
        if (worldLayout->threadIds.size == 1)
        {
            this->ctx.isSingleTweet = true;
            this->ctx.singleTweetId = *worldLayout->threadIds.get(worldLayout->threadIds.start);
            sprintf(this->hash, "%lld", this->ctx.singleTweetId);
        }
        else
        {
            this->ctx.isSingleTweet = false;
            this->ctx.sourceTweetIds.clear();
            this->ctx.sourceTweetIds.appendRange(&worldLayout->arrowSourceTweetIds);

            uint8_t bytes[3000];
            int bytesLength = 0;
            list_for_each(sourceTweetId, this->ctx.sourceTweetIds)
            {
                for (int i = 0; i < 8; i++)
                {
                    // Encode in little endian
                    bytes[bytesLength++] = (uint8_t)(*sourceTweetId >> i * 8);
                }
            }

            this->hash[0] = 'A';
            int base64Length = base64Encode(bytes, bytesLength, this->hash + 1);
            this->hash[1 + base64Length] = '\0';
        }
        _jsSetLocationHash(this->hash);
    }

    void goToRandomThread(FrameContext *frameCtx)
    {
        int threadIdx = rand() % frameCtx->tweetData->threads.size;
        int64_t targetTweetId = frameCtx->tweetData->threads.get(threadIdx)->id;
        char locationHash[30];
        sprintf(locationHash, "%lld", targetTweetId);
        _jsSetLocationHash(locationHash);
    }

    void setLocationHashChangeCallback(ResetCallback resetCallback)
    {
        this->resetCallback = resetCallback;
        int userDataInt;
        Location *source = this;
        memcpy(&userDataInt, &source, sizeof(int));
        _jsSetLocationHashChangeCallback("locationHashChangeCallback", userDataInt);
    }
};

extern "C"
{
    void locationHashChangeCallback(char *newHash, void *userData)
    {
        Location *location = (Location *)userData;
        location->updateFromHash(newHash);
    }
}