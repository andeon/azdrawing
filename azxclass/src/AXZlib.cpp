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

#include <zlib.h>

#include "AXZlib.h"

#include "AXFile.h"


//-----------------------

#define _ZSTREAM    ((z_stream *)m_pz)

//-----------------------


/*!
    @class AXZlib
    @brief zlib 圧縮/展開クラス

    @ingroup etc
*/


AXZlib::~AXZlib()
{
    //zlib終了

    _end_zlib();

    //

    delete (z_stream *)m_pz;
}

AXZlib::AXZlib()
{
    m_pz = new z_stream;

    m_nType  = TYPE_NONE;
    m_dwSize = 0;
}

//! 作業用バッファ確保

BOOL AXZlib::allocBuf(int bufsize)
{
    return m_memBuf.alloc(bufsize);
}

//! 終了処理（共通）

BOOL AXZlib::end()
{
    if(m_nType == TYPE_ENCODE_FILE)
        return endEncFile();
    else
    {
        _end_zlib();
        return TRUE;
    }
}

//! zlib リセット

BOOL AXZlib::reset()
{
    switch(m_nType)
    {
        //圧縮：バッファ
        case TYPE_ENCODE_BUF:
            return (::deflateReset(_ZSTREAM) == Z_OK);
        //展開：バッファ
        case TYPE_DECODE_BUF:
            return (::inflateReset(_ZSTREAM) == Z_OK);
    }

    return FALSE;
}


//============================
// サブ
//============================


//! 初期化

BOOL AXZlib::_init(int type,int nLevel)
{
    _end_zlib();

    AXMemZero(m_pz, sizeof(z_stream));

    switch(type)
    {
        //圧縮
        case TYPE_ENCODE_FILE:
        case TYPE_ENCODE_BUF:
            if(::deflateInit(_ZSTREAM, nLevel) != Z_OK)
                return FALSE;
            break;
        //展開
        case TYPE_DECODE_FILE:
        case TYPE_DECODE_BUF:
            if(::inflateInit(_ZSTREAM) != Z_OK)
                return FALSE;
            break;
    }

    m_nType = type;

    return TRUE;
}

//! zlib 終了

void AXZlib::_end_zlib()
{
    if(m_nType == TYPE_ENCODE_FILE || m_nType == TYPE_ENCODE_BUF)
        ::deflateEnd(_ZSTREAM);
    else if(m_nType == TYPE_DECODE_FILE || m_nType == TYPE_DECODE_BUF)
        ::inflateEnd(_ZSTREAM);

    m_nType = TYPE_NONE;
}


//============================
// 圧縮（ファイルへ）
//============================


//! 圧縮初期化（ファイルへ出力）
/*!
    allocBuf() であらかじめバッファが作成されていればそのまま使う。@n
    バッファが確保されていなければ自動作成される。

    @param nLevel 圧縮率（0〜9）
*/

BOOL AXZlib::initEncFile(AXFile *pfile,int nLevel)
{
    //バッファ確保

    if(m_memBuf.isNULL())
    {
        if(!m_memBuf.alloc(8192)) return FALSE;
    }

    //zlib 初期化

    if(!_init(TYPE_ENCODE_FILE, nLevel)) return FALSE;

    m_pfile  = pfile;
    m_dwSize = 0;

    _ZSTREAM->next_out  = m_memBuf;
    _ZSTREAM->avail_out = m_memBuf.getSize();

    return TRUE;
}

//! データ送る

BOOL AXZlib::putEncFile(const void *pBuf,DWORD size)
{
    int ret,bufsize;

    if(m_nType != TYPE_ENCODE_FILE) return FALSE;

    bufsize = m_memBuf.getSize();

    _ZSTREAM->next_in  = (LPBYTE)pBuf;
    _ZSTREAM->avail_in = size;

    while(_ZSTREAM->avail_in)
    {
        ret = ::deflate(_ZSTREAM, Z_NO_FLUSH);
        if(ret != Z_OK && ret != Z_STREAM_END) return FALSE;

        //書き出し

        if(_ZSTREAM->avail_out == 0)
        {
            m_pfile->write(m_memBuf, bufsize);

            _ZSTREAM->next_out  = m_memBuf;
            _ZSTREAM->avail_out = bufsize;

            m_dwSize += bufsize;
        }
    }

    return TRUE;
}

//! 圧縮終了

