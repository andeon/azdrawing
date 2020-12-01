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

#include "AXScrollBar.h"

#include "AXAppRes.h"


//-----------------------------

#define SCRBOXSIZE          15      //※サイズを変えるときは AXScrollView.cpp の方も変更

#define DOWNF_LEFTTOP       1
#define DOWNF_RIGHTBOTTOM   2
#define DOWNF_BAR           3

#define TIMERID_UPDATE          0   //バードラッグ時の更新
#define TIMERID_REPEAT_START    1   //矢印ボックスの押し中のリピート開始
#define TIMERID_REPEAT          2   //リピート中

//-----------------------------

/*!
    @class AXScrollBar
    @brief スクロールバーウィジェット

    <h2>[Notify通知]</h2>
    - SBN_SCROLL@n
      スクロール位置が変化した時。lParam = 現在の位置。

    @ingroup widget
*/
/*!
    @var AXScrollBar::SBS_HORZ
    @brief 水平スクロール
    @var AXScrollBar::SBS_VERT
    @brief 垂直スクロール
*/

/*
    m_nRange    : 0でバーなし
    m_nInc      : 矢印ボタンで一回に移動する値

    ※バーボタンは、矢印ボックスの1px分と重なる。
*/

//---------------------


AXScrollBar::~AXScrollBar()
{

}

AXScrollBar::AXScrollBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createScrollBar();
}

AXScrollBar::AXScrollBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createScrollBar();
}

//! 作成

void AXScrollBar::_createScrollBar()
{
    m_uType = TYPE_SCROLLBAR;

    m_nPos = m_nMin = m_nRange = 0;
    m_nMax = m_nPage = m_nInc = 1;
    m_fDown = 0;
}

//! 標準サイズ計算

void AXScrollBar::calcDefSize()
{
    m_nDefW = SCRBOXSIZE;
    m_nDefH = SCRBOXSIZE;

    if(m_uStyle & SBS_VERT)
        m_nDefH += SCRBOXSIZE + 10;
    else
        m_nDefW += SCRBOXSIZE + 10;
}

//! スクロールボックスの幅取得

int AXScrollBar::getScrollBoxWidth()
{
    return SCRBOXSIZE;
}

//! ステータスセット
/*!
    @param page バー部分の幅（値の範囲で）
    @param inc  矢印ボックスでの1回の増減値
*/

void AXScrollBar::setStatus(int min,int max,int page,int inc)
{
    if(min >= max) max = min + 1;

    if(page < 1) page = 1; else if(page > max - min) page = max - min;

    m_nMin   = min;
    m_nMax   = max;
    m_nPage  = page;
    m_nInc   = inc;
    m_nRange = max - min - page;

    if(m_nPos < min) m_nPos = min;
    else if(m_nPos > max - page) m_nPos = max - page;

    redraw();
}

//! ページ値変更
/*!
    ※現在位置が変更する場合あり
*/

void AXScrollBar::setPage(int page)
{
    if(page < 1) page = 1;
    else if(page > m_nMax - m_nMin) page = m_nMax - m_nMin;

    if(page != m_nPage)
    {
        m_nPage  = page;
        m_nRange = m_nMax - m_nMin - page;

        if(m_nPos < m_nMin) m_nPos = m_nMin;
        else if(m_nPos > m_nMax - m_nPage) m_nPos = m_nMax - m_nPage;

        redraw();
    }
}

//! 位置セット

BOOL AXScrollBar::setPos(int pos)
{
    if(pos < m_nMin) pos = m_nMin;
    else if(pos > m_nMax - m_nPage) pos = m_nMax - m_nPage;

    if(m_nPos == pos)
        return FALSE;
    else
    {
        m_nPos = pos;
        redraw();
        return TRUE;
    }
}

//! 位置を終端にセット

BOOL AXScrollBar::setPosEnd()
{
	return setPos(m_nMax - m_nPage);
}

//! 指定分位置を移動

BOOL AXScrollBar::movePos(int dir)
{
    return setPos(m_nPos + dir);
}


//============================
//ハンドラ
//============================


//! スクロール時

BOOL AXScrollBar::onScroll()
{
    getNotify()->onNotify(this, 0, m_nPos);

    return TRUE;
}

//! 描画

BOOL AXScrollBar::onPaint(AXHD_PAINT *phd)
{
    if(m_uStyle & SBS_VERT)
        _drawVertScr();
    else
        _drawHorzScr();

    return TRUE;
}

//! ボタン押し

BOOL AXScrollBar::onButtonDown(AXHD_MOUSE *phd)
{
    int area;

    if(!m_fDown && phd->button == BUTTON_LEFT)
    {
        area = _buttonDown(phd->x, phd->y);

        if(area)
        {
            m_fDown	= area;
            grabPointer();
        }

        redrawUpdate();
    }

    return TRUE;
}

