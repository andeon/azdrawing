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

#include "AXLineEdit.h"

#include "AXEditString.h"
#include "AXTopWindow.h"
#include "AXString.h"
#include "AXFont.h"
#include "AXAppRes.h"
#include "AXApp.h"
#include "AXUtil.h"


//-------------------------

#define XSPACE  3
#define YSPACE  3
#define SPINW   13

#define BTTF_DRAG       1
#define BTTF_SPINUP     2
#define BTTF_SPINDOWN   3

#define TIMERID_SPINSTART   0
#define TIMERID_SPINREPEAT  1

//--------------------------

/*!
    @class AXLineEdit
    @brief 一行エディットウィジェット

    @ingroup widget
*/

/*!
    @var AXLineEdit::ES_READONLY
    @brief 読み込み専用
    @var AXLineEdit::ES_SPIN
    @brief スピンを付ける

    @var AXLineEdit::EN_CHANGE
    @brief テキスト内容が変化した
*/

/*
    m_nScrX       : スクロール位置(px)
    m_nTextTopPos : 描画時のテキストの先頭位置
    m_nScrAdjX    : 描画時のテキスト先頭位置(px)がスクロール位置よりどれだけずれているか
*/


AXLineEdit::~AXLineEdit()
{
    delete m_pString;
}

AXLineEdit::AXLineEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createLineEdit();
}

AXLineEdit::AXLineEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createLineEdit();
}

//! 作成

void AXLineEdit::_createLineEdit()
{
    m_uType  = TYPE_LINEEDIT;
    m_uFlags |= FLAG_TAKE_FOCUS;

    m_pString = new AXEditString;

    m_nScrX         = 0;
    m_nTextScrTop   = 0;
    m_nScrAdjX      = 0;
    m_fBtt          = 0;

    m_nValMin = m_nValMax = m_nValDig = 0;
}

//! 標準サイズ計算

void AXLineEdit::calcDefSize()
{
    m_nDefW = XSPACE * 2;
    m_nDefH = YSPACE * 2 + m_pFont->getHeight();

    if(isSpin())
    {
        m_nDefW += SPINW + 1;
        if(m_nDefH < 14) m_nDefH = 14;
    }
}

//! (IM) 前編集表示位置

void AXLineEdit::getICPos(AXPoint *pPos)
{
    int x;

    x = XSPACE + m_pString->getCurX() - m_nScrX;

    if(x < XSPACE)
        x = XSPACE;
    else if(x > _getEditRight())
        x = _getEditRight();

    pPos->x = x;
    pPos->y = YSPACE;

    translateTo(NULL, &pPos->x, &pPos->y);
}


//===============================
//
//===============================


//! ウィンドウ幅を文字長さ（半角数字）からセット

void AXLineEdit::setWidthFromLen(int len)
{
    m_nMinW = m_pFont->getTextWidth("9", 1) * len + XSPACE * 2 + 1;
    if(isSpin()) m_nMinW += SPINW;
}

//! テキストセット

void AXLineEdit::setText(LPCUSTR pText)
{
    m_pString->setText(pText, m_pFont);

    _setScroll(0);
    redraw();
}

//! int値セット

void AXLineEdit::setInt(int val)
{
    m_pString->setInt(val, m_pFont);

    _setScroll(0);
    redraw();
}

//! 値のステータスセット
/*!
    @param dig 小数点以下の桁数（0で小数点なし）
*/

void AXLineEdit::setValStatus(int min,int max,int dig)
{
    m_nValMin = min;
    m_nValMax = max;
    m_nValDig = dig;
}

//! 値セット

void AXLineEdit::setVal(int val)
{
    if(val < m_nValMin) val = m_nValMin;
    else if(val > m_nValMax) val = m_nValMax;

    m_pString->setIntFloat(val, m_nValDig, m_pFont);

    _setScroll(0);
    redraw();
}

//! テキスト取得

void AXLineEdit::getText(AXString *pstr)
{
    m_pString->getString(pstr);
}

//! テキスト文字数取得

