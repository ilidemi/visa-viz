#pragma once

#include "deque.cpp"
#include "util.cpp"
#include <stdlib.h>

#define list_for_each(item, list)                                                                            \
    for (auto item = (list).values; item < ((list).values + (list).size); item++)

template <typename T>
struct List
{
    T *values;
    int size;
    int capacity;

    void init()
    {
        this->size = 0;
        this->capacity = 4;
        this->values = (T *)malloc(this->capacity * sizeof(T));
        assertOrAbort(this->values != NULL, "Couldn't allocate the list");
    }

    void initForeverEmpty()
    {
        this->size = 0;
        this->capacity = 0;
        this->values = NULL;
    }

    void free()
    {
        this->size = 0;
        this->capacity = 0;
        ::free(this->values);
        this->values = NULL;
    }

    void clear()
    {
        this->size = 0;
    }

    void reserve(int newCapacity)
    {
        assertOrAbort(newCapacity < (1 << 30), "List size is too big");
        int ceilPowerOfTwo = 4;
        while (ceilPowerOfTwo < newCapacity)
        {
            ceilPowerOfTwo <<= 1;
        }

        if (this->capacity != ceilPowerOfTwo)
        {
            this->capacity = ceilPowerOfTwo;
            this->values = (T *)realloc(this->values, this->capacity * sizeof(T));
            assertOrAbort(this->values != NULL, "Couldn't resize the list");
        }
    }

    void resize(int newSize)
    {
        assertOrAbort(newSize < (1 << 30), "List size is too big");
        this->reserve(newSize);
        this->size = newSize;
    }

    void append(const T &value)
    {
        if (this->size == this->capacity)
        {
            this->capacity *= 2;
            this->values = (T *)realloc(this->values, this->capacity * sizeof(T));
            assertOrAbort(this->values != NULL, "Couldn't grow the list");
        }

        this->values[this->size] = value;
        this->size++;
    }

    void appendRange(List<T> *values)
    {
        int targetSize = this->size + values->size;
        if (targetSize > this->capacity)
        {
            while (targetSize > this->capacity)
            {
                this->capacity *= 2;
            }

            this->values = (T *)realloc(this->values, this->capacity * sizeof(T));
            assertOrAbort(this->values != NULL, "Couldn't grow the list");
        }

        list_for_each(item, *values)
        {
            this->values[this->size] = *item;
            this->size++;
        }
    }

    void appendRange(Deque<T> *values)
    {
        int targetSize = this->size + values->size;
        if (targetSize > this->capacity)
        {
            while (targetSize > this->capacity)
            {
                this->capacity *= 2;
            }

            this->values = (T *)realloc(this->values, this->capacity * sizeof(T));
            assertOrAbort(this->values != NULL, "Couldn't grow the list");
        }

        deque_for_each(item, *values)
        {
            this->values[this->size] = *item;
            this->size++;
        }
    }

    T *getNext()
    {
        if (this->size == this->capacity)
        {
            this->capacity *= 2;
            this->values = (T *)realloc(this->values, this->capacity * sizeof(T));
            assertOrAbort(this->values != NULL, "Couldn't grow the list");
        }

        T *result = this->values + this->size;
        this->size++;
        return result;
    }

    T *get(int index)
    {
        assertOrAbort(index < this->size, "List index overflow");
        assertOrAbort(index >= 0, "List index underflow");
        return this->values + index;
    }

    void pop()
    {
        assertOrAbort(this->size > 0, "Can't pop an empty list");
        this->size--;
    }
};
