/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#ifndef _AZDRAW_IMAGE1_H
#define _AZDRAW_IMAGE1_H

#include "AXDef.h"

class AXRectSize;
class AXImage;

class CImage1
{
protected:
    LPBYTE  m_pBuf;
    int     m_nWidth,
            m_nHeight,
            m_nPitch;

    void _setMinMax(AXPoint *pMin,AXPoint *pMax,int x,int y);

public:
    CImage1();
    ~CImage1();

    LPBYTE getBuf() const { return m_pBuf; }
    int getWidth() const { return m_nWidth; }
    int getHeight() const { return m_nHeight; }
    int getPitch() const { return m_nPitch; }

    LPBYTE getBufPt(int x,int y) { return m_pBuf + y * m_nPitch + (x >> 3); }

    void free();
    BOOL create(int width,int height);
    void clear();

    void setPixel(int x,int y,int val);
    int getPixel(int x,int y);

    void line(int x1,int y1,int x2,int y2,int val);
    void box(int x,int y,int w,int h,int val);
    void drawBezierTemp(AXPoint *pPos,AXRectSize *prcs,int type);

    void blendXor(AXImage *pimgDst,int x,int y,int w,int h);
};

#endif
