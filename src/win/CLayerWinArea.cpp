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

#include "CLayerWinArea.h"

#include "CDraw.h"
#include "CConfig.h"
#include "CResource.h"
#include "CLayerList.h"
#include "defGlobal.h"
#include "defStrID.h"

#include "AXScrollBar.h"
#include "AXImage.h"
#include "AXFont.h"
#include "AXMenu.h"
#include "AXGC.h"
#include "AXAppRes.h"
#include "AXApp.h"
#include "AXUtilStr.h"


//-------------------

#define LWA_MINWIDTH    150
#define LWA_MASK_X      33
#define LWA_PAINT_X     48
#define LWA_GROUP_X     63
#define LWA_CHECK_X     78
#define LWA_ALPHA_X     90
#define LWA_ALPHA_RIGHT 4
#define LWA_ALPHA_Y     22
#define LWA_ALPHA_H     9

#define CKDOWNINFO_NONE     0
#define CKDOWNINFO_EXINFO   1
#define CKDOWNINFO_INFOCMD  2
#define CKDOWNINFO_ALPHABAR 3

#define DRAGF_MOVEBEGIN     1
#define DRAGF_MOVEDND       2
#define DRAGF_ALPHA         3

#define COL_SPACEBK         0xaaaaaa
#define COL_FRAME_NORM      0xaaaaaa
#define COL_FRAME_NORM_SEL  0xd14c4c
#define COL_FRAME_HL        0xffffff
#define COL_FRAME_HL_SEL    0xd14c6d

//-------------------


/*!
    @class CLayerWinArea
    @brief レイヤウィンドウの、レイヤ一覧部分
*/



CLayerWinArea::~CLayerWinArea()
{
    delete m_pimg;
}

CLayerWinArea::CLayerWinArea(AXWindow *pParent)
    : AXWindow(pParent, 0, LF_EXPAND_WH)
{
    m_pimg = new AXImage;

    m_nViewCnt = 0;
    m_fDrag    = 0;

    setFont(g_pres->m_pfontSmall);
}

//! スクロール情報セット

void CLayerWinArea::setScrollInfo()
{
    int n = m_nH / LAYERITEM_H;

    if(n < 0) n = 1;

    m_pScrV->setStatus(0, g_pdraw->getLayerCnt(), n);
}


//===========================
// サブ
//===========================


//! 不透明度バーの幅取得（不透明度値計算用）

int CLayerWinArea::_getAlphaBarW()
{
    return ((m_nW < 160)? 160: m_nW) - LWA_ALPHA_X - LWA_ALPHA_RIGHT - 2;
}

//! 押し時の各情報の判定と実行
/*!
    @param pNo    押された位置のレイヤ番号が入る
    @param pAlpha 不透明バー時、押された位置の値(NULL OK)
    @return [0]レイヤ範囲外 [1]情報範囲外 [2]チェック等実行された [3]不透明バー
*/

int CLayerWinArea::_checkDownInfo(int x,int y,LPINT pNo,LPINT pAlpha,UINT state,BOOL bDblClk)
{
    int scr,no;
    BOOL bShift = state & STATE_SHIFT;

    scr = m_pScrV->getPos();

    //レイヤ番号

    no = y / LAYERITEM_H + scr;

    if(no < 0 || no >= g_pdraw->getLayerCnt()) return CKDOWNINFO_NONE;

    //------------

    y -= (no - scr) * LAYERITEM_H;

    *pNo = no;

    //-------------- 判定

    //表示/非表示

    if(x >= LWA_MASK_X && x < LWA_MASK_X + 13 && y >= 3 && y <= 15)
    {
        g_pdraw->layer_revView(no);
        drawOne(no, FALSE, TRUE, TRUE);
        return CKDOWNINFO_INFOCMD;
    }

    //マスク

    if(x >= LWA_MASK_X && x < LWA_MASK_X + 13 && y >= 19 && y <= 31)
    {
        g_pdraw->layer_changeMask(no, (bShift || bDblClk));
        return CKDOWNINFO_INFOCMD;
    }

    //塗りつぶし判定元

    if(x >= LWA_PAINT_X && x < LWA_PAINT_X + 13 && y >= 19 && y <= 31)
    {
        g_pdraw->layer_revPaintFlag(no);
        return CKDOWNINFO_INFOCMD;
    }

    //グループ

    if(x >= LWA_GROUP_X && x < LWA_GROUP_X + 13 && y >= 19 && y <= 31)
    {
        g_pdraw->layer_changeGroup(no, (bShift || bDblClk));
        return CKDOWNINFO_INFOCMD;
    }

    //チェック

    if(x >= LWA_CHECK_X && x < LWA_CHECK_X + 9 && y >= 22 && y <= 30)
    {
        g_pdraw->layer_revCheck(no);
        return CKDOWNINFO_INFOCMD;
    }

    //不透明バー

    if(x >= LWA_ALPHA_X && x < LWA_ALPHA_X + _getAlphaBarW() + 1 &&
       y >= LWA_ALPHA_Y && y < LWA_ALPHA_Y + LWA_ALPHA_H)
    {
        if(pAlpha)
            *pAlpha = (int)((x - LWA_ALPHA_X) * 128.0 / _getAlphaBarW() + 0.5);

        return CKDOWNINFO_ALPHABAR;
    }

    return CKDOWNINFO_EXINFO;
}

