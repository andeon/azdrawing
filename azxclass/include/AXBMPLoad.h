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

#ifndef _AX_BMPLOAD_H
#define _AX_BMPLOAD_H

#include "AXFileBuf.h"
#include "AXMem.h"

class AXString;

class AXBMPLoad
{
public:
    int     m_nWidth,
            m_nHeight,
            m_nPitch,
            m_nBits,
            m_nPalCnt,
            m_nResoH,
            m_nResoV;
    AXMem   m_memPalette;

    DWORD   m_maskR,m_maskG,m_maskB;

protected:
    AXFileBuf   m_src;
    AXMem       m_memLine;
    BOOL        m_bVRev;
    int         m_nYCnt;
    int         m_nRShiftR,m_nGShiftR,m_nBShiftR,
                m_nRbit,m_nGbit,m_nBbit;

protected:
    BOOL _load();
    BOOL _fileheader(LPDWORD pOffset);
    BOOL _infoheader();
    BOOL _bitfield(DWORD dwComp);
    BOOL _palette();

public:
    void close();

    BOOL openFile(const AXString &filename);
    BOOL openBuf(LPVOID pBuf,DWORD dwSize);

    int readLine();
    void lineTo32bit(LPDWORD pDst);
};

#endif
