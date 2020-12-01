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
#include <zlib.h>

#include "AXUtilZlib.h"

#include "AXFile.h"
#include "AXMem.h"


/*!
    @defgroup AXUtilZlib AXUtilZlib
    zlib 関連ユーティリティ

    @ingroup util

    @{
*/


//==================================
//展開
//==================================


//! ファイルから読み込んで展開

BOOL AXZlib_DecodeFromFile(LPBYTE pDstBuf,DWORD dwDstSize,AXFile *pFile,DWORD dwSrcSize,DWORD dwBufSize)
{
    z_stream z;
    AXMem mem;
    DWORD remain = dwSrcSize;
    int ret;
    LONGLONG rsize;

    //メモリ確保

    if(!mem.alloc(dwBufSize)) return FALSE;

    //zlib初期化

    ::memset(&z, 0, sizeof(z_stream));
    if(inflateInit(&z) != Z_OK) return FALSE;

    z.next_out	= pDstBuf;
    z.avail_out	= dwDstSize;

    //読み込み

    while(1)
    {
        //ファイルから読み込み

        if(z.avail_in == 0)
        {
            rsize = pFile->read(mem, (remain < dwBufSize)? remain: dwBufSize);
            if(rsize <= 0)
            {
                inflateEnd(&z);
                return FALSE;
            }

            z.next_in	= mem;
            z.avail_in	= rsize;

            remain -= rsize;
        }

        //展開

        ret = inflate(&z, Z_NO_FLUSH);
        if(ret == Z_STREAM_END)
            break;
        else if(ret != Z_OK)
        {
            inflateEnd(&z);
            return FALSE;
        }
    }

    inflateEnd(&z);

    return TRUE;
}

//! バッファから展開

BOOL AXZlib_DecodeFromBuf(LPBYTE pDstBuf,DWORD dwDstSize,LPBYTE pSrcBuf,DWORD dwSrcSize)
{
    z_stream z;
    int ret;

    //zlib初期化

    ::memset(&z, 0, sizeof(z_stream));
    if(inflateInit(&z) != Z_OK) return FALSE;

    z.next_out	= pDstBuf;
    z.avail_out	= dwDstSize;

    z.next_in	= pSrcBuf;
    z.avail_in	= dwSrcSize;

    //展開

    while(1)
    {
        ret = inflate(&z, Z_NO_FLUSH);
        if(ret == Z_STREAM_END)
            break;
        else if(ret != Z_OK)
        {
            inflateEnd(&z);
            return FALSE;
        }
    }

    inflateEnd(&z);

    return TRUE;
}


//==================================
//圧縮
//==================================


//! ファイルへ圧縮出力
/*!
	@return 圧縮後のデータサイズ（0でエラー）
*/

DWORD AXZlib_EncodeToFile(AXFile *pFile,LPBYTE pSrcBuf,DWORD dwSrcSize,int nLevel,DWORD dwBufSize)
{
    z_stream z;
    AXMem mem;
    LPBYTE pWork;
    int flush,ret;
    DWORD outsize = 0,size;

    //作業用バッファ

    if(!mem.alloc(dwBufSize)) return 0;

    pWork = mem;

    //zlib初期化

    ::memset(&z, 0, sizeof(z_stream));
    if(deflateInit(&z, nLevel) != Z_OK) return 0;

    flush       = Z_NO_FLUSH;
    z.next_in   = pSrcBuf;
    z.avail_in  = dwSrcSize;
    z.next_out  = pWork;
    z.avail_out = dwBufSize;

    //書き込み

    while(1)
    {
        //入力データが終わり
        if(z.avail_in == 0) flush = Z_FINISH;

        ret = deflate(&z, flush);
        if(ret == Z_STREAM_END) break;
        if(ret != Z_OK)
        {
            deflateEnd(&z);
            return 0;
        }

        //書き出し

        if(z.avail_out == 0)
        {
            pFile->write(pWork, dwBufSize);

            z.next_out  = pWork;
            z.avail_out = dwBufSize;

            outsize += dwBufSize;
        }
    }

    //残りを書き出し

    size = dwBufSize - z.avail_out;
    if(size)
    {
        pFile->write(pWork, size);
        outsize += size;
    }

    deflateEnd(&z);

    return outsize;
}

//! AXMemAuto でバッファ上へ出力
/*!
	@attention pmem はあらかじめ確保しておくこと
*/

DWORD AXZlib_EncodeToMemAuto(AXMemAuto *pmem,LPBYTE pSrcBuf,DWORD dwSrcSize,int nLevel,DWORD dwBufSize)
{
    z_stream z;
    AXMem mem;
    LPBYTE pWork;
    int flush,ret;
    DWORD outsize = 0,size;

    //メモリ確保

    if(!mem.alloc(dwBufSize)) return 0;

    pWork = mem;

    //zlib初期化

    ::memset(&z, 0, sizeof(z_stream));
    if(deflateInit(&z, nLevel) != Z_OK) return 0;

    flush		= Z_NO_FLUSH;
    z.next_in	= pSrcBuf;
    z.avail_in	= dwSrcSize;
    z.next_out	= pWork;
    z.avail_out	= dwBufSize;

    //書き込み

    while(1)
    {
        //入力データが終わり
        if(z.avail_in == 0) flush = Z_FINISH;

        ret = deflate(&z, flush);
        if(ret == Z_STREAM_END) break;
        if(ret != Z_OK)
        {
            deflateEnd(&z);
            return 0;
        }

        //書き出し

        if(z.avail_out == 0)
        {
            pmem->addDat(pWork, dwBufSize);

            z.next_out	= pWork;
            z.avail_out	= dwBufSize;

            outsize += dwBufSize;
        }
    }

    //残りを書き出し

    size = dwBufSize - z.avail_out;
    if(size)
    {
        pmem->addDat(pWork, size);
        outsize += size;
    }

    deflateEnd(&z);

    return outsize;
}

//@}
