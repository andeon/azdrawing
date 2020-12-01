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

#include "AXToolBar.h"

#include "AXList.h"
#include "AXImageList.h"
#include "AXPixmap.h"
#include "AXToolTipWin.h"
#include "AXAppRes.h"
#include "AXApp.h"


#define SPACEBT         3       //各ボタンのXY余白
#define SEPW            8       //セパレータ幅
#define DROPW           13      //ドロップボックス幅
#define SPACEBOTTOM     2       //TBS_SPACEBOTTOM 時の余白

#define DOWNF_BUTTON    1
#define DOWNF_DROP      2

#define TOPITEM     (AXToolBarItem *)m_pDat->getTop()


//**********************************
// AXToolBarItem
//**********************************


class AXToolBarItem:public AXListItem
{
public:
    UINT    m_uID,
            m_uFlags;
    int	    m_nImgNo,
            m_nToolTipNo;
    ULONG   m_lParam;

    int     m_nX,m_nY,m_nW;

public:
    AXToolBarItem(UINT uID,int nImgNo,int nToolTipNo,UINT uFlags,ULONG lParam,int nIconW);

    AXToolBarItem *prev() const { return (AXToolBarItem *)m_pPrev; }
    AXToolBarItem *next() const { return (AXToolBarItem *)m_pNext; }

    BOOL isCheckGroup() { return m_uFlags & AXToolBar::BF_CHECKGROUP; }
    BOOL isCheckOn();
    void check() { m_uFlags |= AXToolBar::BF_CHECKED; }
    void uncheck() { m_uFlags &= ~AXToolBar::BF_CHECKED; }
};

//! 作成

AXToolBarItem::AXToolBarItem(UINT uID,int nImgNo,int nToolTipNo,UINT uFlags,ULONG lParam,int nIconW)
{
    m_uID       = uID;
    m_uFlags    = uFlags;
    m_nImgNo    = nImgNo;
    m_nToolTipNo = nToolTipNo;
    m_lParam    = lParam;

    //幅

    if(uFlags & AXToolBar::BF_SEP)
        m_nW = SEPW;
    else
    {
        m_nW = nIconW + SPACEBT * 2;

        if(uFlags & AXToolBar::BF_DROPDOWN) m_nW += DROPW;
    }
}

//! チェックがONの状態か

BOOL AXToolBarItem::isCheckOn()
{
    return (m_uFlags & (AXToolBar::BF_CHECKBUTTON | AXToolBar::BF_CHECKGROUP)) && (m_uFlags & AXToolBar::BF_CHECKED);
}


//**********************************
// AXToolBar
//**********************************


/*!
    @class AXToolBar
    @brief ツールバーウィジェット

    - ボタンのみ。文字列は付けられない。
    - ボタンが押されたら、onCommand() で通知される。@n
      ボタン側が押されたら COMFROM_TOOLBAR。ドロップボックス側が押されたら COMFROM_TOOLBAR_DROP。
    - TBS_TOOLTIP_TR フラグが ON の場合、ツールチップ番号は文字列ID。@n
      OFF なら、ツールチップ番号は setToolTipText() で指定された文字列のインデックス番号。@n
      いずれにしても、負の値でツールチップなし。

    @ingroup widget
*/
/*!
    @var AXToolBar::TBS_FRAME
    @brief 各ボタンに常に枠を表示する
    @var AXToolBar::TBS_TOOLTIP
    @brief ツールチップあり
    @var AXToolBar::TBS_SEPBOTTOM
    @brief 下にセパレータを付ける
    @var AXToolBar::TBS_SPACEBOTTOM
    @brief 下に余白を付ける
    @var AXToolBar::TBS_TOOLTIP_TR
    @brief ツールチップの番号は文字列IDとする

    @var AXToolBar::BF_BUTTON
    @brief 通常ボタン
    @var AXToolBar::BF_CHECKBUTTON
    @brief 単体のチェックボタン
    @var AXToolBar::BF_CHECKGROUP
    @brief グループのチェックボタン
    @var AXToolBar::BF_SEP
    @brief セパレータ
    @var AXToolBar::BF_DROPDOWN
    @brief 右にドロップボックスを付ける
    @var AXToolBar::BF_WRAP
    @brief 改行する（次のボタンから）
    @var AXToolBar::BF_DISABLE
    @brief 無効フラグ
    @var AXToolBar::BF_CHECKED
    @brief チェックフラグ
*/


