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

#include "CFilterConfigList.h"

#include "AXConfWrite.h"
#include "AXUtilStr.h"


/*!
    @class CFilterConfigList
    @brief フィルタ設定データリスト
*/


//! データを追加または入れ替え

void CFilterConfigList::setDat(UINT id,const AXByteString &str)
{
    CFilterConfigItem *p;

    p = findID(id);

    if(p)
        p->m_str = str;
    else
        AXList::add(new CFilterConfigItem(id, str));
}

//! IDからアイテム検索

CFilterConfigItem *CFilterConfigList::findID(UINT id)
{
    CFilterConfigItem *p;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_uID == id) return p;
    }

    return NULL;
}

//! ファイルに書き込み

void CFilterConfigList::writeConf(AXConfWrite *pcf)
{
    CFilterConfigItem *p;
    char m[16];

    for(p = getTopItem(); p; p = p->next())
    {
        AXIntToStr(m, p->m_uID);
        pcf->putStr(m, p->m_str);
    }
}


//********************************
//CFilterConfigItem
//********************************


CFilterConfigItem::CFilterConfigItem(UINT id,const AXByteString &str)
    : AXListItem()
{
    m_uID = id;
    m_str = str;
}

//! 識別子から値取得

BOOL CFilterConfigItem::getVal(char sig,int *pRet)
{
    LPSTR p;

    for(p = m_str; *p; )
    {
        if(*p != sig)
        {
            //次へ

            for(p++; *p && (*p < 'a' || *p > 'z'); p++);
        }
        else
        {
            //値取得

            *pRet = ::atoi(p + 1);

            return TRUE;
        }
    }

    return FALSE;
}
