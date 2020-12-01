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

#ifndef _AX_BMPSAVE_H
#define _AX_BMPSAVE_H

#include "AXMem.h"
#include "AXFile.h"

class AXString;

class AXBMPSave
{
public:
    struct INFO
    {
        int     nWidth,
                nHeight,
                nBits,
                nResoH,
                nResoV;
    };

protected:
    AXBMPSave::INFO *m_pInfo;
    AXFile      m_file;
    AXMem       m_memLine;
    int         m_nPitch;

protected:
    void _fileheader();
    void _infoheader();

public:
    int getPitch() const { return m_nPitch; }
    LPBYTE getLineBuf() const { return (LPBYTE)m_memLine; }

    BOOL openFile(const AXString &filename,AXBMPSave::INFO *pInfo);
    void close();

    void putLine();
    void putLineFrom32bit(LPDWORD pSrc);
};

#endif
