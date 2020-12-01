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

#include "CResizeCanvasDlg.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXApp.h"

#include "defStrID.h"


//------------------------

#define WID_CK_ALIGN    100

//------------------------


/*!
    @class CResizeCanvasDlg
    @brief キャンバスサイズ変更ダイアログ
*/


CResizeCanvasDlg::CResizeCanvasDlg(AXWindow *pOwner,int *pVal)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl,*pl2;
    int i;
    AXCheckButton *pck;

    m_pVal = pVal;

    //

    _trgroup(STRGID_DLG_RESIZECANVAS);

    setTitle(STRID_RCDLG_TITLE);

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(10);

    //

    plTop->addItem(pl = new AXLayoutMatrix(2));

    //幅

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,8,7), _string(STRGID_WORD, STRID_WORD_WIDTH)));
    pl->addItem(m_peditW = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,7)));

    m_peditW->setInit(6, 1, 9999, pVal[0]);

    //高さ

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,8,7), _string(STRGID_WORD, STRID_WORD_HEIGHT)));
    pl->addItem(m_peditH = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,7)));

    m_peditH->setInit(6, 1, 9999, pVal[1]);

    //配置

    pl->addItem(new AXLabel(this, 0, LF_RIGHT, MAKE_DW4(0,0,8,0), STRID_RCDLG_ALIGN));
    pl->addItem(pl2 = new AXLayoutMatrix(3));

    for(i = 0; i < 9; i++)
    {
        pl2->addItem(pck = new AXCheckButton(this,
                                    AXCheckButton::CBS_BUTTON|AXCheckButton::CBS_RADIO|AXCheckButton::CBS_REALSIZE,
                                    0, WID_CK_ALIGN + i, 0, (LPCUSTR)NULL, (i == 0)));

        pck->setMinSize(30, 30);

        if(i == 0)
            m_pckAlign = pck;
    }

    //-------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //

    m_peditW->setFocus();
    m_peditW->selectAll();

    //

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CResizeCanvasDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //OK
        case 1:
            m_pVal[0] = m_peditW->getVal();
            m_pVal[1] = m_peditH->getVal();
            m_pVal[2] = m_pckAlign->getGroupSel();

            endDialog(1);
            break;
        //キャンセル
        case 2:
            endDialog(0);
            break;
    }

    return TRUE;
}
