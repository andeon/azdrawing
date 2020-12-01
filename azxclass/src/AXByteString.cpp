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

#include "AXByteString.h"

#include "AXUtilStr.h"

/*!
    @class AXByteString
    @brief 文字列クラス（マルチバイト文字列）

    ASCII 文字列や UTF8 文字列など、1Byte の NULL 文字で終わる文字列。
    @attention 文字列の長さが変わったら m_nLen の値を変更すること

    @ingroup etc
*/


//---------------------------


AXByteString::~AXByteString()
{
    if(m_pBuf)
        ::free(m_pBuf);
}

AXByteString::AXByteString()
{
    _malloc(31);
}

AXByteString::AXByteString(int len)
{
    _malloc(len);
}


//============================
//確保関連
//============================


//! 最大長さから確保サイズ取得

int AXByteString::_calcSize(int len)
{
    int size;

    for(size = 32; size < (1 << 28); size <<= 1)
    {
        if(len < size) break;
    }

    return size;
}

//! 初期バッファ確保

void AXByteString::_malloc(int len)
{
    int size;

    m_nMallocSize = m_nLen = 0;

    //

    size = _calcSize(len);

    m_pBuf = (LPSTR)::malloc(size);
    if(!m_pBuf) return;

    ::memset(m_pBuf, 0, size);

    m_nMallocSize = size;
}

//! バッファリサイズ

BOOL AXByteString::resize(int len)
{
    LPSTR pNew;
    int newsize;

    //拡張の必要なし

    if(m_nMallocSize > len) return TRUE;

    //拡張

    newsize = _calcSize(len);

    pNew = (LPSTR)::realloc(m_pBuf, newsize);
    if(!pNew) return FALSE;

    m_pBuf          = pNew;
    m_nMallocSize   = newsize;

    return TRUE;
}

//! 文字列長さ再計算

void AXByteString::recalcLen()
{
    m_nLen = ::strlen(m_pBuf);
}

//! 空にする

void AXByteString::empty()
{
    *m_pBuf = 0;
    m_nLen  = 0;
}

//! 空かどうか

BOOL AXByteString::isEmpty() const
{
    return (*m_pBuf == 0);
}

//! 空でないかどうか

BOOL AXByteString::isNoEmpty() const
{
    return (*m_pBuf != 0);
}


//================================
//
//================================


//! 指定位置の文字取得

char AXByteString::operator [](int pos) const
{
    if(pos < m_nLen)
        return m_pBuf[pos];
    else
        return 0;
}

//! 1文字セット

void AXByteString::operator =(char ch)
{
    m_pBuf[0] = ch;
    m_pBuf[1] = 0;

    m_nLen = 1;
}

//! 文字列セット
/*!
    @param text NULL で空にする
*/

void AXByteString::operator =(LPCSTR text)
{
    if(!text)
        empty();
    else
    {
        int len = ::strlen(text);

        if(resize(len))
        {
            ::memcpy(m_pBuf, text, len + 1);
            m_nLen = len;
        }
    }
}

//! 文字列セット

void AXByteString::operator =(const AXByteString &str)
{
    int len = str.getLen();

    if(resize(len))
    {
        ::memcpy(m_pBuf, (LPSTR)str, len + 1);
        m_nLen = len;
    }
}

//! 1文字追加

void AXByteString::operator +=(char ch)
{
    if(resize(m_nLen + 1))
    {
        m_pBuf[m_nLen]     = ch;
        m_pBuf[m_nLen + 1] = 0;

        m_nLen++;
    }
}

//! 文字列追加

void AXByteString::operator +=(LPCSTR text)
{
    int len = ::strlen(text);

    if(resize(m_nLen + len))
    {
        ::memcpy(m_pBuf + m_nLen, text, len + 1);
        m_nLen += len;
    }
}

//! int値を文字列にして追加

void AXByteString::operator +=(int val)
{
    char m[16];

    AXIntToStr(m, val);
    *this += m;
}

//! 文字列比較

BOOL AXByteString::operator ==(LPCSTR text) const
{
    return (::strcmp(m_pBuf, text) == 0);
}


//==========================


//! 長さを指定してセット

void AXByteString::set(LPCSTR text,int len)
{
    if(resize(len))
    {
        ::memcpy(m_pBuf, text, len);
        m_pBuf[len] = 0;

        m_nLen = len;
    }
}

//! 文字列追加

void AXByteString::append(LPCSTR text,int len)
{
    if(resize(m_nLen + len))
    {
        ::memcpy(m_pBuf + m_nLen, text, len);

        m_nLen += len;
        m_pBuf[m_nLen] = 0;
    }
}

//! text をセットした後 val の数値を文字列にして追加

void AXByteString::setAddInt(LPCSTR text,int val)
{
    *this = text;
    *this += val;
}

//! 指定位置に文字セット

void AXByteString::setAt(int pos,char ch)
{
    if(pos >= 0 && pos < m_nMallocSize)
        m_pBuf[pos] = ch;
}

//! 先頭に文字列を追加

void AXByteString::prefix(LPCSTR text)
{
    int len = ::strlen(text);

    if(!resize(m_nLen + len)) return;

    ::memmove(m_pBuf + len, m_pBuf, m_nLen + 1);
    ::memcpy(m_pBuf, text, len);

    m_nLen += len;
}

//! 小文字変換

void AXByteString::toLower()
{
    LPSTR pc = m_pBuf;

    for(; *pc; pc++)
    {
        if(*pc >= 'A' && *pc <= 'Z')
            *pc += 0x20;
    }
}

//! 置き換え

void AXByteString::replace(char ch,char chNew)
{
    LPSTR pc = m_pBuf;

    for(; *pc; pc++)
    {
        if(*pc == ch) *pc = chNew;
    }
}

//! 文字検索

int AXByteString::find(char ch)
{
    LPCSTR pc;
    int pos;

    for(pc = m_pBuf, pos = 0; *pc; pc++, pos++)
    {
        if(*pc == ch) return pos;
    }

    return -1;
}