//! 右クリックメニュー表示

void CLayerWinArea::_showRMenu(AXHD_MOUSE *phd)
{
    CLayerItem *pItem;
    AXMenu *pm,*pmPal1,*pmPal2;
    int i,n,no;

    //レイヤ

    no = phd->y / LAYERITEM_H + m_pScrV->getPos();

    pItem = g_pdraw->getLayer(no);
    if(!pItem) return;

    //----- メニュー

    _trgroup(STRGID_LAYERRMENU);

    pm = new AXMenu;

    //パレット

    pmPal1 = new AXMenu;
    pmPal2 = new AXMenu;

    n = CConfig::LAYERPAL_NUM / 2;

    for(i = 0; i < n; i++)
    {
        pmPal1->addOD(1000 + i, NULL, g_pconf->dwLayerPal[i], 20, 16);
        pmPal2->addOD(1000 + n + i, NULL, g_pconf->dwLayerPal[i + n], 20, 16);
    }

    pm->addTr(STRID_LAYERRMENU_PALETTE1, pmPal1);
    pm->addTr(STRID_LAYERRMENU_PALETTE2, pmPal2);
    pm->addSep();

    //チェック

    pm->addTrCheck(STRID_LAYERRMENU_PAINT, pItem->isPaint());
    pm->addTrCheck(STRID_LAYERRMENU_HILIGHT, pItem->isHilight());
    pm->addSep();
    pm->addTrCheck(STRID_LAYERRMENU_MASK, pItem->isMask());
    pm->addTrCheck(STRID_LAYERRMENU_MASKUNDER, pItem->isMaskUnder());
    pm->addSep();
    pm->addTrCheck(STRID_LAYERRMENU_GROUP, pItem->isGroup());
    pm->addTrCheck(STRID_LAYERRMENU_GROUPUNDER, pItem->isGroupUnder());

    //

    n = (int)pm->popup(this, phd->rootx, phd->rooty, 0);
    delete pm;

    if(n == -1) return;

    //処理

    if(n >= 1000)
    {
        //パレットからレイヤ色セット

        pItem->m_dwCol = g_pconf->dwLayerPal[n - 1000];

        g_pdraw->updateAll();
        drawOne(no, FALSE, TRUE, TRUE);
    }
    else
    {
        switch(n)
        {
            case STRID_LAYERRMENU_PAINT:
                g_pdraw->layer_revPaintFlag(no);
                break;
            case STRID_LAYERRMENU_HILIGHT:
                g_pdraw->layer_revHilight(no);
                break;
            case STRID_LAYERRMENU_MASK:
                g_pdraw->layer_changeMask(no, 2);
                break;
            case STRID_LAYERRMENU_MASKUNDER:
                g_pdraw->layer_changeMask(no, 1);
                break;
            case STRID_LAYERRMENU_GROUP:
                g_pdraw->layer_changeGroup(no, 2);
                break;
            case STRID_LAYERRMENU_GROUPUNDER:
                g_pdraw->layer_changeGroup(no, 1);
                break;
        }
    }
}


//===========================
// ハンドラ
//===========================


//! サイズ変更時

