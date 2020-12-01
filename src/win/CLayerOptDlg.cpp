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

#include "CLayerOptDlg.h"

#include "CConfig.h"
#include "CLayerList.h"
#include "CValBar.h"

#include "AXLayout.h"
#include "AXGroupBox.h"
#include "AXLabel.h"
#include "AXLineEdit.h"
#include "AXColorPrev.h"
#include "AXHSVPicker.h"
#include "AXCheckButton.h"
#include "AXApp.h"
#include "AXUtilColor.h"
#include "AXImage.h"

#include "defGlobal.h"
#include "defStrID.h"


//-----------------------

#define WID_RGB         100
#define WID_HSV         101
#define WID_BAR1        102
#define WID_BAR2        103
#define WID_BAR3        104
#define WID_COLEDIT1    105
#define WID_COLEDIT2    106
#define WID_COLEDIT3    107
#define WID_HSVPICKER   108
#define WID_PALETTE     109

//-----------------------

class CLayerOptDlg_pal:public AXWindow
{
protected:
    AXImage m_img;

public:
    CLayerOptDlg_pal(AXWindow *pParent);

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
};

//-----------------------

/*!
    @class CLayerOptDlg
    @brief レイヤ設定ダイアログ

    戻り値：[0]キャンセル [1]レイヤ一覧のみ更新 [2]キャンバスなども更新
*/


