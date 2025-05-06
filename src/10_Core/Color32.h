#pragma once

#include <algorithm>                // for clamp
#include "08_LangUtils/TypeDefs.h"  // for u8, u32

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


        constexpr Color32() = default;

        constexpr explicit Color32(u32 v): r((v >> 0) & 0xFF), g((v >> 8) & 0xFF), b((v >> 16) & 0xFF),
                                           a((v >> 24) & 0xFF)
        {
        }

        constexpr explicit Color32(u8 red, u8 green, u8 blue, u8 alpha): r(red), g(green), b(blue), a(alpha)
        {
        }

        constexpr explicit Color32(float red, float green, float blue, float alpha = 0.0f)
        {
            setRGBA_f(red, green, blue, alpha);
        }


        // Get components as u8
        u8 getR() const;
        u8 getG() const;
        u8 getB() const;
        u8 getA() const;

        // Get components as float [0.0, 1.0]
        float getRf() const;
        float getGf() const;
        float getBf() const;
        float getAf() const;

        // Set components with u8
        void setR(u8 v);
        void setG(u8 v);
        void setB(u8 v);
        void setA(u8 v);

        // Set components with float
        void setRf(float v);
        void setGf(float v);
        void setBf(float v);
        void setAf(float v);

        constexpr void setRGBA_uint8(u8 red, u8 green, u8 blue, u8 alpha)
        {
            r = red;
            g = green;
            b = blue;
            a = alpha;
        }

        constexpr void setRGBA_f(float red, float green, float blue, float alpha)
        {
            setRGBA_uint8(toU8(red), toU8(green), toU8(blue), toU8(alpha));
        }

    private:
        constexpr u8 toU8(float v)
        {
            float clamped = std::clamp(v, 0.0f, 1.0f);
            return static_cast<u8>(clamped * 255.0f + 0.5f);
        }
    };
}
