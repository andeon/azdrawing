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

#include "CBrushWin_value.h"

#include "CBrushTreeItem.h"
#include "CBrushManager.h"
#include "CValBar2.h"
#include "CSelImgBar.h"
#include "CSelImgDlg.h"
#include "CPressCurveView.h"
#include "CConfig.h"
#include "CResource.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXComboBox.h"
#include "AXCheckButton.h"
#include "AXArrowButton.h"
#include "AXLineEdit.h"
#include "AXTab.h"
#include "AXMenu.h"
#include "AXUtilStr.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"

//

#include "CBrushWin_value_tabwin.h"

//---------------------------


/*!
    @class CBrushWin_value
    @brief [ブラシウィンドウ] 各値の編集部分
*/


CBrushWin_value::CBrushWin_value(AXWindow *pParent)
    : AXWindow(pParent, WS_BK_FACE, LF_EXPAND_W | LF_FIX_H)
{
    AXLayout *pl;
    int i;

    m_pwinTab = NULL;

    setLayout(pl = new AXLayoutVert);

    _trgroup(STRGID_BRUSHVALUE);

    //共通部分

    _createCommon();

    //タブ

    pl->addItem(m_ptab = new AXTab(this, AXTab::TABS_ICONHEIGHT | AXTab::TABS_TOPTAB, LF_EXPAND_W,
                                   WID_TAB, MAKE_DW4(0,3,0,0)));

    m_ptab->setImageList(g_pres->m_pilBrushValTab);

    for(i = 0; i < 5; i++)
        m_ptab->addItem(NULL, i, 0, 19);

    m_ptab->setCurSel(g_pconf->btBrushWinTab);

    //タブウィンドウ作成

    _changeTab(FALSE);

    //

    calcDefSize();

    m_nH = (g_pconf->nBrushWinH[1] <= 0)? m_nDefH: g_pconf->nBrushWinH[1];
}

//! 共通部分ウィジェット作成

void CBrushWin_value::_createCommon()
{
    AXLayout *plm,*pl;
    AXWindow *pwin;
    int i;
    AXString str;

    getLayout()->addItem(plm = new AXLayoutMatrix(2, LF_EXPAND_W));
    plm->setSpacing(5);

    //データタイプ

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,3), STRID_BRVAL_DATTYPE));

    plm->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 4));
    pl->setPaddingBottom(3);

    pl->addItem(m_pcbDatType = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_DATTYPE, 0));
    pl->addItem(new AXButton(this, AXButton::BS_REAL_WH, LF_EXPAND_H, WID_BT_SAVE, 0, STRID_BRVAL_SAVE));

    m_pcbDatType->addItemTr(STRID_BRVAL_DATTYPE_LINK, 0);
    m_pcbDatType->addItemTr(STRID_BRVAL_DATTYPE_MANUAL, 0);

    //サイズ

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,3), STRID_BRVAL_SIZE));

    plm->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 4));
    pl->setPaddingBottom(3);

    pl->addItem(m_pbarSize = new CValBar2(this, 0, LF_EXPAND_W|LF_CENTER_Y, WID_BAR_SIZE, 0,
                                          1, CBrushTreeItem::BRUSHSIZE_MIN, CBrushTreeItem::BRUSHSIZE_MAX, CBrushTreeItem::BRUSHSIZE_MIN));

    pl->addItem(pwin = new AXArrowButton(this, AXArrowButton::ARBTS_DOWN, LF_CENTER_Y, WID_BT_SIZEMENU, 0));
    pwin->setMinSize(-15, -15);

    //濃度

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,3), STRID_BRVAL_VAL));
    plm->addItem(m_pbarVal = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_VAL, MAKE_DW4(0,0,0,3), 0, 1, 255, 255));

    //補正

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,3), STRID_BRVAL_HOSEI));

    plm->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 4));
    pl->setPaddingBottom(3);

    pl->addItem(m_pcbHoseiType = new AXComboBox(this, 0, 0, WID_CB_HOSEITYPE, 0));
    pl->addItem(m_pcbHoseiStr = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_HOSEISTR, 0));

    for(i = 0; i < 4; i++)
        m_pcbHoseiType->addItemTr(STRID_BRVAL_HOSEI_NONE + i, 0);

    m_pcbHoseiType->setAutoWidth();

    for(i = 0; i <= CBrushTreeItem::HOSEISTR_MAX; i++)
    {
        str.setInt(i + 1);
        m_pcbHoseiStr->addItem(str);
    }

    //塗りタイプ

    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,4,0), STRID_BRVAL_PIXTYPE));
    plm->addItem(m_pcbPixType = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_PIXTYPE, 0));

    _trgroup(STRGID_PIXELTYPE);

    for(i = 0; i < 6; i++)
        m_pcbPixType->addItemTr(STRID_PIXTYPE_BLEND + i, i);

    _trgroup(STRGID_BRUSHVALUE);
}

