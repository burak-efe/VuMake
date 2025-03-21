#pragma once

#include <cstdint>
#include <algorithm>

#include "VuCommon.h"

namespace Vu
{
    class Color32
    {
    public:
        // Underlying 32-bit color storage
        uint32 value;

        // Constructors
        constexpr explicit Color32() : value(0xFFFFFFFF)
        {
        }

        // Default white, full alpha
        constexpr explicit Color32(uint32 v) : value(v)
        {
        }

        constexpr explicit Color32(uint8 r, uint8 g, uint8 b, uint8 a = 255)
        {
            setRGBA_uint8(r, g, b, a);
        }

        constexpr explicit Color32(float r, float g, float b, float a = 1.0f)
        {
            setRGBA_f(r, g, b, a);
        }

        // Get components as u8
        uint8 r() const { return (value >> 0) & 0xFF; }
        uint8 g() const { return (value >> 8) & 0xFF; }
        uint8 b() const { return (value >> 16) & 0xFF; }
        uint8 a() const { return (value >> 24) & 0xFF; }

        // Get components as float [0.0, 1.0]
        float rf() const { return r() / 255.0f; }
        float gf() const { return g() / 255.0f; }
        float bf() const { return b() / 255.0f; }
        float af() const { return a() / 255.0f; }

        // Set components with u8
        void setR(uint8 v) { value = (value & 0xFFFFFF00) | v; }
        void setG(uint8 v) { value = (value & 0xFFFF00FF) | (v << 8); }
        void setB(uint8 v) { value = (value & 0xFF00FFFF) | (v << 16); }
        void setA(uint8 v) { value = (value & 0x00FFFFFF) | (v << 24); }

        // Set components with float
        void setRf(float v) { setR(toU8(v)); }
        void setGf(float v) { setG(toU8(v)); }
        void setBf(float v) { setB(toU8(v)); }
        void setAf(float v) { setA(toU8(v)); }

        // Set all at once
        constexpr void setRGBA_uint8(uint8 r, uint8 g, uint8 b, uint8 a = 255)
        {
            value = (a << 24) | (b << 16) | (g << 8) | r;
        }

        constexpr void setRGBA_f(float r, float g, float b, float a = 1.0f)
        {
            setRGBA_uint8(toU8(r), toU8(g), toU8(b), toU8(a));
        }

    private:
        static constexpr uint8 toU8(float v)
        {
            return static_cast<uint8>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
        }
    };
}
