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

#include "AXFileWriteBuf.h"
#include "AXString.h"
#include "AXByteString.h"


/*!
    @class AXFileWriteBuf
    @brief バッファ付きのファイル書き込みクラス

    @ingroup etc
*/


AXFileWriteBuf::AXFileWriteBuf()
{
    m_pBuf      = NULL;
    m_nBufSize  = 0;
    m_nNowSize  = 0;
}

AXFileWriteBuf::~AXFileWriteBuf()
{
    close();
}

//! 閉じる

void AXFileWriteBuf::close()
{
    if(m_file.isOpen())
    {
        flush();
        m_file.close();
    }

    AXFree((void **)&m_pBuf);
}

//! 開く

BOOL AXFileWriteBuf::open(const AXString &filename,int bufsize)
{
    close();

    //開く

    if(!m_file.openWrite(filename)) return FALSE;

    //バッファ確保

    m_pBuf = (LPBYTE)AXMalloc(bufsize);
    if(!m_pBuf)
    {
        m_file.close();
        return FALSE;
    }

    m_nBufSize = bufsize;
    m_nNowSize = 0;

    return TRUE;
}

//! バッファの内容を書き込み

void AXFileWriteBuf::flush()
{
    if(m_nNowSize)
    {
        m_file.write(m_pBuf, m_nNowSize);
        m_nNowSize = 0;
    }
}

//! データ出力

void AXFileWriteBuf::put(const void *pBuf,int size)
{
    LPBYTE ps = (LPBYTE)pBuf;
    int n;

    while(size > 0)
    {
        //転送サイズ

        n = (m_nNowSize + size >= m_nBufSize)? m_nBufSize - m_nNowSize: size;

        //バッファにコピー

        ::memcpy(m_pBuf + m_nNowSize, ps, n);
        m_nNowSize += n;

        //バッファが一杯なら書き込み

        if(m_nNowSize == m_nBufSize)
            flush();

        //

        ps   += n;
        size -= n;
    }
}


//========================


//! ASCII文字列出力

void AXFileWriteBuf::putStr(LPCSTR pText)
{
    put(pText, ::strlen(pText));
}

//! 文字列を長さ（可変サイズ）とUTF8文字列で出力

void AXFileWriteBuf::putStrLenAndUTF8(const AXString &str)
{
    AXByteString utf8;
    int len,n;

    str.toUTF8(&utf8);

    //長さ

    len = utf8.getLen();

    if(len == 0)
    {
        putBYTE(0);
        return;
    }

    for(n = len; n; n >>= 7)
    {
        putBYTE((n < 128)? n: (n & 0x7f) | 0x80);
    }

    //UTF8

    put((LPSTR)utf8, len);
}

//! 1バイト出力

void AXFileWriteBuf::putBYTE(BYTE val)
{
    put(&val, 1);
}

//! 2バイト（リトルエンディアン）出力

void AXFileWriteBuf::putWORDLE(WORD val)
{
    BYTE dat[2];

    dat[0] = (BYTE)val;
    dat[1] = (BYTE)(val >> 8);

    put(dat, 2);
}

//! 4バイト（リトルエンディアン）出力

void AXFileWriteBuf::putDWORDLE(DWORD val)
{
    BYTE dat[4];

    dat[0] = (BYTE)val;
    dat[1] = (BYTE)(val >> 8);
    dat[2] = (BYTE)(val >> 16);
    dat[3] = (BYTE)(val >> 24);

    put(dat, 4);
}

//! 2バイト（ビッグエンディアン）出力

void AXFileWriteBuf::putWORDBE(WORD val)
{
    BYTE dat[2];

    dat[0] = (BYTE)(val >> 8);
    dat[1] = (BYTE)val;

    put(dat, 2);
}

//! 4バイト（ビッグエンディアン）出力

void AXFileWriteBuf::putDWORDBE(DWORD val)
{
    BYTE dat[4];

    dat[0] = (BYTE)(val >> 24);
    dat[1] = (BYTE)(val >> 16);
    dat[2] = (BYTE)(val >> 8);
    dat[3] = (BYTE)val;

    put(dat, 4);
}