//! タブ選択変更時

void CBrushWin_value::_changeTab(BOOL bLayout)
{
    //現在のウィンドウ削除

    if(m_pwinTab)
    {
        m_pwinTab->removeFromLayout();
        delete m_pwinTab;

        m_pwinTab = NULL;
    }

    //ウィンドウ作成

    _trgroup(STRGID_BRUSHVALUE);

    switch(g_pconf->btBrushWinTab)
    {
        case 0:
            m_pwinTab = new CBrushWinTab_min(this);
            break;
        case 1:
            m_pwinTab = new CBrushWinTab_random(this);
            break;
        case 2:
            m_pwinTab = new CBrushWinTab_press(this);
            break;
        case 3:
            m_pwinTab = new CBrushWinTab_style(this);
            break;
        case 4:
            m_pwinTab = new CBrushWinTab_etc(this);
            break;
    }

    //セット

    if(m_pwinTab)
    {
        //値をセット

        m_pwinTab->setValue(BRUSHMAN->getEdit());

        //レイアウト

        getLayout()->addItem(m_pwinTab);

        if(bLayout)
        {
            m_pwinTab->calcDefSize();
            layout();
        }
    }
}

//====================

//! 直接ブラシサイズが変更された時

void CBrushWin_value::setBrushSize()
{
    m_pbarSize->setRange(CBrushTreeItem::BRUSHSIZE_MIN, BRUSHMANEDIT->m_wSizeCtlMax);
    m_pbarSize->setPos(BRUSHMANEDIT->m_wSize);
}

//! 直接ブラシ濃度が変更された時

void CBrushWin_value::setBrushVal()
{
    m_pbarVal->setPos(BRUSHMANEDIT->m_btVal);
}

//! ブラシデータ変更時

void CBrushWin_value::changeBrush()
{
    //データタイプ有効/無効

    m_pcbDatType->enable((BRUSHMAN->getSelBrush() != 0));

    //保存ボタン有効/無効

    enableWidget(WID_BT_SAVE, (BRUSHMAN->getSelBrush() && BRUSHMAN->getSelBrush()->m_btDatType == CBrushTreeItem::DATTYPE_MANUAL));

    //編集用データセット

    setValue();
}

//! 値をセット

void CBrushWin_value::setValue()
{
    CBrushTreeItem *p = BRUSHMAN->getEdit();

    m_pcbDatType->setCurSel(p->m_btDatType);
    m_pbarSize->setRange(CBrushTreeItem::BRUSHSIZE_MIN, p->m_wSizeCtlMax);
    m_pbarSize->setPos(p->m_wSize);
    m_pbarVal->setPos(p->m_btVal);
    m_pcbHoseiType->setCurSel(p->m_btHoseiType);
    m_pcbHoseiStr->setCurSel(p->m_btHoseiStr);
    m_pcbPixType->setCurSel(p->m_btPixType);

    //タブウィンドウ

    if(m_pwinTab)
        m_pwinTab->setValue(p);
}

