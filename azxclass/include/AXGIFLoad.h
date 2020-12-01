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

#ifndef _AX_GIFLOAD_H
#define _AX_GIFLOAD_H

#include "AXMem.h"
#include "AXBuf.h"

class AXString;

class AXGIFLoad
{
public:
    int     m_nWidth,
            m_nHeight,
            m_nBits,
            m_nBkIndex,
            m_nTPIndex,
            m_nLocalPalCnt,
            m_nGlobalPalCnt;
    AXMem   m_memImg,
            m_memGlobalPal,
            m_memLocalPal;

protected:
    AXBuf   m_buf;
    AXMem   m_memDat;

protected:
    BOOL _topheader();
    BOOL _palette(AXMem *pmem,int cnt);
    BOOL _nextImgBlock();
    BOOL _skipSubBlock();
    BOOL _block_grpctrl();
    BOOL _block_image();

public:
    void free();

    BOOL loadFile(const AXString &filename);
    BOOL loadBuf(LPVOID pBuf,DWORD dwSize);

    BOOL getNextImage();
    void lineTo32bit(LPDWORD pDst,int y,BOOL bTPZero);
    void to32bit(LPDWORD pDst,BOOL bTPZero);

    DWORD getPalCol(int index);
    void getPalette(DWORD **ppBuf,LPINT pCnt);
};

#endif