//! ボタン離し

BOOL AXScrollBar::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fDown && phd->button == BUTTON_LEFT)
    {
        m_fDown = 0;

        redrawUpdate();
        ungrabPointer();

        //ドラッグ更新が残っていれば更新

        if(isTimerExist(TIMERID_UPDATE))
            onScroll();

        //全タイマー削除

        delTimerAll();
    }

    return TRUE;
}

//! マウス移動（バードラッグ）

BOOL AXScrollBar::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_fDown == DOWNF_BAR)
        _barDrag(phd->x ,phd->y);

    return TRUE;
}

//! タイマー（ドラッグ時更新）

BOOL AXScrollBar::onTimer(UINT uTimerID,ULONG lParam)
{
    int pos;

    switch(uTimerID)
    {
        //ドラッグ時更新
        case TIMERID_UPDATE:
            delTimer(uTimerID);
            onScroll();
            break;
        //リピート開始
        case TIMERID_REPEAT_START:
            delTimer(uTimerID);
            addTimer(TIMERID_REPEAT, 40);
            break;
        //リピート中
        case TIMERID_REPEAT:
            if(m_fDown == DOWNF_LEFTTOP)
                pos = m_nPos - m_nInc;
            else
                pos = m_nPos + m_nInc;

            if(setPos(pos))
                onScroll();
            break;
    }

    return TRUE;
}


//============================
//内部処理
//============================


//! ボタン押し時の処理
/*!
    @return 押された範囲のタイプ
*/

int AXScrollBar::_buttonDown(int x,int y)
{
    int area,bw,bpos,scrw,hvx,pos = m_nPos;

    _getBarStat(&bw, &bpos);
    if(bw == 0) return 0;   //バーなしの場合は何も処理しない

    area = _getCurArea(x, y, bw, bpos);

    //

    if(m_uStyle & SBS_VERT)
        scrw = m_nH, hvx = y;
    else
        scrw = m_nW, hvx = x;

    //

    switch(area)
    {
        //バー以外のスクロール部分（マウス位置がバーの中心に来るように）
        case 0:
            pos = _DTOI((double)(hvx - SCRBOXSIZE + 1 - bw / 2) * m_nRange / (scrw - SCRBOXSIZE * 2 + 2 - bw)) + m_nMin;
            break;
        //-1
        case 1:
            pos -= m_nInc;
            addTimer(TIMERID_REPEAT_START, 500);
            break;
        //+1
        case 2:
            pos += m_nInc;
            addTimer(TIMERID_REPEAT_START, 500);
            break;
        //バードラッグ開始（m_nDragAdj : 押された位置がバー先頭からどれだけずれているか）
        case 3:
            m_nDragAdj = hvx - bpos;
            break;
    }

    //位置セット

    if(setPos(pos))
        onScroll();

    return area;
}

//! バードラッグ時の処理

void AXScrollBar::_barDrag(int x,int y)
{
    int bw,bpos,scrw,hvx,pos;

    _getBarStat(&bw, &bpos);

    if(m_uStyle & SBS_VERT)
        scrw = m_nH, hvx = y;
    else
        scrw = m_nW, hvx = x;

    pos = _DTOI((double)(hvx - SCRBOXSIZE + 1 - m_nDragAdj) * m_nRange / (scrw - SCRBOXSIZE * 2 + 2 - bw)) + m_nMin;

    //その都度 onScroll() を送ると重いので、タイマーで遅延させる

    if(setPos(pos))
        addTimer(TIMERID_UPDATE, 5);
}

//! マウス位置から、どのエリアか判定
/*!
    ※バーなしの場合、常に0が返る
    @return [0]バー以外のスクロール部分 [1]左/上ボックス [2]右/下ボックス [3]バー
*/

int AXScrollBar::_getCurArea(int x,int y,int bw,int bpos)
{
    if(bw == 0) return 0;

    if(m_uStyle & SBS_VERT)
    {
        //垂直

        if(y < SCRBOXSIZE) return 1;
        if(y >= m_nH - SCRBOXSIZE) return 2;
        if(bpos <= y && y < bpos + bw) return 3;
    }
    else
    {
        //水平

        if(x < SCRBOXSIZE) return 1;
        if(x >= m_nW - SCRBOXSIZE) return 2;
        if(bpos <= x && x < bpos + bw) return 3;
    }

    return 0;
}

//! バーの情報取得
/*!
    @param pBarW バーの幅px。バーなしの場合は0が入る
    @param pBarPos バーの位置
*/

