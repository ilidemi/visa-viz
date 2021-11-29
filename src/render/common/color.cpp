#pragma once

struct Color
{
    unsigned char r, g, b, a;

    static Color create(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
    {
        return { r, g, b, a };
    }

    Color makeTransparent()
    {
        return create(this->r, this->g, this->b, 0);
    }
};