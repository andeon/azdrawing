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

#include "CDeviceOptDlg.h"

#include "CCmdListTreeView.h"
#include "CConfig.h"
#include "CDevList.h"
#include "CApp.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXButton.h"
#include "AXComboBox.h"
#include "AXListBox.h"
#include "AXListBoxItem.h"
#include "AXMessageBox.h"

#include "defGlobal.h"
#include "defStrID.h"


//------------------------

#define WID_CB_DEVICE       100
#define WID_BTT_DEL         101
#define WID_BTT_CLEAR       102
#define WID_LB_BUTTON       103
#define WID_TREE_CMDLIST    104

//------------------------

//コンボボックスアイテム

class CDevOpt_DevItem:public AXListBoxItem
{
public:
    AXMem   m_memDat;
    int     m_nBttCnt,
            m_nDatCnt;
    BOOL    m_bDefault;

public:
    CDevOpt_DevItem(LPCUSTR pstr,ULONG lParam);

    CDevOpt_DevItem *next() { return (CDevOpt_DevItem *)m_pNext; }

    int getCmd(UINT btt);
};

//ボタン確認ウィジェット

class CDevOpt_button:public AXLabel
{
public:
    CDevOpt_button(AXWindow *pParent,DWORD dwPadding,WORD wStrID);

    BOOL onButtonDown(AXHD_MOUSE *phd);
};


//------------------------


/*!
    @class CDeviceOptDlg
    @brief デバイス設定ダイアログ
*/
/*
    [デバイス一覧コンボボックス]
    m_lParam : CDevItem*
    m_memDat : 動作データ

    [ボタン一覧 m_lParam]
    上位WORD : コマンドID（0xffff で指定なし）
    下位WORD : ボタン＋装飾キー
*/


CDeviceOptDlg::CDeviceOptDlg(AXWindow *pOwner)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl;

    _trgroup(STRGID_DLG_DEVICEOPT);

    setTitle(STRID_DEVOPT_TITLE);

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(10);

    //デバイス一覧

    plTop->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 0));
    pl->setPaddingBottom(6);

    pl->addItem(m_pcbDevice = new AXComboBox(this, 0, LF_EXPAND_W|LF_CENTER_Y, WID_CB_DEVICE, MAKE_DW4(0,0,6,0)));
    pl->addItem(new AXButton(this, WS_DISABLE, 0, WID_BTT_DEL, MAKE_DW4(0,0,2,0), STRID_DEVOPT_DEVDEL));
    pl->addItem(new AXButton(this, 0, 0, WID_BTT_CLEAR, 0, STRID_DEVOPT_DEVCLEAR));

    //ボタン＋コマンドリスト

    plTop->addItem(pl = new AXLayoutMatrix(2, LF_EXPAND_WH));

    pl->addItem(new AXLabel(this, 0, 0, MAKE_DW4(0,0,8,3), STRID_DEVOPT_BTTMOD));
    pl->addItem(new AXLabel(this, 0, 0, MAKE_DW4(0,0,0,3), STRID_DEVOPT_ACTION));
    pl->addItem(m_pBttList = new AXListBox(this, AXScrollView::SVS_VERT|AXScrollView::SVS_SUNKEN, LF_EXPAND_H, WID_LB_BUTTON, MAKE_DW4(0,0,8,0)));
    pl->addItem(m_pTree = new CCmdListTreeView(this, CCmdListTreeView::STYLE_DEVICE, LF_EXPAND_WH, WID_TREE_CMDLIST, 0));

    //ヘルプ

    plTop->addItem(new AXLabel(this, AXLabel::LS_BORDER, LF_EXPAND_W, MAKE_DW4(0,4,0,0), STRID_DEVOPT_HELP));

    //ボタン確認

    plTop->addItem(new CDevOpt_button(this, MAKE_DW4(0,4,0,0), STRID_DEVOPT_BUTTONCHECK));

    //OKキャンセル

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(12);

    //

    _setDevList();
    _setBttList();

    m_pBttList->setAutoWidth(TRUE);

    m_pTree->setCmdList();

    //

    calcDefSize();
    resize((m_nDefW < 500)? 500: m_nDefW, (m_nDefH < 400)? 400: m_nDefH);

    show();
}

//! 通知

