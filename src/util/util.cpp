#pragma once
#include <stdio.h>
#include <stdlib.h>

[[noreturn]]
void abortWithMessage(const char *message)
{
    printf("Assertion failed: %s. Aborting\n", message);
    abort();
}

void assertOrAbort(bool hasAssertPassed, const char *message)
{
    if (!hasAssertPassed)
    {
        abortWithMessage(message);
    }
}

void printBinary(char *bytes, int length)
{
    for (int i = 0; i < length; i++)
    {
        if (i != 0 && i % 32 == 0)
        {
            printf("\n");
        }
        printf("%02x ", bytes[i] & 0xff);
    }
    printf("\n");
}

void printBinaryAroundAddress(void *ptr, int radius)
{
    printBinary((char *)ptr - radius, radius);
    printf("v\n");
    printBinary((char *)ptr, radius);
}