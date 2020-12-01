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

#include "AXMultiEditArea.h"

#include "AXMultiEdit.h"
#include "AXEditString.h"
#include "AXTopWindow.h"
#include "AXScrollBar.h"
#include "AXString.h"
#include "AXFont.h"
#include "AXAppRes.h"
#include "AXApp.h"
#include "AXUtil.h"


//-------------------------

#define XSPACE  2
#define YSPACE  1

//--------------------------

/*!
    @class AXMultiEditArea
    @brief AXMultiEdit のテキスト表示部分（AXMultiEdit の子）

    @ingroup widgetsub
*/


AXMultiEditArea::~AXMultiEditArea()
{

}

AXMultiEditArea::AXMultiEditArea(AXWindow *pParent,UINT uStyle,AXEditString *pString)
    : AXScrollArea(pParent, uStyle)
{
    m_pString = pString;
    m_fBtt    = 0;
    m_uFlags  |= FLAG_REDRAW;
}

//! スクロール表示判定

BOOL AXMultiEditArea::isShowScroll(int size,BOOL bHorz)
{
    if(bHorz)
        return (size < m_pString->getMaxWidth() + XSPACE * 2);
    else
        return (size < m_pString->getMaxLine() * m_pFont->getHeight() + YSPACE * 2);
}

//! (IM) 前編集表示位置

void AXMultiEditArea::getICPos(AXPoint *pPos)
{
    int x,y;

    x = XSPACE + m_pString->getCurX() - getHorzPos();
    y = YSPACE + m_pString->getCurLine() * m_pFont->getHeight() - getVertPos();

    if(x < XSPACE) x = XSPACE;
    else if(x > m_nW) x = m_nW;

    if(y < YSPACE) y = YSPACE;
    else if(y > m_nH) y = m_nH;

    translateTo(NULL, &x, &y);

    pPos->set(x, y);
}


//==============================
//サブ処理
//==============================


//! スクロール情報セット

void AXMultiEditArea::setScrollInfo()
{
    //+1 はカーソルの分
    scrH()->setStatus(0, m_pString->getMaxWidth() + 1, m_nW - XSPACE * 2);
    scrV()->setStatus(0, m_pString->getMaxLine() * m_pFont->getHeight(), m_nH - YSPACE * 2);
}

//! スクロール位置調整

void AXMultiEditArea::_adjustScroll()
{
    int scrx,scry,curx,cury,w,h;

    scrx    = getHorzPos();
    scry    = getVertPos();
    curx    = m_pString->getCurX();
    cury    = m_pString->getCurLine() * m_pFont->getHeight();
    w       = m_nW - XSPACE * 2;
    h       = m_nH - YSPACE * 2 - m_pFont->getHeight();

    if(scrx >= m_pString->getMaxWidth())
        scrx = 0;
    else if(curx - scrx < 0)
        scrx = curx;
    else if(curx - scrx >= w)
        scrx = curx - w + 1;

    if(scry >= m_pString->getMaxLine() * m_pFont->getHeight())
        scry = 0;
    else if(cury - scry < 0)
        scry = cury;
    else if(cury - scry > h)
        scry = cury - h;

    scrH()->setPos(scrx);
    scrV()->setPos(scry);
}


//==============================
//ハンドラ
//==============================


//! 通知

BOOL AXMultiEditArea::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(uNotify == SAN_SCROLL_HORZ || uNotify == SAN_SCROLL_VERT)
        redraw();

    return TRUE;
}

//! サイズ変更

BOOL AXMultiEditArea::onSize()
{
    setScrollInfo();

    m_img.recreate(m_nW, m_nH, 32, 32);

    redraw();

    return TRUE;
}

//! ボタン押し時

