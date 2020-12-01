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

#include "CGradEditDlg.h"

#include "CGradList.h"
#include "CValBar2.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXButton.h"
#include "AXLineEdit.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXImage.h"
#include "AXUtilStr.h"

#include "defStrID.h"


//------------------

#define WID_BAR_VAL     100
#define WID_BTT_LEFT    101
#define WID_BTT_RIGHT   102
#define WID_BTT_DEL     103
#define WID_EDIT        104

#define GRAD_GETPOS(v)  ((v) & CGradListItem::POS_MASK)
#define GRAD_GETVAL(v)  ((v) >> CGradListItem::VAL_SHIFT)

//------------------

class CGradEditDlg_edit:public AXWindow
{
public:
    enum
    {
        NOTIFY_CHANGECUR,
    };
    enum
    {
        UNDERH  = 15,   //!< グラデーションより下の高さ
        POINTW  = 11,
        POINTHF = 5
    };

protected:
    LPINT       m_pDat;
    int         m_nPtCnt,
                m_nCurPos,
                m_fDrag;

    AXImage     m_img;

protected:
    void _notify();
    void _newPoint(int pos);
    void _everyPoint(int pt);

    int _getWinPosFromGrad(int pos);
    int _getGradPosFromWin(int x);
    int _getPtFromMouse(int mx,int my);

    void _drawAll();
    void _drawGrad();
    void _drawPoint(BOOL bClear);
    void _drawOnePoint(int pt);

public:
    CGradEditDlg_edit(AXWindow *pParent,UINT uItemID,DWORD dwPadding,LPINT pDat,int cnt);

    int getPtCnt() { return m_nPtCnt; }
    int getCurrentVal() { return m_pDat[m_nCurPos]; }

    void delPoint(int pt);
    void movPoint(BOOL bRight);
    void setCurVal(int val);

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
};

//------------------


/*!
    @class CGradEditDlg
    @brief グラデーション編集ダイアログ
*/


CGradEditDlg::CGradEditDlg(AXWindow *pOwner,CGradListItem *pItem)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXLayout *plTop,*pl;

    m_pItem = pItem;

    //データコピー

    m_memDat.alloc(sizeof(int) * CGradListItem::POINT_MAX);
    m_memDat.copyFrom(pItem->m_memDat, pItem->m_memDat.getSize());

    //

    _trgroup(STRGID_DLG_GRADEDIT);

    setTitle(STRID_GRADEDIT_TITLE);

    //

    setLayout(plTop = new AXLayoutVert(0, 6));
    plTop->setSpacing(8);

    //名前

    plTop->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 6));

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, _string(STRGID_WORD, STRID_WORD_NAME)));
    pl->addItem(m_peditName = new AXLineEdit(this, 0, LF_EXPAND_W));

    m_peditName->setText(pItem->m_strName);

    //編集

    plTop->addItem(m_pGrad = new CGradEditDlg_edit(this, WID_EDIT, MAKE_DW4(0,0,0,8),
                                                   m_memDat, pItem->m_memDat.getSize() / sizeof(int)));

    //位置と濃度

    plTop->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 5));

    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, _string(STRGID_WORD, STRID_WORD_POS)));
    pl->addItem(m_plbPos = new AXLabel(this, AXLabel::LS_SUNKEN, LF_CENTER_Y));
    pl->addItem(new AXLabel(this, 0, LF_CENTER_Y, 0, _string(STRGID_WORD, STRID_WORD_OPACITY)));
    pl->addItem(m_pbarVal = new CValBar2(this, 0, LF_EXPAND_W|LF_CENTER_Y, WID_BAR_VAL, 0, 0, 0, 255, 0));

    m_plbPos->setWidthFromLen(7);

    //ボタン

    plTop->addItem(pl = new AXLayoutHorz(LF_RIGHT|LF_EXPAND_X, 3));

    pl->addItem(new AXButton(this, 0, 0, WID_BTT_LEFT, 0, "<"));
    pl->addItem(new AXButton(this, 0, 0, WID_BTT_RIGHT, 0, ">"));
    pl->addItem(new AXButton(this, 0, 0, WID_BTT_DEL, 0, _string(STRGID_WORD, STRID_WORD_DELETE)));

    //説明

    plTop->addItem(new AXLabel(this, AXLabel::LS_SUNKEN, LF_EXPAND_W, MAKE_DW4(0,4,0,0), STRID_GRADEDIT_INFO));

    //OKキャンセル

    plTop->addItem(pl = createOKCancelButton());
    pl->setPaddingTop(15);

    //---------

    _setCurrentVal();

    calcDefSize();
    resize((m_nDefW < 450)? 450: m_nDefW, m_nDefH);

    show();
}

