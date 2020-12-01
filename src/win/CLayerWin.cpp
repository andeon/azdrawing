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

#include "CLayerWin.h"

#include "CLayerWinArea.h"
#include "CMainWin.h"
#include "CLayerOptDlg.h"
#include "CLayerList.h"
#include "CConfig.h"
#include "CDraw.h"
#include "CResource.h"

#include "AXLayout.h"
#include "AXToolBar.h"
#include "AXScrollBar.h"
#include "AXMenu.h"
#include "AXUtilStr.h"
#include "AXFileDialog.h"
#include "AXDND.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"

#define _AZDW_MENUDAT_LAYERMENU
#include "menudat.h"


//------------------------

#define CMDID_TB_MENU   1

//------------------------

/*!
    @class CLayerWin
    @brief レイヤウィンドウ
*/


CLayerWin *CLayerWin::m_pSelf = NULL;


CLayerWin::CLayerWin(AXWindow *pOwner,UINT addstyle)
    : AXTopWindow(pOwner,
                  WS_HIDE | WS_TITLE | WS_CLOSE | WS_BORDER | WS_DISABLE_IM | WS_DROP | addstyle)
{
    AXLayout *pl,*plh;

    m_pSelf = this;

    //

    setTitle("layer");

    enableDND();

    attachAccel(((AXTopWindow *)m_pOwner)->getAccel());

    //レイアウト

    setLayout(pl = new AXLayoutVert);

    _createToolBar();

    pl->addItem(plh = new AXLayoutHorz(LF_EXPAND_WH, 0));
    plh->addItem(m_pArea = new CLayerWinArea(this));
    plh->addItem(m_pScrV = new AXScrollBar(this, AXScrollBar::SBS_VERT, LF_EXPAND_H));

    m_pArea->setScrollBar(m_pScrV);
}

//! 表示/非表示 切り替え

void CLayerWin::showChange()
{
    if(g_pconf->isViewLayerWin())
        showRestore();
    else
        hide();
}

//! 一覧全体更新

void CLayerWin::updateLayerAll(BOOL bScr)
{
    if(bScr) m_pArea->setScrollInfo();
    m_pArea->draw();
}

//! 指定レイヤを更新

void CLayerWin::updateLayerNo(int no)
{
    m_pArea->drawOne(no, TRUE, TRUE, TRUE);
}

//! 指定レイヤのプレビュー更新

void CLayerWin::updateLayerNoPrev(int no)
{
    m_pArea->drawOne(no, TRUE, FALSE, TRUE);
}

//! カレントレイヤの情報更新

void CLayerWin::updateLayerCurInfo()
{
    m_pArea->drawOne(g_pdraw->m_nCurLayerNo, FALSE, TRUE, TRUE);
}

//! レイヤ設定

void CLayerWin::layerOption(CLayerItem *pItem)
{
    CLayerOptDlg *pDlg = new CLayerOptDlg(MAINWIN, pItem);
    int ret;

    ret = pDlg->runDialog();

    if(ret)
    {
        //更新

        if(ret == 2) g_pdraw->updateAll();

        m_pArea->drawOne(g_pdraw->getLayerPos(pItem), FALSE, TRUE, TRUE);
    }
}


//=========================
//サブ
//=========================


//! ツールバー作成

void CLayerWin::_createToolBar()
{
    int i;
    WORD wID[] = {
        STRID_LAYERMENU_NEW, STRID_LAYERMENU_COPY, STRID_LAYERMENU_DEL,
        STRID_LAYERMENU_COMBINECLEAR, STRID_LAYERMENU_COMBINEDEL, STRID_LAYERMENU_CLEAR,
        STRID_LAYERMENU_MOVEUP, STRID_LAYERMENU_MOVEDOWN, CMDID_TB_MENU
    };

    m_pToolBar = new AXToolBar(this, AXToolBar::TBS_SPACEBOTTOM | AXToolBar::TBS_TOOLTIP | AXToolBar::TBS_TOOLTIP_TR, LF_EXPAND_W);
    m_pToolBar->setImageList(g_pres->m_pilLayerTB);
    m_pToolBar->setToolTipTrGroup(STRGID_LAYERTB_TTIP);

    getLayout()->addItem(m_pToolBar);

    for(i = 0; i < 9; i++)
        m_pToolBar->addItem(wID[i], i, wID[i], 0, 0);
}

//! メニュー表示

void CLayerWin::_showMenu()
{
    AXMenu *pm;
    AXRect rc;

    _trgroup(STRGID_LAYERWINMENU);

    pm = new AXMenu;

    pm->addTrArray(g_layermenudat, sizeof(g_layermenudat)/sizeof(WORD));

    m_pToolBar->getItemRect(CMDID_TB_MENU, &rc, TRUE);
    pm->popup(this, rc.left, rc.bottom, 0);

    delete pm;
}

//! 画像からレイヤ新規作成

void CLayerWin::_layerNewFromImg()
{
    AXString filter,str;

    filter = "Image File(BMP/PNG/GIF/JPEG)\t*.bmp;*.png;*.gif;*.jpg;*.jpeg\tAll Files\t*\t";

    if(AXFileDialog::openFile(this, filter, 0, g_pconf->strOpenDir, 0, &str))
        g_pdraw->layer_newFromImg(&str);
}

