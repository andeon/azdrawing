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

#include "CGridOptDlg.h"

#include "CConfig.h"

#include "AXLayout.h"
#include "AXSpacerItem.h"
#include "AXGroupBox.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXColorButton.h"
#include "AXArrowButton.h"
#include "AXMenu.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"


//-----------------------

#define WID_GRIDMENU    100

//-----------------------

/*!
    @class CGridOptDlg
    @brief グリッド設定ダイアログ
*/


CGridOptDlg::CGridOptDlg(AXWindow *pOwner)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*plh,*pl;
    AXWindow *pGroup;

    _trgroup(STRGID_DLG_GRIDOPT);

    setTitle(STRID_GRIDOPT_TITLE);

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(10);

    plTop->addItem(plh = new AXLayoutHorz(0, 8));

    //------- グリッド

    plh->addItem(pGroup = new AXGroupBox(this, 0, 0, 0, _str(STRID_GRIDOPT_GRID)));

    pGroup->setLayout(pl = new AXLayoutMatrix(3));
    pl->setSpacing(8);

    //幅

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,6), _string(STRGID_WORD, STRID_WORD_WIDTH)));
    pl->addItem(m_peditGridW = new AXLineEdit(pGroup, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,5,6)));
    pl->addItem(new AXArrowButton(pGroup, AXArrowButton::ARBTS_DOWN, LF_CENTER_Y, WID_GRIDMENU, MAKE_DW4(0,0,0,6)));

    m_peditGridW->setInit(5, 5, 9999, g_pconf->nGridW);

    //高さ

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,6), _string(STRGID_WORD, STRID_WORD_HEIGHT)));
    pl->addItem(m_peditGridH = new AXLineEdit(pGroup, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,6)));
    pl->addItem(new AXSpacerItem);

    m_peditGridH->setInit(5, 5, 9999, g_pconf->nGridH);

    //色

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,6), _string(STRGID_WORD, STRID_WORD_COLOR)));
    pl->addItem(m_pbtGrid = new AXColorButton(pGroup, AXColorButton::CBTS_CHOOSE, 0, 0, MAKE_DW4(0,0,0,6), g_pconf->dwGridCol));
    pl->addItem(new AXSpacerItem);

    //濃度

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,0), _string(STRGID_WORD, STRID_WORD_OPACITY)));
    pl->addItem(m_peditGridO = new AXLineEdit(pGroup, AXLineEdit::ES_SPIN, 0));
    pl->addItem(new AXLabel(pGroup, 0, LF_CENTER_Y, MAKE_DW4(2,0,0,0), "[1-255]"));

    m_peditGridO->setInit(5, 1, 255, g_pconf->dwGridCol >> 24);

    //------- 分割線

    plh->addItem(pGroup = new AXGroupBox(this, 0, 0, 0, _str(STRID_GRIDOPT_GRIDSPLIT)));

    pGroup->setLayout(pl = new AXLayoutMatrix(2));
    pl->setSpacing(8);

    //幅

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,6), STRID_GRIDOPT_COLSPLIT));
    pl->addItem(m_peditSplitX = new AXLineEdit(pGroup, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,6)));

    m_peditSplitX->setInit(5, 2, 20, g_pconf->nGridSplitX);

    //高さ

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,6), STRID_GRIDOPT_ROWSPLIT));
    pl->addItem(m_peditSplitY = new AXLineEdit(pGroup, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,6)));

    m_peditSplitY->setInit(5, 2, 20, g_pconf->nGridSplitY);

    //色

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,6), _string(STRGID_WORD, STRID_WORD_COLOR)));
    pl->addItem(m_pbtSplit = new AXColorButton(pGroup, AXColorButton::CBTS_CHOOSE, 0, 0, MAKE_DW4(0,0,0,6), g_pconf->dwGridSplitCol));

    //濃度

    pl->addItem(new AXLabel(pGroup, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,0), _string(STRGID_WORD, STRID_WORD_OPACITY)));
    pl->addItem(m_peditSplitO = new AXLineEdit(pGroup, AXLineEdit::ES_SPIN, 0));

    m_peditSplitO->setInit(5, 1, 255, g_pconf->dwGridSplitCol >> 24);

    //-------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CGridOptDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //登録リスト
        case WID_GRIDMENU:
            _runMenu();
            break;

        //OK
        case 1:
            g_pconf->nGridW     = m_peditGridW->getVal();
            g_pconf->nGridH     = m_peditGridH->getVal();
            g_pconf->dwGridCol  = m_pbtGrid->getColor() | ((DWORD)m_peditGridO->getVal() << 24);

            g_pconf->nGridSplitX    = m_peditSplitX->getVal();
            g_pconf->nGridSplitY    = m_peditSplitY->getVal();
            g_pconf->dwGridSplitCol = m_pbtSplit->getColor() | ((DWORD)m_peditSplitO->getVal() << 24);

            endDialog(TRUE);
            break;
        //キャンセル
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}

//! 登録メニュー

void CGridOptDlg::_runMenu()
{
    AXMenu *pm;
    int i;
    AXString str;
    AXRectSize rcs;

    pm = new AXMenu;

    for(i = 0; i < CConfig::GRIDLIST_NUM; i++)
    {
        str = '[';
        str += (i + 1);
        str += "] ";
        str += (int)(g_pconf->dwGridList[i] >> 16);
        str += 'x';
        str += (int)(g_pconf->dwGridList[i] & 0xffff);

        pm->add(1000 + i, str);
    }

    pm->addSep();

    for(i = 0; i < CConfig::GRIDLIST_NUM; i++)
    {
        str = "set -> [";
        str += (i + 1);
        str += ']';

        pm->add(2000 + i, str);
    }

    //

    getWidget(WID_GRIDMENU)->getWindowRect(&rcs);

    i = (int)pm->popup(NULL, rcs.x, rcs.y + rcs.h, 0);

    delete pm;

    //

    if(i != -1)
    {
        if(i >= 2000)
        {
            //リストに登録

            i -= 2000;

            g_pconf->dwGridList[i] = (m_peditGridW->getVal() << 16) | m_peditGridH->getVal();
        }
        else
        {
            //リストから呼び出し

            i -= 1000;

            m_peditGridW->setVal(g_pconf->dwGridList[i] >> 16);
            m_peditGridH->setVal(g_pconf->dwGridList[i] & 0xffff);
        }
    }
}
