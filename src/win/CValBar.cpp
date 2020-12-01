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

#include "CValBar.h"
#include "CValBar2.h"

#include "AXFont.h"
#include "AXAppRes.h"
#include "AXUtilStr.h"



//***************************
// CValBar
//***************************


/*!
    @class CValBar
    @brief バーウィジェット（バーのみ）
*/


CValBar::CValBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uID, dwPadding)
{
    m_nMin = 0;
    m_nMax = 100;
    m_nPos = 0;
}


CValBar::CValBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uID,DWORD dwPadding,
                           int min,int max,int pos)
    : AXWindow(pParent, uStyle, uLayoutFlags, uID, dwPadding)
{
    m_nMin = min;
    m_nMax = max;
    m_nPos = pos;
}

//! 位置セット

void CValBar::setPos(int pos)
{
    if(pos < m_nMin)
        pos = m_nMin;
    else if(pos > m_nMax)
        pos = m_nMax;

    if(pos != m_nPos)
    {
        m_nPos = pos;
        redraw();
    }
}

//! 範囲セット

void CValBar::setRange(int min,int max)
{
    m_nMin = min;
    m_nMax = max;

    redraw();
}


//======================


void CValBar::calcDefSize()
{
    m_nDefW = 5;
    m_nDefH = 4;
}

//! 値変更

void CValBar::_changePos(int x)
{
    int pos;

    pos = (int)((double)x * (m_nMax - m_nMin) / (m_nW - 2) + 0.5) + m_nMin;

    if(pos < m_nMin) pos = m_nMin;
    else if(pos > m_nMax) pos = m_nMax;

    if(pos != m_nPos)
    {
        m_nPos = pos;

        redraw();

        getNotify()->onNotify(this, 0, 0);
    }
}

//! 描画

BOOL CValBar::onPaint(AXHD_PAINT *phd)
{
    int w;

    //枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::BLACK);

    //バー

    w = (int)((double)(m_nPos - m_nMin) * (m_nW - 2) / (m_nMax - m_nMin) + 0.5);

    if(w)
        drawFillBox(1, 1, w, m_nH - 2, AXAppRes::FACEFOCUS);

    //バー残り

    if(m_nW - 2 - w > 0)
        drawFillBox(1 + w, 1, m_nW - 2 - w, m_nH - 2, AXAppRes::WHITE);

    return TRUE;
}

//! ボタン押し時

BOOL CValBar::onButtonDown(AXHD_MOUSE *phd)
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

BOOL CValBar::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_DRAG))
    {
        m_uFlags &= ~FLAG_DRAG;
        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動時

BOOL CValBar::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_DRAG)
        _changePos(phd->x);

    return TRUE;
}


//***************************
// CValBar2
//***************************


/*!
    @class CValBar2
    @brief バーウィジェット（バー＋数値表示＋スピン）

    Shift+左クリックで+10倍，Ctrl+左クリックで-10倍
*/


CValBar2::CValBar2(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uID,DWORD dwPadding,
                        int dig,int min,int max,int pos)
    : AXWindow(pParent, uStyle, uLayoutFlags, uID, dwPadding)
{
    if(pos < min) pos = min; else if(pos > max) pos = max;

    m_nDig = dig;
    m_nMin = min;
    m_nMax = max;
    m_nPos = pos;
}

//! 位置セット

BOOL CValBar2::setPos(int pos)
{
    if(pos < m_nMin)
        pos = m_nMin;
    else if(pos > m_nMax)
        pos = m_nMax;

    if(pos == m_nPos)
        return FALSE;
    else
    {
        m_nPos = pos;
        redraw();

        return TRUE;
    }
}

//! 範囲セット

void CValBar2::setRange(int min,int max)
{
    m_nMin = min;
    m_nMax = max;

    redraw();
}


//======================


void CValBar2::calcDefSize()
{
    m_nDefW = 10;
    m_nDefH = m_pFont->getHeight() + 2;
}

//! 値変更

void CValBar2::_changePos(int x)
{
    int pos;

    pos = (int)((double)x * (m_nMax - m_nMin) / (m_nW - 2 - 8) + 0.5) + m_nMin;

    if(pos < m_nMin) pos = m_nMin;
    else if(pos > m_nMax) pos = m_nMax;

    if(pos != m_nPos)
    {
        m_nPos = pos;

        redrawUpdate();

        getNotify()->onNotify(this, VALBAR2N_CHANGE, 0);
    }
}

//! 描画

BOOL CValBar2::onPaint(AXHD_PAINT *phd)
{
    int w;

    //枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::BLACK);

    //------ スピン

    //背景

    drawFillBox(m_nW - 8, 1, 7, m_nH - 2, AXAppRes::FACE);

    //左の縦線

    drawLineV(m_nW - 9, 1, m_nH - 2, AXAppRes::BLACK);

    //横線

    drawLineH(m_nW - 8, m_nH / 2, 7, AXAppRes::BLACK);

    //矢印

    drawArrowUpSmall(m_nW - 5, 2, AXAppRes::BLACK);
    drawArrowDownSmall(m_nW - 5, m_nH - 3, AXAppRes::BLACK);

    //------- バー

    w = (int)((double)(m_nPos - m_nMin) * (m_nW - 2 - 8) / (m_nMax - m_nMin) + 0.5);

    if(w)
        drawFillBox(1, 1, w, m_nH - 2, (isEnabled())? AXAppRes::FACEFOCUS: AXAppRes::FACEDARK);

    //バー残り

    if(m_nW - 2 - 8 - w > 0)
        drawFillBox(1 + w, 1, m_nW - 2 - 8 - w, m_nH - 2, (isEnabled())? AXAppRes::WHITE: AXAppRes::FACELIGHT);

    //------ 数値

    char m[32];
    int x,len;

    len = AXIntToFloatStr(m, m_nPos, m_nDig);

    AXDrawText dt(m_id);

    x = m_nW - 4 - 8 - m_pFont->getTextWidth(m, len);

    dt.draw(*m_pFont, x, 1, m, len, AXAppRes::TC_NORMAL);

    dt.end();

    return TRUE;
}

//! ボタン押し時

BOOL CValBar2::onButtonDown(AXHD_MOUSE *phd)
{
    int n;

    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_DRAG))
    {
        if(phd->x >= m_nW - 8 || (phd->state & (STATE_CTRL | STATE_SHIFT)))
        {
            //値の増減（スピン または バー部分を+Ctrl/+Shift)

            if(phd->state & STATE_SHIFT)
                n = 10;
            else if(phd->state & STATE_CTRL)
                n = -10;
            else
            {
                if(phd->y >= m_nH / 2)
                    n = -1;
                else
                    n = 1;
            }

            if(setPos(m_nPos + n))
                getNotify()->onNotify(this, VALBAR2N_CHANGE, 0);
        }
        else
        {
            //バー部分

            m_uFlags |= FLAG_DRAG;
            grabPointer();

            _changePos(phd->x);
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL CValBar2::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_DRAG))
    {
        m_uFlags &= ~FLAG_DRAG;
        ungrabPointer();

        getNotify()->onNotify(this, VALBAR2N_UP, 0);
    }

    return TRUE;
}

//! マウス移動時

BOOL CValBar2::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_DRAG)
        _changePos(phd->x);

    return TRUE;
}
