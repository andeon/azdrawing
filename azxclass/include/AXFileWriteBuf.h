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

#ifndef _AX_FILEWRITEBUF_H
#define _AX_FILEWRITEBUF_H

#include "AXFile.h"

class AXFileWriteBuf
{
protected:
    AXFile  m_file;
    LPBYTE  m_pBuf;
    int     m_nBufSize,
            m_nNowSize;

public:
    virtual ~AXFileWriteBuf();
    AXFileWriteBuf();

    void close();
    BOOL open(const AXString &filename,int bufsize=4096);

    void flush();
    void put(const void *pBuf,int size);

    void putStr(LPCSTR pText);
    void putStrLenAndUTF8(const AXString &str);
    void putBYTE(BYTE val);
    void putWORDLE(WORD val);
    void putDWORDLE(DWORD val);
    void putWORDBE(WORD val);
    void putDWORDBE(DWORD val);
};

#endif
