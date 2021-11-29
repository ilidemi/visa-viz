#pragma once

#include "../src/util/deque.cpp"
#include "assert.cpp"

void testDeque()
{
    Deque<int> deque;
    deque.init();
    ASSERT(deque.size == 0);

    deque.appendRight(1234);
    ASSERT(deque.size == 1);
    ASSERT(*deque.get(0) == 1234);

    deque.appendLeft(5678);
    ASSERT(deque.size == 2);
    ASSERT(*deque.get(-1) == 5678);
    ASSERT(*deque.get(0) == 1234);

    deque.appendRight(3);
    deque.appendRight(4);
    deque.appendRight(5);
    ASSERT(deque.size == 5);
    ASSERT(*deque.get(-1) == 5678);
    ASSERT(*deque.get(0) == 1234);
    ASSERT(*deque.get(1) == 3);
    ASSERT(*deque.get(2) == 4);
    ASSERT(*deque.get(3) == 5);

    deque.appendLeft(6);
    deque.appendLeft(7);
    deque.appendLeft(8);
    deque.appendLeft(9);
    ASSERT(deque.size == 9);
    ASSERT(*deque.get(-5) == 9);
    ASSERT(*deque.get(-4) == 8);
    ASSERT(*deque.get(-3) == 7);
    ASSERT(*deque.get(-2) == 6);
    ASSERT(*deque.get(-1) == 5678);
    ASSERT(*deque.get(0) == 1234);
    ASSERT(*deque.get(1) == 3);
    ASSERT(*deque.get(2) == 4);
    ASSERT(*deque.get(3) == 5);

    // deque.get(-6);
    // deque.get(4);
}

void testDequeWrappingRight()
{
    Deque<int> deque;
    deque.init();

    deque.appendRight(0);
    deque.popLeft();
    deque.appendRight(1);
    deque.popLeft();
    deque.appendRight(2);
    deque.popLeft();
    deque.appendRight(3);
    deque.popLeft();
    deque.appendRight(4);
    ASSERT(deque.size == 1);
    ASSERT(*deque.get(4) == 4);
}