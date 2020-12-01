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

//---------------------

#define LINFO_TOPNUM    3   //先頭データの数

#define LINFO_LASTNO    0   //拡張の最後のアイテムのインデックス
#define LINFO_ONEEX     1   //拡張の一つのサイズ
#define LINFO_LASTEX    2   //拡張の最後のサイズ
#define LINFO_SIZE      3   //各横/縦のサイズの先頭（上位ビットONで拡張）

#define SIZEF_EXPAND    0x80000000

//---------------------

/*!
    @class AXLayoutMatrix
    @brief Matrixレイアウトクラス

    横に順番に並び、指定数で下に折り返す。

    @ingroup layout
*/

//---------------------


AXLayoutMatrix::~AXLayoutMatrix()
{

}

AXLayoutMatrix::AXLayoutMatrix(int colcnt)
    : AXLayout()
{
    if(colcnt < 1) colcnt = 1;

    m_nSepW = colcnt;
}

AXLayoutMatrix::AXLayoutMatrix(int colcnt,UINT uLayoutFlags)
    : AXLayout()
{
    if(colcnt < 1) colcnt = 1;

    m_nSepW   = colcnt;
    m_uLFlags = uLayoutFlags;
}

//! レイアウト

void AXLayoutMatrix::layout()
{
    AXRect rcMain;
    LPINT pColInfo,pRowInfo;
    AXLayoutItem *p;
    int defW,defH,padW,padH,col,row,x,y,w,h,tmp,itemw,itemh,left,top;
    int nRowCnt,maxH;
    UINT flag;

    //格納元の範囲

    getClientRect(&rcMain);
    rcMain.move(m_nX, m_nY);
    rcMain.deflate(m_nSpaceW);

    //横情報取得

    pColInfo = new int[LINFO_TOPNUM + m_nSepW];
    nRowCnt = getColInfo(rcMain.width(), pColInfo);

    //縦情報取得

    pRowInfo = new int[LINFO_TOPNUM + nRowCnt];
    getRowInfo(rcMain.height(), pRowInfo);

    //===============

    top = rcMain.top;

    for(row = 0, p = m_pTop; p; row++)
    {
        left = rcMain.left;
        maxH = 0;

        for(col = 0; p && col < m_nSepW; col++, p = p->m_pLNext)
        {
            if(!p->isVisible()) continue;

            //

            defW    = p->getLayoutW();
            defH    = p->getLayoutH();
            flag    = p->m_uLFlags;

            padW    = p->m_padLeft + p->m_padRight;
            padH    = p->m_padTop + p->m_padBottom;

            //----------- 幅

            w   = defW;
            tmp = pColInfo[LINFO_SIZE + col];   //上位ビットONで領域拡張

            if(flag & LF_COL_W)
            {
                //縦列の最大幅

                itemw   = tmp & (~SIZEF_EXPAND);
                w       = itemw - padW;
            }
            else if(tmp & SIZEF_EXPAND)
            {
                //縦列に一つでも EXPAND_X or EXPAND_W がある場合

                tmp &= ~SIZEF_EXPAND;

                itemw = (pColInfo[LINFO_LASTNO] == col)? pColInfo[LINFO_LASTEX]: pColInfo[LINFO_ONEEX];
                if(itemw < tmp) itemw = tmp;

                if(flag & LF_EXPAND_W) w = itemw - padW;
            }
            else
                //サイズ固定の場合、アイテム幅は縦列の最大幅
                itemw = tmp;

            //----------- 高さ

            h   = defH;
            tmp = pRowInfo[LINFO_SIZE + row];

            if(flag & LF_ROW_H)
            {
                itemh   = tmp & (~SIZEF_EXPAND);
                h       = itemh - padH;
            }
            else if(tmp & SIZEF_EXPAND)
            {
                tmp &= ~SIZEF_EXPAND;

                itemh = (pRowInfo[LINFO_LASTNO] == row)? pRowInfo[LINFO_LASTEX]: pRowInfo[LINFO_ONEEX];
                if(itemh < tmp) itemh = tmp;

                if(flag & LF_EXPAND_H) h = itemh - padH;
            }
            else
                itemh = tmp;

            //----------- X位置

            x = left + p->m_padLeft;

            if(flag & LF_CENTER_X)
                x += (itemw - w - padW) / 2;
            else if(flag & LF_RIGHT)
                x += itemw - w - p->m_padRight;

            //------------ Y位置

            y = top + p->m_padTop;

            if(flag & LF_CENTER_Y)
                y += (itemh - h - padH) / 2;
            else if(flag & LF_BOTTOM)
                y += itemh - h - p->m_padBottom;

            //--------------

            //位置・サイズ変更

            p->moveresize(x, y, w, h);

            //

            left += itemw;

            if(itemh > maxH) maxH = itemh;
        }

        top += maxH;
    }

    //

    delete []pColInfo;
    delete []pRowInfo;
}

