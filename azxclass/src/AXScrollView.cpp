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

#include "AXScrollView.h"

#include "AXLayout.h"
#include "AXScrollBar.h"
#include "AXScrollArea.h"
#include "AXEdgeBox.h"
#include "AXAppRes.h"


#define SCRBOXSIZE  15  //スクロールバーのボックスサイズ(AXScrollBarと同じ)


//************************************
// AXScrollView
//************************************

/*!
    @class AXScrollView
    @brief スクロール付きのビューウィジェット

    - AXScrollArea と AXScrollBar を管理するクラス。
    - エリア部分は AXScrollArea を継承したクラスを作成し、親ウィンドウをこのクラスにして new で作成、setScrollArea() でセット。@n
      もしくは、直接 m_pScrArea にセット。
    - スクロールされた場合、AXScrollArea::onNotify() で SAN_SCROLL_HORZ/VERT 通知が来る。@n
      （pwin = AXScrollArea*）
    - AXScrollArea::onSize() 内でスクロール情報を変更すること（ AXScrollBar::setStatus() ）
    - デフォルトで、スクロールバーは自動で表示/非表示する。@n
      ※ AXScrollArea の isShowScroll() を継承して処理することが必要。
    - スクロールの再構成が必要な場合は setReconfig() で。

    @ingroup widget
*/

/*!
    @var AXScrollView::SVS_HORZ
    @brief 水平スクロールバー
    @var AXScrollView::SVS_VERT
    @brief 垂直スクロールバー
    @var AXScrollView::SVS_HORZ_FIX
    @brief 水平スクロールバーを常に表示する（自動で非表示にしない）
    @var AXScrollView::SVS_VERT_FIX
    @brief 垂直スクロールバーを常に表示する（自動で非表示にしない）
    @var AXScrollView::SVS_FRAME
    @brief １px通常枠を付ける
    @var AXScrollView::SVS_SUNKEN
    @brief くぼみ枠を付ける
*/


AXScrollView::~AXScrollView()
{

}

AXScrollView::AXScrollView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createScrollView();
}

AXScrollView::AXScrollView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createScrollView();
}

//! 作成

void AXScrollView::_createScrollView()
{
    m_uType     = TYPE_SCROLLVIEW;

    m_pScrArea  = NULL;
    m_pScrH     = NULL;
    m_pScrV     = NULL;
    m_pEdge     = NULL;
    m_bSetLayout = TRUE;

    //レイアウト

    setLayout(new AXLayoutMatrix(1));

    //水平スクロール

    if(m_uStyle & SVS_HORZ)
        m_pScrH = new AXScrollBar(this, AXScrollBar::SBS_HORZ, LF_EXPAND_W);

    //垂直スクロール

    if(m_uStyle & SVS_VERT)
        m_pScrV = new AXScrollBar(this, AXScrollBar::SBS_VERT, LF_EXPAND_H);

    //余白

    if((m_uStyle & (SVS_HORZ | SVS_VERT)) == (SVS_HORZ | SVS_VERT))
        m_pEdge = new AXEdgeBox(this, 0, LF_COL_W | LF_ROW_H);
}

//! レイアウトセット

void AXScrollView::_setLayout()
{
    BOOL bVert = FALSE, bHorz = FALSE;

    //取り外す

    m_pLayout->removeItemAll();

    //--------- 取り付ける

    //ビュー

    m_pLayout->addItem(m_pScrArea);

    //垂直

    if(m_pScrV && m_pScrV->isVisible())
    {
        m_pLayout->addItem(m_pScrV);
        bVert = TRUE;
    }

    //水平

    if(m_pScrH && m_pScrH->isVisible())
    {
        m_pLayout->addItem(m_pScrH);
        bHorz = TRUE;
    }

    //余白

    if(m_pEdge)
    {
        if(bHorz && bVert)
        {
            m_pLayout->addItem(m_pEdge);
            m_pEdge->show();
        }
        else
            m_pEdge->hide();
    }

    //------------

    //matrix横の数

    m_pLayout->setMatrixColumn(bVert? 2: 1);

    //フラグOFF

    m_bSetLayout = FALSE;
}


//==========================
//レイアウト
//==========================


//! レイアウト

void AXScrollView::layout()
{
    if(m_bSetLayout)
        _setLayout();

    AXWindow::layout();
}

//! 標準サイズ計算

void AXScrollView::calcDefSize()
{
    if(m_bSetLayout)
        _setLayout();

    AXWindow::calcDefSize();
}

//! 範囲取得

void AXScrollView::getClientRect(AXRect *prc)
{
    prc->setFromSize(0, 0, m_nW, m_nH);

    if(m_uStyle & (SVS_FRAME | SVS_SUNKEN))
        prc->deflate(1);
}


//==========================
//ハンドラ
//==========================


//! 内部データ変更などによるスクロール表示/非表示の再構成

void AXScrollView::reconfig()
{
    onSize();

    if(m_bSetLayout)
        layout();
}

//! 通知

BOOL AXScrollView::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pScrH && uNotify == AXScrollBar::SBN_SCROLL)
    {
        //水平スクロール

        m_pScrArea->onNotify(m_pScrArea, AXScrollArea::SAN_SCROLL_HORZ, lParam);
        return TRUE;
    }
    else if(pwin == m_pScrV && uNotify == AXScrollBar::SBN_SCROLL)
    {
        //垂直スクロール

        m_pScrArea->onNotify(m_pScrArea, AXScrollArea::SAN_SCROLL_VERT, lParam);
        return TRUE;
    }

    return FALSE;
}

