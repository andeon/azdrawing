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

#ifndef _AZDRAW_IMAGE8_H_
#define _AZDRAW_IMAGE8_H_

#include "AXDef.h"

class AXImage32;
class AXImage;

class CImage8
{
protected:
    LPBYTE  m_pBuf;
    int     m_nWidth,
            m_nHeight;
    DWORD   m_dwAllocSize; //!< 確保サイズ（4BYTE単位）

public:
    ~CImage8();
    CImage8();

    BOOL isExist() const { return (m_pBuf != NULL); }
    LPBYTE getBuf() const { return m_pBuf; }
    int getWidth() const { return m_nWidth; }
    int getHeight() const { return m_nHeight; }

    LPBYTE getBufPt(int x,int y) { return m_pBuf + y * m_nWidth + x; }
    BYTE getPixel(int x,int y) { return *(m_pBuf + y * m_nWidth + x); }

    void free();
    BOOL create(int width,int height);
    BOOL clone(const CImage8 &src);

    void createBrushImg_fillCircle(int size);
    BYTE getPixelTexture(int x,int y);

    void clear();
    void copy(const CImage8 &src);

    BOOL createFrom32bit(const AXImage32 &src,BOOL bAlpha);
    BOOL createFrom32bit_brush(const AXImage32 &src,BOOL bAlpha);

    void drawTexturePrev(AXImage *pimgDst);
    void drawBrushPrev(AXImage *pimgDst);
};

#endif
