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

#ifndef _AZDRAW_TILEIMG_H_
#define _AZDRAW_TILEIMG_H_

#include "AXDef.h"

class CImage32;
class CImage8;
class AXRect;

class CTileImg
{
protected:
    BYTE    m_buf[64 * 64];

protected:
    void _encWriteRLE(LPBYTE pDstBuf,LPINT pDstPos,LPBYTE pDat,int cnt,int type);

public:
    LPBYTE getBuf() { return m_buf; }
    LPBYTE getBufPt(int x,int y) { return m_buf + (y << 6) + x; }
    BYTE getPixel(int x,int y) { return m_buf[(y << 6) + x]; }
    void setPixel(int x,int y,int val) { m_buf[(y << 6) + x] = val; }

    void clear();
    void clear(BYTE val);
    void copy(CTileImg *psrc);
    BOOL isFullZero();

    void reverseHorz();

    void blend32Bit(CImage32 *pimgDst,int dx,int dy,DWORD col,int alpha,const AXRect &rcClip);
    void blend32Bit_real(CImage32 *pimgDst,int dx,int dy,DWORD col,int alpha,const AXRect &rcClip,BOOL bAlpha);

    void convFrom32Bit(const CImage32 &src,int sx,int sy,BOOL bAlpha);
    void convFrom8Bit(LPBYTE pSrc,int sx,int sy,int sw,int sh,int pitchs);
    void bltTo8Bit(CImage8 *pimg,int dx,int dy);

    WORD encodePackBits(LPBYTE pDst);
    void decodePackBits(LPBYTE pSrcBuf,int size);
};

#endif
