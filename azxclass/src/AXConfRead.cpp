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
#include <stdlib.h>
#include <ctype.h>

#include "AXConfRead.h"

#include "AXFile.h"
#include "AXString.h"
#include "AXByteString.h"
#include "AXRect.h"
#include "AXMem.h"
#include "AXUtil.h"
#include "AXUtilStr.h"


/*!
    @class AXConfRead
    @brief 構成ファイル読み込みクラス（UTF8）

    - グループ名・キー名は大/小文字を区別しない。

    @ingroup etc
*/

/*
    m_nGroupIndex : -1 でグループが存在しない。
*/


AXConfRead::AXConfRead()
{
    m_pBuf      = NULL;
    m_ppIndex   = NULL;
    m_nIndexCnt   = 0;
    m_nGroupIndex = -1;
    m_nNowIndex   = -1;
}

AXConfRead::~AXConfRead()
{
    free();
}

//! 解放

void AXConfRead::free()
{
    AXFree((void **)&m_pBuf);
    AXFree((void **)&m_ppIndex);

    m_nIndexCnt   = 0;
    m_nGroupIndex = -1;
}

//! ファイルから読み込み

BOOL AXConfRead::loadFile(const AXString &filename)
{
    free();

    if(!AXFile::readFileFull(filename, (void **)&m_pBuf, NULL, TRUE))
        return FALSE;

    return _createIndex();
}

//! ホームディレクトリ＋szPathAdd のファイルから読み込み

BOOL AXConfRead::loadFileHome(LPCSTR szPathAdd)
{
    AXString str;

    str.path_setHomePath();
    str.path_add(szPathAdd);

    return loadFile(str);
}

//! バッファから読み込み

BOOL AXConfRead::loadBuf(LPVOID pBuf,DWORD dwSize)
{
    free();

    //確保

    m_pBuf = (LPSTR)AXMalloc(dwSize + 1);
    if(!m_pBuf) return FALSE;

    //コピー

    ::memcpy(m_pBuf, pBuf, dwSize);

    *(m_pBuf + dwSize) = 0;

    return _createIndex();
}

//! カレントグループセット

BOOL AXConfRead::setGroup(LPCSTR szName)
{
    AXByteString strName;
    BOOL ret = FALSE;
    char **ppIndex = m_ppIndex;
    LPSTR pc;
    int i;

    m_nGroupIndex = -1;
    m_nNowIndex   = -1;

    if(!m_pBuf) return FALSE;

    strName = szName;
    strName.toLower();

    for(i = 0; i < m_nIndexCnt; i++, ppIndex++)
    {
        if(**ppIndex == '[')
        {
            pc = *ppIndex + 1;

            //先頭文字比較

            if(*pc != strName[0]) continue;

            //文字列比較

            if(::strcmp(pc, strName) == 0)
            {
                m_nGroupIndex = i;
                m_nNowIndex   = i;

                ret = TRUE;

                break;
            }
        }
    }

    return ret;
}


//=================================
// 値取得
//=================================


//! int値取得

int AXConfRead::getInt(LPCSTR szKey,int nDef)
{
    LPSTR p = _searchKey(szKey);

    if(p)
        return ::atoi(p);
    else
        return nDef;
}

//! 16進数値取得

DWORD AXConfRead::getHex(LPCSTR szKey,DWORD dwDef)
{
    LPSTR p = _searchKey(szKey);

    if(p)
        return (DWORD)::strtoul(p, NULL, 16);
    else
        return dwDef;
}

//! 文字列取得
/*!
    @param szDef NULL で空にする
*/

void AXConfRead::getStr(LPCSTR szKey,AXString *pstr,LPCSTR szDef)
{
    LPSTR p = _searchKey(szKey);

    if(p)
        pstr->setUTF8(p);
    else
        *pstr = szDef;
}

//! 文字列取得
/*!
    @param pstr UTF8文字列が入る
    @param szDef NULL で空にする
*/

void AXConfRead::getStr(LPCSTR szKey,AXByteString *pstr,LPCSTR szDef)
{
    LPSTR p = _searchKey(szKey);

    if(p)
        *pstr = p;
    else
        *pstr = szDef;
}

//! 文字列取得
//! @param szDef NULL で空にする

void AXConfRead::getStr(LPCSTR szKey,LPSTR szBuf,int bufsize,LPCSTR szDef)
{
    LPSTR p = _searchKey(szKey);
    int len;

    if(p)
    {
        len = ::strlen(p);
        if(len >= bufsize) len = bufsize - 1;

        ::memcpy(szBuf, p, len);
        szBuf[len] = 0;
    }
    else
    {
        if(szDef)
            ::strcpy(szBuf, szDef);
        else
            szBuf[0] = 0;
    }
}

//! 番号をキー名として文字列取得
//! @param szDef NULL で空にする

