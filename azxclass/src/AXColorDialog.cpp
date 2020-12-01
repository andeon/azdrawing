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

#include "AXColorDialog.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXHSVPicker.h"
#include "AXColorPrev.h"
#include "AXLineEdit.h"
#include "AXSliderBar.h"
#include "AXUtilColor.h"
#include "AXApp.h"
#include "AXTrSysID.h"


#define WID_HSV     200
#define WID_SLIDER  300
#define WID_EDIT    350


/*!
    @class AXColorDialog
    @brief カラー選択ダイアログ

    - AXColorDialog::getColor() で関数呼び出し。

    @ingroup dialog
*/


AXColorDialog::~AXColorDialog()
{

}

AXColorDialog::AXColorDialog(AXWindow *pOwner,LPDWORD pRetCol)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl;
    int i,max,c[3];
    char label[6] = {'R','H','G','S','B','V'};
    WORD wstr[2] = {0,0};

    m_pRetCol = pRetCol;

    //タイトル

    setTitle(_string(TRSYS_GROUPID, TRSYS_TITLE_COLORDIALOG));

    //トップレイアウト

    plTop = new AXLayoutVert;
    plTop->setSpacing(10);
    setLayout(plTop);

    //----------- 色部分

    plTop->addItem(pl = new AXLayoutHorz);

    //色プレビュー

    m_prev = new AXColorPrev(this, AXColorPrev::CPS_SUNKEN, 0, 0, MAKE_DW4(0,0,20,0));
    m_prev->setMinSize(60, 50);
    m_prev->setColor(*pRetCol);

    pl->addItem(m_prev);

    //HSVピッカー

    m_picker = new AXHSVPicker(this, 0, 0, WID_HSV, MAKE_DW4(0,0,0,15), 100);
    m_picker->setColor(*pRetCol);

    pl->addItem(m_picker);

    //---------- 数値部分

    plTop->addItem(pl = new AXLayoutMatrix(6));

    for(i = 0; i < 6; i++)
    {
        max = (i == 1)? 359: 255;

        //ラベル

        wstr[0] = label[i];
        pl->addItem(new AXLabel(this, 0, 0, MAKE_DW4(0,0,5,0), wstr));

        //スライダー

        m_slider[i] = new AXSliderBar(this, 0, 0, WID_SLIDER + i, MAKE_DW4(0,0,5,0));
        m_slider[i]->setMinWidth(130);
        m_slider[i]->setStatus(0, max, 0);
        pl->addItem(m_slider[i]);

        //エディット

        m_edit[i] = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT + i, (i & 1)? 0: MAKE_DW4(0,0,12,2));
        m_edit[i]->setWidthFromLen(4);
        m_edit[i]->setValStatus(0, max);
        pl->addItem(m_edit[i]);
    }

    //数値セット(RGB)

    c[0] = _GETR(*pRetCol);
    c[1] = _GETG(*pRetCol);
    c[2] = _GETB(*pRetCol);

    for(i = 0; i < 3; i++)
    {
        m_slider[i * 2]->setPos(c[i]);
        m_edit[i * 2]->setInt(c[i]);
    }

    //数値セット(HSV)

    AXRGBtoHSV(*pRetCol, c);

    for(i = 0; i < 3; i++)
    {
        m_slider[i * 2 + 1]->setPos(c[i]);
        m_edit[i * 2 + 1]->setInt(c[i]);
    }

    //-------- ボタン

    pl = createOKCancelButton(100);
    pl->setPadding(0, 15, 0, 0);

    plTop->addItem(pl);
}

//! 実行

BOOL AXColorDialog::run()
{
    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();

    return runDialog();
}

//! (関数)色取得

BOOL AXColorDialog::getColor(AXWindow *pOwner,LPDWORD pRetCol)
{
    AXColorDialog *pdlg = new AXColorDialog(pOwner, pRetCol);
    return pdlg->run();
}


//===========================
//
//===========================


//! 通知

BOOL AXColorDialog::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();
    int n;

    if(id == WID_HSV)
    {
        //HSV -> スライダーとエディットの値セット

        double hsv[3];

        m_picker->getHSVColor(hsv);

        m_slider[0]->setPos(_GETR(lParam));
        m_slider[2]->setPos(_GETG(lParam));
        m_slider[4]->setPos(_GETB(lParam));

        m_slider[1]->setPos(_DTOI(hsv[0]));
        m_slider[3]->setPos(_DTOI(hsv[1] * 255));
        m_slider[5]->setPos(_DTOI(hsv[2] * 255));

        for(n = 0; n < 6; n++)
            m_edit[n]->setInt(m_slider[n]->getPos());

        m_prev->setColor(lParam);
    }
    else if(id >= WID_SLIDER && id < WID_SLIDER + 6)
    {
        //スライダー（HSVマップも連動）

        n = id - WID_SLIDER;

        m_edit[n]->setInt(m_slider[n]->getPos());
        _changeVal(n);

        //HSV

        if(n == 1)
            m_picker->setHue(m_slider[1]->getPos());
        else if(n == 3 || n == 5)
            m_picker->setSV(m_slider[3]->getPos() / 255.0, m_slider[5]->getPos() / 255.0);
        else
            m_picker->setColor(m_prev->getColor());
    }
    else if(id >= WID_EDIT && id < WID_EDIT + 6)
    {
        //エディット（HSVマップは連動しない）

        if(uNotify == AXLineEdit::EN_CHANGE)
        {
            n = id - WID_EDIT;

            m_slider[n]->setPos(m_edit[n]->getVal());
            _changeVal(n);
        }
    }
    else if(id == 1)
    {
        //OK
        *m_pRetCol = m_prev->getColor();
        endDialog(TRUE);
    }
    else if(id == 2)
        //キャンセル
        endDialog(FALSE);

    return TRUE;
}

//! エディットで数値が変更された時

void AXColorDialog::_changeVal(int no)
{
    int c[3],i;
    DWORD col;

    if(!(no & 1))
    {
        //------- RGB

        for(i = 0; i < 3; i++)
            c[i] = m_slider[i * 2]->getPos();

        col = (c[0] << 16) | (c[1] << 8) | c[2];

        //RGB -> HSV

        AXRGBtoHSV(col, c);

        for(i = 0; i < 3; i++)
        {
            m_slider[i * 2 + 1]->setPos(c[i]);
            m_edit[i * 2 + 1]->setInt(c[i]);
        }
    }
    else
    {
        //-------- HSV

        for(i = 0; i < 3; i++)
            c[i] = m_slider[i * 2 + 1]->getPos();

        //HSV -> RGB

        AXHSVtoRGB(c[0], c[1] / 255.0, c[2] / 255.0, c);

        for(i = 0; i < 3; i++)
        {
            m_slider[i * 2]->setPos(c[i]);
            m_edit[i * 2]->setInt(c[i]);
        }

        col = (c[0] << 16) | (c[1] << 8) | c[2];
    }

    //プレビュー

    m_prev->setColor(col);
}
