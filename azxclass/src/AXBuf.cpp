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

#include <string.h>

#include "AXBuf.h"

#include "AXString.h"


/*!
    @class AXBuf
    @brief バッファ内の値操作クラス

    - 内部でメモリを確保するわけではなく、指定された位置の値を直接操作する補助型クラス。
    - ファイルからデータをメモリに読み込み、値を読み込む場合などに。
    - エンディアン指定可。

    @ingroup etc
*/


AXBuf::AXBuf()
{
    m_pNow = m_pTop = NULL;
}

//! 初期化
/*!
    @param dwSize 書き込み時は関係なし
*/

void AXBuf::init(LPVOID pBuf,DWORD dwSize,int endian)
{
    m_pNow      = (LPBYTE)pBuf;
    m_pTop      = m_pNow;
    m_dwSize    = dwSize;
    m_endian    = endian;
}

//! 残りサイズ取得

int AXBuf::getRemainSize() const
{
    return m_dwSize - (DWORD)(m_pNow - m_pTop);
}

//! 残りのデータが指定サイズ以上あるか

BOOL AXBuf::isRemain(DWORD size) const
{
    return (m_dwSize - (DWORD)(m_pNow - m_pTop) >= size);
}

//! 指定サイズ分移動（＋方向のみ）

BOOL AXBuf::seek(int size)
{
    if(!isRemain(size))
        return FALSE;
    else
    {
        m_pNow += size;
        return TRUE;
    }
}

//! 位置をセット

BOOL AXBuf::setPos(DWORD pos)
{
    if(pos >= m_dwSize) return FALSE;

    m_pNow = m_pTop + pos;

    return TRUE;
}

//! 位置をセット（前の位置を返す）

int AXBuf::setPosRetBk(int pos)
{
    int ret = m_pNow - m_pTop;

    m_pNow = m_pTop + pos;

    return ret;
}


//=============================
//読み込み
//=============================


//! 読み込み調整

BOOL AXBuf::_read(LPVOID pbuf,DWORD size)
{
    //最後尾を超える場合、ゼロで埋める

    if(!isRemain(size))
    {
        ::memset(pbuf, 0, size);
        m_pNow = m_pTop + m_dwSize;

        return FALSE;
    }

    return TRUE;
}

//! 指定サイズ分取得（エンディアン関係なし）
/*!
    @return FALSEでバッファのデータが足りない
*/

BOOL AXBuf::getDat(LPVOID pbuf,DWORD size)
{
    if(!_read(pbuf, size)) return FALSE;

    ::memcpy(pbuf, m_pNow, size);

    m_pNow += size;

    return TRUE;
}

//! BYTE値取得

BOOL AXBuf::getBYTE(LPVOID pbuf)
{
    if(!_read(pbuf, 1)) return FALSE;

    *((LPBYTE)pbuf) = *m_pNow;
    m_pNow++;

    return TRUE;
}

//! WORD値取得

BOOL AXBuf::getWORD(LPVOID pbuf)
{
    WORD val = 0;

    if(!_read(pbuf, 2)) return FALSE;

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            val = *((LPWORD)m_pNow);
            break;
        case ENDIAN_LITTLE:
            val = ((WORD)m_pNow[1] << 8) | m_pNow[0];
            break;
        case ENDIAN_BIG:
            val = ((WORD)m_pNow[0] << 8) | m_pNow[1];
            break;
    }

    *((LPWORD)pbuf) = val;
    m_pNow += 2;

    return TRUE;
}

//! DWORD値取得

BOOL AXBuf::getDWORD(LPVOID pbuf)
{
    DWORD val = 0;

    if(!_read(pbuf, 4)) return FALSE;

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            val = *((LPDWORD)m_pNow);
            break;
        case ENDIAN_LITTLE:
            val = ((DWORD)m_pNow[3] << 24) | (m_pNow[2] << 16) | (m_pNow[1] << 8) | m_pNow[0];
            break;
        case ENDIAN_BIG:
            val = ((DWORD)m_pNow[0] << 24) | (m_pNow[1] << 16) | (m_pNow[2] << 8) | m_pNow[3];
            break;
    }

    *((LPDWORD)pbuf) = val;
    m_pNow += 4;

    return TRUE;
}