AXToolBar::~AXToolBar()
{
    delete m_pDat;
    delete m_pImg;
}

AXToolBar::AXToolBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createToolBar();
}

AXToolBar::AXToolBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createToolBar();
}

//! 作成

void AXToolBar::_createToolBar()
{
    m_uType = TYPE_TOOLBAR;

    m_pDat      = new AXList;
    m_pImg      = new AXPixmap;
    m_pImgList  = NULL;
    m_pOnItem   = NULL;
    m_fDown     = 0;
    m_nIconW = m_nIconH = 0;
    m_trGroupID = 0;

    if(m_uStyle & TBS_TOOLTIP)
        m_pToolTip = new AXToolTipWin(0);
    else
        m_pToolTip = NULL;
}

//! 標準サイズ計算

void AXToolBar::calcDefSize()
{
    int line;

    m_nDefW = _getMaxWidth(&line);
    m_nDefH = line * (m_nIconH + SPACEBT * 2);

    if(m_uStyle & TBS_SEPBOTTOM) m_nDefH += 6;
    if(m_uStyle & TBS_SPACEBOTTOM) m_nDefH += SPACEBOTTOM;
}

//! 再構成

void AXToolBar::reconfig()
{
    AXToolBarItem *p;
    int x = 0,y = 0;

    for(p = TOPITEM; p; p = p->next())
    {
        p->m_nX = x;
        p->m_nY = y;

        x += p->m_nW;

        if(p->m_uFlags & BF_WRAP)
        {
            x = 0;
            y += m_nIconH + SPACEBT * 2;
        }
    }
}


//==============================
//
//==============================


//! イメージリストセット

void AXToolBar::setImageList(AXImageList *pImg)
{
    m_pImgList = pImg;

    m_nIconW = pImg->getOneW();
    m_nIconH = pImg->getHeight();
}

//! ツールチップ文字列の文字列グループID指定
/*!
    TBS_TOOLTIP_TR が ON の場合に有効
*/

void AXToolBar::setToolTipTrGroup(WORD wGroupID)
{
    m_trGroupID = wGroupID;
}

//! ツールチップ文字列セット
/*!
    '\\t' で区切って複数指定。
*/

void AXToolBar::setToolTipText(LPCUSTR pText)
{
    m_strToolTip = pText;
}

//! 文字列IDの配列からツールチップ文字列セット
/*!
    ・文字列グループはあらかじめセットしておく。@n
    ・0xffff の値は無視される。

    @param wMask IDの値にこの値がマスクされる（チェックボタン用にビットをONにしている場合など）
*/

void AXToolBar::setToolTipTextTr(const LPWORD pStrIDArray,int cnt,WORD wMask)
{
    LPWORD p = pStrIDArray;

    m_strToolTip.empty();

    for(; cnt > 0; cnt--, p++)
    {
        if(*p != 0xffff)
        {
            m_strToolTip += _str(*p & wMask);
            m_strToolTip += '\t';
        }
    }
}

//! 連続した文字列IDからツールチップ文字列セット

void AXToolBar::setToolTipTextTr(int nStartID,int cnt)
{
    m_strToolTip.empty();

    for(; cnt > 0; cnt--, nStartID++)
    {
        m_strToolTip += _str(nStartID);
        m_strToolTip += '\t';
    }
}

//-----------------

//! ボタン追加
/*!
    ※イメージリストセット後。

    @param uID      アイテムID
    @param nImgNo   イメージ番号（0〜）
    @param nToolTipNo   ツールチップ番号（負の値でなし）
*/

