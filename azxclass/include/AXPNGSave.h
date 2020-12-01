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

#ifndef _AX_PNGSAVE_H
#define _AX_PNGSAVE_H

#include "AXFile.h"
#include "AXMem.h"

class AXString;
class AXPNGSave_z;

class AXPNGSave
{
public:
    typedef struct
    {
        int     nWidth,
                nHeight,
                nBits,
                nPalCnt;
        AXMem   memPalette;
    }INFO;

protected:
    AXFile  m_file;
    AXPNGSave::INFO *m_pInfo;

    int         m_nPitch;
    DWORD       m_dwSizePos;
    AXPNGSave_z *m_pz;

protected:
    void putThunk(LPCSTR name,LPBYTE pDat,int size);
    void _IHDR();
    void _PLTE();

public:
    AXPNGSave();
    ~AXPNGSave();

    int getPitch() const { return m_nPitch; }

    BOOL openFile(const AXString &filename,AXPNGSave::INFO *pInfo);
    void close();

    BOOL startImg(int nLevel=6);
    BOOL putImg(LPBYTE pBuf,int nSize=-1);
    BOOL endImg();

    void put_tRNS(DWORD col);
    void put_pHYs(int h,int v);

    void lineFrom32bit(LPBYTE pDst,LPDWORD pSrc,int bits);
};

#endif
