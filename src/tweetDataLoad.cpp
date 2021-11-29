#pragma once

#include "tweetSchema.cpp"
#include <stdio.h>
#include <string.h>

void *alignedMalloc(size_t size, int alignmentBytes, void **outUnalignedPtr)
{
    assertOrAbort(
        alignmentBytes == 1 || alignmentBytes == 2 || alignmentBytes == 4 || alignmentBytes == 8,
        "alignmentBytes has to be a power of two");
    int paddingBits = alignmentBytes * 8 - 1;
    void *memory = malloc(size + paddingBits);
    uint32_t mask = ~(uint32_t)(alignmentBytes - 1);
    void *result = (void *)(((uintptr_t)memory + paddingBits) & (uintptr_t)mask);
    if (outUnalignedPtr != NULL)
    {
        *outUnalignedPtr = memory;
    }
    return result;
}

TweetData *getTweetDataFromPaths(const char *dataPath, const char *pointersPath)
{
    FILE *dataFile = fopen(dataPath, "rb");
    assertOrAbort(dataFile != NULL, "Couldn't open data");

    fseek(dataFile, 0, SEEK_END);
    int dataSize = ftell(dataFile);
    rewind(dataFile);
    printf("data size: %d\n", dataSize);

    char *data = (char *)alignedMalloc(dataSize, 8, NULL);
    assertOrAbort(data != NULL, "Couldn't allocate data");
    size_t dataRead = fread(data, 1, dataSize, dataFile);
    assertOrAbort(dataRead > 0, "Couldn't read data");

    FILE *pointersFile = fopen(pointersPath, "rb");
    assertOrAbort(pointersFile != NULL, "Couldn't open pointers");

    fseek(pointersFile, 0, SEEK_END);
    int pointersSize = ftell(pointersFile);
    rewind(pointersFile);
    printf("pointers size: %d\n", pointersSize);

    void *unalignedPointers;
    uint32_t *pointers = (uint32_t *)alignedMalloc(pointersSize, 4, &unalignedPointers);
    assertOrAbort(pointers != NULL, "Couldn't allocate pointers");
    size_t pointersRead = fread(pointers, 1, pointersSize, pointersFile);
    assertOrAbort(pointersRead > 0, "Couldn't read pointers");

    uint32_t dataAddress;
    memcpy(&dataAddress, &data, 4);
    int pointersCount = pointersSize / 4;
    for (int i = 0; i < pointersCount; i++)
    {
        uint32_t pointerOffset = pointers[i];
        uint32_t pointer;
        memcpy(&pointer, data + pointerOffset, 4);
        pointer += dataAddress;
        memcpy(data + pointerOffset, &pointer, 4);
    }

    free(unalignedPointers);

    TweetData *tweetData;
    memcpy(&tweetData, data + dataSize - 4, 4);

    assertOrAbort(
        tweetData->fromPorts.size == tweetData->toPorts.size, "Counts of from and to ports must be equal");

    return tweetData;
}

TweetData *getTweetData()
{
    return getTweetDataFromPaths("/data/data.bin", "/data/pointers.bin");
}