//! 通知

BOOL CGradEditDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    switch(pwin->getItemID())
    {
        //編集
        case WID_EDIT:
            if(uNotify == CGradEditDlg_edit::NOTIFY_CHANGECUR)
                _setCurrentVal();
            break;
        //濃度
        case WID_BAR_VAL:
            if(uNotify == CValBar2::VALBAR2N_CHANGE)
                m_pGrad->setCurVal(m_pbarVal->getPos());
            break;
        //<
        case WID_BTT_LEFT:
            m_pGrad->movPoint(FALSE);
            break;
        //>
        case WID_BTT_RIGHT:
            m_pGrad->movPoint(TRUE);
            break;
        //削除
        case WID_BTT_DEL:
            m_pGrad->delPoint(-1);
            break;

        //OK
        case 1:
            m_peditName->getText(&m_pItem->m_strName);

            m_pItem->setDat(m_memDat, m_pGrad->getPtCnt());

            endDialog(TRUE);
            break;
        //キャンセル
        case 2:
            endDialog(FALSE);
            break;
    }

    return TRUE;
}

//! 現在のポイントの情報セット

void CGradEditDlg::_setCurrentVal()
{
    char m[32];
    int val = m_pGrad->getCurrentVal();

    //位置

    AXIntToFloatStr(m, (int)((double)GRAD_GETPOS(val) / CGradListItem::POS_MAX * 1000 + 0.5), 3);
    m_plbPos->setText(m);

    //濃度

    m_pbarVal->setPos(GRAD_GETVAL(val));
}


//**********************************
// CGradEditDlg_edit
//**********************************



CGradEditDlg_edit::CGradEditDlg_edit(AXWindow *pParent,UINT uItemID,DWORD dwPadding,LPINT pDat,int cnt)
    : AXWindow(pParent, 0, LF_EXPAND_W, uItemID, dwPadding)
{
    m_pDat    = pDat;
    m_nPtCnt  = cnt;
    m_nCurPos = 0;
    m_fDrag   = 0;

    m_nMinW = 60;
    m_nMinH = 40;
}

//! 描画

BOOL CGradEditDlg_edit::onPaint(AXHD_PAINT *phd)
{
    m_img.put(m_id, 0, 0, 0, 0, m_nW, m_nH);
    return TRUE;
}

//! サイズ変更時

BOOL CGradEditDlg_edit::onSize()
{
    m_img.recreate(m_nW, m_nH, 32, 0);

    _drawAll();

    return TRUE;
}

//! ボタン押し

BOOL CGradEditDlg_edit::onButtonDown(AXHD_MOUSE *phd)
{
    if(m_fDrag == 0 && phd->button == BUTTON_LEFT)
    {
        int pt,n;

        //押されたポイント取得

        pt = _getPtFromMouse(phd->x, phd->y);
        if(pt == -2) return TRUE;

        //

        if(pt == -1)
        {
            //ポイントがなければ、新規ポイント作成

            if(phd->state & STATE_SHIFT) return TRUE;

            n = _getGradPosFromWin(phd->x);

            _newPoint(n);
        }
        else if(phd->state & STATE_CTRL)
        {
            //+Ctrl : 等間隔

            _everyPoint(pt);
        }
        else if(phd->state & STATE_SHIFT)
        {
            //+Shift : 削除

            delPoint(pt);
        }
        else
        {
            //---- カレント変更

            m_nCurPos = pt;

            _drawPoint(FALSE);
            redraw();

            _notify();

            //D&D開始（端の位置は除く）

            n = GRAD_GETPOS(m_pDat[m_nCurPos]);

            if(n != 0 && n != CGradListItem::POS_MAX)
            {
                m_fDrag = TRUE;
                grabPointer();
            }
        }
    }

    return TRUE;
}

//! ボタン離し

BOOL CGradEditDlg_edit::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fDrag && phd->button == BUTTON_LEFT)
    {
        m_fDrag = FALSE;
        ungrabPointer();
    }

    return TRUE;
}