//===================

//! 通知

BOOL CBrushWin_value::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //タブ
        case WID_TAB:
            if(uNotify == AXTab::TABN_SELCHANGE)
            {
                g_pconf->btBrushWinTab = m_ptab->getCurSel();

                _changeTab(TRUE);
            }
            break;
        //データタイプ
        case WID_CB_DATTYPE:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                BRUSHMAN->changeDatType(m_pcbDatType->getCurSel());

                //保存ボタン
                enableWidget(WID_BT_SAVE, (BRUSHMAN->getSelBrush() && BRUSHMAN->getSelBrush()->m_btDatType == CBrushTreeItem::DATTYPE_MANUAL));
            }
            break;
        //手動保存
        case WID_BT_SAVE:
            BRUSHMAN->saveBrushManual();
            break;
        //サイズ
        case WID_BAR_SIZE:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeBrushSize(m_pbarSize->getPos());
            break;
        //サイズメニュー
        case WID_BT_SIZEMENU:
            _showSizeMenu();
            break;
        //濃度
        case WID_BAR_VAL:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeVal(m_pbarVal->getPos());
            break;
        //補正タイプ
        case WID_CB_HOSEITYPE:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
                BRUSHMAN->changeHoseiType(m_pcbHoseiType->getCurSel());
            break;
        //補正強さ
        case WID_CB_HOSEISTR:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
                BRUSHMAN->changeHoseiStr(m_pcbHoseiStr->getCurSel());
            break;
        //塗りタイプ
        case WID_CB_PIXTYPE:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
                BRUSHMAN->changePixType(m_pcbPixType->getCurSel());
            break;
    }

    return TRUE;
}

//! ブラシサイズメニュー（最大値指定）表示

void CBrushWin_value::_showSizeMenu()
{
    AXMenu *pm;
    int ret,i,max,now;
    AXRectSize rcs;
    WORD size[8] = {100, 500, 1000, 2000, 3000, 4000, 5000, 6000};
    char m[32];
    AXString str;

    //------ メニュー

    pm = new AXMenu;

    for(i = 0; i < 8; i++)
    {
        AXIntToFloatStr(m, size[i], 1);

        str = "max ";
        str += m;

        pm->add(i, str);
    }

    getWidget(WID_BT_SIZEMENU)->getWindowRect(&rcs);
    ret = (int)pm->popup(NULL, rcs.x, rcs.y + rcs.h, 0);

    delete pm;

    //------ 処理

    if(ret == -1) return;

    max = size[ret];
    now = m_pbarSize->getPos();

    if(now > max) now = max;

    m_pbarSize->setRange(CBrushTreeItem::BRUSHSIZE_MIN, max);
    m_pbarSize->setPos(now);

    BRUSHMAN->changeBrushSizeMax(now, max);
}


//********************************************
// CBrushWinTab_base （タブウィンドウ基底クラス）
//********************************************


CBrushWinTab_base::CBrushWinTab_base(AXWindow *pParent)
    : AXWindow(pParent, WS_BK_FACE, LF_EXPAND_WH)
{

}


//*********************************
// CBrushWinTab_min （最小＆間隔）
//*********************************


CBrushWinTab_min::CBrushWinTab_min(AXWindow *pParent)
    : CBrushWinTab_base(pParent)
{
    AXLayout *pl;

    setLayout(pl = new AXLayoutVert(0, 3));
    pl->setSpacing(5);

    //サイズ最小

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_SIZEMIN));
    pl->addItem(m_pbarSize = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_SIZEMIN, 0, 1, 0, 1000, 0));

    //濃度最小

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_VALMIN));
    pl->addItem(m_pbarVal = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_VALMIN, 0, 1, 0, 1000, 0));

    //間隔

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_INTERVAL));
    pl->addItem(m_pbarInter = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_INTERVAL, 0, 2, 5, 500, 6));
}

