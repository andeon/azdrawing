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

#include "AXStrDialog.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXLineEdit.h"


/*!
    @class AXStrDialog
    @brief 一つの一行エディットで、文字列を取得するダイアログ

    @ingroup dialog
*/



AXStrDialog::AXStrDialog(AXWindow *pOwner,LPCUSTR pTitle,LPCUSTR pMessage,AXString *pstr)
    : AXDialog(pOwner,
               WS_TITLE | WS_CLOSE | WS_BORDER | WS_MENUBTT | WS_TABMOVE |
               WS_HIDE | WS_TRANSIENT_FOR | WS_BK_FACE)
{
    AXLayout *plTop,*pl;

    m_pstrRet = pstr;

    if(pTitle)
        setTitle(pTitle);

    //

    setLayout(plTop = new AXLayoutVert(0, 6));
    plTop->setSpacing(10);

    //メッセージ

    plTop->addItem(new AXLabel(this, 0, 0, 0, pMessage));

    //エディット

    plTop->addItem(m_pEdit = new AXLineEdit(this, 0, LF_EXPAND_W));

    m_pEdit->setText(*m_pstrRet);

    //OKキャンセル

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(10);

    //

    m_pEdit->setFocus();
    m_pEdit->selectAll();

    //

    calcDefSize();
    resize((m_nDefW < 300)? 300: m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL AXStrDialog::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //OK
        case 1:
            m_pEdit->getText(m_pstrRet);

            endDialog(TRUE);
            break;
        //キャンセル
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}



//! ダイアログ表示関数
/*!
    @param pstr 結果の文字列が入る。デフォルトの文字列を指定したい場合はここにセットしておく。
*/

BOOL AXStrDialog::getString(AXWindow *pOwner,LPCUSTR pTitle,LPCUSTR pMessage,AXString *pstr)
{
    AXStrDialog *pdlg = new AXStrDialog(pOwner, pTitle, pMessage, pstr);

    return pdlg->runDialog();
}