void AXConfRead::getStrNo(int keyno,AXString *pstr,LPCSTR szDef)
{
    char m[16];

    AXIntToStr(m, keyno);

    getStr(m, pstr, szDef);
}

//! 番号をキー名とした文字列配列取得
/*!
    デフォルトは空文字列。

    @param keytop キー番号の先頭
    @param maxcnt 配列の最大数
*/

void AXConfRead::getStrArray(int keytop,AXString *pstrArray,int maxcnt)
{
    int i;

    for(i = 0; i < maxcnt; i++)
        getStrNo(keytop + i, pstrArray + i, NULL);
}

//! カンマで区切られた値を配列として取得
/*!
    デフォルトの初期値はあらかじめバッファにセットしておくこと。

    @param bufcnt  バッファの配列の最大個数
    @param bytecnt 値のバイト数（1,2,4）
    @param bHex    16進数値か
    @return 取得された数
*/

int AXConfRead::getArrayVal(LPCSTR szKey,LPVOID pBuf,int bufcnt,int bytecnt,BOOL bHex)
{
    AXByteString str;
    LPBYTE p = (LPBYTE)pBuf;
    LPSTR pc;
    int n,cnt = 0;

    getStr(szKey, &str);
    str += ',';
    str.replace(',', 0);

    //

    for(pc = str; *pc && bufcnt > 0; bufcnt--, pc += ::strlen(pc) + 1)
    {
        if(bHex)
            n = ::strtoul(pc, NULL, 16);
        else
            n = ::atoi(pc);

        //セット

        switch(bytecnt)
        {
            case 1:
                *(p++) = n;
                break;
            case 2:
                *((LPWORD)p) = n;
                p += 2;
                break;
            case 4:
                *((LPINT)p) = n;
                p += 4;
                break;
        }

        cnt++;
    }

    return cnt;
}

//! AXMemAuto にデータを追加していきながら、カンマで区切られた値を取得
/*!
    pmem はあらかじめ確保しておくこと。

    @return FALSE でキーが存在しなかった
*/

BOOL AXConfRead::getArrayVal(LPCSTR szKey,AXMemAuto *pmem,int bytecnt,BOOL bHex)
{
    AXByteString str;
    LPSTR pc;
    int n;

    getStr(szKey, &str, "-");
    if(str == "-") return FALSE;

    str += ',';
    str.replace(',', 0);

    //

    for(pc = str; *pc; pc += ::strlen(pc) + 1)
    {
        if(bHex)
            n = ::strtoul(pc, NULL, 16);
        else
            n = ::atoi(pc);

        //セット

        switch(bytecnt)
        {
            case 1:
                pmem->addBYTE(n);
                break;
            case 2:
                pmem->addWORD(n);
                break;
            case 4:
                pmem->addDWORD(n);
                break;
        }
    }

    return TRUE;
}

//! AXRectSize に、カンマで区切られた4つの値をセット

void AXConfRead::getRectSize(LPCSTR szKey,AXRectSize *prc,int nDefX,int nDefY,int nDefW,int nDefH)
{
    int n[4];

    n[0] = nDefX;
    n[1] = nDefY;
    n[2] = nDefW;
    n[3] = nDefH;

    getArrayVal(szKey, n, 4, 4);

    prc->x = n[0];
    prc->y = n[1];
    prc->w = n[2];
    prc->h = n[3];
}

//! AXRectSize に、カンマで区切られた4つの値をセット

void AXConfRead::getRectSize(LPCSTR szKey,AXRectSize *prc,int nDef)
{
    int n[4];

    n[0] = n[1] = n[2] = n[3] = nDef;

    getArrayVal(szKey, n, 4, 4);

    prc->x = n[0];
    prc->y = n[1];
    prc->w = n[2];
    prc->h = n[3];
}

//! AXPoint に取得（カンマで区切られた２つの値）

void AXConfRead::getPoint(LPCSTR szKey,AXPoint *ppt,int nDef)
{
    int n[2];

    n[0] = n[1] = nDef;

    getArrayVal(szKey, n, 2, 4);

    ppt->x = n[0];
    ppt->y = n[1];
}

//! AXSize に取得（カンマで区切られた２つの値）

void AXConfRead::getSize(LPCSTR szKey,AXSize *psize,int defw,int defh)
{
    int n[2];

    n[0] = defw, n[1] = defh;

    getArrayVal(szKey, n, 2, 4);

    psize->w = n[0];
    psize->h = n[1];
}

//! Base64のバイナリデータ取得
/*!
    key=size:Base64
*/

void AXConfRead::getBase64(LPCSTR szKey,AXMem *pmem)
{
    LPSTR p = _searchKey(szKey);
    int size;

    pmem->free();

    if(!p) return;

    size = ::atoi(p);

    if(!pmem->alloc(size)) return;

    //':' 以降が Base64

    for(; *p && *p != ':'; p++);
    if(*p == ':') p++;

    //デコード

    AXDecodeBase64(*pmem, p, size);
}