void AXScrollBar::_getBarStat(LPINT pBarW,LPINT pBarPos)
{
    int pos,bw,scrw;

    if(m_nRange == 0)
        //バーなしの場合
        bw = pos = 0;
    else
    {
        //スクロール部分幅
        //※矢印ボックスの1px分はバーと重なる

        scrw = (m_uStyle & SBS_VERT)? m_nH: m_nW;
        scrw -= SCRBOXSIZE * 2 - 2;

        if(scrw < 0) scrw = 0;

        //バー幅

        bw = _DTOI((double)scrw / (m_nMax - m_nMin) * m_nPage);
        if(bw < 7) bw = 7;

        //バー位置

        pos	= _DTOI((double)(scrw - bw) / m_nRange * (m_nPos - m_nMin));
        if(pos < 0) pos = 0; else if(pos > scrw - bw) pos = scrw - bw;
    }

    *pBarW   = bw;
    *pBarPos = pos + SCRBOXSIZE - 1;
}


//============================
//描画
//============================


//! 水平スクロール描画

void AXScrollBar::_drawHorzScr()
{
    int scrw,x,bw,arrowcol,n;
    DRAWRECT rc[2];

    scrw     = m_nW - SCRBOXSIZE * 2;
    arrowcol = (m_nRange == 0)? AXAppRes::TEXTDISABLE: AXAppRes::TEXTNORMAL;

    //バー部分

    if(scrw > 0)
    {
        _getBarStat(&bw, &x);

        if(bw == 0)
        {
            //バーなし

            drawFillBox(SCRBOXSIZE, 0, scrw, SCRBOXSIZE, AXAppRes::FACEDARK);
        }
        else
        {
            //バーあり

            n = 0;

            if(x - SCRBOXSIZE > 0)
            {
                setDrawRect(rc, SCRBOXSIZE, 0, x - SCRBOXSIZE, SCRBOXSIZE);
                n++;
            }

            if(m_nW - x - bw - SCRBOXSIZE > 0)
            {
                setDrawRect(rc + n, x + bw, 0, m_nW - x - bw - SCRBOXSIZE, SCRBOXSIZE);
                n++;
            }

            drawFillBoxs(rc, n, AXAppRes::FACEDARK);
            drawButton(x, 0, bw, SCRBOXSIZE, 0);
        }
    }

    //左BOX

    n = (m_fDown == DOWNF_LEFTTOP);

    drawButton(0, 0, SCRBOXSIZE, SCRBOXSIZE, (n)? DRAWBUTTON_DOWN: 0);
    drawArrowLeft(SCRBOXSIZE / 2 + n, SCRBOXSIZE / 2 + n, arrowcol);

    //右BOX

    x = m_nW - SCRBOXSIZE;
    n = (m_fDown == DOWNF_RIGHTBOTTOM);

    drawButton(x, 0, SCRBOXSIZE, SCRBOXSIZE, (n)? DRAWBUTTON_DOWN: 0);
    drawArrowRight(x + SCRBOXSIZE / 2 + n, SCRBOXSIZE / 2 + n, arrowcol);
}

//! 垂直スクロール描画

void AXScrollBar::_drawVertScr()
{
    int scrw,y,bw,arrowcol,n;
    DRAWRECT rc[2];

    scrw     = m_nH - SCRBOXSIZE * 2;
    arrowcol = (m_nRange == 0)? AXAppRes::TEXTDISABLE: AXAppRes::TEXTNORMAL;

    //スクロール部分

    if(scrw > 0)
    {
        _getBarStat(&bw, &y);

        if(bw == 0)
        {
            //バーなし

            drawFillBox(0, SCRBOXSIZE, SCRBOXSIZE, scrw, AXAppRes::FACEDARK);
        }
        else
        {
            //バーあり

            n = 0;

            if(y - SCRBOXSIZE > 0)
            {
                setDrawRect(rc, 0, SCRBOXSIZE, SCRBOXSIZE, y - SCRBOXSIZE);
                n++;
            }

            if(m_nH - y - bw - SCRBOXSIZE > 0)
            {
                setDrawRect(rc + n, 0, y + bw, SCRBOXSIZE, m_nH - y - bw - SCRBOXSIZE);
                n++;
            }

            drawFillBoxs(rc, n, AXAppRes::FACEDARK);
            drawButton(0, y, SCRBOXSIZE, bw, 0);
        }
    }

    //上BOX

    n = (m_fDown == DOWNF_LEFTTOP);

    drawButton(0, 0, SCRBOXSIZE, SCRBOXSIZE, (n)? DRAWBUTTON_DOWN: 0);
    drawArrowUp(SCRBOXSIZE / 2 + n, SCRBOXSIZE / 2 + n, arrowcol);

    //下BOX

    y = m_nH - SCRBOXSIZE;
    n = (m_fDown == DOWNF_RIGHTBOTTOM);

    drawButton(0, y, SCRBOXSIZE, SCRBOXSIZE, (n)? DRAWBUTTON_DOWN: 0);
    drawArrowDown(SCRBOXSIZE / 2 + n, y + SCRBOXSIZE / 2 + n, arrowcol);
}

