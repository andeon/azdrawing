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
/*
    各サブウィジェット

    CImgListLabel、CImgListButton、CScaleRotBar
    CArrowMenuBtt、CSelImgBar、CImgPrev、CPressCurveView
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "toolwinWidget.h"
#include "CArrowMenuBtt.h"
#include "CSelImgBar.h"
#include "CImgPrev.h"
#include "CPressCurveView.h"

#include "AXFont.h"
#include "AXImageList.h"
#include "AXAppRes.h"



//****************************
// CImgListLabel
//****************************

/*!
    @class CImgListLabel
    @brief （ツールウィンドウ）イメージリストのイメージを表示する
*/


CImgListLabel::CImgListLabel(AXWindow *pParent,UINT uLayoutFlags,DWORD dwPadding,AXImageList *pimgList,int nImgNo)
    : AXWindow(pParent, 0, uLayoutFlags, 0, dwPadding)
{
    m_pimgList  = pimgList;
    m_nImgNo    = nImgNo;
}

void CImgListLabel::calcDefSize()
{
    m_nDefW = m_pimgList->getOneW();
    m_nDefH = m_pimgList->getHeight();
}

BOOL CImgListLabel::onPaint(AXHD_PAINT *phd)
{
    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    m_pimgList->put(m_id, 0, 0, m_nImgNo, FALSE);

    return TRUE;
}


//****************************
// CImgListButton
//****************************

/*!
    @class CImgListButton
    @brief （ツールウィンドウ）イメージリストのイメージを使ったボタン
*/


CImgListButton::CImgListButton(AXWindow *pParent,UINT uLayoutFlags,UINT uID,DWORD dwPadding,
                               AXImageList *pimgList,int nImgNo)
    : AXButton(pParent, 0, uLayoutFlags, uID, dwPadding)
{
    m_pimgList  = pimgList;
    m_nImgNo    = nImgNo;

    /* キーでのボタン押しを無効にする */
    m_uFlags &= ~FLAG_TAKE_FOCUS;
}

void CImgListButton::calcDefSize()
{
    m_nDefW = m_pimgList->getOneW() + 6;
    m_nDefH = m_pimgList->getHeight() + 6;
}

BOOL CImgListButton::onPaint(AXHD_PAINT *phd)
{
    int add = 0;

    //枠

    if(AXButton::isPress())
    {
        drawFrameSunken(0, 0, m_nW, m_nH);
        add = 1;
    }
    else
        drawFrameRaise(0, 0, m_nW, m_nH);

    //背景

    drawFillBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::FACE);

    //イメージ

    m_pimgList->put(m_id, 3 + add, 3 + add, m_nImgNo, FALSE);

    return TRUE;
}


//****************************
// CScaleRotBar
//****************************

/*!
    @class CScaleRotBar
    @brief ツールウィンドウの表示倍率・回転バー
*/


CScaleRotBar::CScaleRotBar(AXWindow *pParent,UINT uStyle,UINT uID,DWORD dwPadding,int min,int max)
    : AXWindow(pParent, uStyle, LF_EXPAND_W, uID, dwPadding)
{
    m_nMin = min;
    m_nMax = max;
    m_nPos = min;
}

//! 位置セット

void CScaleRotBar::setPos(int pos)
{
    if(pos != m_nPos)
    {
        m_nPos = pos;
        redraw();
    }
}

void CScaleRotBar::calcDefSize()
{
    m_nDefH = m_pFont->getHeight() + 2;
}

//! 描画

BOOL CScaleRotBar::onPaint(AXHD_PAINT *phd)
{
    int cx,n,len;
    BOOL bRotate;
    char m[32];

    bRotate = m_uStyle & SRBS_ROTATE;
    cx      = m_nW / 2;

    //外枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::BLACK);

    //背景

    drawFillBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::WHITE);

    //中央線

    drawLineV(cx, 1, m_nH - 2, AXAppRes::FRAMEDARK);

    //------- カーソル

    if(bRotate)
    {
        //回転

        if(m_nPos >= 0)
            n = (int)(cx + m_nPos * (m_nW - cx - 1) / 18000.0 + 0.5);
        else
            n = (int)(cx + m_nPos * cx / 18000.0 + 0.5);
    }
    else
    {
        //拡大

        if(m_nPos >= 100)
            n = (int)(cx + (double)(m_nPos - 100) * (m_nW - cx - 1) / (m_nMax - 100) + 0.5);
        else
            n = (int)((double)(m_nPos - m_nMin) * cx / (100 - m_nMin) + 0.5);
    }

    drawLineV(n, 0, m_nH, AXAppRes::FRAMEDEFBTT);

    //---------- 数値

    if(bRotate)
        ::sprintf(m, "%d.%02d", m_nPos / 100, ::abs(m_nPos % 100));
    else
        ::sprintf(m, "%d%%", m_nPos);

    len = ::strlen(m);

    if((bRotate && m_nPos >= 0) || (!bRotate && m_nPos >= 100))
        n = 3;
    else
        //右寄せ
        n = m_nW - 3 - m_pFont->getTextWidth(m, len);

    AXDrawText dt(m_id);
    dt.draw(*m_pFont, n, 1, m, len, AXAppRes::TC_DISABLE);
    dt.end();

    return TRUE;
}

