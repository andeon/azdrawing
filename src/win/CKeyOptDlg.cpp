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

#include "CKeyOptDlg.h"

#include "CCmdListTreeView.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXButton.h"
#include "AXMessageBox.h"
#include "AXApp.h"
#include "AXTrSysID.h"

#include "defStrID.h"


//------------------------

#define WID_BTT_ALLCLEAR    100

//------------------------


/*!
    @class CKeyOptDlg
    @brief キー設定ダイアログ
*/


CKeyOptDlg::CKeyOptDlg(AXWindow *pOwner)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl;
    AXWindow *pwin;

    _trgroup(STRGID_DLG_KEYOPT);

    setTitle(STRID_KEYOPT_TITLE);

    setLayout(plTop = new AXLayoutVert(0, 8));
    plTop->setSpacing(10);

    //ツリー+TIPS

    plTop->addItem(pl = new AXLayoutHorz(LF_EXPAND_WH, 8));

    pl->addItem(m_pTree = new CCmdListTreeView(this, CCmdListTreeView::STYLE_KEY, LF_EXPAND_WH, 0, 0));
    pl->addItem(new AXLabel(this, AXLabel::LS_SUNKEN, 0, 0, STRID_KEYOPT_TIPS));

    //ボタン

    plTop->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 0));

    pl->addItem(new AXButton(this, 0, LF_EXPAND_X, WID_BTT_ALLCLEAR, MAKE_DW4(0,0,4,0), STRID_KEYOPT_ALLCLEAR));
    pl->addItem(pwin = new AXButton(this, 0, 0, 1, MAKE_DW4(0,0,4,0), TRSYS_GROUPID, TRSYS_OK));
    pl->addItem(new AXButton(this, 0, 0, 2, 0, TRSYS_GROUPID, TRSYS_CANCEL));

    setDefaultButton(pwin);

    //

    m_pTree->setCmdList();

    //

    calcDefSize();
    resize((m_nDefW < 500)? 500: m_nDefW, (m_nDefH < 400)? 400: m_nDefH);

    show();
}

//! 通知

BOOL CKeyOptDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //全てクリア
        case WID_BTT_ALLCLEAR:
            if(AXMessageBox::message(this, NULL, _string(STRGID_MESSAGE, STRID_MES_CLEARALL),
                                     AXMessageBox::YES | AXMessageBox::NO,
                                     AXMessageBox::YES) == AXMessageBox::YES)
                m_pTree->clearAll();
            break;

        //OK
        case 1:
            m_pTree->setKeyDat();

            endDialog(1);
            break;
        //キャンセル
        case 2:
            endDialog(0);
            break;
    }

    return TRUE;
}