void CBrushWinTab_min::setValue(CBrushTreeItem *p)
{
    m_pbarSize->setPos(p->m_wMinSize);
    m_pbarVal->setPos(p->m_wMinVal);
    m_pbarInter->setPos(p->m_wInterval);
}

BOOL CBrushWinTab_min::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //サイズ最小
        case WID_BAR_SIZEMIN:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeSizeMin(m_pbarSize->getPos());
            break;
        //濃度最小
        case WID_BAR_VALMIN:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeValMin(m_pbarVal->getPos());
            break;
        //間隔
        case WID_BAR_INTERVAL:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeInterval(m_pbarInter->getPos());
            break;
    }

    return TRUE;
}


//*********************************
// CBrushWinTab_random （ランダム）
//*********************************


CBrushWinTab_random::CBrushWinTab_random(AXWindow *pParent)
    : CBrushWinTab_base(pParent)
{
    AXLayout *pl,*plh;

    setLayout(pl = new AXLayoutVert(0, 3));
    pl->setSpacing(5);

    //ランダムサイズ

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_RANDOM_SIZE));
    pl->addItem(m_pbarSize = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_RANSIZE, 0, 1, 0, 1000, 0));

    //ランダム位置

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_RANDOM_POS));
    pl->addItem(m_pbarPos = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_RANPOS, 0, 2, 0, 5000, 0));

    //円形・矩形

    pl->addItem(plh = new AXLayoutHorz(0, 3));

    plh->addItem(m_pckCir = new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_CK_RANCIRCLE, 0, STRID_BRVAL_RANDOM_CIRCLE, FALSE));
    plh->addItem(m_pckBox = new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_CK_RANBOX, 0, STRID_BRVAL_RANDOM_BOX, FALSE));
}

void CBrushWinTab_random::setValue(CBrushTreeItem *p)
{
    int f = p->m_btFlag & CBrushTreeItem::BFLAG_RANDPOS_BOX;

    m_pbarSize->setPos(p->m_wRan_sizeMin);
    m_pbarPos->setPos(p->m_wRan_posLen);

    m_pckCir->setCheck(!f);
    m_pckBox->setCheck(f);
}

BOOL CBrushWinTab_random::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //ランダムサイズ
        case WID_BAR_RANSIZE:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeRandomSize(m_pbarSize->getPos());
            break;
        //ランダム位置
        case WID_BAR_RANPOS:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeRandomPos(m_pbarPos->getPos());
            break;
        //円形
        case WID_CK_RANCIRCLE:
            BRUSHMAN->changeRandomPosType(0);
            break;
        //矩形
        case WID_CK_RANBOX:
            BRUSHMAN->changeRandomPosType(1);
            break;
    }

    return TRUE;
}


//*********************************
// CBrushWinTab_press （筆圧）
//*********************************


CBrushWinTab_press::CBrushWinTab_press(AXWindow *pParent)
    : CBrushWinTab_base(pParent)
{
    AXLayout *pl,*plh;

    setLayout(plh = new AXLayoutHorz(0, 15));
    plh->setSpacing(5);

    //サイズ

    plh->addItem(pl = new AXLayoutVert(0, 3));

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_PRESS_SIZE));
    pl->addItem(m_pSize = new CPressCurveView(this, WID_PRESS_SIZE, 60));
    pl->addItem(m_peditSize = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_SIZE, 0));

    m_peditSize->setWidthFromLen(5);
    m_peditSize->setValStatus(CBrushTreeItem::PRESS_MIN, CBrushTreeItem::PRESS_MAX, 2);

    //濃度

    plh->addItem(pl = new AXLayoutVert(0, 3));

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_PRESS_VAL));
    pl->addItem(m_pVal = new CPressCurveView(this, WID_PRESS_VAL, 60));
    pl->addItem(m_peditVal = new AXLineEdit(this, AXLineEdit::ES_SPIN, 0, WID_EDIT_VAL, 0));

    m_peditVal->setWidthFromLen(5);
    m_peditVal->setValStatus(CBrushTreeItem::PRESS_MIN, CBrushTreeItem::PRESS_MAX, 2);
}

