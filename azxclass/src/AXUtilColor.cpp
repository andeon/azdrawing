/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "AXUtilColor.h"


/*!
    @defgroup AXUtilColor AXUtilColor
    色関連ユーティリティ

    @ingroup util

    @{
*/


//===============================
// RGB -> HSV
//===============================


//! RGB (0-255) -> HSV (0.0-1.0)

void AXRGBtoHSV(int rr,int gg,int bb,double *pHSV)
{
    double r,g,b,max,min,d,h,s,v;

    r = rr * (1.0 / 255.0);
    g = gg * (1.0 / 255.0);
    b = bb * (1.0 / 255.0);

    if(r >= g) max = r; else max = g; if(b > max) max = b;
    if(r <= g) min = r; else min = g; if(b < min) min = b;

    v = max;
    d = max - min;

    if(max == 0) s = 0; else s = d / max;

    if(s == 0)
        h = 0;
    else
    {
        if(r == max)
            h = (g - b) / d * (1.0 / 6.0);
        else if(g == max)
            h = (2 + (b - r) / d) * (1.0 / 6.0);
        else
            h = (4 + (r - g) / d) * (1.0 / 6.0);

        if(h < 0) h += 1.0; else if(h >= 1.0) h -= 1.0;
    }

    pHSV[0] = h;
    pHSV[1] = s;
    pHSV[2] = v;
}

void AXRGBtoHSV(DWORD rgb,double *pHSV)
{
    AXRGBtoHSV((rgb >> 16) & 0xff, (rgb >> 8) & 0xff, rgb & 0xff, pHSV);
}

//! RGB -> HSV (H:0-359 SV:0-255)

void AXRGBtoHSV(DWORD rgb,LPINT pHSV)
{
    double d[3];

    AXRGBtoHSV(rgb, d);

    pHSV[0] = (int)(d[0] * 360 + 0.5);
    pHSV[1] = (int)(d[1] * 255 + 0.5);
    pHSV[2] = (int)(d[2] * 255 + 0.5);

    if(pHSV[0] >= 360) pHSV[0] -= 360;
}


//===============================
// HSV -> RGB
//===============================


//! HSV (H:0-359 SV:0.0-1.0) -> RGB (0-255)

void AXHSVtoRGB(int h,double s,double v,LPINT pRGB)
{
    double c1,c2,c3,r,g,b,tf;
    int t;

    if(s == 0)
        r = g = b = v;
    else
    {
        t = (h * 6) % 360;
        tf = t / 360.0;
        c1 = v * (1 - s);
        c2 = v * (1 - s * tf);
        c3 = v * (1 - s * (1 - tf));

        switch(h / 60)
        {
            case 0: r = v;  g = c3; b = c1; break;
            case 1: r = c2; g = v;  b = c1; break;
            case 2: r = c1; g = v;  b = c3; break;
            case 3: r = c1; g = c2; b = v;  break;
            case 4: r = c3; g = c1; b = v;  break;
            case 5: r = v;  g = c1; b = c2; break;
        }
    }

    pRGB[0] = (int)(r * 255.0 + 0.5);
    pRGB[1] = (int)(g * 255.0 + 0.5);
    pRGB[2] = (int)(b * 255.0 + 0.5);
}

DWORD AXHSVtoRGB(int h,double s,double v)
{
    int c[3];

    AXHSVtoRGB(h, s, v, c);

    return (c[0] << 16) | (c[1] << 8) | c[2];
}

//! 整数高速版 (H:0-359 SV:0-255)

DWORD AXHSVtoRGB_fast(int h,int s,int v)
{
    int c1,c2,c3,r,g,b;
    int t;

    if(s == 0)
        r = g = b = v;
    else
    {
        t = (h * 6) % 360;
        c1 = (v * (255 - s)) / 255;
        c2 = (v * (255 - s * t / 360)) / 255;
        c3 = (v * (255 - s * (360 - t) / 360)) / 255;

        switch(h / 60)
        {
            case 0: r = v;  g = c3; b = c1; break;
            case 1: r = c2; g = v;  b = c1; break;
            case 2: r = c1; g = v;  b = c3; break;
            case 3: r = c1; g = c2; b = v;  break;
            case 4: r = c3; g = c1; b = v;  break;
            case 5: r = v;  g = c1; b = c2; break;
        }
    }

    return (r << 16) | (g << 8) | b;
}


//==============================
// HLS <-> RGB
//==============================


double _funcHLStoRGB(int h,double min,double max)
{
    if(h >= 360) h -= 360;
    else if(h < 0) h += 360;

    if(h < 60)
        return min + (max - min) * h / 60;
    else if(h < 180)
        return max;
    else if(h < 240)
        return min + (max - min) * (240 - h) / 60;

    return min;
}


//! HLS -> RGB(0-255)
/*!
    @param l,s 0.0-1.0
*/

void AXHLStoRGB(int h,double l,double s,LPINT pRGB)
{
    double r,g,b,max,min;

    if(l <= 0.5)
        max = l * (1 + s);
    else
        max = l * (1 - s) + s;

    min = 2 * l - max;

    if(s == 0)
        r = g = b = l;
    else
    {
        r = _funcHLStoRGB(h + 120, min, max);
        g = _funcHLStoRGB(h, min, max);
        b = _funcHLStoRGB(h - 120, min, max);
    }

    pRGB[0] = (int)(r * 255 + 0.5);
    pRGB[1] = (int)(g * 255 + 0.5);
    pRGB[2] = (int)(b * 255 + 0.5);
}

//! HLS -> RGB

DWORD AXHLStoRGB(int h,double l,double s)
{
    int c[3];

    AXHLStoRGB(h, l, s, c);

    return (c[0] << 16) | (c[1] << 8) | c[2];
}

//! RGB -> HLS (0.0-1.0)

void AXRGBtoHLS(int r,int g,int b,double *pHLS)
{
    int min,max;
    double dmin,dmax,d,h,l,s;

    if(r >= g) max = r; else max = g; if(b > max) max = b;
    if(r <= g) min = r; else min = g; if(b < min) min = b;

    dmin = min / 255.0;
    dmax = max / 255.0;

    l = (dmax + dmin) * 0.5;

    if(max == min)
        h = s = 0;
    else
    {
        d = dmax - dmin;

        //S

        if(l <= 0.5)
            s = d / (dmax + dmin);
        else
            s = d / (2 - dmax - dmin);

        //H

        if(r == max)
            h = (g - b) / 255.0 / d * (1.0 / 6.0);
        else if(g == max)
            h = (2 + (b - r) / 255.0 / d) * (1.0 / 6.0);
        else if(b == max)
            h = (4 + (r - g) / 255.0 / d) * (1.0 / 6.0);

        if(h < 0) h += 1.0;
        else if(h >= 1.0) h -= 1.0;
    }

    pHLS[0] = h;
    pHLS[1] = l;
    pHLS[2] = s;
}

//@}