//! ボタン押し時

BOOL CScaleRotBar::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_DRAG))
    {
        m_uFlags |= FLAG_DRAG;
        grabPointer();

        _changePos(phd->x);

        getNotify()->onNotify(this, SRBN_DOWN, 0);
    }

    return TRUE;
}

//! ボタン離し時

BOOL CScaleRotBar::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_DRAG))
    {
        m_uFlags &= ~FLAG_DRAG;
        ungrabPointer();

        delTimerAll();

        getNotify()->onNotify(this, SRBN_UP, 0);
    }

    return TRUE;
}

//! マウス移動時

BOOL CScaleRotBar::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_DRAG)
    {
        if(_changePos(phd->x))
            addTimer(0, 5);
    }

    return TRUE;
}

//! 位置変更

BOOL CScaleRotBar::_changePos(int x)
{
    int pos,ctx;

    ctx = m_nW / 2;

    if(x < 0) x = 0; else if(x >= m_nW) x = m_nW - 1;

    //位置

    if(m_uStyle & SRBS_ROTATE)
    {
        if(x >= ctx)
            pos = (int)((x - ctx) * 18000.0 / (m_nW - ctx - 1) + 0.5);
        else
            pos = (int)((x - ctx) * 18000.0 / ctx - 0.5);
    }
    else
    {
        if(x >= ctx)
            pos = (int)((double)(x - ctx) * (m_nMax - 100) / (m_nW - ctx - 1) + 0.5) + 100;
        else
            pos = (int)((double)x * (100 - m_nMin) / ctx + 0.5) + m_nMin;
    }

    //セット

    if(pos != m_nPos)
    {
        m_nPos = pos;
        redrawUpdate();
        return TRUE;
    }

    return FALSE;
}

//! タイマー

BOOL CScaleRotBar::onTimer(UINT uTimerID,ULONG lParam)
{
    delTimer(uTimerID);

    getNotify()->onNotify(this, SRBN_MOVE, 0);

    return TRUE;
}


//****************************
// CArrowMenuBtt
//****************************

/*!
    @class CArrowMenuBtt
    @brief 矢印のメニューボタン（プレビューウィンドウやイメージビューウィンドウに付く）

    ボタンが押された時、親の onNotify が実行される。
*/


CArrowMenuBtt::CArrowMenuBtt(AXWindow *pParent)
    : AXWindow(pParent, 0, 0)
{
    resize(17, 17);
}

//! 親のサイズにあわせて位置セット

void CArrowMenuBtt::moveParent()
{
    move(m_pParent->getWidth() - m_nW, 0);
}

//! ボタン押し時

BOOL CArrowMenuBtt::onButtonDown(AXHD_MOUSE *phd)
{
    m_pParent->sendNotify(this, 0, 0);

    return TRUE;
}

//! 描画

BOOL CArrowMenuBtt::onPaint(AXHD_PAINT *phd)
{
    //枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::WHITE);
    drawBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::BLACK);

    //背景

    drawFillBox(2, 2, m_nW - 4, m_nH - 4, AXAppRes::FACEFOCUS);

    //矢印

    drawArrowDown(m_nW / 2, m_nH / 2, AXAppRes::BLACK);

    return TRUE;
}


//****************************
// CImgPrev
//****************************

/*!
    @class CImgPrev
    @brief AXImage を表示するだけのウィジェット（枠付き）
*/


CImgPrev::CImgPrev(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,DWORD dwPadding,int w,int h)
    : AXWindow(pParent, uStyle, uLayoutFlags, 0, dwPadding)
{
    m_nMinH = h + 2;

    if(!(m_uStyle & STYLE_RESIZE))
    {
        m_nMinW = w + 2;

        m_img.create(w, h);
    }
}

void CImgPrev::calcDefSize()
{
    m_nDefW = 10;
    m_nDefH = 5;
}

BOOL CImgPrev::onPaint(AXHD_PAINT *phd)
{
    drawFrameSunken(0, 0, m_nW, m_nH);

    m_img.put(m_id, 1, 1, 0, 0, m_nW - 2, m_nH - 2);

    return TRUE;
}

