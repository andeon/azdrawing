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

#include "CScaleCanvasDlg.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXComboBox.h"
#include "AXGroupBox.h"
#include "AXApp.h"

#include "defStrID.h"


//---------------------

#define WID_EDIT_SIZEW      100
#define WID_EDIT_SIZEH      101
#define WID_EDIT_PERSW      102
#define WID_EDIT_PERSH      103
#define WID_CK_CHANGEDPI    104
#define WID_EDIT_DPI        105

//---------------------


/*!
    @class CScaleCanvasDlg
    @brief キャンバス拡大縮小ダイアログ
*/


CScaleCanvasDlg::CScaleCanvasDlg(AXWindow *pOwner,VALUE *pVal)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*plh,*pl;
    AXWindow *pGB;

    m_pVal = pVal;

    m_szPix.w = pVal->nWidth;
    m_szPix.h = pVal->nHeight;
    m_szPer.w = m_szPer.h = 1000;

    //

    _trgroup(STRGID_DLG_SCALECANVAS);

    setTitle(STRID_SCDLG_TITLE);

    setLayout(plTop = new AXLayoutVert(0, 8));
    plTop->setSpacing(8);

    //------ サイズと%

    plTop->addItem(plh = new AXLayoutHorz(0, 8));

    //サイズ

    plh->addItem(pGB = new AXGroupBox(this, 0, 0, 0, _str(STRID_SCDLG_SIZE)));

    pGB->setLayout(pl = new AXLayoutMatrix(2));
    pl->setSpacing(8);

    pl->addItem(new AXLabel(pGB, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,6), _string(STRGID_WORD, STRID_WORD_WIDTH)));
    pl->addItem(m_peditSizeW = new AXLineEdit(pGB, AXLineEdit::ES_SPIN, 0, WID_EDIT_SIZEW, MAKE_DW4(0,0,0,6)));

    pl->addItem(new AXLabel(pGB, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,0), _string(STRGID_WORD, STRID_WORD_HEIGHT)));
    pl->addItem(m_peditSizeH = new AXLineEdit(pGB, AXLineEdit::ES_SPIN, 0, WID_EDIT_SIZEH, 0));

    m_peditSizeW->setInit(5, 1, 9999, pVal->nWidth);
    m_peditSizeH->setInit(5, 1, 9999, pVal->nHeight);

    //%

    plh->addItem(pGB = new AXGroupBox(this, 0, 0, 0, _str(STRID_SCDLG_PERS)));

    pGB->setLayout(pl = new AXLayoutMatrix(2));
    pl->setSpacing(8);

    pl->addItem(new AXLabel(pGB, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,6), _string(STRGID_WORD, STRID_WORD_WIDTH)));
    pl->addItem(m_peditPersW = new AXLineEdit(pGB, AXLineEdit::ES_SPIN, 0, WID_EDIT_PERSW, MAKE_DW4(0,0,0,6)));

    pl->addItem(new AXLabel(pGB, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,0), _string(STRGID_WORD, STRID_WORD_HEIGHT)));
    pl->addItem(m_peditPersH = new AXLineEdit(pGB, AXLineEdit::ES_SPIN, 0, WID_EDIT_PERSH, 0));

    m_peditPersW->setInit(6, 1, 20000, 1, 1000);
    m_peditPersH->setInit(6, 1, 20000, 1, 1000);

    //--------

    //縦横比維持

    plTop->addItem(m_pckAspect = new AXCheckButton(this, 0, LF_RIGHT, 0, 0, STRID_SCDLG_KEEPASPECT, TRUE));

    //---------

    plTop->addItem(pl = new AXLayoutMatrix(2));

    //DPI変更

    pl->addItem(m_pckDPI = new AXCheckButton(this, 0, LF_CENTER_Y, WID_CK_CHANGEDPI, MAKE_DW4(0,0,8,6), STRID_SCDLG_CHANGEDPI, FALSE));
    pl->addItem(m_peditDPI = new AXLineEdit(this, WS_DISABLE|AXLineEdit::ES_SPIN, LF_CENTER_Y, WID_EDIT_DPI, MAKE_DW4(0,0,0,6)));

    m_peditDPI->setInit(6, 1, 10000, pVal->nDPI);

    //補間方法

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,8,0), STRID_SCDLG_TYPE));
    pl->addItem(m_pcbType = new AXComboBox(this, 0, 0));

    _trgroup(STRGID_SCALETYPE);

    m_pcbType->addItemMulTr(0, 2);
    m_pcbType->setAutoWidth();
    m_pcbType->setCurSel(pVal->nType);

    //-------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(6);

    //

    m_peditSizeW->setFocus();
    m_peditSizeW->selectAll();

    //

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CScaleCanvasDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //サイズ幅
        case WID_EDIT_SIZEW:
            if(uNotify == AXLineEdit::EN_CHANGE)
                _changeVal(0);
            break;
        //サイズ高さ
        case WID_EDIT_SIZEH:
            if(uNotify == AXLineEdit::EN_CHANGE)
                _changeVal(1);
            break;
        //%幅
        case WID_EDIT_PERSW:
            if(uNotify == AXLineEdit::EN_CHANGE)
                _changeVal(2);
            break;
        //%高さ
        case WID_EDIT_PERSH:
            if(uNotify == AXLineEdit::EN_CHANGE)
                _changeVal(3);
            break;
        //DPI変更
        case WID_CK_CHANGEDPI:
            _setChangeDPI();
            break;
        //DPI値
        case WID_EDIT_DPI:
            _changeDPI();
            break;

        //OK
        case 1:
            m_pVal->nWidth  = m_peditSizeW->getVal();
            m_pVal->nHeight = m_peditSizeH->getVal();
            m_pVal->nDPI    = (m_pckDPI->isChecked())? m_peditDPI->getVal(): -1;
            m_pVal->nType   = m_pcbType->getCurSel();

            endDialog(1);
            break;
        //キャンセル
        case 2:
            endDialog(0);
            break;
    }

    return TRUE;
}