BOOL CLayerWinArea::onSize()
{
    m_pimg->recreate((m_nW < LWA_MINWIDTH)? LWA_MINWIDTH: m_nW, m_nH, 16, LAYERITEM_H);

    //表示数

    m_nViewCnt = (m_nH + LAYERITEM_H - 1) / LAYERITEM_H;
    if(m_nViewCnt <= 0) m_nViewCnt = 1;

    setScrollInfo();
    draw();

    return TRUE;
}

//! 描画

BOOL CLayerWinArea::onPaint(AXHD_PAINT *phd)
{
    m_pimg->put(m_id, phd->x, phd->y, phd->x, phd->y, phd->w, phd->h);
    return TRUE;
}

//! マウスホイール（スクロール）

BOOL CLayerWinArea::onMouseWheel(AXHD_MOUSE *phd,BOOL bUp)
{
    if(m_pScrV->movePos((bUp)? -1: 1))
        draw();

    return TRUE;
}

//! ダブルクリック時

BOOL CLayerWinArea::onDblClk(AXHD_MOUSE *phd)
{
    int ret,no;

    if(phd->button == BUTTON_LEFT && !m_fDrag)
    {
        //チェックなど処理（不透明バーは処理なし）

        ret = _checkDownInfo(phd->x, phd->y, &no, NULL, phd->state, TRUE);

        //レイヤ設定

        if(ret == CKDOWNINFO_EXINFO)
            m_pParent->sendCommand(STRID_LAYERMENU_OPTION, 0, 0);
    }

    return TRUE;
}

//! ボタン押し

