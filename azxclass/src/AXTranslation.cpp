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
#include <unistd.h>

#include "AXTranslation.h"

#include "AXString.h"
#include "AXFile.h"
#include "AXUtil.h"


/*!
    @class AXTranslation
    @brief 翻訳バイナリデータクラス（AXAppで使われる）

    @ingroup core
*/


AXTranslation::AXTranslation()
{
    m_pBuf = m_pNowGroup = NULL;
    m_bLoadFile = FALSE;
}

AXTranslation::~AXTranslation()
{
    free();
}

//! 解放

void AXTranslation::free()
{
    if(m_pBuf && m_bLoadFile)
        ::free(m_pBuf);

    m_pBuf = m_pNowGroup = NULL;
}

//! 翻訳ファイル読み込み
/*!
    @param szLang 言語名。"ja_JP"など。NULLでシステムの言語。
    @param strDir 検索ディレクトリ
*/

BOOL AXTranslation::loadFile(LPCSTR szLang,const AXString &strDir)
{
    AXString str;
    AXFile file;
    BYTE ver;
    DWORD dwAll;

    free();

    //ファイル名

    _getFileName(&str, szLang, strDir);

    //読み込み

    if(!file.openRead(str)) return FALSE;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    if(!file.readCompare("AZXTRANS")) return FALSE;

    file.read(&ver, 1);
    if(ver != 0) return FALSE;

    file.read(&m_dwGroupSize, 4);
    file.read(&m_dwStringSize, 4);

    dwAll = m_dwGroupSize + m_dwStringSize;

    m_pBuf = (LPBYTE)::malloc(dwAll);
    if(!m_pBuf) return FALSE;

    if(!file.readSize(m_pBuf, dwAll)) return FALSE;

    file.close();

    //

    m_bLoadFile = TRUE;

    return TRUE;
}

//! バッファからセット（埋め込みのデフォルト言語用）

void AXTranslation::setBuf(LPBYTE pbuf)
{
    m_pBuf      = pbuf + 8;
    m_pNowGroup = NULL;
    m_bLoadFile = FALSE;

    //先頭8バイトは各サイズ

    AXGetDWORDLE(pbuf, &m_dwGroupSize);
    AXGetDWORDLE(pbuf + 4, &m_dwStringSize);
}

//! カレントグループをセット

BOOL AXTranslation::setGroup(WORD wGroupID)
{
    m_pNowGroup = _findGroup(wGroupID);

    return (m_pNowGroup != NULL);
}

//! カレントグループから文字列取得

LPCUSTR AXTranslation::getString(WORD wStrID) const
{
    return _findString(m_pNowGroup, wStrID);
}

//! 指定グループから文字列取得
/*!
    カレントグループはそのまま変更なし。
*/

LPCUSTR AXTranslation::getString(WORD wGroupID,WORD wStrID) const
{
    LPBYTE pGroup = _findGroup(wGroupID);

    return _findString(pGroup, wStrID);
}


//========================
//サブ処理
//========================


//! グループ検索

LPBYTE AXTranslation::_findGroup(WORD wID) const
{
    LPBYTE p = m_pBuf;
    int i;
    WORD id,cnt;
    DWORD offset;

    if(!p) return NULL;

    AXGetWORDLE(p, &cnt);
    p += 2;

    for(i = cnt; i > 0; i--)
    {
        AXGetWORDLE(p, &id);
        p += 2;

        AXGetDWORDLE(p, &offset);
        p += 4;

        if(wID == id)
            return m_pBuf + m_dwGroupSize + offset;
    }

    return NULL;
}

//! 文字列検索
/*!
    @return 文字列部分の位置が返る
*/

LPCUSTR AXTranslation::_findString(LPBYTE pGroup,WORD wID) const
{
    LPBYTE p = pGroup;
    WORD id,cnt;
    int i;
    UINT len;

    if(!p) return NULL;

    AXGetWORDLE(p, &cnt);
    p += 2;

    for(i = cnt; i > 0; i--)
    {
        //ID

        AXGetWORDLE(p, &id);
        p += 2;

        //長さ

        if(*p < 128)
            len = *(p++);
        else
            len = AXGetVariableLenVal(p, (void **)&p);

        //

        if(id == wID) return (LPCUSTR)p;

        p += len << 1;
    }

    return NULL;
}

//! 読み込むファイル名取得

void AXTranslation::_getFileName(AXString *pstr,LPCSTR szLang,const AXString &strDir)
{
    AXString str;

    //言語名

    if(szLang)
        str = szLang;
    else
    {
        LPSTR pc = ::getenv("LANG");

        if(!pc)
            str = "en_US";
        else
        {
            str = pc;
            str.findAndEnd('.');   //文字コード部分を除く
        }
    }

    str += ".axt";

    //ファイル名

    *pstr = strDir;
    pstr->path_add(str);
}
