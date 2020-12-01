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

#include "AXDirFileList.h"

#include "AXMem.h"
#include "AXDir.h"
#include "AXFileStat.h"
#include "AXByteString.h"


//--------------------------

struct AXDIRFILELISTINFO
{
    const AXString  *pstrFilter;
    UINT        uFlags;
    AXMemAuto   *pmem;
    AXFILESTAT  *pst;
};

//--------------------------

/*!
    @class AXDirFileList
    @brief ディレクトリのファイルリストデータクラス

    @ingroup etc
*/

/*!
    @var AXDirFileList::FLAG_SUBDIR
    @brief サブディレクトリのファイルを含む
*/


AXDirFileList::AXDirFileList()
{
    m_pBuf = m_pNow = NULL;
    m_nFileCnt = 0;
}

AXDirFileList::~AXDirFileList()
{
    free();
}

//! 解放

void AXDirFileList::free()
{
    if(m_pBuf)
    {
        ::free(m_pBuf);
        m_pBuf = NULL;
    }

    m_pNow     = NULL;
    m_nFileCnt = 0;
}

//! リストセット
/*!
    @param strDir    検索対象ディレクトリ
    @param strFilter ファイルのフィルタ（'\\t' で複数を区切る）[例： "*.txt\\t*.png"]@nデフォルトで大/小文字を比較しない。
*/

BOOL AXDirFileList::setList(const AXString &strDir,const AXString &strFilter,UINT uFlags)
{
    AXMemAuto mem;
    AXDIRFILELISTINFO info;
    AXFILESTAT st;

    free();

    //

    if(!mem.alloc(8 * 1024, 8 * 1024)) return FALSE;

    //検索

    info.pmem       = &mem;
    info.pst        = &st;
    info.pstrFilter = &strFilter;
    info.uFlags     = uFlags;

    _find(strDir, &info);

    //終端

    mem.addBYTE(TYPE_END);

    //コピー

    m_pBuf = (LPBYTE)::malloc(mem.getNowSize());
    if(!m_pBuf) return FALSE;

    ::memcpy(m_pBuf, mem, mem.getNowSize());

    //

    m_pNow = m_pBuf;

    return TRUE;
}

//! 次を取得
/*!
    TYPE_FILE : ファイル名（ディレクトリ含まない）@n
    TYPE_DIR : ディレクトリパス@n
    TYPE_DIREND : ディレクトリ終了（戻るディレクトリのパス）@n
    TYPE_END : 終了

    @return データの種類（AXDirFileList::LISTTYPE）
*/

int AXDirFileList::getNext(AXString *pstr)
{
    int type,len;

    if(!m_pBuf) return TYPE_END;

    type = *(m_pNow++);

    //パス

    if(type != TYPE_END)
    {
        len = *((LPWORD)m_pNow);
        m_pNow += 2;

        pstr->setUTF8((LPSTR)m_pNow, len);

        m_pNow += len;
    }

    return type;
}


//===========================
//検索
//===========================


//! ファイル検索

void AXDirFileList::_find(const AXString &strDir,AXDIRFILELISTINFO *pinfo)
{
    AXDir dir;
    AXString str;

    if(!dir.open(strDir)) return;

    while(dir.read())
    {
        if(!dir.getFileStat(pinfo->pst)) continue;

        if((pinfo->pst)->isDirectory())
        {
            //-------- ディレクトリ

            //"." ".."

            if(dir.isSpecPath()) continue;

            //サブディレクトリを含まないなら飛ばす

            if(!(pinfo->uFlags & FLAG_SUBDIR)) continue;

            //追加

            str = strDir;
            str.path_add(dir.getCurName());

            _addList(TYPE_DIR, str, pinfo->pmem);

            //再帰

            _find(str, pinfo);

            //再帰から戻ったら元のディレクトリセット

            _addList(TYPE_DIREND, strDir, pinfo->pmem);
        }
        else
        {
            //------- ファイル

            //フィルタ判別

            if((dir.getCurName()).compareFilter(*(pinfo->pstrFilter), '\t', TRUE))
            {
                //追加

                _addList(TYPE_FILE, dir.getCurName(), pinfo->pmem);

                m_nFileCnt++;
            }
        }
    }

    dir.close();
}

//! リストに追加

void AXDirFileList::_addList(BYTE type,const AXString &strPath,AXMemAuto *pmem)
{
    AXByteString str;
    WORD len;

    //UNICODE -> UTF8

    strPath.toUTF8(&str);

    //追加

    len = str.getLen();

    pmem->addDat(&type, 1);
    pmem->addDat(&len, 2);
    pmem->addDat((LPSTR)str, len);
}
