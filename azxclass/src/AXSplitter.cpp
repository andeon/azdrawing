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

#include "AXSplitter.h"

#include "AXCursor.h"
#include "AXAppRes.h"
#include "AXApp.h"


//------------------------

#define BARSIZE 5


/*!
    @class AXSplitter
    @brief スプリッター（分割バー）

    - 上下または左右にレイアウトされているアイテムが対象。
    - 親のサイズが変化した時に拡張するアイテムは LF_EXPAND_* フラグを付け、それ以外は LF_FIX_* で固定サイズにする。

    @ingroup widget
*/
/*!
    @var AXSplitter::SPLS_VERT
    @brief 垂直に分割する（デフォルト）
    @var AXSplitter::SPLS_HORZ
    @brief 水平に分割する
*/


AXSplitter::~AXSplitter()
{

}

AXSplitter::AXSplitter(AXWindow *pParent,UINT uStyle)
    : AXWindow(pParent, uStyle, (uStyle & SPLS_HORZ)? LF_EXPAND_H: LF_EXPAND_W)
{
    _createSplitter();
}

//! 作成

void AXSplitter::_createSplitter()
{
    m_uType = TYPE_SPLITTER;

    if(m_uStyle & SPLS_HORZ)
        setCursor(axapp->getCursor(AXApp::CURSOR_HSPLIT));
    else
        setCursor(axapp->getCursor(AXApp::CURSOR_VSPLIT));
}

//! 標準サイズ計算

void AXSplitter::calcDefSize()
{
    if(m_uStyle & SPLS_HORZ)
    {
        m_nDefW = BARSIZE;
        m_nDefH = 7;
    }
    else
    {
        m_nDefW = 7;
        m_nDefH = BARSIZE;
    }
}


//==============================
//ハンドラ
//==============================


//! ボタン押し時

BOOL AXSplitter::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_DOWN) &&
       m_pLPrev && m_pLNext)
    {
        m_uFlags |= FLAG_DOWN;

        m_nStartPos = (m_uStyle & SPLS_HORZ)? phd->rootx: phd->rooty;

        m_pLPrev->getRect(&m_rcPrev);
        m_pLNext->getRect(&m_rcNext);

        grabPointer();
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXSplitter::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_DOWN))
    {
        m_uFlags &= ~FLAG_DOWN;

        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動時

BOOL AXSplitter::onMouseMove(AXHD_MOUSE *phd)
{
    AXRect rc1,rc2;
    int change,min;

    if(m_uFlags & FLAG_DOWN)
    {
        rc1 = m_rcPrev;
        rc2 = m_rcNext;

        if(m_uStyle & SPLS_HORZ)
        {
            //--------- 水平

            change = phd->rootx - m_nStartPos;

            rc1.right += change;
            rc2.left  += change;

            //上

            min = m_pLPrev->getLayoutMinW();
            if(min < 1) min = 1;

            if(rc1.right - rc1.left + 1 < min)
            {
                rc1.right = rc1.left + min - 1;
                rc2.left  = rc1.right + 1 + BARSIZE;
            }

            //下

            min = m_pLNext->getLayoutMinW();
            if(min < 1) min = 1;

            if(rc2.right - rc2.left + 1 < min)
            {
                rc2.left  = rc2.right + 1 - min;
                rc1.right = rc2.left - BARSIZE - 1;
            }

            //移動

            m_pLPrev->moveresize(rc1);
            m_pLNext->moveresize(rc2);
            move(rc1.right + 1, 0);
        }
        else
        {
            //--------- 垂直

            change = phd->rooty - m_nStartPos;

            rc1.bottom += change;
            rc2.top    += change;

            //上

            min = m_pLPrev->getLayoutMinH();
            if(min < 1) min = 1;

            if(rc1.bottom - rc1.top + 1 < min)
            {
                rc1.bottom = rc1.top + min - 1;
                rc2.top    = rc1.bottom + 1 + BARSIZE;
            }

            //下

            min = m_pLNext->getLayoutMinH();
            if(min < 1) min = 1;

            if(rc2.bottom - rc2.top + 1 < min)
            {
                rc2.top    = rc2.bottom + 1 - min;
                rc1.bottom = rc2.top - BARSIZE - 1;
            }

            //移動

            m_pLPrev->moveresize(rc1);
            m_pLNext->moveresize(rc2);
            move(0, rc1.bottom + 1);
        }
    }

    return TRUE;
}

//! 描画

BOOL AXSplitter::onPaint(AXHD_PAINT *phd)
{
    AXDrawable::DRAWSEGMENT seg[3];
    int n;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    //つまみ部分

    if(m_uStyle & SPLS_HORZ)
    {
        //--------- 水平

        n = (m_nH - 7) / 2;

        setDrawSeg(seg, 0, n, m_nW - 1, n);
        setDrawSeg(seg + 1, 0, n + 3, m_nW - 1, n + 3);
        setDrawSeg(seg + 2, 0, n + 6, m_nW - 1, n + 6);
    }
    else
    {
        //--------- 垂直

        n = (m_nW - 7) / 2;

        setDrawSeg(seg, n, 0, n, m_nH - 1);
        setDrawSeg(seg + 1, n + 3, 0, n + 3, m_nH - 1);
        setDrawSeg(seg + 2, n + 6, 0, n + 6, m_nH - 1);
    }

    drawLineSeg(seg, 3, AXAppRes::FRAMEDARK);

    return TRUE;
}
