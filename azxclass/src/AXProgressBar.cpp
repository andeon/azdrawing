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

#include "AXProgressBar.h"

#include "AXAppRes.h"

/*!
    @class AXProgressBar
    @brief プログレスバーウィジェット

    - バーのみ、バー＋パーセント表示、バー＋任意テキストの３タイプ。

    @ingroup widget
*/
/*!
    @var AXProgressBar::PBS_SIMPLE
    @brief バーのみ、テキストなし
    @var AXProgressBar::PBS_PERS
    @brief バー＋パーセントテキスト表示
    @var AXProgressBar::PBS_TEXT
    @brief バー＋任意テキスト表示
*/



AXProgressBar::~AXProgressBar()
{

}

AXProgressBar::AXProgressBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createProgressBar();
}

AXProgressBar::AXProgressBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createProgressBar();
}

//! 作成

void AXProgressBar::_createProgressBar()
{
    m_uType = TYPE_PROGRESSBAR;
    m_uPos = m_uMin = 0;
    m_uMax = m_uRange = 100;
}

//! 標準サイズ計算

void AXProgressBar::calcDefSize()
{
    m_nDefW = 6;
    m_nDefH = (m_uStyle & (PBS_PERS | PBS_TEXT))? m_pFont->getHeight() + 6: 16;
}


//===================


//! ステータスセット

void AXProgressBar::setStatus(UINT min,UINT max,UINT pos)
{
    if(min >= max) max = min + 1;
    if(pos < min) pos = min; else if(pos > max) pos = max;

    m_uPos      = pos;
    m_uMin      = min;
    m_uMax      = max;
    m_uRange    = max - min;

    redraw();
}

//! 位置セット

void AXProgressBar::setPos(UINT pos)
{
    if(pos < m_uMin) pos = m_uMin;
    else if(pos > m_uMax) pos = m_uMax;

    if(pos != m_uPos)
    {
        m_uPos = pos;
        redraw();
    }
}

//! 位置+1

void AXProgressBar::incPos()
{
    if(m_uPos != m_uMax)
    {
        m_uPos++;
        redraw();
    }
}

//! テキストセット（PBS_TEXT時）

void AXProgressBar::setText(const AXString &str)
{
    if(m_uStyle & PBS_TEXT)
    {
        m_strText = str;
        redraw();
    }
}


//============================
//ハンドラ
//============================


//! 描画

BOOL AXProgressBar::onPaint(AXHD_PAINT *phd)
{
    int bw,x,y;

    //外枠

    drawFrameSunken(0, 0, m_nW, m_nH);

    //バー＋余白

    bw = _DTOI((double)(m_nW - 4) / m_uRange * (m_uPos - m_uMin));

    if(bw == 0)
        drawFillBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::FACE);
    else
    {
        drawBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::FACE);

        drawFillBox(2, 2, bw, m_nH - 4, AXAppRes::BACKREVTEXT);
        drawFillBox(2 + bw, 2, m_nW - bw - 4, m_nH - 4, AXAppRes::FACE);
    }

    //テキスト

    if(m_uStyle & (PBS_TEXT | PBS_PERS))
    {
        //パーセント

        if(m_uStyle & PBS_PERS)
        {
            m_strText.setInt(_DTOI((double)(m_uPos - m_uMin) * 100.0 / m_uRange));
            m_strText += '%';
        }

        //----------

        AXDrawText dt(m_id);
        AXRectSize rcs;

        rcs.set(2, 2, m_nW - 4, m_nH - 4);

        x = 2 + (m_nW - 4 - m_pFont->getTextWidth(m_strText)) / 2;
        y = (m_nH - m_pFont->getHeight()) / 2;

        //反転部分

        if(bw)
        {
            rcs.w = bw;
            dt.setClipRect(rcs);
            dt.draw(*m_pFont, x, y, m_strText, AXAppRes::TC_REV);
        }

        //通常部分

        rcs.x = 2 + bw;
        rcs.w = m_nW - 4 - bw;

        dt.setClipRect(rcs);
        dt.draw(*m_pFont, x, y, m_strText);

        dt.end();
    }

    return TRUE;
}