//! 移動

BOOL CGradEditDlg_edit::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_fDrag)
    {
        int pos,ppos,npos;

        pos = _getGradPosFromWin(phd->x);

        //左と右の間に調整

        ppos = GRAD_GETPOS(m_pDat[m_nCurPos - 1]);
        npos = GRAD_GETPOS(m_pDat[m_nCurPos + 1]);

        if(pos <= ppos)
            pos = ppos + 1;
        else if(pos >= npos)
            pos = npos - 1;

        //変更

        if(GRAD_GETPOS(m_pDat[m_nCurPos]) != pos)
        {
            m_pDat[m_nCurPos] &= ~CGradListItem::POS_MASK;
            m_pDat[m_nCurPos] |= pos;

            _drawAll();

            _notify();
        }
    }

    return TRUE;
}


//=======================
// 処理
//=======================


//! 通知

void CGradEditDlg_edit::_notify()
{
    m_pParent->onNotify(this, NOTIFY_CHANGECUR, 0);
}

//! 現在のポイントの濃度を変更

void CGradEditDlg_edit::setCurVal(int val)
{
    m_pDat[m_nCurPos] &= CGradListItem::POS_MASK;
    m_pDat[m_nCurPos] |= val << CGradListItem::VAL_SHIFT;

    _drawOnePoint(m_nCurPos);
    _drawGrad();
}

//! 新規ポイント作成

void CGradEditDlg_edit::_newPoint(int pos)
{
    int inspos = -1,i,ppos,pval,npos,nval;

    if(m_nPtCnt >= CGradListItem::POINT_MAX)
        return;

    //挿入位置（正確にはこの後に追加する）
    //※位置が同じ場合は追加しない

    for(i = 0; i < m_nPtCnt - 1; i++)
    {
        if(GRAD_GETPOS(m_pDat[i]) < pos && pos < GRAD_GETPOS(m_pDat[i + 1]))
        {
            inspos = i;
            break;
        }
    }

    if(inspos == -1) return;

    //挿入位置を空ける

    for(i = m_nPtCnt; i > inspos + 1; i--)
        m_pDat[i] = m_pDat[i - 1];

    //セット（濃度は前と次のポイントの中間値）

    ppos = GRAD_GETPOS(m_pDat[inspos]);
    pval = GRAD_GETVAL(m_pDat[inspos]);
    npos = GRAD_GETPOS(m_pDat[inspos + 2]);
    nval = GRAD_GETVAL(m_pDat[inspos + 2]);

    i = pos;
    i |= ((pos - ppos) * (nval - pval) / (npos - ppos) + pval) << CGradListItem::VAL_SHIFT;

    m_pDat[inspos + 1] = i;

    //

    m_nCurPos = inspos + 1;
    m_nPtCnt++;

    _drawAll();

    _notify();
}

//! ポイント削除
/*!
    @param pt -1で現在の位置
*/

void CGradEditDlg_edit::delPoint(int pt)
{
    int i;

    if(pt == -1) pt = m_nCurPos;

    //両端は削除しない

    if(pt == 0 || pt == m_nPtCnt - 1) return;

    //詰める

    for(i = pt; i < m_nPtCnt - 1; i++)
        m_pDat[i] = m_pDat[i + 1];

    //

    m_nPtCnt--;

    _drawAll();
    _notify();
}

//! ポイント選択を左右に移動

void CGradEditDlg_edit::movPoint(BOOL bRight)
{
    //両端

    if(!bRight && m_nCurPos == 0) return;
    if(bRight && m_nCurPos == m_nPtCnt - 1) return;

    //移動

    if(bRight)
        m_nCurPos++;
    else
        m_nCurPos--;

    //

    _drawPoint(FALSE);
    redraw();

    _notify();
}

//! 現在ポイントと指定ポイントの間を等間隔に