BOOL CImgPrev::onSize()
{
    if(m_uStyle & STYLE_RESIZE)
    {
        m_img.recreate(m_nW - 2, m_nH - 2, 16, 0);

        getNotify()->onNotify(this, NOTIFY_RESIZE, 0);
    }

    return TRUE;
}


//****************************
// CSelImgBar
//****************************


/*!
    @class CSelImgBar
    @brief ブラシ/テクスチャ画像選択バーウィジェット
*/


CSelImgBar::CSelImgBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{

}

//! 名前セット

void CSelImgBar::setName(const AXString &name)
{
    m_strName = name;
    redraw();
}

void CSelImgBar::setName(LPCUSTR pText)
{
    m_strName = pText;
    redraw();
}

//! 標準サイズ計算

void CSelImgBar::calcDefSize()
{
    m_nDefW = 10;
    m_nDefH = m_pFont->getHeight() + 6;
}

//! 描画

BOOL CSelImgBar::onPaint(AXHD_PAINT *phd)
{
    //枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::BLACK);

    //背景

    drawFillBox(1, 1, m_nW - 2, m_nH - 2, (isEnabled())? AXAppRes::WHITE: AXAppRes::FACELIGHT);

    //名前

    AXDrawText dt(m_id);

    dt.setClipRect(4, 0, m_nW - 8, m_nH);

    dt.draw(*m_pFont, 4, 3, m_strName, AXAppRes::TC_BLACK);

    dt.end();

    return TRUE;
}

//! ボタン押し

BOOL CSelImgBar::onButtonDown(AXHD_MOUSE *phd)
{
    int type;

    if(phd->button == BUTTON_LEFT || phd->button == BUTTON_RIGHT)
    {
        //右クリックまたは左+Ctrlで特殊扱い

        type = ((phd->button == BUTTON_LEFT && (phd->state & STATE_CTRL)) ||
                 phd->button == BUTTON_RIGHT);

        getNotify()->sendNotify(this,
                                (type)? NOTIFY_RIGHT: NOTIFY_LEFT, 0);
    }

    return TRUE;
}


//****************************
// CPressCurveView
//****************************


/*!
    @class CPressCurveView
    @brief 筆圧カーブウィジェット
*/


CPressCurveView::CPressCurveView(AXWindow *pParent,UINT uItemID,int size)
    : AXWindow(pParent, 0, 0, uItemID, 0)
{
    m_uFlags |= FLAG_REDRAW;
    m_nMinW = m_nMinH = size;

    m_nVal = 100;

    m_img.create(size, size);
}

//! 値セット

void CPressCurveView::setVal(int val)
{
    m_nVal = val;

    redrawUpdate();
}

//! 描画

BOOL CPressCurveView::onPaint(AXHD_PAINT *phd)
{
    if(m_uFlags & FLAG_REDRAW)
        _draw();

    m_img.put(m_id);

    return TRUE;
}

//! ボタン押し

BOOL CPressCurveView::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_TEMP1))
    {
        _changePos(phd->x, phd->y);

        m_uFlags |= FLAG_TEMP1;
        grabPointer();
    }

    return TRUE;
}

//! ボタン離し

BOOL CPressCurveView::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_TEMP1))
    {
        m_uFlags &= ~FLAG_TEMP1;
        ungrabPointer();
    }

    return TRUE;
}

//! 移動

BOOL CPressCurveView::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_TEMP1)
        _changePos(phd->x, phd->y);

    return TRUE;
}

//! マウス位置から値セット

void CPressCurveView::_changePos(int x,int y)
{
    int len,val,max;

    x -= m_nW / 2;
    y -= m_nW / 2;

    len = (int)::sqrt(x * x + y * y);

    max = m_nW - 10;
    if(len > max) len = max;

    //値

    if(x < 0 || y < 0)
        val = 100 - (100 - 1) * len / max;
    else
        val = (600 - 100) * len / max + 100;

    if(val >= 98 && val <= 102) val = 100;

    //変更

    if(m_nVal != val)
    {
        m_nVal = val;

        redrawUpdate();

        getNotify()->onNotify(this, 0, 0);
    }
}

//! 描画

void CPressCurveView::_draw()
{
    int size,size2,i,y,by;
    double gamma,val;

    size  = m_nMinW;
    size2 = m_nMinW - 1;

    //背景

    m_img.clear((isEnabled())? 0xffffff: 0xdddddd);

    //基準線

    m_img.line(0, size2, size2, 0, 0x808080);

    //枠

    m_img.box(0, 0, size, size, 0);

    //曲線

    gamma = m_nVal * 0.01;

    by = size2;

    for(i = 1; i < size; i++)
    {
        val = ::pow((double)i / size2, gamma);

        y = size2 - (int)(val * size2 + 0.5);

        m_img.line(i - 1, by, i, y, 0x0000ff);

        by = y;
    }
}
