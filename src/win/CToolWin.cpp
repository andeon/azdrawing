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

#include "CToolWin.h"

#include "toolwinWidget.h"
#include "CConfig.h"
#include "CDraw.h"
#include "CResource.h"
#include "defGlobal.h"
#include "defStrID.h"

#include "AXLayout.h"
#include "AXToolBar.h"
#include "AXApp.h"


//-------------------

#define WID_SCALEBAR    100
#define WID_ROTBAR      101
#define WID_SCALE_ADD   102
#define WID_SCALE_SUB   103
#define WID_SCALE_CLEAR 104
#define WID_ROT_ADD     105
#define WID_ROT_SUB     106
#define WID_ROT_CLEAR   107

//-------------------

/*!
    @class CToolWin
    @brief ツールウィンドウ
*/


CToolWin *CToolWin::m_pSelf = NULL;


CToolWin::CToolWin(AXWindow *pOwner,UINT addstyle)
    : AXTopWindow(pOwner,
                  WS_HIDE | WS_BK_FACE | WS_TITLE | WS_CLOSE | WS_BORDER | WS_DISABLE_IM |
                  WS_DISABLE_RESIZE | addstyle)
{
    AXLayout *pl;

    m_pSelf = this;

    //

    setFont(g_pres->m_pfontSmall);

    setTitle("tool");

    attachAccel(((AXTopWindow *)m_pOwner)->getAccel());

    //

    setLayout(pl = new AXLayoutVert);

    pl->setSpacing(1);

    _createTB_tool();
    _createTB_sub();
    _createWidget();

    //

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    //

    _initTB_sub();

    changeTool();
}

//! 表示/非表示

void CToolWin::showChange()
{
    if(g_pconf->isViewToolWin())
        showRestore();
    else
        hide();
}

//! ツールバーチェック（ツール）

void CToolWin::checkTB_tool(int no)
{
    m_ptbTool->checkItem(CMDID_TOOL + no, TRUE);
}

//! ツールバーチェック（ブラシ/ドットの描画タイプ）

void CToolWin::checkTB_toolsub_pen(int no)
{
    m_ptbSub[SUBNO_DRAW]->checkItem(CMDID_SUB_TOP + no, TRUE);
}

//! ツールバーチェック（ツールのコマンドボタン）

void CToolWin::checkTB_cmd(UINT uID,BOOL bCheck)
{
    m_ptbTool->checkItem(uID, bCheck);
}

//! 表示倍率変更時

void CToolWin::changeScale()
{
    m_pwidScale->setPos(g_pdraw->m_nCanvasScale);
}

//! 回転変更時

void CToolWin::changeRotate()
{
    m_pwidRot->setPos(g_pdraw->m_nCanvasRot);
}

//! ツール変更時

void CToolWin::changeTool()
{
    int no = g_pdraw->m_toolno,subno;
    AXToolBar *ptbNew;

    //--------- ツール

    m_ptbTool->checkItem(CMDID_TOOL + no, TRUE);

    //--------- サブタイプ

    switch(no)
    {
        case CDraw::TOOL_BRUSH:
        case CDraw::TOOL_DOT:
            subno = SUBNO_DRAW;
            break;
        case CDraw::TOOL_POLYPAINT:
        case CDraw::TOOL_POLYERASE:
            subno = SUBNO_POLYGON;
            break;
        case CDraw::TOOL_PAINTCLOSE:
        case CDraw::TOOL_SEL:
            subno = SUBNO_SEL;
            break;
        case CDraw::TOOL_GRAD:
            subno = SUBNO_GRAD;
            break;
        case CDraw::TOOL_BOXEDIT:
            subno = SUBNO_BOXEDIT;
            break;
        default:
            subno = -1;
            break;
    }

    ptbNew = (subno == -1)? NULL: m_ptbSub[subno];

    //変更

    if(ptbNew != m_ptbSubNow)
    {
        if(m_ptbSubNow)
            m_ptbSubNow->hide();

        m_ptbSubNow = ptbNew;
    }

    if(ptbNew)
    {
        ptbNew->checkItem(CMDID_SUB_TOP + subno * 100 + g_pdraw->m_toolSubNo[no], TRUE);
        ptbNew->show();
    }
}


//=========================
//初期化
//=========================


//! ツールバー作成：ツール

