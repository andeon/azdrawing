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

#include "COptionWin.h"

#include "CConfig.h"
#include "CDraw.h"
#include "CResource.h"
#include "CGradList.h"
#include "CImage8.h"

#include "CMainWin.h"
#include "CValBar2.h"
#include "CSelImgBar.h"
#include "CImgPrev.h"
#include "CSelImgDlg.h"
#include "CGradEditDlg.h"

#include "defGlobal.h"
#include "defStrID.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXTab.h"
#include "AXButton.h"
#include "AXCheckButton.h"
#include "AXColorButton.h"
#include "AXLineEdit.h"
#include "AXListBox.h"
#include "AXComboBox.h"
#include "AXMenu.h"
#include "AXImage.h"
#include "AXImage32.h"
#include "AXColorDialog.h"
#include "AXMessageBox.h"
#include "AXGC.h"
#include "AXUtilStr.h"
#include "AXAppRes.h"
#include "AXApp.h"

#include "COptionWin_tabwin.h"

//-------------------

#define WID_TAB     100

//-------------------

/*!
    @class COptionWin
    @brief オプションウィンドウ
*/


COptionWin *COptionWin::m_pSelf = NULL;


COptionWin::COptionWin(AXWindow *pOwner,UINT addstyle)
    : AXTopWindow(pOwner,
                  WS_HIDE | WS_BK_FACE | WS_TITLE | WS_CLOSE | WS_BORDER | addstyle)
{
    AXLayout *pl;
    int i;

    m_pSelf = this;

    m_pwin = NULL;

    //

    setFont(g_pres->m_pfontSubWin);

    setTitle("option");

    attachAccel(MAINWIN->getAccel());

    //----------

    setLayout(pl = new AXLayoutVert);

    //タブ

    _trgroup(STRGID_OPTWINTAB);

    pl->addItem(m_ptab = new AXTab(this, AXTab::TABS_TOPTAB | AXTab::TABS_FIT, LF_EXPAND_W, WID_TAB, MAKE_DW4(0,0,0,4)));

    for(i = 0; i < 5; i++)
        m_ptab->addItem(i);

    //初期タブ

    m_ptab->setCurSel(g_pconf->btOptWinTab);

    _setTab_main();
}

//! 表示/非表示

void COptionWin::showChange()
{
    if(g_pconf->isViewOptWin())
        showRestore();
    else
        hide();
}

//! ツール変更時

void COptionWin::changeTool()
{
    //タブの選択が「ツール」の場合、タブ内ウィンドウ変更

    if(m_ptab->getCurSel() == 0)
        _setTab_tool();
}


//=========================
//ハンドラ
//=========================


//! 閉じる（非表示）

BOOL COptionWin::onClose()
{
    m_pOwner->onCommand(STRID_MENU_VIEW_OPTIONWIN, 0, 0);

    return TRUE;
}

//! 通知

BOOL COptionWin::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    //タブ選択変更

    if(pwin == m_ptab && uNotify == AXTab::TABN_SELCHANGE)
    {
        g_pconf->btOptWinTab = m_ptab->getCurSel();

        _setTab_main();
    }

    return TRUE;
}


//=========================
//各タブのウィジェットセット
//=========================


//! レイアウトをクリア（タブのみにする）

void COptionWin::_clearLayout()
{
    if(m_pwin)
    {
        m_pwin->removeFromLayout();

        delete m_pwin;

        m_pwin = NULL;
    }
}

//! タブのウィンドウセット

void COptionWin::_setTab_main()
{
    int no;

    _clearLayout();

    no = g_pconf->btOptWinTab;

    if(no == 0)
        //ツール
        _setTab_tool();
    else
    {
        switch(no)
        {
            //定規
            case 1:
                m_pwin = new COptWin_rule;
                break;
            //テクスチャ
            case 2:
                m_pwin = new COptWin_texture;
                break;
            //入り抜き
            case 3:
                m_pwin = new COptWin_headtail;
                break;
            //設定
            case 4:
                m_pwin = new COptWin_option;
                break;
        }

        if(m_pwin)
            m_pLayout->addItem(m_pwin);

        calcDefSize();
        layout();
    }
}

//! ツールタブのウィンドウセット

void COptionWin::_setTab_tool()
{
    _clearLayout();

    switch(g_pdraw->m_toolno)
    {
        case CDraw::TOOL_DOT:
            m_pwin = new COptWin_tool_dot;
            break;
        case CDraw::TOOL_POLYPAINT:
        case CDraw::TOOL_POLYERASE:
            m_pwin = new COptWin_tool_polygon;
            break;
        case CDraw::TOOL_PAINT:
            m_pwin = new COptWin_tool_paint;
            break;
        case CDraw::TOOL_PAINTERASE:
            m_pwin = new COptWin_tool_painterase;
            break;
        case CDraw::TOOL_PAINTCLOSE:
            m_pwin = new COptWin_tool_paintclose;
            break;
        case CDraw::TOOL_GRAD:
            m_pwin = new COptWin_tool_grad;
            break;
        case CDraw::TOOL_MOVE:
            m_pwin = new COptWin_tool_move;
            break;
        case CDraw::TOOL_SEL:
            m_pwin = new COptWin_tool_sel;
            break;
    }

    if(m_pwin)
        m_pLayout->addItem(m_pwin);

    calcDefSize();
    layout();
}


//++++++++++++++++++++++++++++++++++
// 各タブの中身のウィンドウ
//++++++++++++++++++++++++++++++++++


//*********************************
// COptWin_tool_dot （ドットペン）
//*********************************


