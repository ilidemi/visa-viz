#pragma once

#include "../src/tweetDataLoad.cpp"

void testTweetDataLoad()
{
    getTweetDataFromPaths("./data/data.bin", "./data/pointers.bin");
}