void CToolWin::_createTB_tool()
{
    AXString str;
    UINT f;
    int i,no;

    m_ptbTool = new AXToolBar(this, AXToolBar::TBS_SEPBOTTOM | AXToolBar::TBS_TOOLTIP, 0);
    m_ptbTool->setImageList(g_pres->m_pilToolTB);

    getLayout()->addItem(m_ptbTool);

    //ツールチップ

    _addTrString(&str, STRGID_TOOLNAME, CDraw::TOOL_NUM);
    _addTrString(&str, STRGID_TOOLWINCMD, 4);

    m_ptbTool->setToolTipText(str);

    //ツール

    for(i = 0; i < CDraw::TOOL_NUM; i++)
    {
        f = AXToolBar::BF_CHECKGROUP;
        if(i == 9) f |= AXToolBar::BF_WRAP;

        m_ptbTool->addItem(CMDID_TOOL + i, i, i, f, 0);
    }

    //他

    no = CDraw::TOOL_NUM;

    m_ptbTool->addItemSep();
    m_ptbTool->addItem(CMDID_UNDO, no, no, 0, 0);
    m_ptbTool->addItem(CMDID_REDO, no + 1, no + 1, 0, 0);
    m_ptbTool->addItem(CMDID_DESELECT, no + 2, no + 2, 0, 0);
    m_ptbTool->addItem(CMDID_CANVASHREV, no + 3, no + 3, AXToolBar::BF_CHECKBUTTON, 0);
}

//! ツールバー作成：サブタイプ

void CToolWin::_createTB_sub()
{
    AXString str;
    int type,i,id,imgno = 0;
    int num[5] = {CDraw::DRAWTYPE_NUM, 4, 4, 3, CDraw::BOXEDIT_NUM};

    for(type = 0; type < 5; type++)
    {
        m_ptbSub[type] = new AXToolBar(this,
                            WS_HIDE | AXToolBar::TBS_SEPBOTTOM | ((type == SUBNO_DRAW || type == SUBNO_BOXEDIT)? AXToolBar::TBS_TOOLTIP: 0),
                            LF_EXPAND_W);

        m_ptbSub[type]->setImageList(g_pres->m_pilToolSubTB);

        //ツールチップ

        str.empty();

        switch(type)
        {
            case SUBNO_DRAW:
                _addTrString(&str, STRGID_DRAWTYPE, CDraw::DRAWTYPE_NUM);
                m_ptbSub[type]->setToolTipText(str);
                break;
            case SUBNO_BOXEDIT:
                _addTrString(&str, STRGID_BOXEDIT, CDraw::BOXEDIT_NUM);
                m_ptbSub[type]->setToolTipText(str);
                break;
        }

        //ボタン

        id = CMDID_SUB_TOP + type * 100;

        for(i = 0; i < num[type]; i++)
            m_ptbSub[type]->addItem(id++, imgno++, i, AXToolBar::BF_CHECKGROUP, 0);
    }

    //レイアウト時の位置・サイズを取得するため、セットしておく

    m_ptbSubNow = m_ptbSub[0];

    getLayout()->addItem(m_ptbSubNow);

    m_ptbSubNow->show();
}

//! サブタイプのツールバーを初期化
/*!
    m_ptbSub[0] がレイアウト済みなので、他のツールバーをその位置・サイズに合わせる。
*/

void CToolWin::_initTB_sub()
{
    int i;
    AXRectSize rcs;

    m_ptbSub[0]->getRectSize(&rcs);

    for(i = 1; i < 5; i++)
        m_ptbSub[i]->moveresize(rcs.x, rcs.y, rcs.w, rcs.h);
}

//! ツールチップ用文字列に追加

void CToolWin::_addTrString(AXString *pstr,WORD wGroupID,int cnt)
{
    _trgroup(wGroupID);

    for(int i = 0; i < cnt; i++)
    {
        *pstr += _str(i);
        *pstr += '\t';
    }
}

//! ウィジェット作成

