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
    @class AXLayoutHorz
    @brief 水平レイアウトクラス

    @ingroup layout
*/

//---------------------


AXLayoutHorz::~AXLayoutHorz()
{

}

AXLayoutHorz::AXLayoutHorz()
    : AXLayout()
{

}

AXLayoutHorz::AXLayoutHorz(UINT uLayoutFlags,int sepw)
    : AXLayout(uLayoutFlags, sepw)
{

}

//! レイアウト実行

void AXLayoutHorz::layout()
{
    AXLayoutItem *p,*pLastItem = NULL;
    AXRect rcMain;
    int remain,left;
    int cntExpand=0, oneExpand=0, lastExpand=0;
    int x,y,w,h,tmp,itemw,padW,padH,defW,defH;
    UINT flag;

    getClientRect(&rcMain);
    rcMain.move(m_nX, m_nY);
    rcMain.deflate(m_nSpaceW);

    left    = rcMain.left;
    remain  = rcMain.width();

    //----------- 基本情報取得

    for(p = m_pTop; p; p = p->m_pLNext)
    {
        if(!p->isVisible()) continue;

        if(p->m_uLFlags & (LF_EXPAND_X | LF_EXPAND_W))
        {
            cntExpand++;
            pLastItem = p;   //最後のアイテム記録
        }
        else
            //サイズ＆領域固定
            remain -= p->getWidthWithPadding();

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

        //ウィンドウの高さ

        if(!(flag & LF_EXPAND_H))
            h = defH;
        else
        {
            h = rcMain.height() - padH;
            if(h < defH) h = defH;
        }

        //Y位置

        y = rcMain.top + p->m_padTop;

        if(flag & LF_BOTTOM)
            y = rcMain.bottom + 1 - p->m_padBottom - h;
        else if(flag & LF_CENTER_Y)
            y += (rcMain.height() - padH - h) / 2;

        if(y < rcMain.top + p->m_padTop) y = rcMain.top + p->m_padTop;

        //ウィンドウ幅とアイテム全体の幅
        //※最後の拡張アイテムの場合は残りのサイズ

        w = defW;
        itemw = (p == pLastItem)? lastExpand: oneExpand;    //余白も含めたサイズ

        if(!(flag & (LF_EXPAND_X | LF_EXPAND_W)))
            itemw = defW + padW;
        else
        {
            tmp = itemw - padW;
            if(tmp < defW) tmp = defW;

            if(flag & LF_EXPAND_W) w = tmp;    //サイズ伸縮

            itemw = tmp + padW;
        }

        //X位置

        x = left + p->m_padLeft;

        if(flag & LF_CENTER_X)
            x += (itemw - w - padW) / 2;
        else if(flag & LF_RIGHT)
            x = left + itemw - w - p->m_padRight;

        if(x < rcMain.left + p->m_padLeft) x = rcMain.left + p->m_padLeft;

        //位置・サイズ変更

        p->moveresize(x, y, w, h);

        //次の位置

        left += itemw;
        if(p->m_pLNext) left += m_nSepW;
    }
}

//! 標準サイズ計算

void AXLayoutHorz::calcDefSize()
{
    AXLayoutItem *p;
    int maxw = 0,maxh = 0,tmp;

    for(p = m_pTop; p; p = p->m_pLNext)
    {
        p->calcDefSize();

        //幅は各標準値を足していく、高さは最大値

        if(p->isVisible())
        {
            maxw += p->getWidthWithPadding();
            if(p->m_pLNext) maxw += m_nSepW;

            tmp = p->getHeightWithPadding();
            if(tmp > maxh) maxh = tmp;
        }
    }

    m_nDefW = maxw + m_nSpaceW * 2;
    m_nDefH = maxh + m_nSpaceW * 2;
}