BOOL CLayerWinArea::onButtonDown(AXHD_MOUSE *phd)
{
    int ret,no,bk,alpha;

    if(m_fDrag) return TRUE;

    if(phd->button == BUTTON_RIGHT)
    {
        //------- 右ボタン

        _showRMenu(phd);
    }
    else if(phd->button == BUTTON_LEFT)
    {
        //------- 左ボタン

        //各チェックなど処理
        //（レイヤ外 or 処理済なら終了）

        ret = _checkDownInfo(phd->x, phd->y, &no, &alpha, phd->state, FALSE);
        if(ret == CKDOWNINFO_NONE || ret == CKDOWNINFO_INFOCMD) return TRUE;

        //不透明度バー（ドラッグ開始）

        if(ret == CKDOWNINFO_ALPHABAR)
        {
            g_pdraw->layer_changeAlpha(no, alpha, FALSE);
            drawOne(no, FALSE, TRUE, TRUE);

            m_fDrag     = DRAGF_ALPHA;
            m_nDragPos  = no;

            grabPointer();

            return TRUE;
        }

        //----------------

        //カレントレイヤ変更 or プレビュー更新
        //（押された位置がカレントレイヤならプレビュー更新）

        if(no == g_pdraw->m_nCurLayerNo)
            g_pdraw->updateLayerPrev();
        else
        {
            bk = g_pdraw->m_nCurLayerNo;

            g_pdraw->changeCurLayer(no, TRUE);

            //更新

            drawOne(bk, FALSE, TRUE, TRUE);
            drawOne(no, FALSE, TRUE, TRUE);
        }

        //+Ctrl でカレントレイヤのみ表示

        if(phd->state & STATE_CTRL)
            g_pdraw->layer_viewAll(2);

        //押した時カレントレイヤなら、レイヤ移動のD&D判定開始

        if(no == g_pdraw->m_nCurLayerNo)
        {
            m_fDrag = DRAGF_MOVEBEGIN;
            grabPointer();
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL CLayerWinArea::onButtonUp(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && m_fDrag)
    {
        ungrabPointer();

        switch(m_fDrag)
        {
            //D&D、終了
            case DRAGF_MOVEDND:
                unsetCursor();

                _drawDND(0, 2);

                //レイヤ移動
                g_pdraw->layer_moveDND(m_nDragPos, TRUE);
                break;

            //不透明度バー（離し時に更新）
            case DRAGF_ALPHA:
                g_pdraw->updateAll();
                break;
        }

        m_fDrag = 0;
    }

    return TRUE;
}

//! マウス移動時

BOOL CLayerWinArea::onMouseMove(AXHD_MOUSE *phd)
{
    int pos,lcnt,n;

    switch(m_fDrag)
    {
        //不透明度バー
        case DRAGF_ALPHA:
            n = (int)((phd->x - LWA_ALPHA_X) * 128.0 / _getAlphaBarW() + 0.5);

            g_pdraw->layer_changeAlpha(m_nDragPos, n, FALSE);
            drawOne(m_nDragPos, FALSE, TRUE, TRUE);
           break;

        //D&D レイヤ位置移動
        case DRAGF_MOVEBEGIN:
        case DRAGF_MOVEDND:
            //現在のカーソルのレイヤ位置（※高さの半分で補正）

            pos = phd->y / LAYERITEM_H + m_pScrV->getPos();

            if(pos == g_pdraw->m_nCurLayerNo)
                n = 0;
            else if(pos < g_pdraw->m_nCurLayerNo)
                n = LAYERITEM_H / 2;
            else
                n = -LAYERITEM_H / 2;

            pos = (phd->y + n) / LAYERITEM_H + m_pScrV->getPos();

            //カーソル位置にカレントレイヤが来るように調整

            if(pos > g_pdraw->m_nCurLayerNo) pos++;

            lcnt = g_pdraw->getLayerCnt();
            if(pos < 0) pos = 0; else if(pos > lcnt) pos = lcnt;

            //

            if(m_fDrag == DRAGF_MOVEDND)
            {
                //----- ドラッグ移動中

                _drawDND(pos, 0);
                m_nDragPos = pos;
            }
            else
            {
                if(pos != g_pdraw->m_nCurLayerNo)
                {
                    //---- ドラッグ判定中、D&D開始

                    _drawDND(pos, 1);
                    m_nDragPos = pos;

                    setCursor(g_pdraw->getCursorDat(CDraw::CURSOR_LAYERMOVE));

                    m_fDrag = DRAGF_MOVEDND;
                }
            }
            break;
    }

    return TRUE;
}

//! 右クリックメニューのパレット描画

BOOL CLayerWinArea::onOwnerDraw(AXDrawable *pdraw,AXHD_OWNERDRAW *phd)
{
    AXGC gc;

    //背景

    if(phd->uState & ODS_SELECTED)
        pdraw->drawFillBox(phd->rcs.x, phd->rcs.y, phd->rcs.w, phd->rcs.h, AXAppRes::BACKMENUSEL);

    //枠

    pdraw->drawBox(phd->rcs.x + 4, phd->rcs.y + 2, phd->rcs.w - 8, phd->rcs.h - 4, AXAppRes::BLACK);

    //色

    gc.createColor(phd->lParam1);

    pdraw->drawFillBox(phd->rcs.x + 5, phd->rcs.y + 3, phd->rcs.w - 10, phd->rcs.h - 6, gc);

    return TRUE;
}


//===========================
// 描画
//===========================


//! 全体描画

void CLayerWinArea::draw()
{
    int i,scr;

    m_pimg->clear(COL_SPACEBK);

    scr = m_pScrV->getPos();

    for(i = 0; i < m_nViewCnt; i++)
        drawOne(scr + i, TRUE, TRUE, FALSE);

    redraw();
}

//! 一つ描画
/*!
    @return FALSE で、layerno が表示範囲外
*/

BOOL CLayerWinArea::drawOne(int layerno,BOOL bPrev,BOOL bInfo,BOOL bRedraw)
{
    CLayerItem *p;
    int scr,bSel,y,x,w,n;
    DWORD colBk,colFr,col1,col2;
    LPBYTE pPat;
    char m[16];
    BYTE pat_mask[7] = { 0x38,0x7c,0xfe,0xfe,0xfe,0x7c,0x38 };
    BYTE pat_mask2[18] = {
        0x38,0x00, 0x7c,0x00, 0xfe,0x00, 0xfe,0x00, 0xfe,0x00, 0x7c,0x00, 0x38,0x00, 0x03,0x80, 0x01,0x00
    };
    BYTE pat_paint[18] = {
        0x08,0x00, 0x14,0x00, 0x22,0x00, 0x47,0x00, 0x6f,0x80, 0x5f,0x00, 0x4e,0x00, 0x44,0x00, 0xc0,0x00
    };
    BYTE pat_group[16] = {
        0x70,0x00, 0x88,0x00, 0x87,0x00, 0x80,0x80, 0x80,0x80, 0x80,0x80, 0x80,0x80, 0xff,0x80
    };
    BYTE pat_group2[16] = {
        0x70,0x00, 0x88,0x00, 0x87,0x00, 0x80,0x80, 0x9c,0x80, 0x88,0x80, 0x80,0x80, 0xff,0x80
    };
    BYTE pat_check[5] = { 0x08,0x10,0xf0,0x60,0x60 };

    //---------

    scr = m_pScrV->getPos();
    if(layerno < scr || layerno >= scr + m_nViewCnt) return FALSE;

    p = g_pdraw->getLayer(layerno);
    if(!p) return FALSE;

    //

    bSel = (layerno == g_pdraw->m_nCurLayerNo);
    w    = (m_nW < LWA_MINWIDTH)? LWA_MINWIDTH: m_nW;
    y    = (layerno - scr) * LAYERITEM_H;

    //背景色・外枠の色

    if(bSel)
    {
        if(p->m_btFlag & CLayerItem::FLAG_HILIGHT)
            colBk = 0xffc6fb, colFr = COL_FRAME_HL_SEL;
        else
            colBk = 0xffd1c6, colFr = COL_FRAME_NORM_SEL;
    }
    else
    {
        if(p->m_btFlag & CLayerItem::FLAG_HILIGHT)
            colBk = 0xc5ccf2, colFr = COL_FRAME_HL;
        else
            colBk = 0xffffff, colFr = COL_FRAME_NORM;
    }

    //外枠

    m_pimg->box(0, y, w, LAYERITEM_H, colFr);

    //プレビュー枠の周り

    m_pimg->box(1, y + 1, 30, 33, colBk);

    //プレビューイメージ

    if(bPrev)
    {
        m_pimg->box(2, y + 2, 28, 31, 0);
        m_pimg->blt(3, y + 3, p->m_imgPrev, 0, 0, CLayerItem::PREVIMG_W, CLayerItem::PREVIMG_H);
    }

    //情報

    if(bInfo)
    {
        //背景

        m_pimg->fillBox(31, y + 1, w - 31 - 1, LAYERITEM_H - 2, colBk);

        //レイヤ名

        m_pimg->drawText(*m_pFont, 50, y + 3, w - 50 - 3, m_pFont->getHeight(),
                         p->m_strName, p->m_strName.getLen(), (bSel)? _RGB(200,0,0): 0);

        //表示/非表示チェック

        x = LWA_MASK_X;

        m_pimg->box(x, y + 3, 13, 13, 0);
        m_pimg->fillBox(x + 1, y + 4, 11, 11, 0xffffff);

        if(p->m_btFlag & CLayerItem::FLAG_VIEW)
            m_pimg->fillBox(x + 2, y + 5, 9, 9, 0x002080);
        else
        {
            m_pimg->line(x + 1, y +  4, x + 11, y + 14, 0);
            m_pimg->line(x + 1, y + 14, x + 11, y +  4, 0);
        }

        //レイヤマスク

        if(p->m_btFlag & CLayerItem::FLAG_MASK)
            col1 = 0x00e01d, col2 = 0xffffff;
        else if(p->m_btFlag & CLayerItem::FLAG_MASKUNDER)
            col1 = 0x76aa0f, col2 = 0xffffff;
        else
            col1 = 0xffffff, col2 = 0xc0c0c0;

        m_pimg->box(LWA_MASK_X, y + 19, 13, 13, 0);
        m_pimg->fillBox(LWA_MASK_X + 1, y + 20, 11, 11, col1);

        if(p->m_btFlag & CLayerItem::FLAG_MASKUNDER)
            m_pimg->drawPattern(LWA_MASK_X + 2, y + 21, col2, pat_mask2, 9, 9);
        else
            m_pimg->drawPattern(LWA_MASK_X + 3, y + 22, col2, pat_mask, 7, 7);

        //塗りつぶし判定元

        if(p->m_btFlag & CLayerItem::FLAG_PAINT)
            col1 = 0xff2dab, col2 = 0xffffff;
        else
            col1 = 0xffffff, col2 = 0xc0c0c0;

        m_pimg->box(LWA_PAINT_X, y + 19, 13, 13, 0);
        m_pimg->fillBox(LWA_PAINT_X + 1, y + 20, 11, 11, col1);
        m_pimg->drawPattern(LWA_PAINT_X + 2, y + 21, col2, pat_paint, 9, 9);

        //グループ

        if(p->m_btFlag & CLayerItem::FLAG_GROUP)
            col1 = 0xff8000, pPat = pat_group;
        else if(p->m_btFlag & CLayerItem::FLAG_GROUPUNDER)
            col1 = 0xffee00, pPat = pat_group2;
        else
            col1 = 0xffffff, pPat = pat_group;

        m_pimg->box(LWA_GROUP_X, y + 19, 13, 13, 0);
        m_pimg->fillBox(LWA_GROUP_X + 1, y + 20, 11, 11, col1);
        m_pimg->drawPattern(LWA_GROUP_X + 2, y + 22, 0, pPat, 9, 8);

        //チェック

        m_pimg->box(LWA_CHECK_X, y + 22, 9, 9, 0);
        m_pimg->fillBox(LWA_CHECK_X + 1, y + 23, 7, 7, (p->m_btFlag & CLayerItem::FLAG_CHECK)? 0: 0xffffff);

        if(p->m_btFlag & CLayerItem::FLAG_CHECK)
            m_pimg->drawPattern(LWA_CHECK_X + 2, y + 24, 0xffffff, pat_check, 5, 5);

        //濃度

        n = w - LWA_ALPHA_X - LWA_ALPHA_RIGHT;
        x = (int)((n - 2) * p->m_btAlpha / 128.0 + 0.5);

        m_pimg->box(LWA_ALPHA_X, y + LWA_ALPHA_Y, n, LWA_ALPHA_H, 0);
        if(x) m_pimg->fillBox(LWA_ALPHA_X + 1, y + LWA_ALPHA_Y + 1, x, LWA_ALPHA_H - 2, _RGB(149,186,243));

        x = AXIntToStr(m, (int)(p->m_btAlpha * 100.0 / 128.0 + 0.5));
        m_pimg->drawNumber(LWA_ALPHA_X + n - 4 - 5 * x, y + LWA_ALPHA_Y + 2, m, 0x0000ff);
    }

    //レイヤの線の色

    if(bPrev || bInfo)
    {
        m_pimg->box(23, y + 26, 7, 7, 0);
        m_pimg->line(22, y + 25, 22 + 7, y + 25, 0xffffff);
        m_pimg->line(22, y + 25, 22, y + 25 + 7, 0xffffff);

        m_pimg->fillBox(24, y + 27, 5, 5, p->m_dwCol);
    }

    //更新

    if(bRedraw)
        redraw(0, y, m_nW, y + LAYERITEM_H);

    return TRUE;
}

//! D&D時、挿入位置の描画
/*!
    @param flag [0]消去&描画 [1]描画のみ [2]消去のみ
*/

void CLayerWinArea::_drawDND(int newpos,int flag)
{
    int w,y,scr;

    if(flag == 0 && newpos == m_nDragPos) return;

    scr = m_pScrV->getPos();
    w   = m_pimg->getWidth();

    //前の線を消去

    if(flag != 1)
    {
        y = (m_nDragPos - scr) * LAYERITEM_H;

        m_pimg->lineH(0, y, w, _getLayerFrameCol(m_nDragPos));
        m_pimg->lineH(0, y - 1, w, _getLayerFrameCol(m_nDragPos - 1));
    }

    //現在の線を描画

    if(flag != 2)
    {
        y = (newpos - scr) * LAYERITEM_H;

        m_pimg->lineH(0, y, w, 0xff0000);
        m_pimg->lineH(0, y - 1, w, 0xff0000);
    }

    redraw();
}

//! 指定レイヤのフレーム色取得

DWORD CLayerWinArea::_getLayerFrameCol(int no)
{
    CLayerItem *p = g_pdraw->getLayer(no);

    if(!p)
        return COL_SPACEBK;
    else if(p->m_btFlag & CLayerItem::FLAG_HILIGHT)
        return (no == g_pdraw->m_nCurLayerNo)? COL_FRAME_HL_SEL: COL_FRAME_HL;
    else
        return (no == g_pdraw->m_nCurLayerNo)? COL_FRAME_NORM_SEL: COL_FRAME_NORM;
}
