#include <gtest/gtest.h>
#include "10_Core/collections/VuList.h"

struct TestElement
{
    int value;
};

TEST(VuListTest, DefaultConstructor)
{
    VuList<TestElement> list(0, AllocatorType::Standard);
    EXPECT_EQ(list.count, 0);
    EXPECT_EQ(list.capacity, 0);
    EXPECT_EQ(list.data, nullptr);
}

TEST(VuListTest, AddElement)
{
    VuList<TestElement> list(1, AllocatorType::Standard);
    TestElement         element{42};
    list.add(element);
    EXPECT_EQ(list.count, 1);
    EXPECT_EQ(list[0].value, 42);
}

TEST(VuListTest, AddMultipleElements)
{
    VuList<TestElement> list(1, AllocatorType::Standard);
    for (int i = 0; i < 5; ++i)
    {
        list.add(TestElement{i});
    }
    EXPECT_EQ(list.count, 5);
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(list[i].value, i);
    }
}

TEST(VuListTest, RemoveElement)
{
    VuList<TestElement> list(1, AllocatorType::Standard);
    for (int i = 0; i < 5; ++i)
    {
        list.add(TestElement{i});
    }
    list.remove(2);
    EXPECT_EQ(list.count, 4);
    EXPECT_EQ(list[2].value, 3);
}

TEST(VuListTest, RemoveElementOutOfBounds)
{
    VuList<TestElement> list(0, AllocatorType::Standard);
    list.add(TestElement{42});
    EXPECT_ANY_THROW(list.remove(1););
}

TEST(VuListTest, SubscriptOperatorOutOfBounds)
{
    VuList<TestElement> list(1, AllocatorType::Standard);
    list.add(TestElement{42});
    EXPECT_ANY_THROW(list[1]);
}

TEST(VuListTest, Resize)
{
    VuList<TestElement> list(1, AllocatorType::Standard);
    for (int i = 0; i < 10; ++i)
    {
        list.add(TestElement{i});
    }
    EXPECT_EQ(list.count, 10);
    EXPECT_GE(list.capacity, 10);
    for (int i = 0; i < 10; ++i)
    {
        EXPECT_EQ(list[i].value, i);
    }
}