int AXLineEdit::getTextLen()
{
    return m_pString->getLen();
}

//! 値の範囲内に補正してint値取得

int AXLineEdit::getVal()
{
    int i,mul,n;

    if(m_nValDig == 0)
        n = m_pString->getInt();
    else
    {
        for(i = 0, mul = 1; i < m_nValDig; i++, mul *= 10);

        n = AXDoubleToInt(m_pString->getDouble() * mul);
    }

    if(n < m_nValMin) n = m_nValMin;
    else if(n > m_nValMax) n = m_nValMax;

    return n;
}

//! テキストをint値として取得

int AXLineEdit::getInt()
{
    return m_pString->getInt();
}

//! テキストをdouble値として取得

double AXLineEdit::getDouble()
{
    return m_pString->getDouble();
}

//! すべて選択（フォーカスがあることが前提）

void AXLineEdit::selectAll()
{
    if(isFocused() && m_pString->selectAll())
        redraw();
}

//! 初期設定
/*!
    setWidthFromLen()、setValStatus()、setVal() をまとめてセット。
*/

void AXLineEdit::setInit(int widthlen,int min,int max,int val)
{
    setWidthFromLen(widthlen);
    setValStatus(min, max);
    setVal(val);
}

//! 初期設定
/*!
    setWidthFromLen()、setValStatus()、setVal() をまとめてセット。
*/

void AXLineEdit::setInit(int widthlen,int min,int max,int dig,int val)
{
    setWidthFromLen(widthlen);
    setValStatus(min, max, dig);
    setVal(val);
}


//===============================
//サブ
//===============================


//! エディット部分の右端位置取得

int AXLineEdit::_getEditRight()
{
    if(isSpin())
        return m_nW - 1 - SPINW - 2;
    else
        return m_nW - XSPACE;
}

//! スピンの各高さ取得

void AXLineEdit::_getSpinHeight(LPINT pH1,LPINT pH2)
{
    *pH1 = (m_nH - 2) / 2;
    *pH2 = (m_nH - 2) - *pH1;
}

//! スクロール調整

void AXLineEdit::_adjustScroll()
{
    int w,curx;

    curx = m_pString->getCurX();
    w    = _getEditRight() - XSPACE;

    if(m_nScrX >= m_pString->getMaxWidth())
        _setScroll(0);
    else if(curx - m_nScrX < 0)      //カーソル位置が左側にある
        _setScroll(curx);
    else if(curx - m_nScrX >= w)     //カーソル位置が右側にある ※カーソル用に1pxプラス
        _setScroll(curx - w + 1);
}

//! スクロール位置変更

void AXLineEdit::_setScroll(int scrx)
{
    LPCUSTR pt;
    LPWORD pw;
    int pos,tx;

    m_nScrX = (scrx < 0)? 0: scrx;

    //テキスト先頭位置

    pt = m_pString->getText();
    pw = m_pString->getWidthBuf();

    for(pos = 0, tx = 0; *pt; pt++, pw++, pos++)
    {
        if(tx + *pw > m_nScrX) break;

        tx += *pw;
    }

    m_nTextScrTop   = pos;
    m_nScrAdjX      = tx - m_nScrX;
}

//! スピンでの値操作

void AXLineEdit::_spinUpDown()
{
    int val = getVal();
    AXString strBk = m_pString->getText();

    if(m_fBtt == BTTF_SPINUP)
    {
        if(val != m_nValMax)
        {
            val++;
            if(val > m_nValMax) val = m_nValMax;
        }
    }
    else
    {
        if(val != m_nValMin)
        {
            val--;
            if(val < m_nValMin) val = m_nValMin;
        }
    }

    setVal(val);

    //通知

    if(strBk != m_pString->getText())
        getNotify()->onNotify(this, EN_CHANGE, 0);
}


//==============================
//ハンドラ
//==============================


//! フォーカスIN

BOOL AXLineEdit::onFocusIn(int detail)
{
    if(detail == FOCUSDETAIL_TABMOVE) m_pString->selectAll();
    redrawUpdate();

    return TRUE;
}