//! BYTE値取得（データが足りない場合、デフォルトで0）

BYTE AXBuf::getBYTE()
{
    BYTE val;

    getBYTE(&val);

    return val;
}

//! WORD値取得（データが足りない場合、デフォルトで0）

WORD AXBuf::getWORD()
{
    WORD val;

    getWORD(&val);

    return val;
}

//! DWORD値取得（データが足りない場合、デフォルトで0）

DWORD AXBuf::getDWORD()
{
    DWORD val;

    getDWORD(&val);

    return val;
}

//! 指定文字列長さ分読み込んで文字列比較

BOOL AXBuf::getStrCompare(LPCSTR pText)
{
    int len = ::strlen(pText);

    if(!isRemain(len)) return FALSE;

    m_pNow += len;

    return (::strncmp(pText, (LPSTR)(m_pNow - len), len) == 0);
}

//! 長さ（可変）+UTF8 から文字列取得

BOOL AXBuf::getStrLenAndUTF8(AXString *pstr)
{
    int len,shift;
    BYTE bt;

    //長さ

    for(len = 0, shift = 0; 1; shift += 7)
    {
        if(!getBYTE(&bt)) return FALSE;

        len |= (int)(bt & 0x7f) << shift;

        if(bt < 128) break;
    }

    //UTF8

    if(!isRemain(len)) return FALSE;

    pstr->setUTF8((LPCSTR)m_pNow, len);

    m_pNow += len;

    return TRUE;
}


//=============================
//書き込み
//=============================


//! 指定サイズ書き込み（エンディアン関係なし）

void AXBuf::setDat(const void *pbuf,DWORD size)
{
    ::memcpy(m_pNow, pbuf, size);
    m_pNow += size;
}

//! 2バイト値書き込み

void AXBuf::setWORD(const void *pbuf)
{
    WORD val = *((LPWORD)pbuf);

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            *((LPWORD)m_pNow) = val;
            break;
        case ENDIAN_LITTLE:
            m_pNow[0] = val & 0xff;
            m_pNow[1] = val >> 8;
            break;
        case ENDIAN_BIG:
            m_pNow[0] = val >> 8;
            m_pNow[1] = val & 0xff;
            break;
    }

    m_pNow += 2;
}

//! 4バイト値書き込み

void AXBuf::setDWORD(const void *pbuf)
{
    DWORD val = *((LPDWORD)pbuf);

    switch(m_endian)
    {
        case ENDIAN_SYSTEM:
            *((LPDWORD)m_pNow) = val;
            break;
        case ENDIAN_LITTLE:
            m_pNow[0] = val & 0xff;
            m_pNow[1] = (val >> 8) & 0xff;
            m_pNow[2] = (val >> 16) & 0xff;
            m_pNow[3] = val >> 24;
            break;
        case ENDIAN_BIG:
            m_pNow[0] = val >> 24;
            m_pNow[1] = (val >> 16) & 0xff;
            m_pNow[2] = (val >> 8) & 0xff;
            m_pNow[3] = val & 0xff;
            break;
    }

    m_pNow += 4;
}

//! 文字列セット

void AXBuf::setStr(LPCSTR pText)
{
    int len = ::strlen(pText);

    ::memcpy(m_pNow, pText, len);

    m_pNow += len;
}

//! 0 を指定サイズ分セット

void AXBuf::setZero(int size)
{
    for(; size > 0; size--)
        *(m_pNow++) = 0;
}

//! BYTE値セット

void AXBuf::setBYTE(BYTE val)
{
    *(m_pNow++) = val;
}

//! WORD値セット

void AXBuf::setWORD(WORD val)
{
    setWORD(&val);
}

//! DWORD値セット

void AXBuf::setDWORD(DWORD val)
{
    setDWORD(&val);
}
