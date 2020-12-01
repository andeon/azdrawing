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

#ifndef _AX_IMAGE_H
#define _AX_IMAGE_H

#include "AXDef.h"


class AXImage8;
class AXImage32;
class AXGC;
class AXRectSize;
class AXFont;

class AXImage
{
public:
    enum { COL_XOR = 0xffffffff };

protected:
    LPVOID  m_pImg,
            m_pShmInfo;
    int     m_nWidth,
            m_nHeight,
            m_nPitch,
            m_nBytes;

protected:
    void _to32bit(const AXImage32 &imgSrc);
    void _to24bit(const AXImage32 &imgSrc);
    void _to16bit(const AXImage32 &imgSrc);

public:
    AXImage();
    virtual ~AXImage();

    int getWidth() const { return m_nWidth; }
    int getHeight() const { return m_nHeight; }
    int getPitch() const { return m_nPitch; }
    int getBytes() const { return m_nBytes; }
    BOOL isExist() const { return (m_pImg != NULL); }

    void free();
    BOOL create(int w,int h);
    BOOL recreate(int w,int h,int hblock,int vblock);

    void put(UINT dstID) const;
    void put(UINT dstID,int dx,int dy) const;
    void put(UINT dstID,int dx,int dy,int sx,int sy,int w,int h) const;
    void put(UINT dstID,const AXGC &gc,int dx,int dy,int sx,int sy,int w,int h) const;

    BOOL createFromImage32(const AXImage32 &imgSrc);
    void setFromImage32(const AXImage32 &imgSrc);
    BOOL createDisableImg(const AXImage32 &imgSrc);

    LPBYTE getBuf() const;
    LPBYTE getBufPt(int x,int y) const;
    void getRGBFromBuf(LPBYTE pbuf,LPINT pr,LPINT pg,LPINT pb) const;

    void setPixel(int x,int y,DWORD col);
    void setPixelPx(int x,int y,DWORD colPix);
    void setPixelBuf(LPBYTE pBuf,DWORD col);
    void setPixelBuf(LPBYTE pBuf,int r,int g,int b);
    void setPixelBufXor(LPBYTE pBuf);
    void setPixelBufPx(LPBYTE pBuf,DWORD colPix);
    void blendPixel(int x,int y,DWORD col);
    void blendPixelBuf(LPBYTE pBuf,DWORD col);
    void blendPixelBuf(LPBYTE pBuf,int r,int g,int b,int a);

    void clear(DWORD col);
    void line(int x1,int y1,int x2,int y2,DWORD col);
    void lineNoEnd(int x1,int y1,int x2,int y2,DWORD col);
    void lineH(int x,int y,int w,DWORD col);
    void lineV(int x,int y,int h,DWORD col);
    void lineHBuf(LPBYTE pbuf,int w,DWORD col);
    void lines(const AXPoint *ppt,int cnt,DWORD col);
    void box(int x,int y,int w,int h,DWORD col);
    void boxClip(int x,int y,int w,int h,DWORD col);
    void fillBox(int x,int y,int w,int h,DWORD col);
    void fillCheck(int x,int y,int w,int h,DWORD col1,DWORD col2);
    void ellipse(int x1,int y1,int x2,int y2,DWORD col);

    void drawPattern(int x,int y,DWORD col,const BYTE *pPattern,int ptw,int pth);
    void drawPattern(int x,int y,DWORD colFg,DWORD colBg,const BYTE *pPattern,int ptw,int pth);
    void drawNumber(int x,int y,LPCSTR pText,DWORD col);

    void drawText(const AXFont &font,int x,int y,int w,int h,LPCUSTR pText,int len,DWORD col);
    void drawText(const AXFont &font,int x,int y,int w,int h,LPCSTR pText,int len,DWORD col);
    void drawTextWrap(const AXFont &font,int x,int y,int w,int h,LPCUSTR pText,int len,DWORD col);

    void blt(int dx,int dy,const AXImage8 &imSrc,int sx,int sy,int w,int h);
    void blt(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h);
    void bltTP(int dx,int dy,const AXImage8 &imgSrc,int sx,int sy,int w,int h,DWORD col);
    void bltTP(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h,DWORD col);
    void bltAlpha(int dx,int dy,const AXImage32 &imgSrc,int sx,int sy,int w,int h);

    void setPreviewImg(const AXImage32 *pimgSrc,DWORD dwBkCol1,DWORD dwBkCol2);
    void setScaleUpImg(const AXImage32 *pimgSrc,const AXRectSize &rcs,int topx,int topy,int scale,DWORD dwExCol);
};

#endif
