#include "Color32.h"

namespace Vu
{
    u8 Color32::getR() const
    {
        return r;
    }

    u8 Color32::getG() const
    {
        return g;
    }

    u8 Color32::getB() const
    {
        return b;
    }

    u8 Color32::getA() const
    {
        return a;
    }

    float Color32::getRf() const
    {
        return r / 255.0f;
    }

    float Color32::getGf() const
    {
        return g / 255.0f;
    }

    float Color32::getBf() const
    {
        return b / 255.0f;
    }

    float Color32::getAf() const
    {
        return a / 255.0f;
    }

    void Color32::setR(u8 v)
    {
        r = v;
    }

    void Color32::setG(u8 v)
    {
        g = v;
    }

    void Color32::setB(u8 v)
    {
        b = v;
    }

    void Color32::setA(u8 v)
    {
        a = v;
    }

    void Color32::setRf(float v)
    {
        r = toU8(v);
    }

    void Color32::setGf(float v)
    {
        g = toU8(v);
    }

    void Color32::setBf(float v)
    {
        b = toU8(v);
    }

    void Color32::setAf(float v)
    {
        a = toU8(v);
    }


}
