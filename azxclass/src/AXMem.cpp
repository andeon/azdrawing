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

#include <stdlib.h>
#include <string.h>

#include "AXMem.h"


//************************************
// AXMem
//************************************

/*!
    @class AXMem
    @brief メモリ管理クラス

    @ingroup etc
*/


AXMem::~AXMem()
{
    free();
}

AXMem::AXMem()
{
    m_pBuf   = NULL;
    m_dwSize = 0;
}

AXMem::AXMem(DWORD size)
{
    m_pBuf   = NULL;
    m_dwSize = 0;

    alloc(size);
}

//! 解放

void AXMem::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);

        m_pBuf   = NULL;
        m_dwSize = 0;
    }
}

//! 確保

BOOL AXMem::alloc(DWORD size)
{
    free();

    //

    if(size)
    {
        m_pBuf = ::malloc(size);
        if(!m_pBuf) return FALSE;

        m_dwSize = size;
    }

    return TRUE;
}

//! 確保＆ゼロクリア

BOOL AXMem::allocClear(DWORD size)
{
    BOOL ret = alloc(size);

    if(ret) ::memset(m_pBuf, 0, size);

    return ret;
}

//! サイズ変更

BOOL AXMem::realloc(DWORD size)
{
    LPVOID pnew;

    if(!m_pBuf) return FALSE;

    pnew = ::realloc(m_pBuf, size);
    if(!pnew) return FALSE;

    m_pBuf   = pnew;
    m_dwSize = size;

    return TRUE;
}

//! AXMem からコピー

void AXMem::copy(const AXMem &memSrc)
{
    if(alloc(memSrc.getSize()))
        ::memcpy(m_pBuf, memSrc, memSrc.getSize());
}

//! バッファからコピー

void AXMem::copyFrom(LPVOID pSrc,DWORD size)
{
    if(m_pBuf)
        ::memcpy(m_pBuf, pSrc, size);
}

//! バッファにコピー

void AXMem::copyTo(LPVOID pDst,DWORD size)
{
    if(m_pBuf)
        ::memcpy(pDst, m_pBuf, size);
}


//************************************
// AXMemAuto
//************************************

/*!
    @class AXMemAuto
    @brief 自動拡張メモリクラス

    @ingroup etc
*/


AXMemAuto::AXMemAuto()
{
    m_pBuf      = NULL;
    m_dwNowSize = 0;
}

AXMemAuto::~AXMemAuto()
{
    free();
}

//! 解放

void AXMemAuto::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);

        m_pBuf      = NULL;
        m_dwNowSize = 0;
    }
}

//! 初期確保
/*!
    @param dwInitSize 初期確保サイズ
    @param dwExSize   拡張時に一度に増やすサイズ
*/

BOOL AXMemAuto::alloc(DWORD dwInitSize,DWORD dwExSize)
{
    free();

    m_pBuf = ::malloc(dwInitSize);
    if(!m_pBuf) return FALSE;

    m_dwAllocSize = dwInitSize;
    m_dwNowSize   = 0;
    m_dwExSize    = dwExSize;

    return TRUE;
}

//! 現在のサイズでリサイズする（余分な領域を切り落とす）

BOOL AXMemAuto::cutNowSize()
{
    LPVOID pnew;

    if(m_dwNowSize == 0) return FALSE;

    pnew = ::realloc(m_pBuf, m_dwNowSize);
    if(!pnew) return FALSE;

    m_pBuf        = pnew;
    m_dwAllocSize = m_dwNowSize;

    return TRUE;
}

//! データ追加

BOOL AXMemAuto::addDat(LPVOID pDat,DWORD dwSize)
{
    LPVOID pnew;
    DWORD newsize;

    if(!m_pBuf) return FALSE;

    //確保サイズを超える場合、再確保

    if(m_dwNowSize + dwSize > m_dwAllocSize)
    {
        newsize = m_dwAllocSize + m_dwExSize;

        if(newsize < m_dwNowSize + dwSize)
            newsize = m_dwNowSize + dwSize;

        //

        pnew = ::realloc(m_pBuf, newsize);
        if(!pnew) return FALSE;

        m_pBuf        = pnew;
        m_dwAllocSize = newsize;
    }

    //追加

    ::memcpy((LPBYTE)m_pBuf + m_dwNowSize, pDat, dwSize);

    m_dwNowSize += dwSize;

    return TRUE;
}

//! BYTE値追加

BOOL AXMemAuto::addBYTE(BYTE val)
{
    return addDat(&val, 1);
}

//! WORD値追加

BOOL AXMemAuto::addWORD(WORD val)
{
    return addDat(&val, 2);
}

//! DWORD値追加

BOOL AXMemAuto::addDWORD(DWORD val)
{
    return addDat(&val, 4);
}

//! リトルエンディアンでWORD値追加

BOOL AXMemAuto::addWORDLE(WORD val)
{
    BYTE b[2];

    b[0] = val & 0xff;
    b[1] = val >> 8;

    return addDat(b, 2);
}

//! リトルエンディアンでDWORD値追加

BOOL AXMemAuto::addDWORDLE(DWORD val)
{
    BYTE b[4];

    b[0] = val & 0xff;
    b[1] = (val >> 8) & 0xff;
    b[2] = (val >> 16) & 0xff;
    b[3] = val >> 24;

    return addDat(b, 4);
}
