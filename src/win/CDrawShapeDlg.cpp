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

#include "CDrawShapeDlg.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXComboBox.h"
#include "AXApp.h"

#include "defStrID.h"


//------------------------

#define WID_CB_SHAPE        100
#define WID_CB_SUB          101
#define WID_CK_ANTIALIAS    102
#define WID_CK_PX           103
#define WID_CK_CM           104

//------------------------


/*!
    @class CDrawShapeDlg
    @brief 座標を指定して図形描画ダイアログ
*/


CDrawShapeDlg::CDrawShapeDlg(AXWindow *pOwner,VALUE *pVal)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*plm,*pl;
    UINT val;
    int i;

    m_pVal = pVal;

    val = m_pVal->uValue;

    //

    _trgroup(STRGID_DLG_DRAWSHAPE);

    setTitle(STRID_DSHAPE_TITLE);

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(10);

    //

    plTop->addItem(plm = new AXLayoutMatrix(2, LF_EXPAND_W));

    //図形

    plm->addItem(new AXLabel(this, 0, 0, MAKE_DW4(0,0,6,5), STRID_DSHAPE_SHAPE));
    plm->addItem(pl = new AXLayoutVert(LF_EXPAND_W, 5));

    pl->addItem(m_pcbShape = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_SHAPE, 0));
    pl->addItem(m_pcbSub = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_SUB, 0));
    pl->addItem(m_pckAA = new AXCheckButton(this, 0, 0, WID_CK_ANTIALIAS, MAKE_DW4(0,0,0,5),
                                            _string(STRGID_WORD, STRID_WORD_ANTIALIAS), val & (1<<8)));

    m_pcbShape->addItemMulTr(STRID_DSHAPE_LINE, 5);
    m_pcbShape->setAutoWidth();
    m_pcbShape->setCurSel((val >> 10) & 15);

    //濃度

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,5), STRID_DSHAPE_DRAWVAL));
    plm->addItem(m_peditVal = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, 0, MAKE_DW4(0,0,0,5)));

    m_peditVal->setInit(5, 1, 255, val & 255);

    //単位

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,10), STRID_DSHAPE_UNIT));
    plm->addItem(pl = new AXLayoutHorz);

    pl->addItem(m_pckUnit = new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_CK_PX, MAKE_DW4(0,0,5,10), "px", !(val & (1<<9)) ));
    pl->addItem(new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_CK_CM, MAKE_DW4(0,0,0,10), "cm", val & (1<<9) ));

    //各値

    for(i = 0; i < 4; i++)
    {
        plm->addItem(m_pLabel[i] = new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,5), "-"));
        plm->addItem(m_pedit[i] = new AXLineEdit(this, 0, 0, 0, MAKE_DW4(0,0,0,5)));

        m_pedit[i]->setWidthFromLen(8);
    }

    //-------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //

    _setSubType();
    _setParam();

    m_pcbSub->setCurSel((val >> 14) & 15);

    //

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CDrawShapeDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    int i;

    switch(pwin->getItemID())
    {
        //図形
        case WID_CB_SHAPE:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                _setSubType();
                _setParam();

                relayoutAndResize();
            }
            break;
        //OK
        case 1:
            m_pVal->uValue = m_peditVal->getVal();

            if(m_pckAA->isChecked()) m_pVal->uValue |= (1<<8);
            if(m_pckUnit->getGroupSel() == 1) m_pVal->uValue |= (1<<9);

            m_pVal->uValue |= m_pcbShape->getCurSel() << 10;
            m_pVal->uValue |= m_pcbSub->getCurSel() << 14;

            for(i = 0; i < 4; i++)
                m_pVal->dParam[i] = m_pedit[i]->getDouble();

            //cm -> px

            if(m_pVal->uValue & (1<<9))
            {
                for(i = 0; i < 4; i++)
                    m_pVal->dParam[i] = m_pVal->dParam[i] * m_pVal->nDPI / 2.54;
            }

            endDialog(TRUE);
            break;
        //キャンセル
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}


//! サブタイプ項目セット

void CDrawShapeDlg::_setSubType()
{
    int type;

    type = m_pcbShape->getCurSel();

    //

    m_pcbSub->deleteItemAll();

    if(type < 3)
    {
        _trgroup(STRGID_DLG_DRAWSHAPE);
        m_pcbSub->addItemMulTr(STRID_DSHAPE_CURBRUSH, 4);
    }
    else
    {
        _trgroup(STRGID_PIXELTYPE);
        m_pcbSub->addItemMulTr(0, 3);
    }

    m_pcbSub->setAutoWidth();
    m_pcbSub->setCurSel(0);

    //アンチエイリアス・濃度

    m_pckAA->enable((type == 4));       //円塗りつぶしのみ
    m_peditVal->enable((type >= 3));    //塗りつぶし系
}

//! パラメータラベルセット

void CDrawShapeDlg::_setParam()
{
    int type,i;
    WORD id[4];

    type = m_pcbShape->getCurSel();

    switch(type)
    {
        case 0:
            id[0] = STRID_DSHAPE_START_X;
            id[1] = STRID_DSHAPE_START_Y;
            id[2] = STRID_DSHAPE_LENGTH;
            id[3] = STRID_DSHAPE_ANGLE;
            break;
        case 1:
        case 3:
            id[0] = STRID_DSHAPE_START_X;
            id[1] = STRID_DSHAPE_START_Y;
            id[2] = STRID_DSHAPE_WIDTH;
            id[3] = STRID_DSHAPE_HEIGHT;
            break;
        case 2:
        case 4:
            id[0] = STRID_DSHAPE_CENTER_X;
            id[1] = STRID_DSHAPE_CENTER_Y;
            id[2] = STRID_DSHAPE_RADIUS;
            id[3] = 0;
            break;
    }

    //セット

    _trgroup(STRGID_DLG_DRAWSHAPE);

    for(i = 0; i < 4; i++)
    {
        if(id[i] == 0)
            m_pLabel[i]->setText((LPCUSTR)NULL);
        else
            m_pLabel[i]->setText(_str(id[i]));
    }
}