void CToolWin::_createWidget()
{
    AXLayout *pl;

    getLayout()->addItem(pl = new AXLayoutMatrix(5, LF_EXPAND_W));

    pl->setPadding(3,4,2,2);

    //表示倍率

    pl->addItem(new CImgListLabel(this, LF_CENTER_Y, MAKE_DW4(0,0,4,3), g_pres->m_pilToolUnder, 0));
    pl->addItem(m_pwidScale = new CScaleRotBar(this, 0, WID_SCALEBAR, MAKE_DW4(0,0,4,3), CDraw::SCALE_MIN, CDraw::SCALE_MAX));
    pl->addItem(new CImgListButton(this, LF_CENTER_Y, WID_SCALE_ADD, MAKE_DW4(0,0,1,3), g_pres->m_pilToolUnder, 2));
    pl->addItem(new CImgListButton(this, LF_CENTER_Y, WID_SCALE_SUB, MAKE_DW4(0,0,1,3), g_pres->m_pilToolUnder, 3));
    pl->addItem(new CImgListButton(this, LF_CENTER_Y, WID_SCALE_CLEAR, MAKE_DW4(0,0,0,3), g_pres->m_pilToolUnder, 4));

    m_pwidScale->setPos(g_pdraw->m_nCanvasScale);

    //回転

    pl->addItem(new CImgListLabel(this, LF_CENTER_Y, MAKE_DW4(0,0,4,0), g_pres->m_pilToolUnder, 1));
    pl->addItem(m_pwidRot = new CScaleRotBar(this, CScaleRotBar::SRBS_ROTATE, WID_ROTBAR, MAKE_DW4(0,0,4,0), 0, 0));
    pl->addItem(new CImgListButton(this, LF_CENTER_Y, WID_ROT_ADD, MAKE_DW4(0,0,1,0), g_pres->m_pilToolUnder, 2));
    pl->addItem(new CImgListButton(this, LF_CENTER_Y, WID_ROT_SUB, MAKE_DW4(0,0,1,0), g_pres->m_pilToolUnder, 3));
    pl->addItem(new CImgListButton(this, LF_CENTER_Y, WID_ROT_CLEAR, 0, g_pres->m_pilToolUnder, 4));
}


//=========================
//ハンドラ
//=========================


//! 閉じる（非表示）

BOOL CToolWin::onClose()
{
    m_pOwner->onCommand(STRID_MENU_VIEW_TOOLWIN, 0, 0);

    return TRUE;
}

//! 通知

BOOL CToolWin::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //表示倍率バー
        /* 押し時は、キャンバス基準位置を中央に */
        case WID_SCALEBAR:
            g_pdraw->changeScale(m_pwidScale->getPos(), TRUE, !m_pwidScale->isDrag(), (uNotify == CScaleRotBar::SRBN_DOWN));
            break;
        //表示倍率+
        case WID_SCALE_ADD:
            g_pdraw->scaleUpDown(TRUE);
            break;
        //表示倍率-
        case WID_SCALE_SUB:
            g_pdraw->scaleUpDown(FALSE);
            break;
        //表示倍率100
        case WID_SCALE_CLEAR:
            g_pdraw->changeScale(100);
            break;
        //回転バー
        case WID_ROTBAR:
            g_pdraw->changeRotate(m_pwidRot->getPos(), TRUE, !m_pwidRot->isDrag(), (uNotify == CScaleRotBar::SRBN_DOWN));
            break;
        //回転+
        case WID_ROT_ADD:
            g_pdraw->changeRotate(g_pdraw->m_nCanvasRot + g_pconf->nRotateWidth * 100);
            break;
        //回転-
        case WID_ROT_SUB:
            g_pdraw->changeRotate(g_pdraw->m_nCanvasRot - g_pconf->nRotateWidth * 100);
            break;
        //回転0
        case WID_ROT_CLEAR:
            g_pdraw->changeRotate(0);
            break;
    }

    return TRUE;
}

//! コマンド

BOOL CToolWin::onCommand(UINT uID,ULONG lParam,int from)
{
    int n;

    //ツール

    if(uID >= CMDID_TOOL && uID < CMDID_TOOL + CDraw::TOOL_NUM)
    {
        g_pdraw->changeTool(uID - CMDID_TOOL);
        return TRUE;
    }

    //サブタイプ

    if(uID >= CMDID_SUB_TOP && uID < CMDID_SUB_TOP + 1000)
    {
        for(n = CMDID_SUB_TOP + 4 * 100; n >= CMDID_SUB_TOP; n -= 100)
        {
            if(uID >= (UINT)n) { n = uID - n; break; }
        }

        g_pdraw->changeToolSubType(n);
        return TRUE;
    }

    //他ボタン

    n = -1;

    switch(uID)
    {
        case CMDID_UNDO:
            n = STRID_MENU_EDIT_UNDO;
            break;
        case CMDID_REDO:
            n = STRID_MENU_EDIT_REDO;
            break;
        case CMDID_DESELECT:
            n = STRID_MENU_SEL_DESELECT;
            break;
        case CMDID_CANVASHREV:
            n = STRID_MENU_VIEW_CANVASHREV;
            break;
    }

    if(n != -1)
        m_pOwner->onCommand(n, 0, 0);

    return TRUE;
}
