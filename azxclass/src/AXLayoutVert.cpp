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

#include "AXLayout.h"

#include "AXRect.h"

/*!
    @class AXLayoutVert
    @brief 垂直レイアウトクラス

    @ingroup layout
*/

//---------------------


AXLayoutVert::~AXLayoutVert()
{

}

AXLayoutVert::AXLayoutVert()
    : AXLayout()
{

}

AXLayoutVert::AXLayoutVert(UINT uLayoutFlags,int sepw)
    : AXLayout(uLayoutFlags, sepw)
{

}

//! レイアウト実行

void AXLayoutVert::layout()
{
    AXLayoutItem *p,*pLastItem = NULL;
    AXRect rcMain;
    int remain,top;
    int cntExpand=0, oneExpand=0, lastExpand=0;
    int x,y,w,h,tmp,itemh,padW,padH,defW,defH;
    UINT flag;

    getClientRect(&rcMain);
    rcMain.move(m_nX, m_nY);
    rcMain.deflate(m_nSpaceW);

    top     = rcMain.top;
    remain  = rcMain.height();

    //----------- 基本情報取得

    for(p = m_pTop; p; p = p->m_pLNext)
    {
        if(!p->isVisible()) continue;

        if(p->m_uLFlags & (LF_EXPAND_Y | LF_EXPAND_H))
        {
            cntExpand++;
            pLastItem = p;  //拡張最後のアイテム記録
        }
        else
            //サイズ＆領域固定
            remain -= p->getHeightWithPadding();

        if(p->m_pLNext) remain -= m_nSepW;
    }

    //拡張アイテムの各高さと最後のアイテムの高さ

    if(cntExpand)
    {
        oneExpand  = remain / cntExpand;
        lastExpand = remain - oneExpand * (cntExpand - 1);
    }

    //--------------- 実際に計算

    for(p = m_pTop; p; p = p->m_pLNext)
    {
        if(!p->isVisible()) continue;

        //

        defW    = p->getLayoutW();
        defH    = p->getLayoutH();
        flag    = p->m_uLFlags;

        padW    = p->m_padLeft + p->m_padRight;
        padH    = p->m_padTop + p->m_padBottom;

        //ウィンドウの幅サイズ

        if(!(flag & LF_EXPAND_W))
            w = defW;
        else
        {
            //領域に合わせて伸縮
            w = rcMain.width() - padW;
            if(w < defW) w = defW;
        }

        //X位置

        x = rcMain.left + p->m_padLeft;

        if(flag & LF_RIGHT)
            x = rcMain.right + 1 - p->m_padRight - w;
        else if(flag & LF_CENTER_X)
            x += (rcMain.width() - padW - w) / 2;

        if(x < rcMain.left + p->m_padLeft)
            x = rcMain.left + p->m_padLeft;

        //ウィンドウ高さとアイテム全体の高さ
        //※最後の拡張アイテムの場合は残りのサイズ

        h = defH;
        itemh = (p == pLastItem)? lastExpand: oneExpand;    //余白も含めたサイズ

        if(!(flag & (LF_EXPAND_Y | LF_EXPAND_H)))
            itemh = defH + padH;
        else
        {
            //サイズまたは領域伸縮

            tmp = itemh - padH;
            if(tmp < defH) tmp = defH;

            if(flag & LF_EXPAND_H) h = tmp; //サイズ伸縮

            itemh = tmp + padH;
        }

        //Y位置

        y = top + p->m_padTop;

        if(flag & LF_CENTER_Y)
            y += (itemh - h - padH) / 2;
        else if(flag & LF_BOTTOM)
            y = top + itemh - h - p->m_padBottom;

        if(y < rcMain.top + p->m_padTop) y = rcMain.top + p->m_padTop;

        //位置・サイズ変更

        p->moveresize(x, y, w, h);

        //次のY位置

        top += itemh;
        if(p->m_pLNext) top += m_nSepW;
    }
}

//! 標準サイズ計算

void AXLayoutVert::calcDefSize()
{
    AXLayoutItem *p;
    int maxw = 0,maxh = 0,tmp;

    for(p = m_pTop; p; p = p->m_pLNext)
    {
        p->calcDefSize();

        //幅は最大値、高さは各標準値を足していく

        if(p->isVisible())
        {
            tmp = p->getWidthWithPadding();
            if(tmp > maxw) maxw = tmp;

            maxh += p->getHeightWithPadding();
            if(p->m_pLNext) maxh += m_nSepW;
        }
    }

    m_nDefW = maxw + m_nSpaceW * 2;
    m_nDefH = maxh + m_nSpaceW * 2;
}