BOOL AXMultiEditArea::onButtonDown(AXHD_MOUSE *phd)
{
    int pos;

    if(m_fBtt) return TRUE;

    if(phd->button == BUTTON_LEFT)
    {
        pos = m_pString->getPosFromMouse(phd->x - XSPACE + getHorzPos(),
                                         phd->y - YSPACE + getVertPos(), m_pFont->getHeight());

        m_pParent->setFocus();

        m_pString->moveCursorPos(pos, phd->state & STATE_SHIFT);

        m_fBtt   = 1;
        m_nBkPos = pos;

        redraw();

        grabPointer();
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXMultiEditArea::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fBtt && phd->button == BUTTON_LEFT)
    {
        m_fBtt = 0;
        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動時

BOOL AXMultiEditArea::onMouseMove(AXHD_MOUSE *phd)
{
    int pos;

    if(m_fBtt == 1)
    {
        //ドラッグで選択拡張

        pos = m_pString->getPosFromMouse(phd->x - XSPACE + getHorzPos(),
                                         phd->y - YSPACE + getVertPos(), m_pFont->getHeight());

        if(pos != m_nBkPos)
        {
            m_pString->expandSel(pos);
            m_pString->setCursorPos(pos);

            m_nBkPos = pos;

            _adjustScroll();
            redraw();
        }
    }

    return TRUE;
}

//! ホイール

BOOL AXMultiEditArea::onMouseWheel(AXHD_MOUSE *phd,BOOL bUp)
{
    if(!m_fBtt)
    {
        if(scrV()->movePos(m_pFont->getHeight() * ((bUp)? -3: 3)))
            redraw();
    }

    return TRUE;
}

//! キー押し時

BOOL AXMultiEditArea::onKeyDown(AXHD_KEY *phd)
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

    ret = m_pString->keyCommand(str, key, state, !(m_uStyle & MEAS_READONLY), m_id, m_pFont, &scr);

    //更新

    if(ret != AXEditString::CMDRET_NONE)
    {
        /* ※ テキストが増えた時には adjustScroll() の前にスクロール情報を更新しておかないと、
           スクロール位置が正しくセットできない */

        setScrollInfo();
        _adjustScroll();

        redraw();

        //テキスト内容の変化

        if(ret == AXEditString::CMDRET_CHANGE)
        {
            //スクロール再構成

            m_pParent->setReconfig();

            //通知

            (m_pParent->getNotify())->onNotify(m_pParent, AXMultiEdit::MEN_CHANGE, 0);
        }
    }

    return TRUE;
}

//! 描画

BOOL AXMultiEditArea::onPaint(AXHD_PAINT *phd)
{
    if(m_uFlags & FLAG_REDRAW)
        _drawimg();

    m_img.put(this, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}

//! イメージに描画

void AXMultiEditArea::_drawimg()
{
    //背景

    m_img.drawFillBox(0, 0, m_nW, m_nH,
            ((m_uFlags & FLAG_ENABLED) && !(m_uStyle & MEAS_READONLY))? AXAppRes::FACEDOC: AXAppRes::FACE);

    //テキスト

    if(m_pString->getLen())
        _drawText();

    //カーソル

    if(m_pParent->isFocused())
    {
        int x,y;

        x = m_pString->getCurX() - getHorzPos();
        y = m_pString->getCurLine() * m_pFont->getHeight() - getVertPos();

        if(x >= 0 && x < m_nW - XSPACE * 2 && y >= 0 && y < m_nH - YSPACE * 2)
            m_img.drawLine(XSPACE + x, YSPACE + y, XSPACE + x, YSPACE + y + m_pFont->getHeight() - 1, axres->gcXor());
    }
}

//! テキスト描画

void AXMultiEditArea::_drawText()
{
    LPCUSTR pt;
    LPWORD pw;
    int pos,x,y,th,maxw,maxh,xtop;

    pt  = m_pString->getText();
    pw  = m_pString->getWidthBuf();
    xtop = x = -getHorzPos();
    y   = -getVertPos();
    th  = m_pFont->getHeight();
    maxw = m_nW - XSPACE * 2;
    maxh = m_nH - YSPACE * 2;

    //

    AXDrawText dt(m_img.getid());

    dt.setClipRect(XSPACE, YSPACE, maxw, maxh);

    for(pos = 0; *pt; pt++, pw++, pos++)
    {
        if(*pt == '\n')
        {
            //改行
            //選択範囲の場合は、改行がわかるように4pxの幅を付ける

            if(m_pString->isPosSel(pos) && x > -4 && x < maxw && y > -th && y < maxh)
                dt.drawRect(x + XSPACE, y + YSPACE, 4, th, AXAppRes::TC_REVBACK);

            x = xtop;
            y += th;
        }
        else
        {
            //文字

            if(x > -(*pw) && x < maxw && y > -th && y < maxh)
            {
                if(m_pString->isPosSel(pos))
                {
                    dt.drawRect(x + XSPACE, y + YSPACE, *pw, th, AXAppRes::TC_REVBACK);
                    dt.draw(*m_pFont, x + XSPACE, y + YSPACE, pt, 1, AXAppRes::TC_REV);
                }
                else
                    dt.draw(*m_pFont, x + XSPACE, y + YSPACE, pt, 1, AXAppRes::TC_NORMAL);
            }

            x += *pw;
        }
    }

    dt.end();
}
