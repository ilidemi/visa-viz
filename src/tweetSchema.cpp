#pragma once

#include "render/common/color.cpp"
#include "util/list.cpp"
#include <stdint.h>

struct User
{
    const char *screenName;
    const wchar_t *userName;
    const char *profileImageUrl;
};

enum class TweetType
{
    Regular,
    Deleted,
    Copyrighted
};

struct HighlightedToken
{
    int startIndex; // Inclusive
    int endIndex; // Exclusive
};

struct TweetText
{
    const wchar_t *text;
    List<HighlightedToken> highlightedTokens;
};

struct Photo
{
    const char *thumbnailUrl;
    const char *fullUrl;
    Color galleryBackgroundColor;
    Color galleryButtonColor;
    Color galleryButtonHoverColor;
};

struct AnimatedGif
{
    const char *thumbnailUrl;
};

struct Video
{
    const char *thumbnailUrl;
    const wchar_t *sourceUser;
};

enum class MediaType
{
    Photos,
    AnimatedGif,
    Video,
    DmcaVideo
};

struct Media
{
    MediaType type;
    float aspectRatio;
    union
    {
        List<Photo> photos;
        AnimatedGif animatedGif;
        Video video;
    };
};

enum class LinkCardType
{
    Summary,
    SummaryLargeImage,
    Player
};

struct LinkCardImage
{
    const char *thumbnailUrl;
    float aspectRatio;
};

struct LinkCard
{
    LinkCardType type;
    LinkCardImage *image;
    const wchar_t *title;
    const wchar_t *description;
    const char *domain;
};

struct Poll
{
    const wchar_t *choices[4];
    const char *votePercentagesText[4];
    float voteFractions[4];
    int choicesCount;
    int winningChoiceIndex;
    const wchar_t *totalVotes;
};

enum class QuotedTweetExpandType
{
    None,
    Poll,
    Thread
};

struct QuotedTweet
{
    TweetType type;
    int64_t id;

    // Regular fields
    User *user;
    const char *date;
    const char *mentions;
    const wchar_t *body;
    QuotedTweetExpandType expandType;
    Media *media;

    // Copyrighted fields
    const char *copyrightedUserScreenName;
};

struct Tweet
{
    TweetType type;
    int64_t id;
    bool continuesThread;

    // Regular fields
    User *user;
    const char *date;
    TweetText body;
    Media *media;
    LinkCard *linkCard;
    Poll *poll;
    QuotedTweet *quotedTweet;
    const char *replyCount;
    const char *retweetCount;
    const char *favoriteCount;

    // Copyrighted fields
    const char *copyrightedUserScreenName;
};

struct Thread
{
    int64_t id;
    List<Tweet> tweets;
};

struct FromPort
{
    int64_t threadId;
    int64_t tweetId;
};

struct ToPort
{
    int64_t threadId;
    int64_t tweetId;
    int order;
};

struct TweetData
{
    List<User> users;
    List<Thread> threads;
    List<FromPort> fromPorts;
    List<ToPort> toPorts;
};