CLayerOptDlg::CLayerOptDlg(AXWindow *pOwner,CLayerItem *pItem)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl,*plG,*plGV;
    AXWindow *pGroup;
    int i;

    m_pItem = pItem;
    m_nColType = 1;     //色タイプ [0]RGB [1]HSV

    //

    _trgroup(STRGID_DLG_LAYEROPT);

    setTitle(STRID_LAYEROPT_TITLE);

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(8);

    //------- 名前・不透明度

    plTop->addItem(pl = new AXLayoutMatrix(2));

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,5), _string(STRGID_WORD, STRID_WORD_NAME)));
    pl->addItem(m_peditName = new AXLineEdit(this, 0, 0, 0, MAKE_DW4(0,0,0,5)));
    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,6,0), _string(STRGID_WORD, STRID_WORD_OPACITY)));
    pl->addItem(m_peditOpacity = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0));

    m_peditName->setWidthFromLen(26);
    m_peditName->setText(pItem->m_strName);

    m_peditOpacity->setInit(4, 0, 100, (int)(pItem->m_btAlpha * 100.0 / 128.0 + 0.5));

    //--------- 色

    plTop->addItem(pGroup = new AXGroupBox(this, 0, LF_EXPAND_W, MAKE_DW4(0,10,0,0), _string(STRGID_WORD, STRID_WORD_COLOR)));

    pGroup->setLayout(plG = new AXLayoutHorz(0, 15));
    plG->setSpacing(8);

    //プレビューとHSV

    plG->addItem(pl = new AXLayoutVert(0, 10));

    pl->addItem(m_pColPrev = new AXColorPrev(pGroup, AXColorPrev::CPS_SUNKEN, 0));
    pl->addItem(m_pHSV = new AXHSVPicker(pGroup, 0, 0, WID_HSVPICKER, 0, 90));

    m_pColPrev->setMinSize(60, 40);
    m_pColPrev->setColor(pItem->m_dwCol);

    m_pHSV->setColor(pItem->m_dwCol);

    //パレット

    plG->addItem(plGV = new AXLayoutVert(LF_EXPAND_W, 0));

    plGV->addItem(new AXLabel(pGroup, 0, 0, MAKE_DW4(0,0,0,3), STRID_LAYEROPT_PALINFO));
    plGV->addItem(m_pPalette = new CLayerOptDlg_pal(pGroup));

    //RGB/HSV

    plGV->addItem(pl = new AXLayoutHorz(0, 5));
    pl->setPadding(0, 6, 0, 5);

    pl->addItem(new AXCheckButton(pGroup, AXCheckButton::CBS_RADIO, 0, WID_RGB, 0, "RGB", FALSE));
    pl->addItem(new AXCheckButton(pGroup, AXCheckButton::CBS_RADIO, 0, WID_HSV, 0, "HSV", TRUE));

    //バー＋値

    plGV->addItem(pl = new AXLayoutMatrix(3, LF_EXPAND_W));

    m_playoutCol = pl;

    for(i = 0; i < 3; i++)
    {
        pl->addItem(m_plabelCol[i] = new AXLabel(pGroup, 0, LF_CENTER_Y, MAKE_DW4(0,0,5,4), "R"));
        pl->addItem(m_pbar[i] = new CValBar(pGroup, 0, LF_EXPAND_W|LF_CENTER_Y, WID_BAR1 + i, MAKE_DW4(0,0,5,4)));
        pl->addItem(m_peditCol[i] = new AXLineEdit(pGroup, AXLineEdit::ES_SPIN, 0, WID_COLEDIT1 + i, MAKE_DW4(0,0,0,4)));

        m_pbar[i]->setMinSize(160, 12);
        m_peditCol[i]->setWidthFromLen(4);
    }

    _changeColAll(TRUE);

    //-------

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //

    m_peditName->setFocus();
    m_peditName->selectAll();

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CLayerOptDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();
    int n;

    switch(id)
    {
        //RGB/HSV切り替え
        case WID_RGB:
        case WID_HSV:
            m_nColType = id - WID_RGB;
            _changeColAll(TRUE);
            break;
        //色バー
        case WID_BAR1:
        case WID_BAR2:
        case WID_BAR3:
            n = id - WID_BAR1;

            m_peditCol[n]->setVal(m_pbar[n]->getPos());
            _updateColPrev();
            break;
        //色エディット
        case WID_COLEDIT1:
        case WID_COLEDIT2:
        case WID_COLEDIT3:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                n = id - WID_COLEDIT1;

                m_pbar[n]->setPos(m_peditCol[n]->getVal());
                _updateColPrev();
            }
            break;
        //HSV
        case WID_HSVPICKER:
            m_pColPrev->setColor(m_pHSV->getColor());
            _changeColAll(FALSE);
            break;
        //パレット
        case WID_PALETTE:
            if(uNotify)
                //登録
                g_pconf->dwLayerPal[lParam] = m_pColPrev->getColor();
            else
            {
                //呼び出し

                m_pColPrev->setColor(g_pconf->dwLayerPal[lParam]);
                m_pHSV->setColor(g_pconf->dwLayerPal[lParam]);

                _changeColAll(FALSE);
            }
            break;

        //OK
        case 1:
            BOOL bUpdate;

            bUpdate = FALSE;

            //名前

            m_peditName->getText(&m_pItem->m_strName);

            m_pItem->m_strName.end(64); //63文字以下

            //不透明度

            n = (int)(m_peditOpacity->getVal() * 128.0 / 100.0 + 0.5);

            if(m_pItem->m_btAlpha != n)
            {
                m_pItem->m_btAlpha = n;
                bUpdate = TRUE;
            }

            //色

            n = m_pColPrev->getColor();

            if(m_pItem->m_dwCol != (DWORD)n)
            {
                m_pItem->m_dwCol = n;
                bUpdate = TRUE;
            }

            endDialog((bUpdate)? 2: 1);
            break;
        //キャンセル
        case 2:
            endDialog(0);
            break;
    }

    return TRUE;
}


//==========================


//! カラータイプが変更orカラー値セット（プレビュー色から）

