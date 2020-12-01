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

#ifndef _AX_IMAGE32_H
#define _AX_IMAGE32_H

#include "AXDef.h"

class AXMem;
class AXRectSize;

class AXImage32
{
public:
#if defined(__BIG_ENDIAN__)
    typedef union
    {
        struct { BYTE a,r,g,b; };
        DWORD c;
    }PIXEL;
#else
    typedef union
    {
        struct { BYTE b,g,r,a; };
        DWORD c;
    }PIXEL;
#endif

protected:
    LPDWORD m_pBuf;
    int     m_nWidth,m_nHeight;

public:
    virtual ~AXImage32();
    AXImage32();
    AXImage32(int w,int h);

    BOOL isExist() const { return (m_pBuf != NULL); }
    LPDWORD getBuf() const { return m_pBuf; }
    LPDWORD getBufPt(int x,int y) const { return m_pBuf + y * m_nWidth + x; }
    int getWidth() const { return m_nWidth; }
    int getHeight() const { return m_nHeight; }

    void free();
    BOOL create(int w,int h);
    BOOL createCopy(const AXImage32 &imgSrc);

    DWORD getPixel(int x,int y) const;
    void setPixel(int x,int y,DWORD col);
    void setPixelBlend(int x,int y,DWORD col);

    void clear(DWORD col);
    void clear(const AXRectSize &rcs,DWORD col);

    void copy(const AXImage32 &imgSrc);
    void copy(const AXImage32 &imgSrc,int x,int y,int w,int h);

    void lineH(int x,int y,int w,DWORD col);
    void box(int x,int y,int w,int h,DWORD col);
    void fillBox(int x,int y,int w,int h,DWORD col);
    void fillCheck(int x,int y,int w,int h,DWORD col1,DWORD col2);

    void replaceExtract(int x,int y,int w,int h,DWORD colDst,DWORD colExt);

    void blt(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h);
    void bltClip(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h);

    int getPalette(AXMem *pmem) const;
};

#endif
