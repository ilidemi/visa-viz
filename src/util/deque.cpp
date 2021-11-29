#pragma once

#include "util.cpp"
#include <stdlib.h>
#include <string.h>

template <typename T>
struct Deque
{
    T *values;
    int start;
    int size;
    int capacity;

    void init()
    {
        this->start = 0;
        this->size = 0;
        this->capacity = 4;
        this->values = (T *)malloc(this->capacity * sizeof(T));
        assertOrAbort(this->values != NULL, "Couldn't allocate the deque");
    }

    void free()
    {
        this->start = 0;
        this->size = 0;
        this->capacity = 0;
        ::free(this->values);
        this->values = NULL;
    }

    int _wrapIndex(int index)
    {
        return (index % this->capacity + this->capacity) % this->capacity;
    }

    void _grow()
    {
        int newCapacity = this->capacity * 2;
        this->values = (T *)realloc(this->values, newCapacity * sizeof(T));
        assertOrAbort(this->values != NULL, "Couldn't grow the deque");

        int wrappedStart = _wrapIndex(this->start);
        if (wrappedStart != 0)
        {
            memcpy(
                this->values + wrappedStart + this->capacity, this->values + wrappedStart,
                (this->capacity - wrappedStart) * sizeof(T));
        }
        this->capacity = newCapacity;
    }

    void appendRight(const T &value)
    {
        if (this->size == this->capacity - 1)
        {
            this->_grow();
        }

        int wrappedIndex = _wrapIndex(this->start + this->size);
        this->values[wrappedIndex] = value;
        this->size++;
    }

    void appendLeft(const T &value)
    {
        if (this->size == this->capacity - 1)
        {
            this->_grow();
        }

        this->start--;
        int wrappedIndex = this->_wrapIndex(this->start);
        this->values[wrappedIndex] = value;
        this->size++;
    }

    T *getNextRight()
    {
        if (this->size == this->capacity - 1)
        {
            this->_grow();
        }

        int wrappedIndex = this->_wrapIndex(this->start + this->size);
        T *result = this->values + wrappedIndex;
        this->size++;
        return result;
    }

    T *getNextLeft()
    {
        if (this->size == this->capacity - 1)
        {
            this->_grow();
        }

        int wrappedIndex = this->_wrapIndex(this->start - 1);
        T *result = this->values + wrappedIndex;
        this->start--;
        this->size++;
        return result;
    }

    T *get(int index)
    {
        assertOrAbort(index < this->start + this->size, "Deque index overflow");
        assertOrAbort(index >= this->start, "Deque index underflow");
        int wrappedIndex = this->_wrapIndex(index);
        return this->values + wrappedIndex;
    }

    void popRight()
    {
        assertOrAbort(this->size > 0, "Can't pop an empty deque");
        this->size--;
    }

    void popLeft()
    {
        assertOrAbort(this->size > 0, "Can't pop an empty deque");
        this->start++;
        this->size--;
    }

    T *_start()
    {
        int index = this->_wrapIndex(this->start);
        return this->values + index;
    }

    bool _hasMore(T *current)
    {
        int endIndex = this->_wrapIndex(this->start + this->size);
        return (current - this->values) != endIndex;
    }

    T *_next(T *current)
    {
        current++;
        if (current == this->values + this->capacity)
        {
            current = this->values;
        }
        return current;
    }
};

#define deque_for_each(item, deque)                                                                          \
    for (auto item = (deque)._start(); (deque)._hasMore(item); item = (deque)._next(item))
