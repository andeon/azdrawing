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

#ifndef _AX_PNGLOAD_H
#define _AX_PNGLOAD_H

#include "AXMem.h"

class AXBuf;
class AXString;
class AXImage32;
class AXImage8;


class AXPNGLoad
{
public:
    int     m_nWidth,
            m_nHeight,
            m_nPitch,
            m_nBits,
            m_nPalCnt,
            m_nTPCol,
            m_nResoH,
            m_nResoV;
    AXMem   m_memImg,
            m_memPalette;

protected:
    AXBuf   *m_pbuf;

    BOOL _load(AXBuf *pbuf);
    BOOL _checkHeader();
    BOOL _thunk_IHDR(LPBYTE pFlag);
    BOOL _thunk_PLTE(DWORD size);
    void _thunk_tRNS(BOOL bGray);
    void _thunk_pHYs();
    BOOL _thunk_IDAT_Normal(DWORD thunksize);
    BOOL _thunk_IDAT_Interlace(DWORD thunksize);

    BOOL _createGrayPalette();
    BOOL _decode_IDAT(LPBYTE pDstBuf,DWORD dwDstSize,DWORD thunksize);
    void _restoreFilter(LPBYTE pbuf,int pitch,int bpp,int y);
    BYTE _paeth(BYTE a,BYTE b,BYTE c);

public:
    void free();

    BOOL loadFile(const AXString &filename);
    BOOL loadBuf(LPVOID pBuf,DWORD dwSize);

    void lineTo32bit(LPDWORD pDst,int y,BOOL bTPAlpha);
    BOOL toImage32(AXImage32 *pimg,BOOL bTPAlpha);
    BOOL toImage8(AXImage8 *pimg);
    BOOL toAppIconImg(AXMem *pmem);
};

#endif