void AXToolBar::addItem(UINT uID,int nImgNo,int nToolTipNo,UINT uFlags,ULONG lParam)
{
    m_pDat->add(new AXToolBarItem(uID, nImgNo, nToolTipNo, uFlags, lParam, m_nIconW));
    setReconfig();
}

//! セパレータ追加

void AXToolBar::addItemSep()
{
    addItem((UINT)-1, 0, -1, AXToolBar::BF_SEP, 0);
}

//! チェックのON/OFF
/*!
    @param check [0] OFF [正] ON [負] 反転（グループの場合は反転はなし）
*/

void AXToolBar::checkItem(UINT uID,int check)
{
    AXToolBarItem *p = _getItem(uID);

    if(p && (p->m_uFlags & (BF_CHECKBUTTON | BF_CHECKGROUP)))
    {
        if(p->isCheckGroup())
        {
            if(check == 0)
                p->uncheck();
            else
                _checkGroup(p);
        }
        else
        {
            if(check == 0)
                p->uncheck();
            else if(check > 0)
                p->check();
            else
                p->m_uFlags ^= BF_CHECKED;
        }

        redraw();
    }
}

//! チェックされているか

BOOL AXToolBar::isCheckItem(UINT uID)
{
    AXToolBarItem *p = _getItem(uID);

    if(p)
        return (p->m_uFlags & BF_CHECKED)? 1: 0;
    else
        return FALSE;
}

//! 有効/無効セット（アイテムID）

void AXToolBar::enableItem(UINT uID,BOOL bEnable)
{
    AXToolBarItem *p = _getItem(uID);

    if(p)
    {
        if(bEnable)
            p->m_uFlags &= ~BF_DISABLE;
        else
            p->m_uFlags |= BF_DISABLE;

        redraw();
    }
}

//! 有効/無効セット（アイテムのインデックス番号）

void AXToolBar::enableItemNo(int no,BOOL bEnable)
{
    AXToolBarItem *p = (AXToolBarItem *)m_pDat->getItem(no);

    if(p)
    {
        if(bEnable)
            p->m_uFlags &= ~BF_DISABLE;
        else
            p->m_uFlags |= BF_DISABLE;

        redraw();
    }
}

//! アイテムの座標取得
//! @param bRoot ルート座標で取得（FALSE でツールバーの座標）

void AXToolBar::getItemRect(UINT uID,AXRect *prc,BOOL bRoot)
{
    AXToolBarItem *p = _getItem(uID);

    if(p)
    {
        prc->setFromSize(p->m_nX, p->m_nY, p->m_nW, m_nIconH + SPACEBT * 2);

        if(bRoot)
            translateRectTo(NULL, prc);
    }
}


//==============================
//サブ
//==============================


//! 全体の最大幅と行数取得

int AXToolBar::_getMaxWidth(LPINT pLineCnt)
{
    AXToolBarItem *p;
    int max = 0,now = 0,line = 1;

    for(p = TOPITEM; p; p = p->next())
    {
        now += p->m_nW;

        if(p->m_uFlags & BF_WRAP)
        {
            if(now > max) max = now;
            now = 0;

            if(p->next()) line++;
        }
    }

    if(now > max) max = now;

    *pLineCnt = line;

    return max;
}

//! カーソル位置からアイテム取得

AXToolBarItem *AXToolBar::_getCurItem(int x,int y)
{
    AXToolBarItem *p;
    int h;

    h = m_nIconH + SPACEBT * 2;

    for(p = TOPITEM; p; p = p->next())
    {
        if(p->m_nX <= x && x < p->m_nX + p->m_nW && p->m_nY <= y && y < p->m_nY + h)
            return p;
    }

    return NULL;
}

//! カーソルがボタン上に乗っている場合の処理

