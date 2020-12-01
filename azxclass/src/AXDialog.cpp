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

#include "AXDialog.h"

#include "AXLayout.h"
#include "AXButton.h"
#include "AXApp.h"
#include "AXKey.h"
#include "AXTrSysID.h"


/*!
    @class AXDialog
    @brief ダイアログウィンドウクラス

    - ダイアログ戻り値はデフォルトで 0。

    @ingroup window
*/


AXDialog::~AXDialog()
{

}

AXDialog::AXDialog(AXWindow *pOwner,UINT uStyle)
    : AXTopWindow(pOwner, uStyle)
{
    m_nDialogRet = 0;

    setWindowType(axapp->getAtom("_NET_WM_WINDOW_TYPE_DIALOG"));
}

//! ダイアログ実行
/*!
    ※レイアウトや表示はあらかじめ行なっておくこと
    @param bDelete TRUEで、終了後自身を delete する
*/

int AXDialog::runDialog(BOOL bDelete)
{
    int ret;

    //実行

    axapp->runDlg(this);

    //終了

    ret = m_nDialogRet;

    if(bDelete)
    {
        delete this;

        axapp->sync();
    }

//    if(m_pOwner) m_pOwner->setActive();

    return ret;
}

//! ダイアログ終了

void AXDialog::endDialog(int ret)
{
    m_nDialogRet = ret;

    axapp->exit();
}

//! OK/キャンセルボタンを作成
/*!
    アイテムID : [1] OK [2] キャンセル

    @param minWidth 最小幅（負の値でデフォルト）
    @return 作成された水平レイアウトが返る
*/

AXLayout *AXDialog::createOKCancelButton(int minWidth)
{
    AXLayout *pl;
    AXButton *pOK,*pCancel;

    pl = new AXLayoutHorz;
    pl->setLayoutFlags(LF_RIGHT);

    pOK     = new AXButton(this, 0, 0, 1, MAKE_DW4(0,0,4,0), TRSYS_GROUPID, TRSYS_OK);
    pCancel = new AXButton(this, 0, 0, 2, 0, TRSYS_GROUPID, TRSYS_CANCEL);

    pl->addItem(pOK);
    pl->addItem(pCancel);

    if(minWidth > 0)
    {
        pOK->setMinWidth(minWidth);
        pCancel->setMinWidth(minWidth);
    }

    setDefaultButton(pOK);

    return pl;
}

//! OK・キャンセルボタンを作成し、デフォルトサイズで表示
/*!
    @param paddingTop 上のレイアウトアイテムとの余白
*/

void AXDialog::createOKCancelAndShow(int paddingTop)
{
    AXLayout *pl;

    if(m_pLayout)
    {
        m_pLayout->addItem(pl = createOKCancelButton());
        pl->setPaddingTop(paddingTop);
    }

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}


//================================
//ハンドラ
//================================


//! キー押し

BOOL AXDialog::onKeyDown(AXHD_KEY *phd)
{
    if(AXTopWindow::onKeyDown(phd))
        return TRUE;

    //ESC で閉じる

    if(!axapp->isGrab() && phd->keysym == KEY_ESCAPE)
    {
        onClose();
        return TRUE;
    }

    return FALSE;
}