BOOL AXZlib::endEncFile()
{
    int ret,bufsize;

    if(m_nType != TYPE_ENCODE_FILE) return FALSE;

    bufsize = m_memBuf.getSize();

    //入力終了

    while(1)
    {
        ret = ::deflate(_ZSTREAM, Z_FINISH);

        if(ret == Z_STREAM_END) break;
        if(ret != Z_OK)
        {
            _end_zlib();
            return FALSE;
        }

        //ファイルに書き出し

        if(_ZSTREAM->avail_out == 0)
        {
            m_pfile->write(m_memBuf, bufsize);

            _ZSTREAM->next_out  = m_memBuf;
            _ZSTREAM->avail_out = bufsize;

            m_dwSize += bufsize;
        }
    }

    //残りをファイルに書き出し

    ret = bufsize - _ZSTREAM->avail_out;

    if(ret)
    {
        m_pfile->write(m_memBuf, ret);

        m_dwSize += ret;
    }

    //zlib 終了

    _end_zlib();

    return TRUE;
}


//============================
// 圧縮（バッファへ）
//============================


//! 圧縮初期化（バッファ）

BOOL AXZlib::initEncBuf(int nLevel)
{
    return _init(TYPE_ENCODE_BUF, nLevel);
}

//! バッファからバッファへ圧縮
/*!
    圧縮サイズは getEncSize() で取得。
*/

BOOL AXZlib::putEncBuf(void *pDstBuf,DWORD dwDstSize,void *pSrcBuf,DWORD dwSrcSize)
{
    int flush,ret;

    if(m_nType != TYPE_ENCODE_BUF) return FALSE;

    m_dwSize = 0;

    _ZSTREAM->next_out  = (LPBYTE)pDstBuf;
    _ZSTREAM->avail_out = dwDstSize;
    _ZSTREAM->next_in   = (LPBYTE)pSrcBuf;
    _ZSTREAM->avail_in  = dwSrcSize;

    flush = Z_NO_FLUSH;

    while(1)
    {
        if(_ZSTREAM->avail_in == 0) flush = Z_FINISH;

        ret = ::deflate(_ZSTREAM, flush);

        if(ret == Z_STREAM_END) break;
        else if(ret != Z_OK) return FALSE;
    }

    m_dwSize = dwDstSize - _ZSTREAM->avail_out;

    return TRUE;
}


//============================
// 展開（ファイルから）
//============================


//! 展開初期化（ファイルから）
/*!
    @param dwSize 圧縮データサイズ（最大でこのサイズ分読み込む）
*/

BOOL AXZlib::initDecFile(AXFile *pfile,DWORD dwSize)
{
    //バッファ確保

    if(m_memBuf.isNULL())
    {
        if(!m_memBuf.alloc(8192)) return FALSE;
    }

    //zlib 初期化

    if(!_init(TYPE_DECODE_FILE, 0)) return FALSE;

    m_pfile  = pfile;
    m_dwSize = dwSize;

    return TRUE;
}

//! 指定サイズ分展開

BOOL AXZlib::getDecFile(void *pBuf,DWORD size)
{
    int readsize,ret;
    DWORD bufsize;

    if(m_nType != TYPE_DECODE_FILE) return FALSE;

    bufsize = m_memBuf.getSize();

    _ZSTREAM->next_out  = (LPBYTE)pBuf;
    _ZSTREAM->avail_out = size;

    while(_ZSTREAM->avail_out)
    {
        //ファイルからバッファに読み込み

        if(_ZSTREAM->avail_in == 0)
        {
            if(m_dwSize == 0) return FALSE;

            readsize = m_pfile->read(m_memBuf, (m_dwSize < bufsize)? m_dwSize: bufsize);
            if(readsize <= 0) return FALSE;

            _ZSTREAM->next_in  = m_memBuf;
            _ZSTREAM->avail_in = readsize;

            m_dwSize -= readsize;
        }

        //展開

        ret = ::inflate(_ZSTREAM, Z_NO_FLUSH);

        if(ret == Z_STREAM_END)
            break;
        else if(ret != Z_OK)
            return FALSE;
    }

    return TRUE;
}


//============================
// 展開（バッファから）
//============================


//! 展開初期化（バッファから）

BOOL AXZlib::initDecBuf()
{
    return _init(TYPE_DECODE_BUF, 0);
}

//! バッファから展開

BOOL AXZlib::getDecBuf(void *pDstBuf,DWORD dwDstSize,void *pSrcBuf,DWORD dwSrcSize)
{
    int ret;

    if(m_nType != TYPE_DECODE_BUF) return FALSE;

    _ZSTREAM->next_out  = (LPBYTE)pDstBuf;
    _ZSTREAM->avail_out = dwDstSize;
    _ZSTREAM->next_in   = (LPBYTE)pSrcBuf;
    _ZSTREAM->avail_in  = dwSrcSize;

    //読み込み

    while(1)
    {
        ret = ::inflate(_ZSTREAM, Z_NO_FLUSH);

        if(ret == Z_STREAM_END)
            break;
        else if(ret != Z_OK)
            return FALSE;
    }

    return TRUE;
}
