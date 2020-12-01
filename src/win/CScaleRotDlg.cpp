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

#include <math.h>

#include "CScaleRotDlg.h"

#include "CDraw.h"
#include "CMainWin.h"
#include "CValBar.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXColorButton.h"
#include "AXApp.h"
#include "AXAppRes.h"

#include "defGlobal.h"
#include "defStrID.h"


//---------------------------------

#define WID_BAR_SCALE   100
#define WID_EDIT_SCALE  101
#define WID_ANGLE       102
#define WID_EDIT_ANGLE  103
#define WID_CK_PREV     104
#define WID_CK_HQPREV   105
#define WID_BT_PREVCOL  106


class CScaleRot_Angle:public AXWindow
{
protected:
    double  m_dAngle;

    void _changeAngle(int x,int y);

public:
    CScaleRot_Angle(AXWindow *pParent,UINT uItemID,DWORD dwPadding);

    double getAngleDouble() { return m_dAngle; }
    int getAngleVal();
    void setAngle(int val);

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

//---------------------------------



//*******************************
// CScaleRotDlg
//*******************************


/*!
    @class CScaleRotDlg
    @brief [矩形編集] 拡大縮小・回転ダイアログ
*/


CScaleRotDlg::CScaleRotDlg(CImage8 *pimg8,AXRect *prc,double *pResVal)
    : AXDialog(MAINWIN, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl;
    UINT val = g_pdraw->m_optBEScaleRot;

    m_pimg8 = pimg8;
    m_prc   = prc;
    m_pResVal = pResVal;

    //

    _trgroup(STRGID_SCALEROT);

    setTitle(STRID_SCROT_TITLE);

    //メインレイアウト

    setLayout(plTop = new AXLayoutVert);

    plTop->setSpacing(8);

    //------ 倍率・角度

    plTop->addItem(pl = new AXLayoutMatrix(3));
    pl->setPaddingBottom(8);

    //倍率

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,6), STRID_SCROT_SCALE));
    pl->addItem(m_pbarScale = new CValBar(this, 0, LF_EXPAND_W|LF_CENTER_Y, WID_BAR_SCALE, MAKE_DW4(0,0,8,6), 1, 5000, 1000));
    pl->addItem(m_peditScale = new AXLineEdit(this, AXLineEdit::ES_SPIN, LF_CENTER_Y, WID_EDIT_SCALE, MAKE_DW4(0,0,0,6)));

    m_pbarScale->setMinSize(200, 12);
    m_peditScale->setInit(6, 1, 10000, 1, 1000);

    //角度

    pl->addItem(new AXLabel(this, 0, 0, MAKE_DW4(0,0,6,0), STRID_SCROT_ANGLE));
    pl->addItem(m_pAngle = new CScaleRot_Angle(this, WID_ANGLE, MAKE_DW4(0,0,6,0)));
    pl->addItem(m_peditAngle = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_ANGLE, 0));

    m_peditAngle->setInit(6, -3600, 3600, 1, 0);

    //----------

    plTop->addItem(pl = new AXLayoutMatrix(2, LF_RIGHT));

    //プレビュー

    pl->addItem(new AXCheckButton(this, 0, LF_CENTER_Y, WID_CK_PREV, MAKE_DW4(0,0,8,4), STRID_SCROT_PREVIEW, val & CDraw::BESCALEROT_F_PREV));
    pl->addItem(m_pbtCol = new AXColorButton(this, AXColorButton::CBTS_CHOOSE, LF_CENTER_Y, WID_BT_PREVCOL, MAKE_DW4(0,0,0,4), val & CDraw::BESCALEROT_COL_MASK));

    //高品質プレビュー

    pl->addItem(new AXCheckButton(this, 0, 0, WID_CK_HQPREV, MAKE_DW4(0,0,8,0), STRID_SCROT_HQ_PREVIEW, val & CDraw::BESCALEROT_F_HQPREV));

    //-----------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CScaleRotDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //倍率バー
        case WID_BAR_SCALE:
            m_peditScale->setVal(m_pbarScale->getPos());
            _prev();
            break;
        //倍率エディット
        case WID_EDIT_SCALE:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                m_pbarScale->setPos(m_peditScale->getVal());
                _prev();
            }
            break;
        //回転角度
        case WID_ANGLE:
            m_peditAngle->setVal(m_pAngle->getAngleVal());
            _prev();
            break;
        //角度エディット
        case WID_EDIT_ANGLE:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                m_pAngle->setAngle(m_peditAngle->getVal());
                _prev();
            }
            break;
        //プレビュー
        case WID_CK_PREV:
            g_pdraw->m_optBEScaleRot ^= CDraw::BESCALEROT_F_PREV;

            _drawprev();
            break;
        //高品質プレビュー
        case WID_CK_HQPREV:
            g_pdraw->m_optBEScaleRot ^= CDraw::BESCALEROT_F_HQPREV;
            _prev();
            break;
        //プレビュー色
        case WID_BT_PREVCOL:
            if(uNotify == AXColorButton::CBTN_PRESS)
            {
                g_pdraw->m_optBEScaleRot &= ~CDraw::BESCALEROT_COL_MASK;
                g_pdraw->m_optBEScaleRot |= (UINT)lParam;

                _prev();
            }
            break;

        //OK
        case 1:
            m_pResVal[0] = m_peditScale->getVal() / 1000.0;
            m_pResVal[1] = m_pAngle->getAngleDouble();

            delTimerAll();
            endDialog(TRUE);
            break;
        //キャンセル
        case 2:
            delTimerAll();
            endDialog(FALSE);
            break;
    }

    return TRUE;
}

