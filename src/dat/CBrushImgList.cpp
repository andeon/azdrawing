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

#include "CBrushImgList.h"

#include "CImage32.h"
#include "CConfig.h"
#include "defGlobal.h"

#include "AXApp.h"


/*!
    @class CBrushImgList
    @brief ブラシ・テクスチャ画像管理のリストクラス

    同じファイルの画像が複数指定されていても一つのデータで済むように、リストにしておく。@n
    データがブラシで使われるごとに参照カウンタを加算していき、解放時はカウンタを減算していく。@n
    カウンタが 0 になった場合、どのブラシでも使われていないということで実際に解放する。@n
    @n
    画像は ユーザーディレクトリ -> システムディレクトリ の順で検索。
*/


//! 画像読み込み
/*!
    すでにリスト上にあるものはそのままポインタを返す。@n
    リスト上にない場合は読み込んで作成する。

    @return NULL でなし
*/

CImage8 *CBrushImgList::loadImage(int type,const AXString &strPath)
{
    CBrushImgItem *p;
    CImage32 img32;
    CImage32::LOADINFO info;
    AXString str;
    int ret;

    //なし
    if(strPath.isEmpty()) return NULL;
    //オプション指定を使う
    if(strPath == "?") return NULL;

    //同じ画像があるか

    p = _searchImage(type, strPath);

    if(p)
    {
        //参照カウンタを +1

        p->m_nRefCnt++;
    }
    else
    {
        //------ 新規追加

        //32bit読み込み

        if(type == CBrushImgItem::TYPE_BRUSH)
            str = g_pconf->strBrushDir;
        else
            str = g_pconf->strTextureDir;

        str.path_add(strPath);

        if(!img32.loadImage(str, &info))
        {
            //システムディレクトリ

            if(type == CBrushImgItem::TYPE_BRUSH)
                axapp->getResourcePath(&str, "brush");
            else
                axapp->getResourcePath(&str, "texture");

            str.path_add(strPath);

            if(!img32.loadImage(str, &info))
                return NULL;
        }

        //データ

        AXList::add(p = new CBrushImgItem);

        p->m_nType      = type;
        p->m_strPath    = strPath;
        p->m_nRefCnt    = 1;

        //8bitイメージへ

        if(type == CBrushImgItem::TYPE_BRUSH)
            ret = p->m_img.createFrom32bit_brush(img32, info.bAlpha);
        else
            ret = p->m_img.createFrom32bit(img32, info.bAlpha);

        if(!ret)
        {
            AXList::deleteItem(p);
            return NULL;
        }
    }

    return &p->m_img;
}

//! イメージ解放

void CBrushImgList::freeImage(CImage8 *pimg)
{
    CBrushImgItem *p;

    if(!pimg) return;

    //検索

    for(p = getTopItem(); p; p = p->next())
    {
        if(pimg == &p->m_img) break;
    }

    //

    if(p)
    {
        //参照カウンタ -1

        p->m_nRefCnt--;

        //参照カウンタが 0 なら削除

        if(p->m_nRefCnt <= 0)
            AXList::deleteItem(p);
    }
}

//! パス名から画像検索

CBrushImgItem *CBrushImgList::_searchImage(int type,const AXString &strPath)
{
    CBrushImgItem *p;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_nType == type && p->m_strPath == strPath)
            return p;
    }

    return NULL;
}