//! フォーカスOUT

BOOL AXLineEdit::onFocusOut(int detail)
{
    m_pString->clearSel();
    redrawUpdate();

    return TRUE;
}

//! ボタン押し時

BOOL AXLineEdit::onButtonDown(AXHD_MOUSE *phd)
{
    int pos;

    if(m_fBtt) return TRUE;

    if(phd->button == BUTTON_LEFT)
    {
        if(isSpin() && phd->x > _getEditRight() + 1)
        {
            //------- スピン部分

            if(phd->x < m_nW - 1)
            {
                int h1,h2;

                setFocus();

                _getSpinHeight(&h1, &h2);

                if(phd->y >= 1 && phd->y < 1 + h1)
                    m_fBtt = BTTF_SPINUP;
                else if(phd->y >= 1 + h1 && phd->y < 1 + h1 + h2)
                    m_fBtt = BTTF_SPINDOWN;

                if(m_fBtt)
                {
                    _spinUpDown();

                    redrawUpdate();
                    grabPointer();

                    addTimer(TIMERID_SPINSTART, 500);
                }
            }
        }
        else
        {
            //------- テキスト部分

            pos = m_pString->getPosFromMouse(phd->x - XSPACE + m_nScrX, 0, 0);

            setFocus();

            m_pString->moveCursorPos(pos, phd->state & STATE_SHIFT);

            m_fBtt   = BTTF_DRAG;
            m_nBkPos = pos;

            redrawUpdate();
            grabPointer();
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXLineEdit::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fBtt && phd->button == BUTTON_LEFT)
    {
        //スピンの場合

        if(m_fBtt == BTTF_SPINUP || m_fBtt == BTTF_SPINDOWN)
        {
            delTimerAll();
            redrawUpdate();
        }

        m_fBtt = 0;
        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動時

BOOL AXLineEdit::onMouseMove(AXHD_MOUSE *phd)
{
    int pos;

    if(m_fBtt == BTTF_DRAG)
    {
        //ドラッグで選択拡張

        pos = m_pString->getPosFromMouse(phd->x - XSPACE + m_nScrX, 0, 0);

        if(pos != m_nBkPos)
        {
            m_pString->expandSel(pos);
            m_pString->setCursorPos(pos);

            m_nBkPos = pos;

            _adjustScroll();
            redrawUpdate();
        }
    }

    return TRUE;
}

//! タイマー

BOOL AXLineEdit::onTimer(UINT uTimerID,ULONG lParam)
{
    switch(uTimerID)
    {
        //スピン押し最初
        case TIMERID_SPINSTART:
            delTimer(uTimerID);
            addTimer(TIMERID_SPINREPEAT, 120);
            break;
        //スピン押しリピート
        case TIMERID_SPINREPEAT:
            _spinUpDown();
            break;
    }

    return TRUE;
}

//! キー押し時

BOOL AXLineEdit::onKeyDown(AXHD_KEY *phd)
{
    AXString str;
    UINT key,state;
    int ret,scr;

    if(m_fBtt) return TRUE;

    //キーと文字列取得

    ((AXTopWindow *)m_pTopLevel)->getKeyString(phd->pEvent, &str, &key);

    state = 0;
    if(phd->state & STATE_SHIFT) state |= AXEditString::SHIFT;
    if(phd->state & STATE_CTRL)  state |= AXEditString::CTRL;
    if(phd->state & STATE_ALT)   state |= AXEditString::ALT;

    //処理

    ret = m_pString->keyCommand(str, key, state, !(m_uStyle & ES_READONLY), m_id, m_pFont, &scr);

    //スクロール変更
    //BACKSPACE - スクロール位置をカーソル位置の一つ前に

    if(scr == AXEditString::SCRRET_BACKSPACE)
    {
        if(m_pString->getCurX() <= m_nScrX)
            _setScroll(m_pString->getCurX() - m_pString->getCurBackWidth());
    }

    //更新

    if(ret != AXEditString::CMDRET_NONE)
    {
        _adjustScroll();
        redrawUpdate();

        //通知（テキスト内容の変化）

        if(ret == AXEditString::CMDRET_CHANGE)
            getNotify()->onNotify(this, EN_CHANGE, 0);
    }

    return TRUE;
}

//! 描画

BOOL AXLineEdit::onPaint(AXHD_PAINT *phd)
{
    int x;

    //枠

    drawFrameSunken(0, 0, m_nW, m_nH);

    //背景

    drawFillBox(1, 1, m_nW - 2, m_nH - 2,
            ((m_uFlags & FLAG_ENABLED) && !(m_uStyle & ES_READONLY))? AXAppRes::FACEDOC: AXAppRes::FACE);

    //スピン

    if(isSpin()) _drawSpin();

    //テキスト

    if(m_pString->getLen())
        _drawText();

    //カーソル

    if(isFocused())
    {
        x = m_pString->getCurX() - m_nScrX;

        if(x >= 0 && x < _getEditRight() - XSPACE)
            drawLine(XSPACE + x, YSPACE, XSPACE + x, YSPACE + m_pFont->getHeight() - 1, axres->gcXor());
    }

    return TRUE;
}

//! スピン描画

void AXLineEdit::_drawSpin()
{
    int x,xa,h1,h2,down;

    x  = m_nW - 1 - SPINW;
    xa = x + SPINW / 2;

    _getSpinHeight(&h1, &h2);

    //上

    down = (m_fBtt == BTTF_SPINUP);

    drawButton(x, 1, SPINW, h1, (down)? DRAWBUTTON_DOWN: 0);
    drawArrowUpSmall(xa, 3 + (h1 - 6) / 2 + down, AXAppRes::TEXTNORMAL);

    //下

    down = (m_fBtt == BTTF_SPINDOWN);

    drawButton(x, 1 + h1, SPINW, h2, (down)? DRAWBUTTON_DOWN: 0);
    drawArrowDownSmall(xa, 4 + h1 + (h2 - 6) / 2 + down, AXAppRes::TEXTNORMAL);
}

//! テキスト描画

void AXLineEdit::_drawText()
{
    int right,pos,bSel,tx,tw,th,tlen,col;
    LPCUSTR pt,ptTop;
    LPWORD pw;

    pos     = m_nTextScrTop;
    pt      = m_pString->getText() + pos;
    pw      = m_pString->getWidthBuf() + pos;
    ptTop   = pt;

    tx      = XSPACE + m_nScrAdjX;
    tw      = 0;
    th      = m_pFont->getHeight();
    tlen    = 0;
    right   = _getEditRight();
    bSel    = m_pString->isPosSel(pos);
    col     = (m_uFlags & FLAG_ENABLED)? AXAppRes::TC_NORMAL: AXAppRes::TC_DISABLE;

    //

    AXDrawText dt(m_id);

    dt.setClipRect(XSPACE, YSPACE, right - XSPACE, m_nH - YSPACE * 2);

    for(; *pt; pt++, pw++, pos++)
    {
        //右端

        if(tx + tw >= right) break;

        //

        tw += *pw;
        tlen++;

        //通常と選択の境

        if((!bSel && pos + 1 == m_pString->getSelTop()) ||     //次の文字が選択先頭
            (bSel && pos + 1 == m_pString->getSelEnd()))       //次の文字が選択終端
        {
            if(bSel)
                dt.drawRect(tx, YSPACE, tw, th, AXAppRes::TC_REVBACK);

            dt.draw(*m_pFont, tx, YSPACE, ptTop, tlen, (bSel)? AXAppRes::TC_REV: col);

            ptTop   = pt + 1;
            tx      += tw;
            tw      = 0;
            tlen    = 0;
            bSel    ^= 1;
        }
    }

	//残り文字

    if(tw)
    {
        if(bSel)
            dt.drawRect(tx, YSPACE, tw, th, AXAppRes::TC_REVBACK);

        dt.draw(*m_pFont, tx, YSPACE, ptTop, tlen, (bSel)? AXAppRes::TC_REV: col);
    }

    dt.end();
}
