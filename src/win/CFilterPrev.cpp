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

#include "CFilterPrev.h"

#include "CLayerImg.h"
#include "defStruct.h"


/*!
    @class CFilterPrev
    @brief フィルタダイアログのプレビューウィジェット
*/


CFilterPrev::CFilterPrev(AXWindow *pParent,UINT uLayoutFlags,UINT uID,DWORD dwPadding,
                         int w,int h,CLayerImg *pimgSrc)
    : AXWindow(pParent, 0, uLayoutFlags, uID, dwPadding)
{
    m_pimgSrc = pimgSrc;

    //キャンバスサイズ

    CLayerImg::getCanvasImgSize(&m_nCanvasW, &m_nCanvasH);

    //プレビューサイズがキャンバスサイズより大きい場合

    if(w - 2 > m_nCanvasW) w = m_nCanvasW + 2;
    if(h - 2 > m_nCanvasH) h = m_nCanvasH + 2;

    //

    m_nMinW = w;
    m_nMinH = h;

    //スクロールありか

    m_bScroll = (w - 2 < m_nCanvasW || h - 2 < m_nCanvasH);

    //ソース画像の表示範囲

    m_rcsSrc.set(0, 0, w - 2, h - 2);

    //キャンバスのフレーム枠（プレビューサイズ内に収める）

    m_rcsFrame.set(0, 0, m_nCanvasW, m_nCanvasH);
    m_rcsFrame.inBoxKeepAspect(w - 2, h - 2, TRUE);

    //キャンバスフレーム枠に対する、プレビュー部分サイズ

    m_szAreaBox.w = (int)((double)m_rcsSrc.w * m_rcsFrame.w / m_nCanvasW + 0.5);
    m_szAreaBox.h = (int)((double)m_rcsSrc.h * m_rcsFrame.h / m_nCanvasH + 0.5);

    //イメージ

    if(m_img.create(w, h))
        m_img.box(0, 0, w, h, 0x000080);
}

//! 描画範囲取得

void CFilterPrev::getDrawRect(RECTANDSIZE *prs)
{
    prs->x1 = m_rcsSrc.x;
    prs->y1 = m_rcsSrc.y;
    prs->x2 = m_rcsSrc.x + m_rcsSrc.w - 1;
    prs->y2 = m_rcsSrc.y + m_rcsSrc.h - 1;
    prs->w  = m_rcsSrc.w;
    prs->h  = m_rcsSrc.h;
}

//! イメージ描画

void CFilterPrev::drawImg(const CLayerImg &img)
{
    img.drawFilterPrev(&m_img, m_rcsSrc);

    redraw();
}


//==============================


//! ドラッグ時、m_rcsSrc の位置セット

BOOL CFilterPrev::_setDragPos(int x,int y)
{
    x = x - 1 - m_rcsFrame.x - m_szAreaBox.w / 2;
    y = y - 1 - m_rcsFrame.y - m_szAreaBox.h / 2;

    x = (int)((double)x * m_nCanvasW / m_rcsFrame.w + 0.5);
    y = (int)((double)y * m_nCanvasH / m_rcsFrame.h + 0.5);

    if(x < 0) x = 0;
    else if(x > m_nCanvasW - m_rcsSrc.w) x = m_nCanvasW - m_rcsSrc.w;

    if(y < 0) y = 0;
    else if(y > m_nCanvasH - m_rcsSrc.h) y = m_nCanvasH - m_rcsSrc.h;

    if(x != m_rcsSrc.x || y != m_rcsSrc.y)
    {
        m_rcsSrc.x = x;
        m_rcsSrc.y = y;
        return TRUE;
    }
    else
        return FALSE;
}

//! ドラッグ中イメージ描画

void CFilterPrev::_drawImgDrag()
{
    int x,y;

    //イメージ

    m_pimgSrc->drawFilterPrev(&m_img, m_rcsSrc);

    //キャンバス枠

    m_img.box(m_rcsFrame.x + 1, m_rcsFrame.y + 1, m_rcsFrame.w, m_rcsFrame.h, 0xff0000);

    //プレビュー部分枠

    x = (int)((double)m_rcsSrc.x * m_rcsFrame.w / m_nCanvasW + m_rcsFrame.x + 0.5);
    y = (int)((double)m_rcsSrc.y * m_rcsFrame.h / m_nCanvasH + m_rcsFrame.y + 0.5);

    if(x + m_szAreaBox.w > m_rcsFrame.x + m_rcsFrame.w)
        x = m_rcsFrame.x + m_rcsFrame.w - m_szAreaBox.w;

    if(y + m_szAreaBox.h > m_rcsFrame.y + m_rcsFrame.h)
        y = m_rcsFrame.y + m_rcsFrame.h - m_szAreaBox.h;

    m_img.box(x + 1, y + 1, m_szAreaBox.w, m_szAreaBox.h, 0x00ff00);

    //

    m_img.put(m_id);
}


//============================
//ハンドラ
//============================


//! 描画

BOOL CFilterPrev::onPaint(AXHD_PAINT *phd)
{
    m_img.put(m_id);
    return TRUE;
}

//! ボタン押し時

BOOL CFilterPrev::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_TEMP1) && m_bScroll)
    {
        m_uFlags |= FLAG_TEMP1;
        grabPointer();

        _setDragPos(phd->x, phd->y);
        _drawImgDrag();
    }

    return TRUE;
}

//! ボタン離し時

BOOL CFilterPrev::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_TEMP1))
    {
        m_uFlags &= ~FLAG_TEMP1;
        ungrabPointer();

        getNotify()->onNotify(this, NOTIFY_UP, 0);
    }

    return TRUE;
}

//! マウス移動時

BOOL CFilterPrev::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_TEMP1)
    {
        if(_setDragPos(phd->x, phd->y))
            _drawImgDrag();
    }

    return TRUE;
}
