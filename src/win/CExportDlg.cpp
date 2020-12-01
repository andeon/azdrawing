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

#include "CExportDlg.h"

#include "AXLayout.h"
#include "AXSpacerItem.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXComboBox.h"
#include "AXGroupBox.h"
#include "AXApp.h"

#include "defStrID.h"


//------------------------

#define WID_CB_FORMAT   100
#define WID_CK_PERS     101
#define WID_EDIT_PERS   102
#define WID_CK_ASPECT   103
#define WID_EDIT_WIDTH  104
#define WID_EDIT_HEIGHT 105

//------------------------


/*!
    @class CExportDlg
    @brief 画像エクスポートダイアログ
*/


CExportDlg::CExportDlg(AXWindow *pOwner,VALUE *pVal)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl;
    AXGroupBox *pGB;

    m_pVal = pVal;

    m_bPers = FALSE;
    m_bKeepAspect = TRUE;

    //

    _trgroup(STRGID_DLG_EXPORT);

    setTitle(STRID_EXPORT_TITLE);

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(10);

    //保存形式

    plTop->addItem(new AXLabel(this, 0, 0, MAKE_DW4(0,0,0,4), STRID_EXPORT_FORMAT));

    plTop->addItem(m_pcbFormat = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_FORMAT, MAKE_DW4(0,0,0,10)));

    _trgroup(STRGID_MAINMENU);

    m_pcbFormat->addItemMulTr(STRID_MENU_FILE_FORMAT_ADW, 9);
    m_pcbFormat->setAutoWidth();
    m_pcbFormat->setCurSel(m_pVal->nFormat);

    _trgroup(STRGID_DLG_EXPORT);

    //------- 拡大縮小

    plTop->addItem(pGB = new AXGroupBox(this, 0, LF_EXPAND_W, 0, _str(STRID_EXPORT_SCALE)));

    pGB->setLayout(pl = new AXLayoutMatrix(2));
    pl->setSpacing(8);

    //幅

    pl->addItem(new AXLabel(pGB, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,6), _string(STRGID_WORD, STRID_WORD_WIDTH)));
    pl->addItem(m_peditW = new AXLineEdit(pGB, AXLineEdit::ES_SPIN, 0, WID_EDIT_WIDTH, MAKE_DW4(0,0,0,6)));

    m_peditW->setInit(6, 1, 9999, m_pVal->nWidth);

    //高さ

    pl->addItem(new AXLabel(pGB, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,6), _string(STRGID_WORD, STRID_WORD_HEIGHT)));
    pl->addItem(m_peditH = new AXLineEdit(pGB, AXLineEdit::ES_SPIN, 0, WID_EDIT_HEIGHT, MAKE_DW4(0,0,0,6)));

    m_peditH->setInit(6, 1, 9999, m_pVal->nHeight);

    //縦横比維持

    pl->addItem(new AXSpacerItem);
    pl->addItem(new AXCheckButton(pGB, 0, 0, WID_CK_ASPECT, MAKE_DW4(0,0,0,6), STRID_EXPORT_KEEPASPECT, m_bKeepAspect));

    //倍率

    pl->addItem(m_pckPers = new AXCheckButton(pGB, 0, LF_RIGHT|LF_CENTER_Y, WID_CK_PERS, MAKE_DW4(0,0,6,6), STRID_EXPORT_PERS, FALSE));
    pl->addItem(m_peditPers = new AXLineEdit(pGB, AXLineEdit::ES_SPIN, 0, WID_EDIT_PERS, MAKE_DW4(0,0,0,6)));

    m_peditPers->setInit(6, 1, 10000, 1, 1000);

    //補間方法

    pl->addItem(new AXLabel(pGB, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,0), STRID_EXPORT_SCALETYPE));
    pl->addItem(m_pcbType = new AXComboBox(pGB, 0, 0));

    _trgroup(STRGID_SCALETYPE);

    m_pcbType->addItemMulTr(0, 2);
    m_pcbType->setAutoWidth();
    m_pcbType->setCurSel(m_pVal->nScaleType);

    //-------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //

    _enable_scale((m_pVal->nFormat >= 3 && m_pVal->nFormat <= 6));

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CExportDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //保存形式
        case WID_CB_FORMAT:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                int sel = m_pcbFormat->getCurSel();

                _enable_scale((sel >= 3 && sel <= 6));
            }
            break;
        //幅
        case WID_EDIT_WIDTH:
            if(m_bKeepAspect && uNotify == AXLineEdit::EN_CHANGE)
                m_peditH->setVal((int)((double)m_peditW->getVal() / m_pVal->nWidth * m_pVal->nHeight + 0.5));
            break;
        //高さ
        case WID_EDIT_HEIGHT:
            if(m_bKeepAspect && uNotify == AXLineEdit::EN_CHANGE)
                m_peditW->setVal((int)((double)m_peditH->getVal() / m_pVal->nHeight * m_pVal->nWidth + 0.5));
            break;
        //縦横比維持
        case WID_CK_ASPECT:
            m_bKeepAspect ^= 1;
            break;
        //倍率チェック
        case WID_CK_PERS:
            m_bPers ^= 1;

            _enable_scale(TRUE);
            break;
        //倍率エディット
        case WID_EDIT_PERS:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                int n = m_peditPers->getVal();

                m_peditW->setVal((int)((double)m_pVal->nWidth * n / 1000.0 + 0.5));
                m_peditH->setVal((int)((double)m_pVal->nHeight * n / 1000.0 + 0.5));
            }
            break;

        //OK
        case 1:
            m_pVal->nFormat    = m_pcbFormat->getCurSel();
            m_pVal->nWidth     = m_peditW->getVal();
            m_pVal->nHeight    = m_peditH->getVal();
            m_pVal->nScaleType = m_pcbType->getCurSel();

            endDialog(1);
            break;
        //キャンセル
        case 2:
            endDialog(0);
            break;
    }

    return TRUE;
}


//! 縮小のウィジェットを有効/無効

void CExportDlg::_enable_scale(BOOL bEnable)
{
    m_peditW->enable(bEnable && !m_bPers);
    m_peditH->enable(bEnable && !m_bPers);
    m_pckPers->enable(bEnable);
    m_peditPers->enable(bEnable && m_bPers);
    m_pcbType->enable(bEnable);
}
