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

#ifndef _AX_IMAGE8_H
#define _AX_IMAGE8_H

#include "AXDef.h"

class AXImage8
{
protected:
    LPBYTE  m_pBuf;
    LPDWORD m_pPal;
    int     m_nWidth,
            m_nHeight,
            m_nPitch,
            m_nPalCnt;

public:
    AXImage8();
    virtual ~AXImage8();

    int getWidth() const { return m_nWidth; }
    int getHeight() const { return m_nHeight; }
    int getPitch() const { return m_nPitch; }
    int getPaletteCnt() const { return m_nPalCnt; }
    LPBYTE getBuf() const { return m_pBuf; }
    LPDWORD getPalBuf() const { return m_pPal; }
    LPBYTE getButPt(int x,int y) const { return m_pBuf + y * m_nPitch + x; }
    DWORD getPalCol(int pal) const { return m_pPal[pal]; }

    void free();
    BOOL create(int w,int h);
    BOOL create(int w,int h,LPDWORD pPal,int palcnt);

    int findPalFromRGB(DWORD col) const;

    void clear(int pal);
};

#endif
