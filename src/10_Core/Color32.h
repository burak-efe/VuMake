#pragma once

#include <cstdint>
#include <algorithm>

#include "VuCommon.h"

namespace Vu
{
    class Color32
    {
    public:
        // Underlying 8-bit color components
        u8 r = 0;
        u8 g = 0;
        u8 b = 0;
        u8 a = 255;

        // Constructors
        constexpr explicit Color32() = default;

        // Default white, full alpha
        constexpr explicit Color32(u32 v) : r((v >> 0) & 0xFF), g((v >> 8) & 0xFF), b((v >> 16) & 0xFF), a((v >> 24) & 0xFF)
        {
        }

        constexpr explicit Color32(u8 red, u8 green, u8 blue, u8 alpha = 255) : r(red), g(green), b(blue), a(alpha)
        {
        }

        constexpr explicit Color32(float red, float green, float blue, float alpha = 1.0f)
        {
            setRGBA_f(red, green, blue, alpha);
        }

        // Get components as u8
        u8 getR() const { return r; }
        u8 getG() const { return g; }
        u8 getB() const { return b; }
        u8 getA() const { return a; }

        // Get components as float [0.0, 1.0]
        float getRf() const { return r / 255.0f; }
        float getGf() const { return g / 255.0f; }
        float getBf() const { return b / 255.0f; }
        float getAf() const { return a / 255.0f; }

        // Set components with u8
        void setR(u8 v) { r = v; }
        void setG(u8 v) { g = v; }
        void setB(u8 v) { b = v; }
        void setA(u8 v) { a = v; }

        // Set components with float
        void setRf(float v) { r = toU8(v); }
        void setGf(float v) { g = toU8(v); }
        void setBf(float v) { b = toU8(v); }
        void setAf(float v) { a = toU8(v); }

        // Set all at once
        constexpr void setRGBA_uint8(u8 red, u8 green, u8 blue, u8 alpha = 255)
        {
            r = red;
            g = green;
            b = blue;
            a = alpha;
        }

        constexpr void setRGBA_f(float red, float green, float blue, float alpha = 1.0f)
        {
            setRGBA_uint8(toU8(red), toU8(green), toU8(blue), toU8(alpha));
        }

    private:
        static constexpr u8 toU8(float v)
        {
            float clamped = std::clamp(v, 0.0f, 1.0f);
            return static_cast<u8>(clamped * 255.0f + 0.5f);
        }
    };
}