COptWin_tool_dot::COptWin_tool_dot()
    : AXWindow(OPTWIN, WS_BK_FACE, 0)
{
    AXLayout *pl,*plh;
    int size;

    _trgroup(STRGID_OPTWIN_DOT);

    setLayout(pl = new AXLayoutVert(0, 6));

    pl->setSpacing(4);

    //サイズ

    size = g_pdraw->m_optToolDot & CDraw::TOOLDOT_MASK_SIZE;

    pl->addItem(plh = new AXLayoutHorz(0, 2));

    plh->addItem(new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_1PX, 0, "1px", (size == 0)));
    plh->addItem(new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_2PX, 0, "2px", (size == 1)));
    plh->addItem(new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_3PX, 0, "3px", (size == 2)));

    //細線

    pl->addItem(new AXCheckButton(this, 0, 0, WID_THIN, 0, STRID_OPTWINDOT_THIN, g_pdraw->m_optToolDot & CDraw::TOOLDOT_FLAG_THIN));
}

BOOL COptWin_tool_dot::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        case WID_1PX:
        case WID_2PX:
        case WID_3PX:
            g_pdraw->m_optToolDot &= ~CDraw::TOOLDOT_MASK_SIZE;
            g_pdraw->m_optToolDot |= pwin->getItemID() - WID_1PX;
            break;
        case WID_THIN:
            g_pdraw->m_optToolDot ^= CDraw::TOOLDOT_FLAG_THIN;
            break;
    }

    return TRUE;
}


//**************************************
// COptWin_tool_polygon （図形塗りつぶし）
//**************************************


COptWin_tool_polygon::COptWin_tool_polygon()
    : AXWindow(OPTWIN, WS_BK_FACE, LF_EXPAND_W)
{
    AXLayout *pl;
    int i;
    UINT val;
    WORD id[] = {STRID_PIXTYPE_BLEND, STRID_PIXTYPE_COMPARE, STRID_PIXTYPE_OVERWRITE};

    if(g_pdraw->m_toolno == CDraw::TOOL_POLYPAINT)
        m_pOptVal = &g_pdraw->m_optToolPolyPaint;
    else
        m_pOptVal = &g_pdraw->m_optToolPolyErase;

    val = *m_pOptVal;

    //

    setLayout(pl = new AXLayoutVert(0, 4));

    pl->setSpacing(4);

    //濃度

    pl->addItem(m_pbarVal = new CValBar2(this,
                                    (val & CDraw::TOOLPOLY_F_BRUSHVAL)? WS_DISABLE: 0,
                                    LF_EXPAND_W, WID_VAL, 0, 0, 1, 255, val & CDraw::TOOLPOLY_VAL_MASK));

    //描画タイプ

    if(g_pdraw->m_toolno == CDraw::TOOL_POLYPAINT)
    {
        pl->addItem(m_pcbPixel = new AXComboBox(this, 0, LF_EXPAND_W, WID_PIXEL, 0));

        _trgroup(STRGID_PIXELTYPE);

        for(i = 0; i < 3; i++)
            m_pcbPixel->addItemTr(id[i], id[i]);

        m_pcbPixel->setCurSel_findParam((val >> CDraw::TOOLPOLY_PIX_SHIFT) & CDraw::TOOLPOLY_PIX_MASK);
    }

    //チェック

    _trgroup(STRGID_WORD);

    pl->addItem(new AXCheckButton(this, 0, 0, WID_BRUSHVAL, 0, STRID_WORD_USEBRUSHVAL,
                                  val & CDraw::TOOLPOLY_F_BRUSHVAL));

    pl->addItem(new AXCheckButton(this, 0, 0, WID_ANTI, 0, STRID_WORD_ANTIALIAS,
                                  val & CDraw::TOOLPOLY_F_ANTI));
}

BOOL COptWin_tool_polygon::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //濃度
        case WID_VAL:
            *m_pOptVal &= ~CDraw::TOOLPOLY_VAL_MASK;
            *m_pOptVal |= m_pbarVal->getPos();
            break;
        //描画タイプ
        case WID_PIXEL:
            *m_pOptVal &= ~(CDraw::TOOLPOLY_PIX_MASK << CDraw::TOOLPOLY_PIX_SHIFT);
            *m_pOptVal |= m_pcbPixel->getItemParam(-1) << CDraw::TOOLPOLY_PIX_SHIFT;
            break;
        //ブラシの濃度を使う
        case WID_BRUSHVAL:
            *m_pOptVal ^= CDraw::TOOLPOLY_F_BRUSHVAL;
            enableWidget(WID_VAL, !(*m_pOptVal & CDraw::TOOLPOLY_F_BRUSHVAL));
            break;
        //アンチエイリアス
        case WID_ANTI:
            *m_pOptVal ^= CDraw::TOOLPOLY_F_ANTI;
            break;
    }

    return TRUE;
}


//**************************************
// COptWin_tool_paint （塗りつぶし）
//**************************************