//! タイマー

BOOL CScaleRotDlg::onTimer(UINT uTimerID,ULONG lParam)
{
    delTimer(uTimerID);

    _drawprev();

    return TRUE;
}

//! プレビュー

void CScaleRotDlg::_prev()
{
    if(g_pdraw->m_optBEScaleRot & CDraw::BESCALEROT_F_PREV)
        addTimer(0, 100);
}

//! プレビュー描画

void CScaleRotDlg::_drawprev()
{
    g_pdraw->draw_boxedit_scaleRotPrev(*m_pimg8, *m_prc,
                        m_peditScale->getVal() / 1000.0,
                        m_pAngle->getAngleDouble());
}


//*************************************
// CScaleRot_Angle - 角度ウィジェット
//*************************************


CScaleRot_Angle::CScaleRot_Angle(AXWindow *pParent,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, 0, 0, uItemID, dwPadding)
{
    m_dAngle = 0;

    m_nMinW = m_nMinH = 61;
}

//! 角度取得

int CScaleRot_Angle::getAngleVal()
{
    int n = (int)(m_dAngle * -1800.0 / M_PI);

    while(n < 0) n += 3600;
    while(n >= 3600) n -= 3600;

    return n;
}

//! 角度セット

void CScaleRot_Angle::setAngle(int val)
{
    m_dAngle = val * M_PI / -1800.0;
    redraw();
}

//! 角度変更

void CScaleRot_Angle::_changeAngle(int x,int y)
{
    m_dAngle = ::atan2(y - m_nW / 2, x - m_nW / 2);
    redrawUpdate();

    m_pNotify->onNotify(this, 0, 0);
}

//! 描画

BOOL CScaleRot_Angle::onPaint(AXHD_PAINT *phd)
{
    int ct,x,y;

    //枠

    drawBox(0, 0, m_nW, m_nH, AXAppRes::BLACK);

    //背景

    drawFillBox(1, 1, m_nW - 2, m_nH - 2, AXAppRes::WHITE);

    //線

    ct = m_nW / 2;

    x = (int)((ct - 1) * ::cos(m_dAngle));
    y = (int)((ct - 1) * ::sin(m_dAngle));

    drawLine(ct, ct, ct + x, ct + y, AXAppRes::BLACK);

    return TRUE;
}

//! ボタン押し

BOOL CScaleRot_Angle::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && !(m_uFlags & FLAG_TEMP1))
    {
        _changeAngle(phd->x, phd->y);

        m_uFlags |= FLAG_TEMP1;
        grabPointer();
    }

    return TRUE;
}

//! ボタン離し

BOOL CScaleRot_Angle::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && (m_uFlags & FLAG_TEMP1))
    {
        m_uFlags &= ~FLAG_TEMP1;
        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動

BOOL CScaleRot_Angle::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_uFlags & FLAG_TEMP1)
        _changeAngle(phd->x, phd->y);

    return TRUE;
}
