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

#ifndef _AX_MEM_H
#define _AX_MEM_H

#include "AXDef.h"

class AXMem
{
protected:
    LPVOID  m_pBuf;
    DWORD   m_dwSize;

public:
    ~AXMem();
    AXMem();
    AXMem(DWORD size);

    LPVOID getBuf() const { return m_pBuf; }
    DWORD getSize() const { return m_dwSize; }
    BOOL isExist() const { return (m_pBuf != NULL); }
    BOOL isNULL() const { return (m_pBuf == NULL); }

    operator LPVOID() const { return m_pBuf; }
    operator LPBYTE() const { return (LPBYTE)m_pBuf; }
    operator LPSTR() const { return (LPSTR)m_pBuf; }
    operator LPWORD() const { return (LPWORD)m_pBuf; }
    operator LPDWORD() const { return (LPDWORD)m_pBuf; }
    operator LPINT() const { return (LPINT)m_pBuf; }

    void free();
    BOOL alloc(DWORD size);
    BOOL allocClear(DWORD size);
    BOOL realloc(DWORD size);

    void copy(const AXMem &memSrc);
    void copyFrom(LPVOID pSrc,DWORD size);
    void copyTo(LPVOID pDst,DWORD size);
};

//-------------------

class AXMemAuto
{
protected:
    LPVOID  m_pBuf;
    DWORD   m_dwAllocSize,
            m_dwNowSize,
            m_dwExSize;

public:
    AXMemAuto();
    ~AXMemAuto();

    LPVOID getBuf() const { return m_pBuf; }
    DWORD getNowSize() const { return m_dwNowSize; }
    DWORD getAllocSize() const { return m_dwAllocSize; }
    BOOL isExist() const { return (m_pBuf != NULL); }

    void clearNowSize() { m_dwNowSize = 0; }
    void setNowSize(DWORD size) { m_dwNowSize = size; }

    operator LPVOID() const { return m_pBuf; }
    operator LPBYTE() const { return (LPBYTE)m_pBuf; }
    operator LPSTR() const { return (LPSTR)m_pBuf; }
    operator LPWORD() const { return (LPWORD)m_pBuf; }
    operator LPDWORD() const { return (LPDWORD)m_pBuf; }

    void free();
    BOOL alloc(DWORD dwInitSize,DWORD dwExSize);
    BOOL cutNowSize();

    BOOL addDat(LPVOID pDat,DWORD dwSize);
    BOOL addBYTE(BYTE val);
    BOOL addWORD(WORD val);
    BOOL addDWORD(DWORD val);

    BOOL addWORDLE(WORD val);
    BOOL addDWORDLE(DWORD val);
};

#endif