COptWin_tool_paint::COptWin_tool_paint()
    : AXWindow(OPTWIN, WS_BK_FACE, LF_EXPAND_W)
{
    AXLayout *pl;
    UINT val = g_pdraw->m_optToolPaint;
    WORD id[] = { STRID_PIXTYPE_BLEND, STRID_PIXTYPE_COMPARE, STRID_PIXTYPE_OVERWRITE, STRID_PIXTYPE_ERASE };
    int i,type;

    setLayout(pl = new AXLayoutVert(0, 3));

    pl->setSpacing(4);

    //濃度

    pl->addItem(m_pbarVal = new CValBar2(this,
                                (val & CDraw::TOOLPAINT_F_USEBRUSH)? WS_DISABLE: 0,
                                LF_EXPAND_W, WID_VAL, 0, 0, 1, 255, val & 255));

    //タイプ

    _trgroup(STRGID_OPTWIN_PAINT);

    type = (val >> CDraw::TOOLPAINT_TYPE_SHIFT) & CDraw::TOOLPAINT_TYPE_MASK;

    pl->addItem(m_pcbType = new AXComboBox(this, 0, LF_EXPAND_W, WID_TYPE, 0));

    m_pcbType->addItemMulTr(STRID_OPTWINPAINT_TYPE1, 2);
    m_pcbType->setCurSel(type);

    //指定濃度

    pl->addItem(m_pbarValCmp = new CValBar2(this, (type == 0)? WS_DISABLE: 0,
                                LF_EXPAND_W, WID_VALCMP, 0, 0, 1, 255,
                                (val >> CDraw::TOOLPAINT_VALCMP_SHIFT) & 255));

    //塗りタイプ

    _trgroup(STRGID_PIXELTYPE);

    pl->addItem(m_pcbPixel = new AXComboBox(this, 0, LF_EXPAND_W, WID_PIXEL, 0));

    for(i = 0; i < 4; i++)
        m_pcbPixel->addItemTr(id[i], id[i]);

    m_pcbPixel->setCurSel_findParam((val >> CDraw::TOOLPAINT_PIX_SHIFT) & CDraw::TOOLPAINT_PIX_MASK);

    //チェック

    _trgroup(STRGID_WORD);

    pl->addItem(new AXCheckButton(this, 0, 0, WID_USEBRUSHVAL, 0, STRID_WORD_USEBRUSHVAL, val & CDraw::TOOLPAINT_F_USEBRUSH));
    pl->addItem(new AXCheckButton(this, 0, 0, WID_NOREF, 0, STRID_WORD_NOREFLAYER, val & CDraw::TOOLPAINT_F_NOREF));
}

BOOL COptWin_tool_paint::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //濃度
        case WID_VAL:
            g_pdraw->m_optToolPaint &= ~CDraw::TOOLPAINT_VAL_MASK;
            g_pdraw->m_optToolPaint |= m_pbarVal->getPos();
            break;
        //タイプ
        case WID_TYPE:
            g_pdraw->m_optToolPaint &= ~(CDraw::TOOLPAINT_TYPE_MASK << CDraw::TOOLPAINT_TYPE_SHIFT);
            g_pdraw->m_optToolPaint |= m_pcbType->getCurSel() << CDraw::TOOLPAINT_TYPE_SHIFT;

            enableWidget(WID_VALCMP, (m_pcbType->getCurSel() == 1));
            break;
        //指定濃度
        case WID_VALCMP:
            g_pdraw->m_optToolPaint &= ~(255 << CDraw::TOOLPAINT_VALCMP_SHIFT);
            g_pdraw->m_optToolPaint |= m_pbarValCmp->getPos() << CDraw::TOOLPAINT_VALCMP_SHIFT;
            break;
        //塗りタイプ
        case WID_PIXEL:
            g_pdraw->m_optToolPaint &= ~(CDraw::TOOLPAINT_PIX_MASK << CDraw::TOOLPAINT_PIX_SHIFT);
            g_pdraw->m_optToolPaint |= m_pcbPixel->getItemParam(-1) << CDraw::TOOLPAINT_PIX_SHIFT;
            break;
        //ブラシの濃度を使う
        case WID_USEBRUSHVAL:
            g_pdraw->m_optToolPaint ^= CDraw::TOOLPAINT_F_USEBRUSH;

            enableWidget(WID_VAL, !(g_pdraw->m_optToolPaint & CDraw::TOOLPAINT_F_USEBRUSH));
            break;
        //判定元無効
        case WID_NOREF:
            g_pdraw->m_optToolPaint ^= CDraw::TOOLPAINT_F_NOREF;
            break;
    }

    return TRUE;
}


//**************************************
// COptWin_tool_painterase （不透明消し）
//**************************************


COptWin_tool_painterase::COptWin_tool_painterase()
    : AXWindow(OPTWIN, WS_BK_FACE, 0)
{
    AXLayout *pl;

    setLayout(pl = new AXLayoutVert(0, 0));

    pl->setSpacing(4);

    //判定元無効

    pl->addItem(new AXCheckButton(this, 0, 0, WID_NOREF, 0, _string(STRGID_WORD, STRID_WORD_NOREFLAYER),
                                  g_pdraw->m_optToolPaintErase & CDraw::TOOLPAINTE_NOREF));
}

BOOL COptWin_tool_painterase::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin->getItemID() == WID_NOREF)
        g_pdraw->m_optToolPaintErase ^= CDraw::TOOLPAINTE_NOREF;

    return TRUE;
}


//**************************************
// COptWin_tool_paintclose （閉領域）
//**************************************


COptWin_tool_paintclose::COptWin_tool_paintclose()
    : AXWindow(OPTWIN, WS_BK_FACE, LF_EXPAND_W)
{
    AXLayout *pl;
    UINT val = g_pdraw->m_optToolPaintClose;
    WORD id[] = { STRID_PIXTYPE_BLEND, STRID_PIXTYPE_COMPARE, STRID_PIXTYPE_OVERWRITE, STRID_PIXTYPE_ERASE };
    int i;

    setLayout(pl = new AXLayoutVert(0, 4));

    pl->setSpacing(4);

    //濃度

    pl->addItem(m_pbarVal = new CValBar2(this,
                                (val & CDraw::TOOLPAINTC_F_USEBRUSH)? WS_DISABLE: 0,
                                LF_EXPAND_W, WID_VAL, 0, 0, 1, 255, val & 255));

    //塗りタイプ

    _trgroup(STRGID_PIXELTYPE);

    pl->addItem(m_pcbPixel = new AXComboBox(this, 0, LF_EXPAND_W, WID_PIXEL, 0));

    for(i = 0; i < 4; i++)
        m_pcbPixel->addItemTr(id[i], id[i]);

    m_pcbPixel->setCurSel_findParam((val >> CDraw::TOOLPAINTC_PIX_SHIFT) & CDraw::TOOLPAINTC_PIX_MASK);

    //チェック

    _trgroup(STRGID_WORD);

    pl->addItem(new AXCheckButton(this, 0, 0, WID_USEBRUSHVAL, 0, STRID_WORD_USEBRUSHVAL, val & CDraw::TOOLPAINTC_F_USEBRUSH));
    pl->addItem(new AXCheckButton(this, 0, 0, WID_NOREF, 0, STRID_WORD_NOREFLAYER, val & CDraw::TOOLPAINTC_F_NOREF));
}