void AXToolBar::_cursorOnBtt(int x,int y)
{
    AXToolBarItem *p;

    if(x < 0 || y < 0)
        p = NULL;
    else
        p = _getCurItem(x, y);

    //セパレータの場合はなし

    if(p && (p->m_uFlags & BF_SEP))
        p = NULL;

    //

    if(p != m_pOnItem)
    {
        m_pOnItem = p;

        if(!(m_uStyle & TBS_FRAME))
            redrawUpdate();

        //ツールチップ

        if(m_pToolTip)
        {
            if(p && p->m_nToolTipNo >= 0)
            {
                if(m_pToolTip->isVisible())
                    _showToolTip(p);
                else
                    addTimer(0, 500);
            }
            else
                _cancelToolTip();
        }
    }
}

//! ツールチップ表示

void AXToolBar::_showToolTip(AXToolBarItem *p)
{
    AXPoint pt;
    AXString str;

    if(m_uStyle & TBS_TOOLTIP_TR)
        str = _string(m_trGroupID, p->m_nToolTipNo);
    else
        m_strToolTip.getSplitPosStr(&str, p->m_nToolTipNo, '\t');

    axapp->getCursorPos(&pt);

    if(!m_pToolTip->showTip(pt.x + 12, pt.y + 16, str))
        _cancelToolTip();
}

//! ツールチップを消す

void AXToolBar::_cancelToolTip()
{
    delTimer(0);

    if(m_pToolTip) m_pToolTip->hide();
}

//! グループのチェック処理

void AXToolBar::_checkGroup(AXToolBarItem *pSel)
{
    AXToolBarItem *p;

    //前方向 OFF

    for(p = pSel->prev(); p; p = p->prev())
    {
        if(!p->isCheckGroup()) break;
        p->uncheck();
    }

    //後ろ方向 OFF

    for(p = pSel->next(); p; p = p->next())
    {
        if(!p->isCheckGroup()) break;
        p->uncheck();
    }

    pSel->check();
}

//! IDからアイテム取得

AXToolBarItem *AXToolBar::_getItem(UINT uID)
{
    AXToolBarItem *p;

    for(p = TOPITEM; p; p = p->next())
    {
        if(p->m_uID == uID) return p;
    }

    return NULL;
}


//==============================
//ハンドラ
//==============================


//! サイズ変更時

BOOL AXToolBar::onSize()
{
    m_pImg->recreate(m_nW, m_nH, 32, 32);

    redraw();

    return TRUE;
}

//! タイマ

BOOL AXToolBar::onTimer(UINT uTimerID,ULONG lParam)
{
    if(m_pOnItem)
        _showToolTip(m_pOnItem);

    delTimer(uTimerID);

    return TRUE;
}

//! 範囲内に入った

BOOL AXToolBar::onEnter(AXHD_ENTERLEAVE *phd)
{
    _cursorOnBtt(phd->x, phd->y);

    return TRUE;
}

//! 範囲外に出た

BOOL AXToolBar::onLeave(AXHD_ENTERLEAVE *phd)
{
    _cursorOnBtt(-1, -1);

    return TRUE;
}

//! ボタン押し時

