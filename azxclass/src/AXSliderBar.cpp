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

#include "AXSliderBar.h"

#include "AXAppRes.h"
#include "AXKey.h"


#define BARW_HF         4
#define BARW_SMALLHF    3


/*!
    @class AXSliderBar
    @brief スライダーバーウイジェット

    - 左/右/HOME/END キーで位置移動。

    @ingroup widget
*/

/*!
    @var AXSliderBar::SLBS_SMALL
    @brief 小さいバー
*/

/*!
    @enum AXSliderBar::SLIDERBARNOTIFY
    @brief onNotify 通知タイプ

    lParam = 位置。

    @var AXSliderBar::SLBN_POS_KEY
    @brief キー操作によって位置が変わった
    @var AXSliderBar::SLBN_BUTTON_DOWN
    @brief ボタンが押され、ドラッグが開始された
    @var AXSliderBar::SLBN_BUTTON_DRAG
    @brief ドラッグ中による位置変更
    @var AXSliderBar::SLBN_BUTTON_UP
    @brief ボタンが離された
*/


AXSliderBar::~AXSliderBar()
{

}

AXSliderBar::AXSliderBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createSliderBar();
}

AXSliderBar::AXSliderBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createSliderBar();
}

//! 作成

void AXSliderBar::_createSliderBar()
{
    m_uType  = TYPE_SLIDERBAR;
    m_uFlags |= FLAG_TAKE_FOCUS;

    m_nMin = m_nPos = 0;
    m_nMax = 100;
}

//! 標準サイズ計算

void AXSliderBar::calcDefSize()
{
    if(m_uStyle & SLBS_SMALL)
    {
        m_nDefW = (BARW_SMALLHF * 2 + 1) + 4;
        m_nDefH = 10;
    }
    else
    {
        m_nDefW = (BARW_HF * 2 + 1) + 4;
        m_nDefH = 15;
    }
}

//! ステータスセット

void AXSliderBar::setStatus(int min,int max,int pos)
{
    if(min >= max) max = min + 1;

    if(pos < min) pos = min; else if(pos > max) pos = max;

    m_nPos   = pos;
    m_nMin   = min;
    m_nMax   = max;

    redraw();
}

//! 位置セット

void AXSliderBar::setPos(int pos)
{
    if(pos < m_nMin) pos = m_nMin;
    else if(pos > m_nMax) pos = m_nMax;

    if(pos != m_nPos)
    {
        m_nPos = pos;
        redraw();
    }
}


//============================
//サブ処理
//============================


//! マウスカーソルの位置からバー位置セット
/*!
    @param bSend onScroll() は位置が変わっていなくても常に実行
*/

void AXSliderBar::_moveCurPos(int x,UINT uNotify,BOOL bSend)
{
    int pos,w,hf;

    hf = (m_uStyle & SLBS_SMALL)? BARW_SMALLHF: BARW_HF;

    w = m_nW - hf * 2 - 1;
    if(w <= 0) return;

    pos = _DTOI((double)(x - hf) / w * (m_nMax - m_nMin)) + m_nMin;

    if(pos < m_nMin) pos = m_nMin;
    else if(pos > m_nMax) pos = m_nMax;

    if(pos != m_nPos)
    {
        m_nPos = pos;
        redrawUpdate();

        onScroll(uNotify);
    }
    else if(bSend)
        onScroll(uNotify);
}


//==============================
//ハンドラ
//==============================


//! 位置変更時

BOOL AXSliderBar::onScroll(UINT uNotify)
{
    getNotify()->onNotify(this, uNotify, m_nPos);
    return TRUE;
}

//! ボタン押し時

BOOL AXSliderBar::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_DOWN_MOUSE))
    {
        setFocus();

        _moveCurPos(phd->x, SLBN_BUTTON_DOWN, TRUE);

        m_uFlags |= FLAG_DOWN_MOUSE;
        grabPointer();
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXSliderBar::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_DOWN_MOUSE))
    {
        m_uFlags &= ~FLAG_DOWN_MOUSE;
        ungrabPointer();

        onScroll(SLBN_BUTTON_UP);
    }

    return TRUE;
}

//! マウス移動時

BOOL AXSliderBar::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_DOWN_MOUSE)
        _moveCurPos(phd->x, SLBN_BUTTON_DRAG, FALSE);

    return TRUE;
}

//! キー押し時

BOOL AXSliderBar::onKeyDown(AXHD_KEY *phd)
{
    int pos = m_nPos;

    if(m_uFlags & FLAG_DOWN_MOUSE) return TRUE;

    //キー

    if(ISKEY_LEFT(phd->keysym))
    {
        //左
        if(m_nPos > m_nMin) pos = m_nPos - 1;
    }
    else if(ISKEY_RIGHT(phd->keysym))
    {
        //右
        if(m_nPos < m_nMax) pos = m_nPos + 1;
    }
    else if(ISKEY_HOME(phd->keysym))
        //HOME
        pos = m_nMin;
    else if(ISKEY_END(phd->keysym))
        //END
        pos = m_nMax;
    else
        return FALSE;

    //位置変更

    if(pos != m_nPos)
    {
        m_nPos = pos;
        redrawUpdate();
        onScroll(SLBN_POS_KEY);
    }

    return TRUE;
}

//! 描画

BOOL AXSliderBar::onPaint(AXHD_PAINT *phd)
{
    int x,hf;
    UINT flag;

    hf = (m_uStyle & SLBS_SMALL)? BARW_SMALLHF: BARW_HF;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    //スクロール枠

    drawFrameSunken(hf, m_nH / 2 - 1, m_nW - hf * 2, 3);

    //つまみ

    x = _DTOI((double)(m_nW - hf * 2 - 1) / (m_nMax - m_nMin) * (m_nPos - m_nMin));

    flag = 0;
    if(m_uFlags & FLAG_FOCUSED) flag |= DRAWBUTTON_FOCUSED;
    if(!(m_uFlags & FLAG_ENABLED)) flag |= DRAWBUTTON_DISABLE;

    drawButton(x, 0, hf * 2 + 1, m_nH, flag);

    return TRUE;
}