BOOL COptWin_tool_paintclose::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //濃度
        case WID_VAL:
            g_pdraw->m_optToolPaintClose &= ~CDraw::TOOLPAINTC_VAL_MASK;
            g_pdraw->m_optToolPaintClose |= m_pbarVal->getPos();
            break;
        //塗りタイプ
        case WID_PIXEL:
            g_pdraw->m_optToolPaintClose &= ~(CDraw::TOOLPAINTC_PIX_MASK << CDraw::TOOLPAINTC_PIX_SHIFT);
            g_pdraw->m_optToolPaintClose |= m_pcbPixel->getItemParam(-1) << CDraw::TOOLPAINTC_PIX_SHIFT;
            break;
        //ブラシの濃度を使う
        case WID_USEBRUSHVAL:
            g_pdraw->m_optToolPaintClose ^= CDraw::TOOLPAINTC_F_USEBRUSH;

            enableWidget(WID_VAL, !(g_pdraw->m_optToolPaintClose & CDraw::TOOLPAINTC_F_USEBRUSH));
            break;
        //判定元無効
        case WID_NOREF:
            g_pdraw->m_optToolPaintClose ^= CDraw::TOOLPAINTC_F_NOREF;
            break;
    }

    return TRUE;
}


//*********************************
// COptWin_tool_move （移動）
//*********************************


COptWin_tool_move::COptWin_tool_move()
    : AXWindow(OPTWIN, WS_BK_FACE, 0)
{
    AXLayout *pl;
    int i;

    _trgroup(STRGID_OPTWIN_MOVE);

    setLayout(pl = new AXLayoutVert(0, 2));

    pl->setSpacing(4);

    //タイプ

    for(i = 0; i < 3; i++)
    {
        pl->addItem(new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0,
                                      WID_TYPE1 + i, 0, STRID_OPTWINMOVE_TYPE1 + i, (g_pdraw->m_optToolMove == i)));
    }
}

BOOL COptWin_tool_move::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();

    switch(id)
    {
        case WID_TYPE1:
        case WID_TYPE2:
        case WID_TYPE3:
            g_pdraw->m_optToolMove = id - WID_TYPE1;
            break;
    }

    return TRUE;
}


//**************************************
// COptWin_tool_sel （選択範囲）
//**************************************


COptWin_tool_sel::COptWin_tool_sel()
    : AXWindow(OPTWIN, WS_BK_FACE, LF_EXPAND_W)
{
    AXLayout *pl;
    int n;

    _trgroup(STRGID_OPTWIN_SEL);

    setLayout(pl = new AXLayoutMatrix(2));

    pl->setSpacing(4);

    //色と濃度

    n = (int)((g_pdraw->m_optToolSel >> 24) * 100.0 / 128.0 + 0.5);

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,4), STRID_OPTWINSEL_COL));
    pl->addItem(m_pbtCol = new AXColorButton(this, AXColorButton::CBTS_CHOOSE, 0, WID_BT_COL, MAKE_DW4(0,0,0,4), g_pdraw->m_optToolSel));

    pl->addItem(new AXLabel(this, 0, LF_RIGHT|LF_CENTER_Y, MAKE_DW4(0,0,5,0), STRID_OPTWINSEL_OPACITY));
    pl->addItem(m_pbarOc = new CValBar2(this, 0, LF_EXPAND_W|LF_CENTER_Y, WID_BAR_OPACITY, 0, 0, 1, 100, n));
}

BOOL COptWin_tool_sel::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //色
        case WID_BT_COL:
            if(uNotify == AXColorButton::CBTN_PRESS)
            {
                g_pdraw->m_optToolSel &= 0xff000000;
                g_pdraw->m_optToolSel |= (UINT)lParam;

                g_pdraw->updateCanvasAndPrevWin(g_pdraw->m_rcfSel, TRUE);
            }
            break;
        //濃度
        case WID_BAR_OPACITY:
            if(uNotify == CValBar2::VALBAR2N_UP)
            {
                int n = (m_pbarOc->getPos() * 128.0 / 100.0 + 0.5);

                g_pdraw->m_optToolSel &= 0xffffff;
                g_pdraw->m_optToolSel |= n << 24;

                g_pdraw->updateCanvasAndPrevWin(g_pdraw->m_rcfSel, TRUE);
            }
            break;
    }

    return TRUE;
}


//**********************************
// COptWin_tool_grad （グラデーション）
//**********************************