void CBrushWinTab_press::setValue(CBrushTreeItem *p)
{
    m_pSize->setVal(p->m_wPressSize);
    m_peditSize->setVal(p->m_wPressSize);

    m_pVal->setVal(p->m_wPressVal);
    m_peditVal->setVal(p->m_wPressVal);
}

BOOL CBrushWinTab_press::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //サイズ
        case WID_PRESS_SIZE:
            BRUSHMAN->changePressSize(m_pSize->getVal());

            m_peditSize->setVal(m_pSize->getVal());
            break;
        //サイズ（エディット）
        case WID_EDIT_SIZE:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                BRUSHMAN->changePressSize(m_peditSize->getVal());

                m_pSize->setVal(m_peditSize->getVal());
            }
            break;
        //濃度
        case WID_PRESS_VAL:
            BRUSHMAN->changePressVal(m_pVal->getVal());

            m_peditVal->setVal(m_pVal->getVal());
            break;
        //濃度（エディット）
        case WID_EDIT_VAL:
            if(uNotify == AXLineEdit::EN_CHANGE)
            {
                BRUSHMAN->changePressVal(m_peditVal->getVal());

                m_pVal->setVal(m_peditVal->getVal());
            }
            break;
    }

    return TRUE;
}


//*********************************
// CBrushWinTab_style （ブラシ形状）
//*********************************


CBrushWinTab_style::CBrushWinTab_style(AXWindow *pParent)
    : CBrushWinTab_base(pParent)
{
    AXLayout *pl,*plh;

    setLayout(pl = new AXLayoutVert(0, 3));
    pl->setSpacing(5);

    //ブラシ画像

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_BRUSHIMG));
    pl->addItem(m_pselImg = new CSelImgBar(this, 0, LF_EXPAND_W, WID_BRUSHIMG, 0));

    //回転タイプ

    pl->addItem(plh = new AXLayoutHorz(LF_EXPAND_W, 5));

    plh->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, STRID_BRVAL_BRUSHIMG_ROTTYPE));
    plh->addItem(m_pcbRotType = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_ROTTYPE, 0));

    m_pcbRotType->addItemTr(STRID_BRVAL_IMGROT_FIX, 0);
    m_pcbRotType->addItemTr(STRID_BRVAL_IMGROT_DIRMOV, 0);

    //回転角度

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_BRUSHIMG_ANGLE));
    pl->addItem(m_pbarAngle = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_ANGLE, 0, 0, 0, 359, 0));

    //回転ランダム

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_BRUSHIMG_RANDOM));
    pl->addItem(m_pbarRand = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_RANDOM, 0, 0, 0, 180, 0));
}

void CBrushWinTab_style::setValue(CBrushTreeItem *p)
{
    _setSelImg();

    m_pcbRotType->setCurSel(p->m_btRotType);
    m_pbarAngle->setPos(p->m_wRotAngle);
    m_pbarRand->setPos(p->m_wRotRandom);
}

//! 画像選択名セット

void CBrushWinTab_style::_setSelImg()
{
    AXString *pstr = &(BRUSHMANEDIT->m_strBrushImg);

    if(pstr->isEmpty())
        m_pselImg->setName(_string(STRGID_SELIMGBAR, STRID_SELIMGBAR_DEFAULT));
    else
        m_pselImg->setName(*pstr);
}