void CGradEditDlg_edit::_everyPoint(int pt)
{
    int i,pt1,pt2,poslen,pos1;

    //位置の小さい順

    if(GRAD_GETPOS(m_pDat[pt]) < GRAD_GETPOS(m_pDat[m_nCurPos]))
        pt1 = pt, pt2 = m_nCurPos;
    else
        pt1 = m_nCurPos, pt2 = pt;

    //カレントと同じ、または間が一つもない時

    if(pt == m_nCurPos) return;
    if(pt1 + 1 == pt2) return;

    //

    pos1   = GRAD_GETPOS(m_pDat[pt1]);
    poslen = GRAD_GETPOS(m_pDat[pt2]) - pos1;

    for(i = pt1 + 1; i < pt2; i++)
    {
        m_pDat[i] &= ~CGradListItem::POS_MASK;
        m_pDat[i] |= (i - pt1) * poslen / (pt2 - pt1) + pos1;
    }

    _drawAll();
}


//=======================
// 計算
//=======================


//! グラデ位置からウィンドウX位置取得

int CGradEditDlg_edit::_getWinPosFromGrad(int pos)
{
    if(pos == 0)
        return POINTHF;
    else if(pos == CGradListItem::POS_MAX)
        return m_nW - 1 - POINTHF;
    else
        return ((pos * (m_nW - 1 - POINTW)) >> CGradListItem::POS_BIT) + POINTHF + 1;
}

//! ウィンドウX位置からグラデ位置取得

int CGradEditDlg_edit::_getGradPosFromWin(int x)
{
    int pos;

    pos = x - (POINTHF + 1);
    pos = pos * CGradListItem::POS_MAX / (m_nW - POINTHF * 2 - 3);

    return pos;
}

//! マウス位置からポイント位置取得
/*!
    @return [-1]なし [-2]範囲外
*/

int CGradEditDlg_edit::_getPtFromMouse(int mx,int my)
{
    int i,x,y;

    y = m_nH - UNDERH + 4;

    //カレントを優先判定

    x = _getWinPosFromGrad(GRAD_GETPOS(m_pDat[m_nCurPos]));

    if(mx >= x - POINTHF && mx <= x + POINTHF && my >= y && my < y + UNDERH - 4)
        return m_nCurPos;

    //各ポインタの四角内か

    for(i = 0; i < m_nPtCnt; i++)
    {
        x = _getWinPosFromGrad(GRAD_GETPOS(m_pDat[i]));

        if(mx >= x - POINTHF && mx <= x + POINTHF && my >= y && my < y + UNDERH - 4)
            return i;
    }

    //範囲外

    if(mx <= POINTHF || mx >= m_nW - POINTHF - 1)
        return -2;

    return -1;
}


//=======================
// 描画
//=======================


//! 全体描画

void CGradEditDlg_edit::_drawAll()
{
    //背景

    m_img.clear(axres->colRGB(AXAppRes::FACE));

    //グラデ枠

    m_img.box(POINTHF, 0, m_nW - POINTHF * 2, m_nH - UNDERH, 0);

    //グラデ

    _drawGrad();

    //ポイント

    _drawPoint(FALSE);

    redraw();
}

//! グラデ部分描画

void CGradEditDlg_edit::_drawGrad()
{
    CGradListItem::drawPrevDat(&m_img, POINTHF + 1, 1,
            m_nW - 2 - POINTHF * 2, m_nH - 2 - UNDERH, m_pDat);

    redraw();
}

//! ポイント描画

void CGradEditDlg_edit::_drawPoint(BOOL bClear)
{
    int i;

    //クリア

    if(bClear)
        m_img.fillBox(0, m_nH - UNDERH, m_nW, UNDERH, axres->colRGB(AXAppRes::FACE));

    //カレント以外描画

    for(i = 0; i < m_nPtCnt; i++)
    {
        if(i != m_nCurPos)
            _drawOnePoint(i);
    }

    //カレントは最後に描画

    _drawOnePoint(m_nCurPos);
}

//! 1つのポイント描画

void CGradEditDlg_edit::_drawOnePoint(int pt)
{
    int x,y;
    DWORD col;

    //描画位置

    x = _getWinPosFromGrad(GRAD_GETPOS(m_pDat[pt]));
    y = m_nH - UNDERH;

    //枠

    col = (pt == m_nCurPos)? 0xff0000: 0;

    m_img.lineV(x, y, 4, col);
    m_img.box(x - POINTHF, y + 4, POINTW, UNDERH - 4, col);

    //色

    col = GRAD_GETVAL(m_pDat[pt]);
    col = (col << 16) | (col << 8) | col;

    m_img.fillBox(x - POINTHF + 1, y + 5, POINTW - 2, UNDERH - 6, col);
}