COptWin_tool_grad::COptWin_tool_grad()
    : AXWindow(OPTWIN, WS_BK_FACE, LF_EXPAND_W)
{
    AXLayout *pl,*plh;
    WORD id[] = { STRID_PIXTYPE_BLEND, STRID_PIXTYPE_COMPARE, STRID_PIXTYPE_OVERWRITE, STRID_PIXTYPE_ERASE };
    UINT val = g_pdraw->m_optToolGrad;
    int i;
    CGradListItem *pItem;

    setLayout(pl = new AXLayoutVert(0, 4));

    pl->setSpacing(4);

    //プレビュー

    pl->addItem(m_pPrev = new CImgPrev(this, CImgPrev::STYLE_RESIZE, LF_EXPAND_W, 0, 0, 14));

    //リスト

    pl->addItem(plh = new AXLayoutHorz(LF_EXPAND_W, 0));

    plh->addItem(m_pcbSel = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_SEL, MAKE_DW4(0,0,4,0)));
    plh->addItem(new AXButton(this, AXButton::BS_NOOUTFRAME|AXButton::BS_REAL_WH, LF_EXPAND_H, WID_BTT_LISTADD, 0, "+"));
    plh->addItem(new AXButton(this, AXButton::BS_NOOUTFRAME|AXButton::BS_REAL_WH, LF_EXPAND_H, WID_BTT_LISTDEL, 0, "-"));
    plh->addItem(new AXButton(this, AXButton::BS_NOOUTFRAME|AXButton::BS_REAL_WH, LF_EXPAND_H, WID_BTT_LISTEDIT, 0, "E"));

    for(pItem = g_pdraw->m_pGrad->getTopItem(); pItem; pItem = pItem->next())
        m_pcbSel->addItem(pItem->m_strName, (ULONG)pItem);

    //濃度

    pl->addItem(m_pbarVal = new CValBar2(this, 0, LF_EXPAND_W, WID_BAR_VAL, 0, 0, 1, 255,
                                         val & CDraw::TOOLGRAD_VAL_MASK));

    //塗りタイプ

    _trgroup(STRGID_PIXELTYPE);

    pl->addItem(m_pcbPix = new AXComboBox(this, 0, LF_EXPAND_W, WID_CB_PIX, 0));

    for(i = 0; i < 4; i++)
        m_pcbPix->addItemTr(id[i], id[i]);

    m_pcbPix->setCurSel_findParam((val >> CDraw::TOOLGRAD_PIX_SHIFT) & CDraw::TOOLGRAD_PIX_MASK);

    //チェック

    _trgroup(STRGID_OPTWIN_GRAD);

    pl->addItem(plh = new AXLayoutHorz(0, 3));

    plh->addItem(new AXCheckButton(this, 0, 0, WID_CK_REVERSE, 0, STRID_OPTWINGRAD_REVERSE, val & CDraw::TOOLGRAD_F_REV));
    plh->addItem(new AXCheckButton(this, 0, 0, WID_CK_LOOP, 0, STRID_OPTWINGRAD_LOOP, val & CDraw::TOOLGRAD_F_LOOP));

    //------

    _changeSel();
}

BOOL COptWin_tool_grad::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    //プレビューリサイズ

    if(pwin == m_pPrev && uNotify == CImgPrev::NOTIFY_RESIZE)
    {
        (g_pdraw->m_pGrad->getSelItem())->drawPrev(m_pPrev->getImg(), 0, 0, 0, 0);
        return TRUE;
    }

    //

    switch(pwin->getItemID())
    {
        //リスト選択変更
        case WID_CB_SEL:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                g_pdraw->m_pGrad->setSelItem((CGradListItem *)m_pcbSel->getItemParam(-1));

                _changeSel();
            }
            break;
        //リスト追加
        case WID_BTT_LISTADD:
            if(g_pdraw->m_pGrad->addGrad())
            {
                m_pcbSel->addItem((g_pdraw->m_pGrad->getSelItem())->m_strName, (ULONG)g_pdraw->m_pGrad->getSelItem());
                _changeSel();
            }
            break;
        //リスト削除
        case WID_BTT_LISTDEL:
            int n;

            n = m_pcbSel->findItemParam((ULONG)g_pdraw->m_pGrad->getSelItem());
            if(n == 0) break;

            if(AXMessageBox::message(OPTWIN, NULL, _string(STRGID_MESSAGE, STRID_MES_DELETE),
                                     AXMessageBox::YES | AXMessageBox::NO,
                                     AXMessageBox::YES) == AXMessageBox::NO)
                break;

            if(g_pdraw->m_pGrad->delGrad())
            {
                m_pcbSel->deleteItem(n);
                _changeSel();
            }
            break;
        //リスト編集
        case WID_BTT_LISTEDIT:
            CGradEditDlg *pdlg;

            pdlg = new CGradEditDlg(MAINWIN, g_pdraw->m_pGrad->getSelItem());

            if(pdlg->runDialog())
            {
                g_pdraw->m_pGrad->updateOn();

                m_pcbSel->setItemText(-1, (g_pdraw->m_pGrad->getSelItem())->m_strName);

                _changeSel();
            }
            break;
        //濃度
        case WID_BAR_VAL:
            g_pdraw->m_optToolGrad &= ~CDraw::TOOLGRAD_VAL_MASK;
            g_pdraw->m_optToolGrad |= m_pbarVal->getPos();
            break;
        //塗りタイプ
        case WID_CB_PIX:
            g_pdraw->m_optToolGrad &= ~(CDraw::TOOLGRAD_PIX_MASK << CDraw::TOOLGRAD_PIX_SHIFT);
            g_pdraw->m_optToolGrad |= m_pcbPix->getItemParam(-1) << CDraw::TOOLGRAD_PIX_SHIFT;
            break;
        //反転
        case WID_CK_REVERSE:
            g_pdraw->m_optToolGrad ^= CDraw::TOOLGRAD_F_REV;
            break;
        //繰り返し
        case WID_CK_LOOP:
            g_pdraw->m_optToolGrad ^= CDraw::TOOLGRAD_F_LOOP;
            break;
    }

    return TRUE;
}

//! 選択データ変更時

void COptWin_tool_grad::_changeSel()
{
    CGradListItem *p = g_pdraw->m_pGrad->getSelItem();
    BOOL bDefault;

    bDefault = (p->m_pPrev == NULL);

    //選択
    m_pcbSel->setCurSel_findParam((ULONG)p);

    //プレビュー
    p->drawPrev(m_pPrev->getImg(), 0, 0, 0, 0);
    m_pPrev->redraw();

    //削除・編集ボタン（デフォルトデータは無効）

    enableWidget(WID_BTT_LISTDEL, !bDefault);
    enableWidget(WID_BTT_LISTEDIT, !bDefault);
}


//*********************************
// COptWin_texture （テクスチャ）
//*********************************