BOOL CDeviceOptDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //デバイスリスト
        case WID_CB_DEVICE:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
                _changeDevice();
            break;
        //削除
        case WID_BTT_DEL:
            if(AXMessageBox::message(this, NULL, _string(STRGID_MESSAGE, STRID_MES_DEV_DELETE),
                                     AXMessageBox::YES | AXMessageBox::NO,
                                     AXMessageBox::YES) == AXMessageBox::YES)
                _deviceDelete();
            break;
        //クリア
        case WID_BTT_CLEAR:
            if(AXMessageBox::message(this, NULL, _string(STRGID_MESSAGE, STRID_MES_DEV_CLEAR),
                                     AXMessageBox::YES | AXMessageBox::NO,
                                     AXMessageBox::YES) == AXMessageBox::YES)
                _clearAll();
            break;
        //ボタンリスト
        case WID_LB_BUTTON:
            if(uNotify == AXListBox::LBN_SELCHANGE)
                m_pTree->setSelCmd(m_pBttList->getItemParam(-1) >> CDevItem::SHIFT_ACTION);
            break;
        //コマンドリスト
        case WID_TREE_CMDLIST:
            if(uNotify == AXTreeView::TVN_SELCHANGE)
                _changeCmd();
            break;

        //OK
        case 1:
            _endOK();

            endDialog(1);
            break;
        //キャンセル
        case 2:
            endDialog(0);
            break;
    }

    return TRUE;
}


//============================
//
//============================


//! 終了時、データをセット

void CDeviceOptDlg::_endOK()
{
    CDevOpt_DevItem *pci;

    //現在のデータをセット

    _setDatToCombo();

    //CDevItem にセット

    for(pci = (CDevOpt_DevItem *)m_pcbDevice->getTopItem(); pci; pci = pci->next())
        ((CDevItem *)pci->m_lParam)->setAction(&pci->m_memDat, pci->m_nDatCnt);
}

//! 現在のデバイスデータを削除（直接削除）

void CDeviceOptDlg::_deviceDelete()
{
    CDevItem *p;
    int sel;

    sel = m_pcbDevice->getCurSel();
    if(sel == -1) return;

    p = (CDevItem *)m_pcbDevice->getItemParam(sel);

    //削除

    g_pconf->pDev->deleteItem(p);

    m_pcbDevice->deleteItem(sel);

    //選択

    m_pSelDev = NULL;

    m_pcbDevice->setCurSel(0);

    _changeDevice();
}

//! 動作設定をクリア

void CDeviceOptDlg::_clearAll()
{
    AXListBoxItem *p;

    for(p = m_pBttList->getTopItem(); p; p = p->next())
    {
        p->m_lParam |= 0xffff0000;

        p->m_strText.setAt(0, ' ');
    }

    m_pBttList->updateArea();
    m_pTree->setSelCmd(0xffff);
}

//! 現在の動作データを、コンボボックスアイテム(m_pSelDev)にセット

void CDeviceOptDlg::_setDatToCombo()
{
    AXListBoxItem *pli;
    int cnt = 0;
    LPDWORD pdw;

    if(!m_pSelDev) return;

    //データ数計算

    for(pli = m_pBttList->getTopItem(); pli; pli = pli->next())
    {
        if((pli->m_lParam >> CDevItem::SHIFT_ACTION) != 0xffff)
            cnt++;
    }

    //メモリ確保

    if(cnt > m_pSelDev->m_nDatCnt)
    {
        if(!m_pSelDev->m_memDat.alloc(cnt * sizeof(DWORD)))
            return;
    }

    m_pSelDev->m_nDatCnt = cnt;

    //データセット

    pdw = m_pSelDev->m_memDat;

    for(pli = m_pBttList->getTopItem(); pli; pli = pli->next())
    {
        if((pli->m_lParam >> CDevItem::SHIFT_ACTION) != 0xffff)
            *(pdw++) = (DWORD)pli->m_lParam;
    }
}

//! デバイス選択変更時

void CDeviceOptDlg::_changeDevice()
{
    //現在のデータを保存

    _setDatToCombo();

    //デバイス選択変更

    m_pSelDev = (CDevOpt_DevItem *)m_pcbDevice->getSelItem();

    //ボタン一覧セット

    _setBttList();

    //コマンド選択なし

    m_pTree->setFocusItem(NULL);

    //「削除」の有効/無効

    getWidget(WID_BTT_DEL)->enable(!m_pSelDev->m_bDefault);
}

//! コマンドリスト選択変更時

