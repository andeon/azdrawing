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

#include "AXTextUTF8.h"

#include "AXString.h"
#include "AXByteString.h"
#include "AXFile.h"


/*!
    @class AXTextUTF8
    @brief UTF8テキスト（複数行）操作クラス

    @ingroup etc
*/


AXTextUTF8::~AXTextUTF8()
{
    free();
}

AXTextUTF8::AXTextUTF8()
{
    m_pBuf = m_pNow = NULL;
    m_dwSize = 0;
}

//! 解放

void AXTextUTF8::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);

        m_pBuf = m_pNow = NULL;
    }
}

//! ファイルから読み込み

BOOL AXTextUTF8::loadFile(const AXString &filename)
{
    AXFile file;
    DWORD size;

    free();

    //

    if(!file.openRead(filename)) return FALSE;

    size = file.getSize();
    if(size == 0) return FALSE;

    m_pBuf = (LPSTR)::malloc(size + 1);
    if(!m_pBuf) return FALSE;

    if(!file.readSize(m_pBuf, size))
    {
        free();
        return FALSE;
    }

    file.close();

    //最後にNULL追加

    *(m_pBuf + size) = 0;

    //

    m_pNow   = m_pBuf;
    m_dwSize = size;

    return TRUE;
}

//! バッファから読み込み
/*!
    内部バッファにデータをコピーする。
*/

BOOL AXTextUTF8::loadBuf(LPVOID pBuf,DWORD dwSize)
{
    free();

    //

    m_pBuf = (LPSTR)::malloc(dwSize + 1);
    if(!m_pBuf) return FALSE;

    ::memcpy(m_pBuf, pBuf, dwSize);

    *(m_pBuf + dwSize) = 0;

    //

    m_pNow   = m_pBuf;
    m_dwSize = dwSize;

    return TRUE;
}

//! 次の1行文字列取得（改行のみの行は除く）

BOOL AXTextUTF8::getNextLine(AXString *pstr)
{
    LPSTR pend,pnext;
    int len;

    //1文字でも文字がある行を検索

    while(1)
    {
        pend = getNextEnter(&pnext);
        if(!pend) return FALSE;

        len = pend - m_pNow;
        if(len != 0) break;

        m_pNow = pnext;
    }

    //セット

    pstr->setUTF8(m_pNow, len);

    m_pNow = pnext;

    return TRUE;
}

//! 次の1行文字列取得（改行のみの行は除く）

BOOL AXTextUTF8::getNextLine(AXByteString *pstr)
{
    LPSTR pend,pnext;
    int len;

    //1文字でも文字がある行を検索

    while(1)
    {
        pend = getNextEnter(&pnext);
        if(!pend) return FALSE;

        len = pend - m_pNow;
        if(len != 0) break;

        m_pNow = pnext;
    }

    //セット

    pstr->set(m_pNow, len);

    m_pNow = pnext;

    return TRUE;
}

//! 次の改行位置のポインタ取得（改行なしで終端に来た場合も含む）
/*!
    @param ppRet 改行の次の文字の位置が返る。NULL可。
    @return 改行のある位置。NULLで終了
*/

LPSTR AXTextUTF8::getNextEnter(char **ppRet)
{
    LPSTR p,pret;

    if(!m_pBuf || *m_pNow == 0) return NULL;

    for(p = m_pNow; *p; p++)
    {
        if(*p == '\n' || *p == '\r')
        {
            if(*p == '\r' && p[1] == '\n')
                pret = p + 2;
            else
                pret = p + 1;

            break;
        }
    }

    if(ppRet) *ppRet = (*p == 0)? p: pret;

    return p;
}