//! アルファ付きPNGに保存

void CLayerWin::_layerSavePNG()
{
    AXString filter,str,dir;

    filter = "PNG file (*.png)\t*.png\t";

    //初期ディレクトリ

    str = MAINWIN->getEditFileName();

    if(str.isEmpty())
        dir = g_pconf->strSaveDir;
    else
        dir.path_removeFileName(str);

    //

    if(AXFileDialog::saveFile(this, filter, 0, dir, 0, &str))
    {
        str.path_setExt("png");

        if(!g_pdraw->m_pCurLayer->m_img.savePNG32(str, g_pdraw->m_nImgW, g_pdraw->m_nImgH, g_pdraw->m_pCurLayer->m_dwCol))
            errMes(STRGID_MESSAGE, STRID_MES_ERR_SAVE);
    }
}


//=========================
//ハンドラ
//=========================


//! 閉じる（非表示）

BOOL CLayerWin::onClose()
{
    m_pOwner->onCommand(STRID_MENU_VIEW_LAYERWIN, 0, 0);

    return TRUE;
}

//! 通知

BOOL CLayerWin::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pScrV && uNotify == AXScrollBar::SBN_SCROLL)
        m_pArea->draw();

    return TRUE;
}

//! D&D

BOOL CLayerWin::onDND_Drop(AXDND *pDND)
{
    AXString str,fname;
    LPCUSTR p;

    pDND->getFiles(&str);
    pDND->endDrop();

    if(str.isNoEmpty())
    {
        str.replace('\t', 0);

        for(p = str; *p; p += AXUStrLen(p) + 1)
        {
            fname = p;
            g_pdraw->layer_newFromImg(&fname, NULL, FALSE);
        }

        //最後に更新
        g_pdraw->updateAllAndLayer(TRUE);
    }

    return TRUE;
}

//! コマンド

BOOL CLayerWin::onCommand(UINT uID,ULONG lParam,int from)
{
    switch(uID)
    {
        //ツールバー、メニュー表示
        case CMDID_TB_MENU:
            _showMenu();
            break;

        //設定
        case STRID_LAYERMENU_OPTION:
            layerOption(g_pdraw->m_pCurLayer);
            break;
        //新規作成
        case STRID_LAYERMENU_NEW:
            g_pdraw->layer_new(TRUE);
            break;
        //画像から新規作成
        case STRID_LAYERMENU_LOADIMG:
            _layerNewFromImg();
            break;
        //複製
        case STRID_LAYERMENU_COPY:
            g_pdraw->layer_copy();
            break;
        //削除
        case STRID_LAYERMENU_DEL:
            g_pdraw->layer_del(TRUE);
            break;
        //クリア
        case STRID_LAYERMENU_CLEAR:
            g_pdraw->layer_curClear();
            break;
        //下レイヤに結合してクリア
        case STRID_LAYERMENU_COMBINECLEAR:
            g_pdraw->layer_combineClear();
            break;
        //下レイヤに結合して削除
        case STRID_LAYERMENU_COMBINEDEL:
            g_pdraw->layer_combineDel(0);
            break;
        //下レイヤに減算結合
        case STRID_LAYERMENU_COMBINE_SUB:
            g_pdraw->layer_combineDel(1);
            break;
        //すべての表示レイヤを結合
        case STRID_LAYERMENU_ALLCOMBINE:
            g_pdraw->layer_combineSome(FALSE);
            break;
        //すべてのチェックレイヤを結合
        case STRID_LAYERMENU_COMBINE_CHECK:
            g_pdraw->layer_combineSome(TRUE);
            break;
        //すべての表示レイヤを新規レイヤに結合
        case STRID_LAYERMENU_ALLCOMBINE_NEW:
            g_pdraw->layer_combineAllViewNew();
            break;
        //上へ移動
        case STRID_LAYERMENU_MOVEUP:
            g_pdraw->layer_moveUp(TRUE);
            break;
        //下へ移動
        case STRID_LAYERMENU_MOVEDOWN:
            g_pdraw->layer_moveDown(TRUE);
            break;
        //すべて表示
        case STRID_LAYERMENU_ALLSHOW:
            g_pdraw->layer_viewAll(1);
            break;
        //すべて非表示
        case STRID_LAYERMENU_ALLHIDE:
            g_pdraw->layer_viewAll(0);
            break;
        //現在のレイヤのみ表示
        case STRID_LAYERMENU_CURSHOW:
            g_pdraw->layer_viewAll(2);
            break;
        //グループレイヤの表示/非表示
        case STRID_LAYERMENU_GROUPSHOWTOGGLE:
            g_pdraw->layer_toggleShowGroup();
            break;
        //チェックレイヤの表示/非表示
        case STRID_LAYERMENU_CHECKSHOWTOGGLE:
            g_pdraw->layer_toggleShowCheck();
            break;
        //アルファ付きPNGに保存
        case STRID_LAYERMENU_SAVEPNG:
            _layerSavePNG();
            break;
    }

    return TRUE;
}
