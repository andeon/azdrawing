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

#include "CFilterBar.h"

#include "AXAppRes.h"


//--------------------

#define SPACEX  3

//--------------------


/*!
    @class CFilterBar
    @brief フィルタ用バー
*/


CFilterBar::CFilterBar(AXWindow *pParent,UINT uLayoutFlags,UINT uID,DWORD dwPadding,
                        int min,int max,int pos,int center)
    : AXWindow(pParent, 0, uLayoutFlags, uID, dwPadding)
{
    if(pos < min) pos = min; else if(pos > max) pos = max;

    m_nMin = min;
    m_nMax = max;
    m_nPos = pos;
    m_nCenter = center;
}

//! 位置セット

BOOL CFilterBar::setPos(int pos)
{
    if(pos < m_nMin)
        pos = m_nMin;
    else if(pos > m_nMax)
        pos = m_nMax;

    if(pos == m_nPos)
        return FALSE;
    else
    {
        _drawCursor(TRUE);

        m_nPos = pos;

        _drawCursor(FALSE);

        return TRUE;
    }
}


//======================


void CFilterBar::calcDefSize()
{
    m_nDefW = 16;
    m_nDefH = 10;
}

//! 値変更

void CFilterBar::_changePos(int x)
{
    int pos;

    pos = (int)((double)(x - SPACEX) * (m_nMax - m_nMin) / (m_nW - 1 - SPACEX * 2) + 0.5) + m_nMin;

    if(pos < m_nMin) pos = m_nMin;
    else if(pos > m_nMax) pos = m_nMax;

    if(pos != m_nPos)
    {
        _drawCursor(TRUE);

        m_nPos = pos;

        _drawCursor(FALSE);

        getNotify()->onNotify(this, NOTIFY_CHANGE, 0);
    }
}

//! カーソル描画

void CFilterBar::_drawCursor(BOOL bErase)
{
    AXDrawable::DRAWSEGMENT seg[4];
    int n;

    //カーソル

    n = SPACEX + (m_nPos - m_nMin) * (m_nW - 1 - SPACEX * 2) / (m_nMax - m_nMin);

    setDrawSeg(seg, n, 2, n, 2);
    setDrawSeg(seg + 1, n - 1, 3, n + 1, 3);
    setDrawSeg(seg + 2, n - 2, 4, n + 2, 4);
    setDrawSeg(seg + 3, n - 3, 5, n + 3, 5);

    drawLineSeg(seg, 4, (bErase)? AXAppRes::FACE: AXAppRes::BLACK);

    //中央線再描画

    if(bErase && m_nCenter > m_nMin)
    {
        n = SPACEX + (m_nCenter - m_nMin) * (m_nW - 1 - SPACEX * 2) / (m_nMax - m_nMin);

        drawLineV(n, 2, m_nH - 2, AXAppRes::FRAMEDARK);
    }
}


//======================
//ハンドラ
//======================


//! 描画

BOOL CFilterBar::onPaint(AXHD_PAINT *phd)
{
    AXDrawable::DRAWSEGMENT seg[4];
    int n;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    //線

    drawLineH(SPACEX, 0, m_nW - SPACEX * 2, AXAppRes::FRAMEDARK);
    drawLineH(SPACEX, 1, m_nW - SPACEX * 2, AXAppRes::FRAMELIGHT);

    //中央線

    if(m_nCenter > m_nMin)
    {
        n = SPACEX + (m_nCenter - m_nMin) * (m_nW - 1 - SPACEX * 2) / (m_nMax - m_nMin);

        drawLineV(n, 2, m_nH - 2, AXAppRes::FRAMEDARK);
    }

    //カーソル

    n = SPACEX + (m_nPos - m_nMin) * (m_nW - 1 - SPACEX * 2) / (m_nMax - m_nMin);

    setDrawSeg(seg, n, 2, n, 2);
    setDrawSeg(seg + 1, n - 1, 3, n + 1, 3);
    setDrawSeg(seg + 2, n - 2, 4, n + 2, 4);
    setDrawSeg(seg + 3, n - 3, 5, n + 3, 5);

    drawLineSeg(seg, 4, AXAppRes::BLACK);

    return TRUE;
}

//! ボタン押し時

BOOL CFilterBar::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_DRAG))
    {
        m_uFlags |= FLAG_DRAG;
        grabPointer();

        _changePos(phd->x);
    }

    return TRUE;
}

//! ボタン離し時

BOOL CFilterBar::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_DRAG))
    {
        m_uFlags &= ~FLAG_DRAG;
        ungrabPointer();

        getNotify()->onNotify(this, NOTIFY_UP, 0);
    }

    return TRUE;
}

//! マウス移動時

BOOL CFilterBar::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_DRAG)
        _changePos(phd->x);

    return TRUE;
}