//! 次の行を取得
/*!
    グループが終了した場合は終わる。

    @return FALSE で終了
*/

BOOL AXConfRead::getNextLine(AXByteString *pstr)
{
    char c;

    if(m_nGroupIndex == -1 || m_nNowIndex == -1 || m_nNowIndex >= m_nIndexCnt)
        return FALSE;

    //次の位置

    for(m_nNowIndex++; m_nNowIndex < m_nIndexCnt; m_nNowIndex++)
    {
        c = *m_ppIndex[m_nNowIndex];

        if(c == '[') return FALSE;  //グループ終了
        if(c == 0) continue;        //無効行

        *pstr = m_ppIndex[m_nNowIndex];
        return TRUE;
    }

    return FALSE;
}

//! 次の行のキー数値と文字列取得

BOOL AXConfRead::getNextStr(int *pKey,AXByteString *pstr)
{
    AXByteString str;
    int pos;

    if(!getNextLine(&str))
        return FALSE;

    pos = str.find('=');

    *pKey = ::atoi(str);
    *pstr = str.at(pos + 1);

    return TRUE;
}


//=================================
// キー検索
//=================================


//! カレントグループからキー検索
/*!
    @return 文字列のポインタ（NULL で見つからなかった）
*/

LPSTR AXConfRead::_searchKey(LPCSTR szKey)
{
    AXByteString strKey;
    char **ppIndex;
    LPSTR pc,pcEnd;
    int i,srclen;
    char cSrcTop;

    if(m_nGroupIndex == -1) return NULL;

    //

    strKey = szKey;
    strKey.toLower();

    cSrcTop = strKey[0];
    srclen  = strKey.getLen();

    //

    ppIndex = m_ppIndex + m_nGroupIndex + 1;

    for(i = m_nIndexCnt - m_nGroupIndex - 1; i > 0; i--, ppIndex++)
    {
        //グループ終了

        if(**ppIndex == '[') break;

        //先頭文字比較

        if(**ppIndex != cSrcTop) continue;

        //'='の位置

        pc = *ppIndex;

        for(pcEnd = pc; *pcEnd && *pcEnd != '='; pcEnd++);

        //キー名の長さ比較

        if((pcEnd - pc) != srclen) continue;

        //文字列比較

        if(::strncmp(pc, strKey, srclen) == 0)
            return pcEnd + 1;
    }

    return NULL;
}


//=================================
// インデックス作成
//=================================


//! 行インデックス作成

BOOL AXConfRead::_createIndex()
{
    m_nIndexCnt = _getLineCnt();

    m_ppIndex = (char **)AXMalloc(m_nIndexCnt * sizeof(char *));
    if(!m_ppIndex) return FALSE;

    _setIndex();

    return TRUE;
}

//! 行数取得

int AXConfRead::_getLineCnt()
{
    LPSTR p = m_pBuf;
    int cnt = 0;

    while(*p)
    {
        //行頭

        cnt++;

        //改行までスキップ

        for(; *p && *p != '\r' && *p != '\n'; p++);

        if(*p)
        {
            if(*p == '\r' && p[1] == '\n') p++;
            p++;
        }
    }

    return cnt;
}

//! インデックスセット
/*!
    改行・コメント先頭をNULL文字にし、グループ・キー名を小文字に変換。
*/

void AXConfRead::_setIndex()
{
    char **ppIndex = m_ppIndex;
    LPSTR p,p2;
    char c;
    int i;

    //-------- インデックスにセット

    for(p = m_pBuf; *p; )
    {
        //行頭

        *(ppIndex++) = p;

        //改行までスキップ

        for(; *p && *p != '\r' && *p != '\n'; p++);

        //改行をNULL文字に

        if(*p)
        {
            if(*p == '\r' && p[1] == '\n')
            {
                p[0] = p[1] = 0;
                p += 2;
            }
            else
                *(p++) = 0;
        }
    }

    //-------- 内容補正

    ppIndex = m_ppIndex;

    for(i = m_nIndexCnt; i > 0; i--, ppIndex++)
    {
        p = *ppIndex;
        c = *p;

        if(c == '[')
        {
            //グループ（小文字に変換）

            for(p2 = p + 1; *p2 && *p2 != ']'; p2++)
            {
                if(*p2 >= 'A' && *p2 <= 'Z')
                    *p2 += 0x20;
            }

            if(*p2 == ']') *p2 = 0;
        }
        else if(isalnum(c) || c == '_')
        {
            //キー（小文字に変換、'='がないものは無効）

            for(p2 = p; *p2 && *p2 != '='; p2++)
            {
                if(*p2 >= 'A' && *p2 <= 'Z')
                    *p2 += 0x20;
            }

            if(*p2 != '=')
                *p = 0;
        }
        else if(c)
            //それ以外は無効
            *p = 0;
    }
}