COptWin_texture::COptWin_texture()
    : AXWindow(OPTWIN, WS_BK_FACE, LF_EXPAND_W)
{
    AXLayout *pl;

    setLayout(pl = new AXLayoutVert(0, 4));

    pl->setSpacing(4);

    //画像選択バー

    pl->addItem(m_pSel = new CSelImgBar(this, 0, LF_EXPAND_W, 0, 0));

    //プレビュー

    pl->addItem(m_pPrev = new CImgPrev(this, 0, 0, 0, 100, 80));

    _changeFile(FALSE);
}

BOOL COptWin_texture::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pSel)
    {
        if(uNotify == CSelImgBar::NOTIFY_RIGHT)
        {
            //なしにする

            g_pdraw->m_strOptTex.empty();
            _changeFile(TRUE);
        }
        else
        {
            //画像選択

            CSelImgDlg *pdlg = new CSelImgDlg(OPTWIN, CSelImgDlg::TYPE_OPT_TEXTURE, g_pdraw->m_strOptTex);

            if(pdlg->run(&g_pdraw->m_strOptTex))
                _changeFile(TRUE);
        }
    }

    return TRUE;
}

//! ファイル変更時

void COptWin_texture::_changeFile(BOOL bLoad)
{
    //ファイル名

    if(g_pdraw->m_strOptTex.isEmpty())
        m_pSel->setName(_string(STRGID_SELIMGBAR, STRID_SELIMGBAR_NONE));
    else
        m_pSel->setName(g_pdraw->m_strOptTex);

    //読み込み

    if(bLoad)
        g_pdraw->loadTextureImg(g_pdraw->m_pimg8Tex, g_pdraw->m_strOptTex);

    //プレビュー

    if(g_pdraw->m_pimg8Tex->isExist())
        g_pdraw->m_pimg8Tex->drawTexturePrev(m_pPrev->getImg());
    else
        (m_pPrev->getImg())->clear(0x808080);

    m_pPrev->redraw();
}


//*********************************
// COptWin_option （設定）
//*********************************


COptWin_option::COptWin_option()
    : AXWindow(OPTWIN, WS_BK_FACE, 0)
{
    AXLayout *pl,*plh;
    int i;

    _trgroup(STRGID_OPTWIN_OPTION);

    setLayout(pl = new AXLayoutVert(0, 3));

    pl->setSpacing(4);

    //イメージ背景色

    pl->addItem(new AXLabel(this, 0, 0, 0, STRID_OPTWINOPT_IMGBKCOL));

    pl->addItem(plh = new AXLayoutHorz(0, 4));
    plh->addItem(m_pcbImgBk = new AXComboBox(this, AXComboBox::CBS_OWNERDRAW, 0, WID_CB_IMGBK, 0));
    plh->addItem(new AXButton(this, AXButton::BS_REAL_WH, LF_EXPAND_H, WID_BTT_IMGBK, 0, "..."));

    m_pcbImgBk->setMinWidth(80);
    m_pcbImgBk->setItemH(17);

    for(i = 0; i < CConfig::IMGBKCOL_NUM; i++)
        m_pcbImgBk->addItem((LPCUSTR)NULL);

    m_pcbImgBk->setCurSel(g_pconf->btImgBkColSel);
}

BOOL COptWin_option::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //イメージ背景色コンボボックス
        case WID_CB_IMGBK:
            if(uNotify == AXComboBox::CBN_SELCHANGE)
            {
                g_pconf->btImgBkColSel = m_pcbImgBk->getCurSel();
                g_pconf->dwImgBkCol    = g_pconf->dwImgBkColList[g_pconf->btImgBkColSel];

                g_pdraw->updateAll();
            }
            break;
        //イメージ背景色、色選択
        case WID_BTT_IMGBK:
            DWORD col;

            col = g_pconf->dwImgBkCol;

            if(AXColorDialog::getColor(OPTWIN, &col))
            {
                g_pconf->dwImgBkCol = col;
                g_pconf->dwImgBkColList[g_pconf->btImgBkColSel] = col;

                m_pcbImgBk->redraw();
                g_pdraw->updateAll();
            }
            break;
    }

    return TRUE;
}

BOOL COptWin_option::onOwnerDraw(AXDrawable *pdraw,AXHD_OWNERDRAW *phd)
{
    AXGC gc;

    //選択

    if(phd->uState & AXWindow::ODS_SELECTED)
        pdraw->drawFillBox(phd->rcs.x, phd->rcs.y, phd->rcs.w, phd->rcs.h, AXAppRes::BACKREVTEXT);

    //色枠

    pdraw->drawBox(phd->rcs.x + 3, phd->rcs.y + 2, phd->rcs.w - 6, phd->rcs.h - 4, AXAppRes::BLACK);

    //色

    gc.createColor(g_pconf->dwImgBkColList[phd->nItemNo]);

    pdraw->drawFillBox(phd->rcs.x + 4, phd->rcs.y + 3, phd->rcs.w - 8, phd->rcs.h - 6, gc);

    return TRUE;
}


//*********************************
// COptWin_headtail （入り抜き）
//*********************************


