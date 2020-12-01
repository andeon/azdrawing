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

#include "AXPopupListBox.h"

#include "AXListBoxItem.h"
#include "AXListBoxItemManager.h"
#include "AXScrollView.h"
#include "AXListBoxArea.h"
#include "AXLayout.h"
#include "AXKey.h"
#include "AXApp.h"


/*!
    @class AXPopupListBox
    @brief ポップアップのリストボックス（コンボボックス用）

    - オーナーウィンドウは、コンボボックス。
    - 選択の変更は AXListBoxItemManager の方にセットされる。
    - オーナードローはオーナーウィンドウの通知ウィンドウで実行される。

    @ingroup window
*/

/*!
    @var AXPopupListBox::PLBS_OWNERDRAW
    @brief リストボックスアイテムをオーナードロー
    @var AXPopupListBox::PLBS_SCRV
    @brief 垂直スクロールを付ける
*/


AXPopupListBox::~AXPopupListBox()
{

}

AXPopupListBox::AXPopupListBox(AXWindow *pOwner,UINT uStyle,AXListBoxItemManager *pDat,
                        int x,int y,int w,int h,int nItemH)
    : AXPopupWindow(pOwner->getTopLevel(), uStyle | WS_DISABLE_IM)
{
    AXLayout *pl;
    AXScrollView *pView;

    m_pDat      = pDat;
    m_pFont     = pOwner->getFont();    //コンボボックスと同じフォント
    m_nBeginSel = pDat->getSel();       //開始時の選択

    //レイアウト

    pl = new AXLayoutVert;
    setLayout(pl);

    //リストボックス

    pView = new AXScrollView(this,
                        AXScrollView::SVS_FRAME | ((uStyle & PLBS_SCRV)? AXScrollView::SVS_VERT: 0),
                        LF_EXPAND_WH);

    pView->setTakeFocus();

    m_pArea = new AXListBoxArea(pView, pOwner,
                    AXListBoxArea::LBAS_POPUP | ((uStyle & PLBS_OWNERDRAW)? AXListBoxArea::LBAS_OWNERDRAW: 0),
                    m_pDat);

    m_pArea->setItemH(nItemH);
    m_pArea->setNotify(this);

    pView->setScrollArea(m_pArea);

    pl->addItem(pView);

    //サイズ＋レイアウト
    //（高さは、枠分を足す）

    resize(w, h + 2);

    //選択が表示されるように

    m_pArea->adjustScrVSel(0);

    //フォーカス

    pView->setFocus();

    //実行

    runPopup(x, y);

    //削除

    delete this;

    axapp->sync();
}

//! 終了

void AXPopupListBox::endPopup(BOOL bCancel)
{
    //キャンセル時は元の選択に戻す

    if(bCancel)
        m_pDat->setSel(m_nBeginSel);

    AXPopupWindow::endPopup(bCancel);
}

//! 通知

BOOL AXPopupListBox::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    //選択時終了

    if(pwin == m_pArea && uNotify == AXListBoxArea::LBAN_POPUPEND)
        endPopup(FALSE);

    return TRUE;
}

//! キー押し

BOOL AXPopupListBox::onKeyDown(AXHD_KEY *phd)
{
    //スペース、ENTER 時は決定して終了

    if(ISKEY_SPACE(phd->keysym) || ISKEY_ENTER(phd->keysym))
    {
        endPopup(FALSE);
        return TRUE;
    }

    //ESCキーはキャンセル

    if(phd->keysym == KEY_ESCAPE)
    {
        endPopup(TRUE);
        return TRUE;
    }

    //他キーはリストへ

    return m_pArea->onKeyDown(phd);
}
