#pragma once

#include <stdint.h>
#include <stdio.h>

int8_t _base64DecodeChar(char base64Char)
{
    if (base64Char >= 'A' && base64Char <= 'Z')
    {
        return base64Char - 'A';
    }
    else if (base64Char >= 'a' && base64Char <= 'z')
    {
        return base64Char - 'a' + 26;
    }
    else if (base64Char >= '0' && base64Char <= '9')
    {
        return base64Char - '0' + 52;
    }
    else if (base64Char == '-')
    {
        return 62;
    }
    else if (base64Char == '_')
    {
        return 63;
    }
    else
    {
        return -1;
    }
}

bool base64Decode(char *base64, int base64Length, uint8_t *bytes, int *outDecodedLength)
{
    int decodedLength = 0;

    for (int i = 0; (i + 3) < base64Length; i += 4)
    {
        int8_t base64Number0 = _base64DecodeChar(base64[i]);
        int8_t base64Number1 = _base64DecodeChar(base64[i + 1]);
        int8_t base64Number2 = _base64DecodeChar(base64[i + 2]);
        int8_t base64Number3 = _base64DecodeChar(base64[i + 3]);

        if (base64Number0 == -1 || base64Number1 == -1 || base64Number2 == -1 || base64Number3 == -1)
        {
            return false;
        }

        // pp000000 | pp111111 | pp222222 | pp333333
        //   aaaaaa     aabbbb     bbbbcc     cccccc
        bytes[decodedLength++] = (base64Number0 << 2) | (base64Number1 >> 4);
        bytes[decodedLength++] = (base64Number1 << 4) | (base64Number2 >> 2);
        bytes[decodedLength++] = (base64Number2 << 6) | base64Number3;
    }

    switch (base64Length % 4)
    {
    case 0:
        break;
    case 1:
        return false;
        break;
    case 2:
    {
        int8_t base64Number0 = _base64DecodeChar(base64[base64Length - 2]);
        int8_t base64Number1 = _base64DecodeChar(base64[base64Length - 1]);
        bytes[decodedLength++] = (base64Number0 << 2) | (base64Number1 >> 4);
        break;
    }
    case 3:
    {
        int8_t base64Number0 = _base64DecodeChar(base64[base64Length - 3]);
        int8_t base64Number1 = _base64DecodeChar(base64[base64Length - 2]);
        int8_t base64Number2 = _base64DecodeChar(base64[base64Length - 1]);
        bytes[decodedLength++] = (base64Number0 << 2) | (base64Number1 >> 4);
        bytes[decodedLength++] = (base64Number1 << 4) | (base64Number2 >> 2);
        break;
    }
    }

    *outDecodedLength = decodedLength;
    return true;
}

char _base64Encode6Bits(int8_t bits)
{
    if (bits < 26)
    {
        return 'A' + bits;
    }
    else if (bits < 52)
    {
        return 'a' + (bits - 26);
    }
    else if (bits < 62)
    {
        return '0' + (bits - 52);
    }
    else if (bits == 62)
    {
        return '-';
    }
    else
    {
        return '_';
    }
}

int base64Encode(uint8_t *bytes, int bytesLength, char *outBase64)
{
    char *base64Start = outBase64;

    for (int i = 0; (i + 2) < bytesLength; i += 3)
    {
        int8_t char0Bits = bytes[i] >> 2;
        int8_t char1Bits = ((bytes[i] & 0b11) << 4) | (bytes[i + 1] >> 4);
        int8_t char2Bits = ((bytes[i + 1] & 0b1111) << 2) | (bytes[i + 2] >> 6);
        int8_t char3Bits = bytes[i + 2] & 0b111111;

        *outBase64++ = _base64Encode6Bits(char0Bits);
        *outBase64++ = _base64Encode6Bits(char1Bits);
        *outBase64++ = _base64Encode6Bits(char2Bits);
        *outBase64++ = _base64Encode6Bits(char3Bits);

    }

    switch (bytesLength % 3)
    {
    case 0:
        break;
    case 1:
    {
        int8_t char0Bits = bytes[bytesLength - 1] >> 2;
        int8_t char1Bits = (bytes[bytesLength - 1] & 0b11) << 4;
        *outBase64++ = _base64Encode6Bits(char0Bits);
        *outBase64++ = _base64Encode6Bits(char1Bits);
        break;
    }
    case 2:
    {
        int8_t char0Bits = bytes[bytesLength - 2] >> 2;
        int8_t char1Bits = ((bytes[bytesLength - 2] & 0b11) << 4) | (bytes[bytesLength - 1] >> 4);
        int8_t char2Bits = (bytes[bytesLength - 1] & 0b1111) << 2;
        *outBase64++ = _base64Encode6Bits(char0Bits);
        *outBase64++ = _base64Encode6Bits(char1Bits);
        *outBase64++ = _base64Encode6Bits(char2Bits);
        break;
    }
    }

    return outBase64 - base64Start;
}