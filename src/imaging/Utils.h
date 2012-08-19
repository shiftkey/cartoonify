//--------------------------------------------------------------------------
// 
//  Copyright (c) Microsoft Corporation.  All rights reserved. 
// 
//  File: Utils.h
//
//  Implementation of Cartoonizer Algorithms
//
//--------------------------------------------------------------------------

#pragma once

#include <functional>
#include <algorithm>

class Util
{
public:
    static const double Wr;
    static const double Wb;
    static const double wg;

    static inline void RGBToYUV(COLORREF color, double& y, double& u, double& v)
    {
        double r = GetRValue(color) / 255.0;
        double g = GetGValue(color) / 255.0;
        double b = GetBValue(color) / 255.0;

        y = Wr * r + Wb * b + wg * g;
        u = 0.436 * (b - y) / (1 - Wb);
        v = 0.615 * (r - y) / (1 - Wr);
    }

    static inline double GetDistance(COLORREF color1, COLORREF color2)
    {
        double y1, u1, v1, y2, u2, v2;
        RGBToYUV(color1, y1, u1, v1);
        RGBToYUV(color2, y2, u2, v2);

        return std::sqrt(std::pow(u1 - u2, 2) + std::pow(v1 - v2, 2));
    }

    static inline float SmoothStep(float a, float b, float x)
    {
        if (x < a) return 0.0f;
        else if (x >= b) return 1.0f;

        x = (x - a) / (b - a);
        return (x * x * (3.0f - 2.0f * x));
    }

    static inline double CombinePlus(double i, double j)
    {
        return i + j;
    }
};