//! サイズ変更時

BOOL AXScrollView::onSize()
{
    UINT flag = 0;
    BOOL bHorz = FALSE,bVert = FALSE;
    AXRect rc;
    int w,h;

    //※枠がある場合があるので、m_nW/m_nH は使わない

    getClientRect(&rc);

    w = rc.width(), h = rc.height();

    //=========== 指定サイズの場合にスクロールを表示するかどうかの判定

    //水平

    if(m_uStyle & SVS_HORZ_FIX)
        flag |= 3;
    else
    {
        if(m_pScrArea->isShowScroll(w - SCRBOXSIZE, TRUE)) flag |= 1;
        if(m_pScrArea->isShowScroll(w, TRUE)) flag |= 2;
    }

    //垂直

    if(m_uStyle & SVS_VERT_FIX)
        flag |= 12;
    else
    {
        if(m_pScrArea->isShowScroll(h - SCRBOXSIZE, FALSE)) flag |= 4;
        if(m_pScrArea->isShowScroll(h, FALSE)) flag |= 8;
    }

    //============ 判定

    if(m_pScrH && !m_pScrV)
    {
        //--------- 水平のみ

        bHorz = ((flag & 2) != 0);
    }
    else if(!m_pScrH && m_pScrV)
    {
        //--------- 垂直のみ

        bVert = ((flag & 8) != 0);
    }
    else
    {
        //--------- 水平・垂直あり

        //水平

        if((flag & 3) == 3)
            bHorz = TRUE;
        else if((flag & 3) == 0)
            bHorz = FALSE;
        else
        {
            if((flag & 12) == 12)
                bHorz = ((flag & 1) != 0);  //垂直バーあり
            else
                bHorz = ((flag & 2) != 0);  //垂直バーなし
        }

        //垂直

        if((flag & 12) == 12)
            bVert = TRUE;
        else if((flag & 12) == 0)
            bVert = FALSE;
        else
        {
            if((flag & 3) == 3)
                bVert = ((flag & 4) != 0);
            else
                bVert = ((flag & 8) != 0);
        }
    }

    //======== 表示/非表示切り替え

    if(m_pScrH && m_pScrH->isVisible() != bHorz)
    {
        m_pScrH->show(bHorz);
        m_bSetLayout = TRUE;
    }

    if(m_pScrV && m_pScrV->isVisible() != bVert)
    {
        m_pScrV->show(bVert);
        m_bSetLayout = TRUE;
    }

    return TRUE;
}

//! 描画

BOOL AXScrollView::onPaint(AXHD_PAINT *phd)
{
    if(m_uStyle & SVS_FRAME)
        drawBox(0, 0, m_nW, m_nH, AXAppRes::FRAMEDARK);
    else if(m_uStyle & SVS_SUNKEN)
        drawFrameSunken(0, 0, m_nW, m_nH);

    return TRUE;
}


//************************************
// AXScrollArea
//************************************

/*!
    @class AXScrollArea
    @brief AXScrollView の表示エリアの派生元クラス

    - AXScrollView を親にして作成すること。
    - エリアサイズの変更時は、onSize() 内で scrH() / scrV() を使ってスクロールバーの情報を変更すること。@n
      AXScrollView 側ではサイズ変更時にスクロールバー情報の変更は行わない。

    @ingroup widget
*/

/*!
    @var AXScrollArea::SAN_SCROLL_HORZ
    @brief 水平スクロールバーの位置が変化した。lParam = スクロール位置。
    @var AXScrollArea::SAN_SCROLL_VERT
    @brief 垂直スクロールバーの位置が変化した。lParam = スクロール位置。
*/


AXScrollArea::~AXScrollArea()
{

}

AXScrollArea::AXScrollArea(AXWindow *pParent,UINT uStyle)
    : AXWindow(pParent, uStyle, LF_EXPAND_WH)
{

}

//! 水平スクロールバー取得

AXScrollBar *AXScrollArea::scrH()
{
    return ((AXScrollView *)m_pParent)->scrH();
}

//! 垂直スクロールバー取得

AXScrollBar *AXScrollArea::scrV()
{
    return ((AXScrollView *)m_pParent)->scrV();
}

//! 水平スクロールバー位置取得

int AXScrollArea::getHorzPos()
{
    if(((AXScrollView *)m_pParent)->scrH())
        return ((AXScrollView *)m_pParent)->scrH()->getPos();
    else
        return 0;
}

//! 垂直スクロールバー位置取得

int AXScrollArea::getVertPos()
{
    if(((AXScrollView *)m_pParent)->scrV())
        return ((AXScrollView *)m_pParent)->scrV()->getPos();
    else
        return 0;
}

//! 水平スクロールバーの最大値取得

int AXScrollArea::getHorzMax()
{
    if(((AXScrollView *)m_pParent)->scrH())
        return ((AXScrollView *)m_pParent)->scrH()->getMax();
    else
        return 0;
}

//! エリアの幅・高さが指定サイズの場合にスクロールを表示するか
/*!
    AXScrollView がスクロールバーを自動で表示/非表示する際の判定に使われる。

    @param size pxサイズ
    @param bHorz TRUEで水平バーの場合。FALSEで垂直バーの場合。
    @return スクロールバーを表示するかどうか
*/

BOOL AXScrollArea::isShowScroll(int size,BOOL bHorz)
{
    return TRUE;
}