COptWin_headtail::COptWin_headtail()
    : AXWindow(OPTWIN, WS_BK_FACE, LF_EXPAND_WH)
{
    AXLayout *pl,*pl2,*plh;
    WORD val;
    int i;
    AXString str;

    if(g_pconf->btOptHeadTail == 0)
        val = g_pdraw->m_wHeadTailLine;
    else
        val = g_pdraw->m_wHeadTailBezir;

    //

    _trgroup(STRGID_OPTWIN_HEADTAIL);

    setLayout(pl = new AXLayoutVert(0, 3));

    pl->setSpacing(4);

    //タイプ

    pl->addItem(plh = new AXLayoutHorz);

    plh->addItem(new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_CK_LINE, 0, STRID_OPTWINHT_LINE, (g_pconf->btOptHeadTail == 0)));
    plh->addItem(new AXCheckButton(this, AXCheckButton::CBS_RADIO, 0, WID_CK_BEZIER, 0, STRID_OPTWINHT_BEZIER, (g_pconf->btOptHeadTail == 1)));

    //---------

    pl->addItem(plh = new AXLayoutHorz(LF_EXPAND_WH, 8));

    //エディット

    plh->addItem(pl2 = new AXLayoutVert(0, 3));

    pl2->addItem(new AXLabel(this, 0, 0, 0, STRID_OPTWINHT_HEAD));
    pl2->addItem(m_peditHead = new AXLineEdit(this, AXLineEdit::ES_SPIN, LF_EXPAND_W, WID_EDIT_HEAD, 0));
    pl2->addItem(new AXLabel(this, 0, 0, 0, STRID_OPTWINHT_TAIL));
    pl2->addItem(m_peditTail = new AXLineEdit(this, AXLineEdit::ES_SPIN, LF_EXPAND_W, WID_EDIT_TAIL, 0));
    pl2->addItem(new AXButton(this, AXButton::BS_REAL_W, LF_EXPAND_W, WID_BT_SET, 0, STRID_OPTWINHT_SET));

    m_peditHead->setInit(4, 0, 100, val >> 8);
    m_peditTail->setInit(4, 0, 100, val & 0xff);

    //リスト

    plh->addItem(m_pList = new AXListBox(this, AXScrollView::SVS_VERT|AXScrollView::SVS_SUNKEN|AXListBox::LBS_FOCUSOUT_SELOUT,
                                         LF_EXPAND_WH, WID_LIST, 0));

    for(i = 0; i < CConfig::HEADTAIL_NUM; i++)
    {
        str.setInt(g_pconf->wHeadTailList[i] >> 8);
        str += "% : ";
        str += (int)(g_pconf->wHeadTailList[i] & 0xff);
        str += '%';

        m_pList->addItem(str);
    }
}

BOOL COptWin_headtail::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //直線
        case WID_CK_LINE:
            g_pconf->btOptHeadTail = 0;

            _setVal(g_pdraw->m_wHeadTailLine);
            break;
        //ベジェ曲線
        case WID_CK_BEZIER:
            g_pconf->btOptHeadTail = 1;

            _setVal(g_pdraw->m_wHeadTailBezir);
            break;
        //入り
        case WID_EDIT_HEAD:
            if(g_pconf->btOptHeadTail == 0)
            {
                g_pdraw->m_wHeadTailLine &= 0xff;
                g_pdraw->m_wHeadTailLine |= (m_peditHead->getVal() << 8);
            }
            else
            {
                g_pdraw->m_wHeadTailBezir &= 0xff;
                g_pdraw->m_wHeadTailBezir |= (m_peditHead->getVal() << 8);
            }
            break;
        //抜き
        case WID_EDIT_TAIL:
            if(g_pconf->btOptHeadTail == 0)
            {
                g_pdraw->m_wHeadTailLine &= 0xff00;
                g_pdraw->m_wHeadTailLine |= m_peditTail->getVal();
            }
            else
            {
                g_pdraw->m_wHeadTailBezir &= 0xff00;
                g_pdraw->m_wHeadTailBezir |= m_peditTail->getVal();
            }
            break;
        //登録
        case WID_BT_SET:
            _setListVal();
            break;
        //リスト
        case WID_LIST:
            if(uNotify == AXListBox::LBN_SELCHANGE)
            {
                int val;

                val = g_pconf->wHeadTailList[m_pList->getCurSel()];

                if(g_pconf->btOptHeadTail == 0)
                    g_pdraw->m_wHeadTailLine = val;
                else
                    g_pdraw->m_wHeadTailBezir = val;

                _setVal(val);
            }
            break;
    }

    return TRUE;
}

//! エディットに値セット

void COptWin_headtail::_setVal(WORD val)
{
    m_peditHead->setVal(val >> 8);
    m_peditTail->setVal(val & 0xff);
}

//! 現在値をリストにセット

void COptWin_headtail::_setListVal()
{
    AXMenu *pm;
    int i,val;
    AXString str;
    AXRectSize rcs;

    //--------- メニュー

    pm = new AXMenu;

    for(i = 0; i < CConfig::HEADTAIL_NUM; i++)
    {
        val = g_pconf->wHeadTailList[i];

        str = '[';
        str += (i + 1);
        str += "] ";
        str += (val >> 8);
        str += ':';
        str += (val & 0xff);

        pm->add(i, str);
    }

    getWidget(WID_BT_SET)->getWindowRect(&rcs);

    i = (int)pm->popup(NULL, rcs.x + rcs.w, rcs.y, 0);

    delete pm;

    //--------- セット

    if(i != -1)
    {
        val = (m_peditHead->getVal() << 8) | m_peditTail->getVal();

        g_pconf->wHeadTailList[i] = val;

        //リストテキスト変更

        str.setInt(val >> 8);
        str += "% : ";
        str += (int)(val & 0xff);
        str += '%';

        m_pList->setItemText(i, str);
    }
}


//*********************************
// COptWin_rule （定規）
//*********************************


//=========================
//COptWin_rule
//=========================


COptWin_rule::COptWin_rule()
    : AXWindow(OPTWIN, WS_BK_FACE, 0)
{
    AXLayout *pl,*plh;

    _trgroup(STRGID_OPTWIN_RULE);

    setLayout(pl = new AXLayoutVert(0, 8));

    pl->setSpacing(4);

    //タイプ

    pl->addItem(m_pwidType = new COptWin_rule_type(this));

    //ボタン

    pl->addItem(plh = new AXLayoutHorz(0, 4));

    plh->addItem(new AXButton(this, WS_DISABLE, 0, WID_BTT_ANGLE, 0, STRID_OPTWINRULE_ANGLE));
    plh->addItem(new AXButton(this, WS_DISABLE, 0, WID_BTT_POS, 0, STRID_OPTWINRULE_CTPOS));

    _enableBtt();
}

