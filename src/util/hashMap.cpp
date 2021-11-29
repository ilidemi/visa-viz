#pragma once

#include "util.cpp"
#include <stdint.h>

template <typename TKey, typename TValue>
struct HashMap
{
    struct Entry
    {
        uint64_t hash;
        TKey key;
        TValue value;
    };

    int size;
    int capacity;
    Entry *entries;

    static uint64_t _hash(TKey key)
    {
        uint64_t z = (uint64_t)(key) + 0x9e3779b97f4a7c15ULL;
        z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
        z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
        z = z ^ (z >> 31);
        return z | 0x8000000000000000ULL; // Final value is never zero
    }

    void init()
    {
        this->size = 0;
        this->capacity = 4;
        this->entries = (typename HashMap<TKey, TValue>::Entry *)malloc(
            this->capacity * sizeof(typename HashMap<TKey, TValue>::Entry));
        assertOrAbort(this->entries != NULL, "Could not allocate memory for hash map");
        for (int i = 0; i < this->capacity; i++)
        {
            this->entries[i].hash = 0;
        }
    }

    void free()
    {
        this->size = 0;
        this->capacity = 0;
        ::free(this->entries);
        this->entries = NULL;
    }

    bool contains(TKey key)
    {
        uint64_t hash = _hash(key);
        int bucket = hash % this->capacity;
        while (this->entries[bucket].hash != 0)
        {
            if (this->entries[bucket].key == key)
            {
                return true;
            }
            bucket = (bucket + 1) % this->capacity;
        }

        return false;
    }

    bool tryGet(TKey key, TValue *outValue)
    {
        uint64_t hash = _hash(key);
        int bucket = hash % this->capacity;
        while (this->entries[bucket].hash != 0)
        {
            if (this->entries[bucket].key == key)
            {
                *outValue = this->entries[bucket].value;
                return true;
            }
            bucket = (bucket + 1) % this->capacity;
        }

        return false;
    }

    TValue get(TKey key)
    {
        uint64_t hash = _hash(key);
        int bucket = hash % this->capacity;
        while (this->entries[bucket].hash != 0)
        {
            if (this->entries[bucket].key == key)
            {
                return this->entries[bucket].value;
            }
            bucket = (bucket + 1) % this->capacity;
        }

        abortWithMessage("Value not found in hash map");
    }

    void insert(TKey key, TValue value)
    {
        if (this->size > 0.7 * this->capacity)
        {
            int oldCapacity = this->capacity;
            Entry *oldEntries = this->entries;

            this->capacity *= 2;
            this->entries = (Entry *)malloc(this->capacity * sizeof(Entry));
            assertOrAbort(this->entries != NULL, "Could not allocate more memory for hash map");

            for (int i = 0; i < this->capacity; i++)
            {
                this->entries[i].hash = 0;
            }

            for (int i = 0; i < oldCapacity; i++)
            {
                if (oldEntries[i].hash != 0)
                {
                    uint64_t hash = _hash(oldEntries[i].key);
                    int bucket = hash % this->capacity;
                    while (this->entries[bucket].hash != 0)
                    {
                        bucket = (bucket + 1) % this->capacity;
                    }
                    this->entries[bucket].hash = _hash(oldEntries[i].key);
                    this->entries[bucket].key = oldEntries[i].key;
                    this->entries[bucket].value = oldEntries[i].value;
                }
            }

            ::free(oldEntries);
        }

        uint64_t hash = _hash(key);
        int bucket = hash % this->capacity;

        while (this->entries[bucket].hash != 0)
        {
            bucket = (bucket + 1) % this->capacity;
        }

        this->entries[bucket].hash = hash;
        this->entries[bucket].key = key;
        this->entries[bucket].value = value;
        this->size++;
    }
};