BOOL CBrushWinTab_style::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //画像選択
        case WID_BRUSHIMG:
            if(uNotify == CSelImgBar::NOTIFY_RIGHT)
            {
                //デフォルト

                AXString str;

                BRUSHMAN->changeBrushImg(str);
                _setSelImg();
            }
            else
            {
                //選択ダイアログ

                AXString str;
                CSelImgDlg *pdlg;

                pdlg = new CSelImgDlg(m_pTopLevel, CSelImgDlg::TYPE_BRUSH_BRUSH, BRUSHMANEDIT->m_strBrushImg);
                if(pdlg->run(&str))
                {
                    BRUSHMAN->changeBrushImg(str);
                    _setSelImg();
                }
            }
            break;
        //回転タイプ
        case WID_CB_ROTTYPE:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
                BRUSHMAN->changeBrushImgRotType(m_pcbRotType->getCurSel());
            break;
        //回転角度
        case WID_BAR_ANGLE:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeBrushImgAngle(m_pbarAngle->getPos());
            break;
        //回転ランダム
        case WID_BAR_RANDOM:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                BRUSHMAN->changeBrushImgRandom(m_pbarRand->getPos());
            break;
    }

    return TRUE;
}


//*********************************
// CBrushWinTab_etc （その他）
//*********************************


CBrushWinTab_etc::CBrushWinTab_etc(AXWindow *pParent)
    : CBrushWinTab_base(pParent)
{
    AXLayout *pl;

    setLayout(pl = new AXLayoutVert(0, 3));
    pl->setSpacing(5);

    //テクスチャ画像

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_BRVAL_TEXTUREIMG));
    pl->addItem(m_pselImg = new CSelImgBar(this, 0, LF_EXPAND_W, WID_TEXTUREIMG, 0));

    //チェック

    pl->addItem(m_pckAnti = new AXCheckButton(this, 0, 0, WID_CK_ANTIALIAS, 0, STRID_BRVAL_ANTIALIAS, FALSE));
    pl->addItem(m_pckCurve = new AXCheckButton(this, 0, 0, WID_CK_CURVE, 0, STRID_BRVAL_CURVE, FALSE));
}

void CBrushWinTab_etc::setValue(CBrushTreeItem *p)
{
    _setSelImg();

    m_pckAnti->setCheck(p->m_btFlag & CBrushTreeItem::BFLAG_ANTIALIAS);
    m_pckCurve->setCheck(p->m_btFlag & CBrushTreeItem::BFLAG_CURVE);
}

//! 画像選択名セット

void CBrushWinTab_etc::_setSelImg()
{
    AXString *pstr = &(BRUSHMANEDIT->m_strTexImg);

    if(pstr->isEmpty())
        m_pselImg->setName(_string(STRGID_SELIMGBAR, STRID_SELIMGBAR_FORCE_NONE));
    else if(*pstr == "?")
        m_pselImg->setName(_string(STRGID_SELIMGBAR, STRID_SELIMGBAR_USEOPT));
    else
        m_pselImg->setName(*pstr);
}

BOOL CBrushWinTab_etc::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //画像選択
        case WID_TEXTUREIMG:
            if(uNotify == CSelImgBar::NOTIFY_RIGHT)
            {
                //オプション指定

                AXString str("?");

                BRUSHMAN->changeTextureImg(str);
                _setSelImg();
            }
            else
            {
                //選択ダイアログ

                AXString str;
                CSelImgDlg *pdlg;

                pdlg = new CSelImgDlg(m_pTopLevel, CSelImgDlg::TYPE_BRUSH_TEXTURE, BRUSHMANEDIT->m_strTexImg);
                if(pdlg->run(&str))
                {
                    BRUSHMAN->changeTextureImg(str);
                    _setSelImg();
                }
            }
            break;
        //アンチエイリアス
        case WID_CK_ANTIALIAS:
            BRUSHMAN->changeAntiAlias(m_pckAnti->isChecked());
            break;
        //曲線
        case WID_CK_CURVE:
            BRUSHMAN->changeCurve(m_pckCurve->isChecked());
            break;
    }

    return TRUE;
}
