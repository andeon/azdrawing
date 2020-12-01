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

#include "AXHeader.h"

#include "AXFont.h"
#include "AXAppRes.h"
#include "AXApp.h"


#define SPACE_Y     2

/*!
    @class AXHeader
    @brief リストビューなどのヘッダ部分ウィジェット

    - 水平スクロールバーと連動させる場合は、HDN_RESIZE 時にスクロールバーの情報変更後、
       AXHeader::setScroll() でヘッダのスクロール位置も変更すること。@n
       （スクロールバーの情報変更時にスクロール位置が調節される場合があるため）

    @ingroup widget
*/

/*!
    @var AXHeader::HDN_RESIZE
    @brief ドラッグにより、アイテムの幅が変更された※ドラッグ中に来る（lParam = AXHeaderItem*）

    @var AXHeaderItem::FLAG_RIGHT
    @brief この列はテキストを右寄せ
    @var AXHeaderItem::FLAG_FIX
    @brief この列は幅を固定する（ドラッグ不可）
    @var AXHeaderItem::FLAG_EXPAND
    @brief ヘッダ全体のサイズに自動拡張する（ただし、最初の列のみ）
    @var AXHeaderItem::FLAG_COL_OWNERDRAW
    @brief この列のアイテムはオーナードロー
*/


AXHeader::~AXHeader()
{

}

AXHeader::AXHeader(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createHeader();
}

AXHeader::AXHeader(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createHeader();
}

//! 作成

void AXHeader::_createHeader()
{
    m_uType = TYPE_HEADER;
    m_nScr  = 0;
}

//! 標準サイズ計算

void AXHeader::calcDefSize()
{
    m_nDefW = 10;
    m_nDefH = m_pFont->getHeight() + SPACE_Y * 2;
}


//==========================
//
//==========================


//! スクロール位置セット

void AXHeader::setScroll(int scr)
{
    m_nScr = scr;
    redraw();
}

//! すべてのアイテムの幅合計取得

int AXHeader::getAllWidth()
{
    AXHeaderItem *p;
    int w = 0;

    for(p = (AXHeaderItem *)m_dat.getTop(); p; p = p->next())
        w += p->m_nWidth;

    return w;
}

//! 指定座標のアイテム取得
/*!
    @param pNo  アイテムの番号が入る（-1でなし）。NULL可
    @param pX   アイテムの左端 X 座標が入る。NULL可
*/

AXHeaderItem *AXHeader::getCurPosItem(int x,LPINT pNo,LPINT pX)
{
	AXHeaderItem *p;
	int xx = -m_nScr,no = 0;

	for(p = (AXHeaderItem *)m_dat.getTop(); p; p = p->next(), no++)
	{
		if(xx <= x && x < xx + p->m_nWidth)
		{
			if(pNo) *pNo = no;
			if(pX)  *pX  = xx;
			return p;
		}

		xx += p->m_nWidth;
	}

	if(pNo) *pNo = -1;

	return NULL;
}


//==========================
//アイテム
//==========================


//! 追加

void AXHeader::addItem(LPCUSTR pText,int width,UINT uFlags)
{
    m_dat.add(new AXHeaderItem(pText, width, uFlags));
}

//! 幅取得

int AXHeader::getItemWidth(int no)
{
    AXHeaderItem *p;

    p = (AXHeaderItem *)m_dat.getItem(no);
    if(!p)
        return 0;
    else
        return p->m_nWidth;
}

//! 幅セット

void AXHeader::setItemWidth(int no,int width)
{
    AXHeaderItem *p;

    p = (AXHeaderItem *)m_dat.getItem(no);
    if(p)
        p->m_nWidth = width;
}


//==============================
//ハンドラ
//==============================


//! サイズ変更時

BOOL AXHeader::onSize()
{
    AXHeaderItem *p = (AXHeaderItem *)m_dat.getTop();

    if(p && (p->m_uFlags & AXHeaderItem::FLAG_EXPAND))
        p->m_nWidth = m_nW;

    return TRUE;
}

//! ボタン押し時

BOOL AXHeader::onButtonDown(AXHD_MOUSE *phd)
{
    AXHeaderItem *p;
    int no,x;

    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_DRAG))
    {
        p = getCurPosItem(phd->x - 4, &no, &x);

        if(p && !(p->m_uFlags & AXHeaderItem::FLAG_FIX) && phd->x > x + p->m_nWidth - 4)
        {
            m_uFlags    |= FLAG_DRAG;
            m_pDragItem = p;
            m_nDragLeft = x + m_nScr;

            grabPointer();
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXHeader::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_DRAG))
    {
        m_uFlags &= ~FLAG_DRAG;

        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動時

BOOL AXHeader::onMouseMove(AXHD_MOUSE *phd)
{
    AXHeaderItem *p;
    int no,x;

    if(m_uFlags & FLAG_DRAG)
    {
        //リサイズドラッグ中

        x = phd->x - (m_nDragLeft - m_nScr);
        if(x < 3) x = 3;

        if(x != m_pDragItem->m_nWidth)
        {
            m_pDragItem->m_nWidth = x;
            redraw();

            getNotify()->onNotify(this, HDN_RESIZE, (ULONG)m_pDragItem);
        }
    }
    else
    {
        //通常時、境界上ならカーソル変更

        p = getCurPosItem(phd->x - 4, &no, &x);

        if(p && !(p->m_uFlags & AXHeaderItem::FLAG_FIX) && phd->x > x + p->m_nWidth - 4)
            setCursor(axapp->getCursor(AXApp::CURSOR_HSPLIT));
        else
            unsetCursor();
    }

    return TRUE;
}

//! 離れた時

BOOL AXHeader::onLeave(AXHD_ENTERLEAVE *phd)
{
    unsetCursor();

    return TRUE;
}

//! 描画

BOOL AXHeader::onPaint(AXHD_PAINT *phd)
{
    AXHeaderItem *p;
    int x,w,xx;

    //背景

    drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    //項目

    AXDrawText dt(m_id);

    x = -m_nScr;

    for(p = (AXHeaderItem *)m_dat.getTop(); p; x += p->m_nWidth, p = p->next())
    {
        if(x + p->m_nWidth <= 0) continue;
        if(x >= m_nW) break;

        w = p->m_nWidth;

        //枠

        drawFrameRaise(x, 0, w, m_nH);

        //テキスト

        dt.setClipRect(x + 2, 0, w - 4, m_nH);

        if(p->m_uFlags & AXHeaderItem::FLAG_RIGHT)
            xx = w - 4 - m_pFont->getTextWidth(p->m_strText);
        else
            xx = 0;

        dt.draw(*m_pFont, x + xx + 2, SPACE_Y, p->m_strText);
    }

    dt.end();

    return TRUE;
}


//*************************************
// AXHeaderItem - アイテムクラス
//*************************************


AXHeaderItem::AXHeaderItem(LPCUSTR pText,int width,UINT uFlags)
{
    m_strText   = pText;
    m_nWidth    = width;
    m_uFlags    = uFlags;
}