BOOL COptWin_rule::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pwidType)
        //タイプ選択変更（ボタンの有効/無効）
        _enableBtt();
    else if(pwin->getItemID() == WID_BTT_ANGLE)
        menuAngle();
    else if(pwin->getItemID() == WID_BTT_POS)
        menuPos();

    return TRUE;
}

//! ボタンの有効/無効

void COptWin_rule::_enableBtt()
{
    enableWidget(WID_BTT_ANGLE,
                 (g_pdraw->m_ruleType == CDraw::RULETYPE_LINE || g_pdraw->m_ruleType == CDraw::RULETYPE_GRIDLINE));

    enableWidget(WID_BTT_POS,
                 (g_pdraw->m_ruleType == CDraw::RULETYPE_CONCLINE ||
                  g_pdraw->m_ruleType == CDraw::RULETYPE_CIRCLE ||
                  g_pdraw->m_ruleType == CDraw::RULETYPE_ELLIPSE));
}

void COptWin_rule::menuAngle()
{
    AXMenu *pm;
    AXString str;
    char m[32];
    int i,angle;
    AXRectSize rcs;
    double dval;

    //現在の角度

    dval = (g_pdraw->m_ruleType == CDraw::RULETYPE_LINE)? g_pdraw->m_rule.dAngleLine: g_pdraw->m_rule.dAngleGrid;

    angle = (int)(dval * 18000 / 3.141592);
    while(angle < 0) angle += 36000;
    while(angle >= 36000) angle -= 36000;

    //メニュー

    pm = new AXMenu;

    for(i = 0; i < CConfig::RULEANGLE_NUM; i++)
    {
        str = '[';
        str += (i + 1);
        str += "] ";
        AXIntToFloatStr(m, g_pconf->wRuleAngle[i], 2);
        str += m;

        pm->add(i, str);
    }

    pm->addSep();

    for(i = 0; i < CConfig::RULEANGLE_NUM; i++)
    {
        AXIntToFloatStr(m, angle, 2);
        str = m;
        str += " -> [";
        str += (i + 1);
        str += ']';

        pm->add(1000 + i, str);
    }

    //

    getWidget(WID_BTT_ANGLE)->getWindowRect(&rcs);

    i = (int)pm->popup(NULL, rcs.x, rcs.y + rcs.h, 0);

    delete pm;

    //処理

    if(i != -1)
    {
        if(i < 1000)
        {
            //呼び出し

            dval = g_pconf->wRuleAngle[i] * 3.141592 / 18000.0;

            if(g_pdraw->m_ruleType == CDraw::RULETYPE_LINE)
                g_pdraw->m_rule.dAngleLine = dval;
            else
                g_pdraw->m_rule.dAngleGrid = dval;
        }
        else
        {
            //登録

            g_pconf->wRuleAngle[i - 1000] = angle;
        }
    }
}

void COptWin_rule::menuPos()
{
    AXMenu *pm;
    AXString str;
    int i;
    AXPoint *ppt;
    AXRectSize rcs;

    //位置

    switch(g_pdraw->m_ruleType)
    {
        case CDraw::RULETYPE_CONCLINE:
            ppt = &g_pdraw->m_rule.ptCtConc;
            break;
        case CDraw::RULETYPE_CIRCLE:
            ppt = &g_pdraw->m_rule.ptCtCir;
            break;
        case CDraw::RULETYPE_ELLIPSE:
            ppt = &g_pdraw->m_rule.ptCtEll;
            break;
    }

    //メニュー

    pm = new AXMenu;

    for(i = 0; i < CConfig::RULEPOS_NUM; i++)
    {
        str = '[';
        str += (i + 1);
        str += "] (";
        str += g_pconf->sRulePos[i * 2];
        str += ',';
        str += g_pconf->sRulePos[i * 2 + 1];
        str += ')';

        pm->add(i, str);
    }

    pm->addSep();

    for(i = 0; i < CConfig::RULEPOS_NUM; i++)
    {
        str = '(';
        str += ppt->x;
        str += ',';
        str += ppt->y;
        str += ") -> [";
        str += (i + 1);
        str += ']';

        pm->add(1000 + i, str);
    }

    getWidget(WID_BTT_POS)->getWindowRect(&rcs);

    i = (int)pm->popup(NULL, rcs.x, rcs.y + rcs.h, 0);

    delete pm;

    //処理

    if(i != -1)
    {
        if(i < 1000)
        {
            //呼び出し

            ppt->x = g_pconf->sRulePos[i * 2];
            ppt->y = g_pconf->sRulePos[i * 2 + 1];
        }
        else
        {
            //登録

            i -= 1000;

            g_pconf->sRulePos[i * 2] = ppt->x;
            g_pconf->sRulePos[i * 2 + 1] = ppt->y;
        }
    }
}

//=========================
//COptWin_rule_type
//=========================

COptWin_rule_type::COptWin_rule_type(AXWindow *pParent)
    : AXWindow(pParent, 0, 0)
{
    m_img.create(g_pres->m_pimg32Rule->getWidth(), g_pres->m_pimg32Rule->getHeight());

    m_nMinW = m_img.getWidth();
    m_nMinH = m_img.getHeight();

    setImage();
}

BOOL COptWin_rule_type::onPaint(AXHD_PAINT *phd)
{
    m_img.put(m_id);
    return TRUE;
}

BOOL COptWin_rule_type::onButtonDown(AXHD_MOUSE *phd)
{
    int n,size,max;

    //選択変更

    size = m_nMinH - 1;
    max  = (m_nMinW - 1) / size;

    n = phd->x / size;
    if(n >= max) n = max - 1;

    g_pdraw->m_ruleType = n;

    setImage();

    //ボタンの有効/無効などのため親にメッセージ

    m_pParent->onNotify(this, 0, 0);

    return TRUE;
}

void COptWin_rule_type::setImage()
{
    g_pres->drawRuleImgSel(g_pdraw->m_ruleType);

    m_img.setFromImage32(*(g_pres->m_pimg32Rule));

    redraw();
}