void CDeviceOptDlg::_changeCmd()
{
    AXTreeViewItem *p;
    AXListBoxItem *pli;
    AXString str;
    UINT cmdBk,cmdNew;

    //ツリー

    p = m_pTree->getFocusItem();
    if(!p) return;

    if(p->m_lParam == 0xffffffff) return;

    cmdNew = p->m_lParam;

    //------ ボタンリスト

    pli = m_pBttList->getSelItem();
    if(!pli) return;

    cmdBk = pli->m_lParam >> CDevItem::SHIFT_ACTION;

    //パラメータ値セット

    pli->m_lParam &= 0xffff;
    pli->m_lParam |= cmdNew << CDevItem::SHIFT_ACTION;

    //ボタン名変更

    if((cmdBk == 0xffff && cmdNew != 0xffff) ||
       (cmdBk != 0xffff && cmdNew == 0xffff))
    {
        pli->m_strText.setAt(0, (cmdNew == 0xffff)? ' ': '*');

        m_pBttList->updateArea();
    }
}

//! デバイス一覧セット

void CDeviceOptDlg::_setDevList()
{
    CDevItem *p;
    CDevOpt_DevItem *pci;
    AXString str;

    for(p = g_pconf->pDev->getTopItem(); p; p = p->next())
    {
        if(p->getDevID() == 2)
            str = _str(STRID_DEVOPT_DEFAULT);
        else
            str = p->getName();

        //追加

        pci = new CDevOpt_DevItem(str, (ULONG)p);

        p->setActEditDat(&pci->m_memDat);

        pci->m_nBttCnt  = p->getBttNum();
        pci->m_nDatCnt  = pci->m_memDat.getSize() / sizeof(DWORD);
        pci->m_bDefault = (p->getDevID() == 2);

        m_pcbDevice->addItem(pci);
    }

    //選択

    m_pcbDevice->setCurSel(0);

    m_pSelDev = (CDevOpt_DevItem *)m_pcbDevice->getSelItem();
}

//! 現在のデバイスのボタン一覧セット

void CDeviceOptDlg::_setBttList()
{
    int i,j,cmd,btt,bttcnt;
    AXString str;
    WORD modList[7] = {
        0, CDevItem::FLAG_CTRL, CDevItem::FLAG_SHIFT, CDevItem::FLAG_ALT,
        CDevItem::FLAG_CTRL|CDevItem::FLAG_SHIFT,
        CDevItem::FLAG_ALT|CDevItem::FLAG_CTRL,
        CDevItem::FLAG_ALT|CDevItem::FLAG_SHIFT
    };

    if(!m_pSelDev) return;

    //すべて削除

    m_pBttList->deleteItemAll();

    //ボタン数

#ifdef _AX_OPT_XI2
    bttcnt = m_pSelDev->m_nBttCnt;
#else
    bttcnt = 11;
#endif

    //セット

    for(i = 0; i < bttcnt; i++)
    {
        for(j = 0; j < 7; j++)
        {
            btt = i | modList[j];

            //設定があれば先頭に "*"

            cmd = m_pSelDev->getCmd(btt);

            if(cmd == -1)
            {
                str = "  ";
                cmd = 0xffff;
            }
            else
                str = "* ";

            //ボタン名

            str += (i + 1);

            if(btt & CDevItem::FLAG_ALT)
                str += "+Alt";

            if(btt & CDevItem::FLAG_CTRL)
                str += "+Ctrl";

            if(btt & CDevItem::FLAG_SHIFT)
                str += "+Shift";

            //

            m_pBttList->addItem(str, ((UINT)cmd << CDevItem::SHIFT_ACTION) | btt);
        }
    }
}


//***************************************
// CDevOpt_DevItem : デバイス一覧のアイテム
//***************************************


CDevOpt_DevItem::CDevOpt_DevItem(LPCUSTR pstr,ULONG lParam)
    : AXListBoxItem(pstr, lParam)
{

}

//! ボタンからコマンドID取得

int CDevOpt_DevItem::getCmd(UINT btt)
{
    LPDWORD p = m_memDat;
    int i;

    for(i = m_nDatCnt; i > 0; i--, p++)
    {
        if((*p & CDevItem::MASK_BUTTONMOD) == btt)
            return *p >> CDevItem::SHIFT_ACTION;
    }

    return -1;
}


//***************************************
// CDevOpt_button : ボタン確認ウィジェット
//***************************************


CDevOpt_button::CDevOpt_button(AXWindow *pParent,DWORD dwPadding,WORD wStrID)
    : AXLabel(pParent, AXLabel::LS_SUNKEN|AXLabel::LS_CENTER, LF_EXPAND_W, dwPadding, wStrID)
{
    m_nMinH = 20;
    m_uFlags |= AXWindow::FLAG_WHEELEVENT_NORMAL;
}

BOOL CDevOpt_button::onButtonDown(AXHD_MOUSE *phd)
{
    AXString str;

    str = "button: ";
    str += phd->button;

    AXLabel::setText(str);

    return TRUE;
}
