#pragma once

#include "../src/util/util.cpp"

void assert(bool condition, const char *filename, int line)
{
    if (!condition)
    {
        char message[1000];
        sprintf(message, "Test failed: %s, line %i", filename, line);
        abortWithMessage(message);
    }
}

#define ASSERT(condition) assert(condition, __FILE__, __LINE__)
