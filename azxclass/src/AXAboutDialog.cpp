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

#include "AXAboutDialog.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXButton.h"
#include "AXTrSysID.h"


/*!
    @class AXAboutDialog
    @brief ソフト情報ダイアログ

    @ingroup dialog
*/


AXAboutDialog::~AXAboutDialog()
{

}

AXAboutDialog::AXAboutDialog(AXWindow *pOwner)
    : AXDialog(pOwner,
               WS_TITLE | WS_CLOSE | WS_BORDER | WS_MENUBTT | WS_TABMOVE |
               WS_HIDE | WS_TRANSIENT_FOR | WS_BK_FACE | WS_DISABLE_IM)
{
    AXLayout *pl;

    setTitle("about");

    pl = new AXLayoutVert(0, 6);
    pl->setSpacing(10);

    setLayout(pl);
}

//! ラベル追加

void AXAboutDialog::addLabel(LPCSTR szText)
{
    getLayout()->addItem(new AXLabel(this, 0, LF_EXPAND_X | LF_CENTER_X, 0, szText));
}

//! 実行

void AXAboutDialog::run()
{
    AXButton *pbt;

    //OKボタン追加

    pbt = new AXButton(this, 0, LF_EXPAND_X | LF_CENTER_X, 100, MAKE_DW4(0,10,0,0), TRSYS_GROUPID, TRSYS_OK);
    getLayout()->addItem(pbt);

    setDefaultButton(pbt);

    //実行

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();

    runDialog();
}

//! 通知

BOOL AXAboutDialog::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == getDefaultButton())
        endDialog(0);

    return TRUE;
}


//===========================
//関数
//===========================


//! ダイアログ表示関数

void AXAboutDialog::about(AXWindow *pOwner,LPCSTR szSoftName,LPCSTR szCopyright)
{
    AXAboutDialog *pdlg = new AXAboutDialog(pOwner);

    pdlg->addLabel(szSoftName);
    pdlg->addLabel(szCopyright);

    pdlg->run();
}