//! 標準サイズ計算

void AXLayoutMatrix::calcDefSize()
{
    AXLayoutItem *p,*p2;
    int w = 0,h = 0,max,i,tmp;

    //各アイテム標準サイズ計算

    for(p = m_pTop; p; p = p->m_pLNext)
        p->calcDefSize();

    //横

    for(p = m_pTop, i = 0; p && i < m_nSepW; p = p->m_pLNext, i++)
    {
        for(p2 = p, max = 0; p2; p2 = getRowNext(p2))
        {
            if(p2->isVisible())
            {
                tmp = p2->getWidthWithPadding();
                if(tmp > max) max = tmp;
            }
        }

        w += max;
    }

    //縦

    for(p = m_pTop; p;)
    {
        for(i = 0, max = 0; p && i < m_nSepW; i++, p = p->m_pLNext)
        {
            if(p->isVisible())
            {
                tmp = p->getHeightWithPadding();
                if(tmp > max) max = tmp;
            }
        }

        h += max;
    }

    //

    m_nDefW = w + m_nSpaceW * 2;
    m_nDefH = h + m_nSpaceW * 2;
}


//============================
//サブ処理
//============================


//! 次の縦位置取得

AXLayoutItem *AXLayoutMatrix::getRowNext(AXLayoutItem *p)
{
    int i;

    for(i = 0; p && i < m_nSepW; i++)
        p = p->m_pLNext;

    return p;
}

//! layout() 時の横列情報取得
/*!
    @return 縦列の数
*/

int AXLayoutMatrix::getColInfo(int width,LPINT pInfo)
{
    AXLayoutItem *pcol,*prow;
    int i,flag,max,tmp,excnt = 0,rowcnt = 0,lastcol = -1;

    for(pcol = m_pTop, i = 0; pcol && i < m_nSepW; pcol = pcol->m_pLNext, i++)
    {
        //縦方向

        flag = 0;
        max  = 0;

        for(prow = pcol; prow; prow = getRowNext(prow))
        {
            if(prow->isVisible())
            {
                flag |= 1;

                if(prow->m_uLFlags & (LF_EXPAND_X | LF_EXPAND_W))
                    flag |= 2;

                tmp = prow->getWidthWithPadding();
                if(tmp > max) max = tmp;
            }

            //縦列数カウント

            if(i == 0) rowcnt++;
        }

        //縦1列の最大標準幅＋拡張フラグ

        pInfo[LINFO_SIZE + i] = max | ((flag & 2)? SIZEF_EXPAND: 0);

        //flag == 0 で縦1列が非表示

        if(flag)
        {
            if(flag & 2)
            {
                excnt++;
                lastcol = i;
            }
            else
                width -= max;
        }
    }

    //

    pInfo[LINFO_LASTNO] = lastcol;

    if(excnt)
    {
        pInfo[LINFO_ONEEX]  = width / excnt;
        pInfo[LINFO_LASTEX] = width - pInfo[LINFO_ONEEX] * (excnt - 1);
    }

    return rowcnt;
}

//! layout() 時の縦列情報取得

void AXLayoutMatrix::getRowInfo(int height,LPINT pInfo)
{
    AXLayoutItem *p;
    int i,j,flag,max,tmp,excnt = 0,lastrow = -1;

    for(p = m_pTop, i = 0; p; i++)
    {
        //横方向

        flag = 0;
        max  = 0;

        for(j = 0; j < m_nSepW && p; j++, p = p->m_pLNext)
        {
            if(p->isVisible())
            {
                flag |= 1;

                if(p->m_uLFlags & (LF_EXPAND_Y | LF_EXPAND_H))
                    flag |= 2;

                tmp = p->getHeightWithPadding();
                if(tmp > max) max = tmp;
            }
        }

        //横1列の最大標準高さ

        pInfo[LINFO_SIZE + i] = max | ((flag & 2)? SIZEF_EXPAND: 0);

        //flag == 0 で横1列が非表示

        if(flag)
        {
            if(flag & 2)
            {
                excnt++;
                lastrow = i;
            }
            else
                height -= max;
        }
    }

    //

    pInfo[LINFO_LASTNO] = lastrow;

    if(excnt)
    {
        pInfo[LINFO_ONEEX]  = height / excnt;
        pInfo[LINFO_LASTEX] = height - pInfo[LINFO_ONEEX] * (excnt - 1);
    }
}