//======================


//! DPI変更チェック時

void CScaleCanvasDlg::_setChangeDPI()
{
    int f = !m_pckDPI->isChecked();

    m_peditSizeW->enable(f);
    m_peditSizeH->enable(f);
    m_peditPersW->enable(f);
    m_peditPersH->enable(f);

    m_peditDPI->enable(!f);

    if(!f) _changeDPI();
}

//! DPI値変更時

void CScaleCanvasDlg::_changeDPI()
{
    double scale;

    scale = (double)m_peditDPI->getVal() / m_pVal->nDPI;

    m_szPix.w = (int)(m_pVal->nWidth * scale + 0.5);
    m_szPix.h = (int)(m_pVal->nHeight * scale + 0.5);
    m_szPer.w = m_szPer.h = (int)(scale * 1000.0 + 0.5);

    _setVal(-1);
}

//! エディットに値セット
/*!
    @param type セットしない値
*/

void CScaleCanvasDlg::_setVal(int type)
{
    if(type != 0) m_peditSizeW->setVal(m_szPix.w);
    if(type != 1) m_peditSizeH->setVal(m_szPix.h);
    if(type != 2) m_peditPersW->setVal(m_szPer.w);
    if(type != 3) m_peditPersH->setVal(m_szPer.h);
}

//! エディット値変更時

void CScaleCanvasDlg::_changeVal(int type)
{
    BOOL bKeep = m_pckAspect->isChecked();

    switch(type)
    {
        //サイズ幅
        case 0:
            m_szPix.w = m_peditSizeW->getVal();
            m_szPer.w = (int)((double)m_szPix.w / m_pVal->nWidth * 1000.0 + 0.5);

            if(bKeep)
            {
                m_szPix.h = (int)((double)m_szPix.w / m_pVal->nWidth * m_pVal->nHeight + 0.5);
                m_szPer.h = m_szPer.w;
            }
            break;
        //サイズ高さ
        case 1:
            m_szPix.h = m_peditSizeH->getVal();
            m_szPer.h = (int)((double)m_szPix.h / m_pVal->nHeight * 1000.0 + 0.5);

            if(bKeep)
            {
                m_szPix.w = (int)((double)m_szPix.h / m_pVal->nHeight * m_pVal->nWidth + 0.5);
                m_szPer.w = m_szPer.h;
            }
            break;
        //%幅
        case 2:
            m_szPer.w = m_peditPersW->getVal();
            m_szPix.w = (int)((double)m_szPer.w / 1000.0 * m_pVal->nWidth + 0.5);

            if(bKeep)
            {
                m_szPer.h = m_szPer.w;
                m_szPix.h = (int)((double)m_szPer.h / 1000.0 * m_pVal->nHeight + 0.5);
            }
            break;
        //%高さ
        case 3:
            m_szPer.h = m_peditPersH->getVal();
            m_szPix.h = (int)((double)m_szPer.h / 1000.0 * m_pVal->nHeight + 0.5);

            if(bKeep)
            {
                m_szPer.w = m_szPer.h;
                m_szPix.w = (int)((double)m_szPer.w / 1000.0 * m_pVal->nWidth + 0.5);
            }
            break;
    }

    _setVal(type);
}
