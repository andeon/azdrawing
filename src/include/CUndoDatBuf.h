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

#ifndef _AZDRAW_UNDODATBUF_H_
#define _AZDRAW_UNDODATBUF_H_

#include "AXDef.h"

class AXString;

class CUndoDatBuf
{
    static int m_nFileGlobalCnt;

protected:
    DWORD   m_dwSize;
    LPBYTE  m_pBuf;
    int     m_nFileNo;

protected:
    BOOL _allocFile();
    BOOL _allocBuf(DWORD size);
    void _getFileName(AXString *pstr,int no);

public:
    CUndoDatBuf();
    ~CUndoDatBuf();

    DWORD getSize() const { return m_dwSize; }

    void free();
    BOOL alloc(DWORD size,BOOL bFile=FALSE);

    BOOL openWrite();
    void write(const void *pBuf,DWORD size);
    void closeWrite(BOOL bReAlloc);

    BOOL openRead();
    void read(void *pBuf,DWORD size);
    void closeRead();
    void seekRead(int size);
};

#endif
