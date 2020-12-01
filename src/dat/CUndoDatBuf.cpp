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

#include <stdlib.h>
#include <string.h>

#include "CUndoDatBuf.h"

#include "CConfig.h"
#include "CApp.h"
#include "defGlobal.h"

#include "AXFile.h"
#include "AXString.h"
#include "AXUtilStr.h"
#include "AXUtilFile.h"


//--------------------

AXFile  g_fileWrite,
        g_fileRead;
LPBYTE  g_pbufWrite,
        g_pbufRead;
DWORD   g_dwWriteSize;

#define FILENO_BUF  -1

//--------------------


/*!
    @class CUndoDatBuf
    @brief アンドゥデータのデータバッファまたはファイルの情報
*/


int CUndoDatBuf::m_nFileGlobalCnt = 0;


CUndoDatBuf::CUndoDatBuf()
{
    m_dwSize  = 0;
    m_pBuf    = NULL;
    m_nFileNo = FILENO_BUF;
}

CUndoDatBuf::~CUndoDatBuf()
{
    free();
}

//! 解放

void CUndoDatBuf::free()
{
    if(m_nFileNo == FILENO_BUF)
        AXFree((void **)&m_pBuf);
    else
    {
        //ファイル削除

        AXString str;

        _getFileName(&str, m_nFileNo);
        AXDeleteFile(str);

        m_nFileNo = FILENO_BUF;
    }
}


//=============================
//確保
//=============================


//! バッファ/ファイル確保

BOOL CUndoDatBuf::alloc(DWORD size,BOOL bFile)
{
    BOOL ret;

    m_dwSize = size;

    if(bFile || size > 500 * 1024 || (g_pconf->uFlags & CConfig::FLAG_UNDO_PUTFILE))
    {
        //ファイル優先

        ret = _allocFile();

        if(!ret)
            ret = _allocBuf(size);
    }
    else
    {
        //バッファ優先

        ret = _allocBuf(size);

        if(!ret)
            ret = _allocFile();
    }

    return ret;
}

//! ファイル作成

BOOL CUndoDatBuf::_allocFile()
{
    AXString str;
    AXFile file;

    if(CAPP->getTmpDir().isEmpty()) return FALSE;

    _getFileName(&str, m_nFileGlobalCnt);

    if(!file.openWrite(str)) return FALSE;

    file.close();

    m_nFileNo = m_nFileGlobalCnt;
    m_nFileGlobalCnt = (m_nFileGlobalCnt + 1) % 10000;

    return TRUE;
}

//! メモリ確保

BOOL CUndoDatBuf::_allocBuf(DWORD size)
{
    m_pBuf = (LPBYTE)::malloc(size);

    return (m_pBuf != NULL);
}

//! テンポラリファイル名取得

void CUndoDatBuf::_getFileName(AXString *pstr,int no)
{
    char m[20];

    AXIntToStrDig(m, no, 4);

    *pstr = CAPP->getTmpDir();
    pstr->path_add(m);
}


//=============================
//データ書き込み
//=============================


//! 書き込みオープン

BOOL CUndoDatBuf::openWrite()
{
    if(m_nFileNo == FILENO_BUF)
    {
        //バッファ

        if(!m_pBuf) return FALSE;

        g_pbufWrite     = m_pBuf;
        g_dwWriteSize   = 0;
    }
    else
    {
        //ファイル

        AXString str;

        _getFileName(&str, m_nFileNo);

        if(!g_fileWrite.openWrite(str)) return FALSE;
    }

    return TRUE;
}

//! 書き込み

void CUndoDatBuf::write(const void *pBuf,DWORD size)
{
    if(m_nFileNo == FILENO_BUF)
    {
        ::memcpy(g_pbufWrite, pBuf, size);

        g_pbufWrite     += size;
        g_dwWriteSize   += size;
    }
    else
        g_fileWrite.write(pBuf, size);
}

//! 書きクローズ
/*!
    @param bReAlloc 実際に書き込まれたサイズにメモリをリサイズ
*/

void CUndoDatBuf::closeWrite(BOOL bReAlloc)
{
    if(m_nFileNo != FILENO_BUF)
        g_fileWrite.close();
    else
    {
        if(bReAlloc && m_dwSize != g_dwWriteSize)
        {
            LPBYTE pnew;

            pnew = (LPBYTE)::realloc(m_pBuf, g_dwWriteSize);
            if(pnew)
                m_pBuf = pnew;
        }
    }
}


//=============================
//データ読み込み
//=============================


//! 読み込みオープン

BOOL CUndoDatBuf::openRead()
{
    if(m_nFileNo == FILENO_BUF)
    {
        if(!m_pBuf) return FALSE;

        g_pbufRead = m_pBuf;
    }
    else
    {
        AXString str;

        _getFileName(&str, m_nFileNo);

        if(!g_fileRead.openRead(str)) return FALSE;
    }

    return TRUE;
}

//! 読み込み

void CUndoDatBuf::read(void *pBuf,DWORD size)
{
    if(m_nFileNo == FILENO_BUF)
    {
        ::memcpy(pBuf, g_pbufRead, size);
        g_pbufRead += size;
    }
    else
        g_fileRead.read(pBuf, size);
}

//! 読みクローズ

void CUndoDatBuf::closeRead()
{
    if(m_nFileNo != FILENO_BUF)
        g_fileRead.close();
}

//! 読み込みシーク

void CUndoDatBuf::seekRead(int size)
{
    if(m_nFileNo == FILENO_BUF)
        g_pbufRead += size;
    else
        g_fileRead.seekCur(size);
}