void CLayerOptDlg::_changeColAll(BOOL bInit)
{
    int i,col[3],max;
    UNICHAR name[2][3] = {{'R','G','B'}, {'H','S','V'}};
    UNICHAR str[2] = {0,0};
    DWORD curcol;

    //各値取得

    curcol = m_pColPrev->getColor();

    if(m_nColType == 0)
    {
        col[0] = _GETR(curcol);
        col[1] = _GETG(curcol);
        col[2] = _GETB(curcol);
    }
    else
        AXRGBtoHSV(curcol, col);

    //バーなどの設定

    for(i = 0; i < 3; i++)
    {
        if(bInit)
        {
            //範囲

            max = (m_nColType && i == 0)? 359: 255;

            m_pbar[i]->setRange(0, max);
            m_peditCol[i]->setValStatus(0, max);

            //"RGB"or"HSV"

            str[0] = name[m_nColType][i];
            m_plabelCol[i]->setText(str);
        }

        //値

        m_pbar[i]->setPos(col[i]);

        m_peditCol[i]->setVal(col[i]);
    }

    //再レイアウト
    if(bInit) m_playoutCol->layout();
}

//! カラープレビュー更新（値から）

void CLayerOptDlg::_updateColPrev()
{
    int c1,c2,c3;
    DWORD col;

    c1 = m_pbar[0]->getPos();
    c2 = m_pbar[1]->getPos();
    c3 = m_pbar[2]->getPos();

    if(m_nColType == 0)
        //RGB
        col = _RGB(c1, c2, c3);
    else
        //HSV
        col = AXHSVtoRGB(c1, c2 / 255.0, c3 / 255.0);

    //更新

    m_pColPrev->setColor(col);
}


//************************************
// CLayerOptDlg_pal : パレット
//************************************

#define PAL_ONEW    16
#define PAL_ONEH    15


CLayerOptDlg_pal::CLayerOptDlg_pal(AXWindow *pParent)
    : AXWindow(pParent, 0, 0)
{
    int ix,iy,x,y,no;

    m_uItemID = WID_PALETTE;

    m_nMinW = PAL_ONEW * 12 + 1;
    m_nMinH = PAL_ONEH * 2 + 1;

    //イメージ

    m_img.create(m_nMinW, m_nMinH);

    for(iy = 0, y = 0, no = 0; iy < 2; iy++, y += PAL_ONEH)
    {
        for(ix = 0, x = 0; ix < 12; ix++, x += PAL_ONEW, no++)
        {
            m_img.box(x, y, PAL_ONEW + 1, PAL_ONEH + 1, 0xffffff);
            m_img.box(x + 1, y + 1, PAL_ONEW - 1, PAL_ONEH - 1, 0);
            m_img.fillBox(x + 2, y + 2, PAL_ONEW - 3, PAL_ONEH - 3, g_pconf->dwLayerPal[no]);
        }
    }
}

BOOL CLayerOptDlg_pal::onPaint(AXHD_PAINT *phd)
{
    m_img.put(m_id);

    return TRUE;
}

BOOL CLayerOptDlg_pal::onButtonDown(AXHD_MOUSE *phd)
{
    BOOL bReg;
    int x,y,no;

    if(phd->button == BUTTON_LEFT)
        bReg = (phd->state & STATE_CTRL)? TRUE: FALSE;
    else if(phd->button == BUTTON_RIGHT)
        bReg = TRUE;
    else
        return TRUE;

    //パレット番号

    x = (phd->x - 1) / PAL_ONEW;
    y = (phd->y - 1) / PAL_ONEH;

    if(x < 0) x = 0; else if(x > 11) x = 11;
    if(y < 0) y = 0; else if(y > 1) y = 1;

    no = y * 12 + x;

    //通知

    m_pTopLevel->onNotify(this, bReg, no);

    //登録時、色更新

    if(bReg)
    {
        m_img.fillBox((no % 12) * PAL_ONEW + 2, (no / 12) * PAL_ONEH + 2,
                PAL_ONEW - 3, PAL_ONEH - 3, g_pconf->dwLayerPal[no]);

        redraw();
    }

    return TRUE;
}
