#include "testDeque.cpp"
#include "testTweetDataLoad.cpp"

int main()
{
    // testTweetDataLoad();

    testDeque();
    testDequeWrappingRight();

    printf("Tests succeeded\n");
}