#include <gtest/gtest.h>


#include "10_Core/VuCommon.h"
#include "10_Core/Color32.h"

// Clamp float to [0,1] while setting
TEST(Color32Test, SaturateFloatSetter) {
    Vu::Color32 color(10.0f, -141.0f, 20.0f, 809043667.4f);
    EXPECT_NEAR(color.getRf(), 1.0f, 0.0001f);
    EXPECT_NEAR(color.getGf(), 0.0f, 0.0001f);
    EXPECT_NEAR(color.getBf(), 1.0f, 0.0001f);
    EXPECT_NEAR(color.getAf(), 1.0f, 0.0001f);
}

// Test default constructor
TEST(Color32Test, DefaultConstructor) {
    Vu::Color32 color;
    EXPECT_EQ(color.getR(), 0);
    EXPECT_EQ(color.getG(), 0);
    EXPECT_EQ(color.getB(), 0);
    EXPECT_EQ(color.getA(), 255);
}

// Test constructor with u32
TEST(Color32Test, ConstructorWithU32) {
    Vu::Color32 color(0x11223344);
    EXPECT_EQ(color.getR(), 0x44);
    EXPECT_EQ(color.getG(), 0x33);
    EXPECT_EQ(color.getB(), 0x22);
    EXPECT_EQ(color.getA(), 0x11);
}

// Test constructor with u8
TEST(Color32Test, ConstructorWithU8) {
    Vu::Color32 color(10_ub, 20_ub, 30_ub, 255_ub);
    EXPECT_EQ(color.getR(), 10);
    EXPECT_EQ(color.getG(), 20);
    EXPECT_EQ(color.getB(), 30);
    EXPECT_EQ(color.getA(), 255);
}

// Test constructor with float
TEST(Color32Test, ConstructorWithFloat) {
    Vu::Color32 color(0.1f, 0.2f, 0.3f, 0.4f);
    EXPECT_NEAR(color.getRf(), 0.1f, 0.01f);
    EXPECT_NEAR(color.getGf(), 0.2f, 0.01f);
    EXPECT_NEAR(color.getBf(), 0.3f, 0.01f);
    EXPECT_NEAR(color.getAf(), 0.4f, 0.01f);
}

// Test set and get methods
TEST(Color32Test, SetAndGetMethods) {
    Vu::Color32 color;
    color.setR(50);
    color.setG(60);
    color.setB(70);
    color.setA(80);
    EXPECT_EQ(color.getR(), 50);
    EXPECT_EQ(color.getG(), 60);
    EXPECT_EQ(color.getB(), 70);
    EXPECT_EQ(color.getA(), 80);
}