BOOL AXToolBar::onButtonDown(AXHD_MOUSE *phd)
{
    if(!m_fDown && m_pOnItem && phd->button == BUTTON_LEFT)
    {
        //無効時ではない時

        if(!(m_pOnItem->m_uFlags & BF_DISABLE))
        {
            _cancelToolTip();

            //通常ボタン上 or ドロップボタン上

            if((m_pOnItem->m_uFlags & BF_DROPDOWN) && phd->x >= m_pOnItem->m_nX + m_pOnItem->m_nW - DROPW)
                m_fDown = DOWNF_DROP;
            else
                m_fDown = DOWNF_BUTTON;

            //チェック処理

            if(m_fDown == DOWNF_BUTTON)
            {
                if(m_pOnItem->m_uFlags & BF_CHECKBUTTON)
                    m_pOnItem->m_uFlags ^= BF_CHECKED;
                else if(m_pOnItem->m_uFlags & BF_CHECKGROUP)
                    _checkGroup(m_pOnItem);
            }

            //

            grabPointer();
            redrawUpdate();
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXToolBar::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fDown && phd->button == BUTTON_LEFT)
    {
        //通知

		if(m_pOnItem)
		{
			getNotify()->sendCommand(m_pOnItem->m_uID, m_pOnItem->m_lParam,
                    (m_fDown == DOWNF_DROP)? COMFROM_TOOLBAR_DROP: COMFROM_TOOLBAR);
        }

        //

        ungrabPointer();
        m_fDown = 0;

        //離された位置で再度判定

        _cursorOnBtt(phd->x, phd->y);

        redrawUpdate();
    }

    return TRUE;
}

//! マウス移動時

BOOL AXToolBar::onMouseMove(AXHD_MOUSE *phd)
{
    if(!m_fDown)
        _cursorOnBtt(phd->x, phd->y);

    return TRUE;
}

//! 描画

BOOL AXToolBar::onPaint(AXHD_PAINT *phd)
{
    if(m_uFlags & FLAG_REDRAW)
        _draw();

    m_pImg->put(this, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}

//! 描画処理

void AXToolBar::_draw()
{
    AXToolBarItem *p;
    int x,y,h,bFrame,tmp,bDown,bDisable;

    //背景

    m_pImg->drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);

    //セパレータ

    if(m_uStyle & TBS_SEPBOTTOM)
    {
        m_pImg->drawLineH(0, m_nH - 4, m_nW, AXAppRes::FRAMEDARK);
        m_pImg->drawLineH(0, m_nH - 3, m_nW, AXAppRes::FRAMELIGHT);
    }

    //------- アイテム

    h = m_nIconH + SPACEBT * 2;

    for(p = TOPITEM; p; p = p->next())
    {
        x = p->m_nX, y = p->m_nY;

        if(p->m_uFlags & BF_SEP)
        {
            //======== セパレータ

            tmp = x + (SEPW - 2) / 2;

            m_pImg->drawLineV(tmp, y + 2, h - 4, AXAppRes::FRAMEDARK);
            m_pImg->drawLineV(tmp + 1, y + 2, h - 4, AXAppRes::FRAMELIGHT);
        }
        else
        {
            bFrame   = ((m_uStyle & TBS_FRAME) || p == m_pOnItem);
            bDisable = ((p->m_uFlags & BF_DISABLE) || !(m_uFlags & FLAG_ENABLED));

            //------- ボタン側

            tmp   = m_nIconW + SPACEBT * 2;
            bDown = (p->isCheckOn() || (m_fDown == DOWNF_BUTTON && p == m_pOnItem));

            //チェック背景

            if(p->isCheckOn())
                m_pImg->drawFillBox(x + 1, y + 1, tmp - 2, h - 2, AXAppRes::FACEDARK);

            //枠

            if(bDown)
                m_pImg->drawFrameSunken(x, y, tmp, h);
            else if(bFrame)
                m_pImg->drawFrameRaise(x, y, tmp, h);

            //画像

            m_pImgList->put(m_pImg->getid(), x + SPACEBT + bDown, y + SPACEBT + bDown, p->m_nImgNo, bDisable);

            //-------- ドロップボックス側

            if(p->m_uFlags & BF_DROPDOWN)
            {
                x += tmp;

                bDown = (m_fDown == DOWNF_DROP && p == m_pOnItem);

                if(bDown)
                    m_pImg->drawFrameSunken(x, y, DROPW, h);
                else if(bFrame)
                    m_pImg->drawFrameRaise(x, y, DROPW, h);

                m_pImg->drawArrowDown(x + DROPW / 2 + bDown, y + h / 2 + bDown,
                              (bDisable)? AXAppRes::TEXTDISABLE: AXAppRes::TEXTNORMAL);
            }
        }
    }
}
