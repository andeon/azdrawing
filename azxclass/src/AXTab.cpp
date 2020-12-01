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

#include "AXTab.h"

#include "AXTabItem.h"
#include "AXFont.h"
#include "AXImageList.h"
#include "AXAppRes.h"
#include "AXApp.h"


#define SPACEX      6
#define SPACEY      3
#define SPACEICON   3
#define MINSPACE    12


//******************************
// AXTabItem
//******************************


AXTabItem::AXTabItem(LPCUSTR pText,int iconno,ULONG lParam,int width)
{
    if(pText) m_strText = pText;
    m_nIconNo   = iconno;
    m_nWidth    = width;
    m_lParam    = lParam;
    m_nTabX     = 0;
}


//******************************
// AXTab
//******************************


/*!
    @class AXTab
    @brief タブウィジェット

    - アイテムの番号を指定する no は、負の値で現在の選択位置となる。

    @ingroup widget
*/
/*!
    @var AXTab::TABS_TOPTAB
    @brief 囲むタイプではなくタブ部分のみにする
    @var AXTab::TABS_FIT
    @brief タブの全体幅が収まらない場合、各タブ位置をずらすことでサイズに合わせる
    @var AXTab::TABS_ICONHEIGHT
    @brief タブの高さをアイコンの高さに固定する

    @var AXTab::TABN_SELCHANGE
    @brief タブの選択が変わった時（lParam = 0）
*/


AXTab::~AXTab()
{
    delete m_pDat;
}

AXTab::AXTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createTab();
}

AXTab::AXTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createTab();
}

//! 作成

void AXTab::_createTab()
{
    m_uType = TYPE_TAB;

    m_pDat     = new AXList;
    m_pFocus   = NULL;
    m_pImgList = NULL;
    m_nTabH    = 0;
}

//! 標準サイズ計算

void AXTab::calcDefSize()
{
    //タブの高さ

    _calcTabH();

    //

    AXWindow::calcDefSize();

    m_nDefW += 2;
    m_nDefH += m_nTabH + 1;
}

//! 再構成

void AXTab::reconfig()
{
    _calcTabX();

    if(!m_pFocus)
        m_pFocus = (AXTabItem *)m_pDat->getTop();

    redraw();
}

//! 範囲取得

void AXTab::getClientRect(AXRect *prc)
{
    prc->set(1, m_nTabH, m_nW - 2, m_nH - 2);
}


//==============================
//
//==============================


//! アイテム追加
/*!
    @param pText   NULL で空
    @param nIconNo アイコン番号（マイナスでなし）
    @param width   タブの幅（余白部分は除く。負の値で自動）
*/

void AXTab::addItem(LPCUSTR pText,int nIconNo,ULONG lParam,int width)
{
    //幅

    if(width < 0)
    {
        width = (pText)? m_pFont->getTextWidth(pText, -1): 0;

        if(m_pImgList && nIconNo >= 0)
        {
            width += m_pImgList->getOneW();
            if(pText) width += SPACEICON;
        }
    }

    width += SPACEX * 2;

    //

    m_pDat->add(new AXTabItem(pText, nIconNo, lParam, width));

    setReconfig();
}

void AXTab::addItem(LPCUSTR pText)
{
    addItem(pText, -1, 0, -1);
}

void AXTab::addItem(LPCUSTR pText,int nIconNo,ULONG lParam)
{
    addItem(pText, nIconNo, lParam, -1);
}

void AXTab::addItem(WORD wStrID)
{
    addItem(_str(wStrID), -1, 0, -1);
}

//! タブ削除

void AXTab::delItem(int no)
{
    AXListItem *p = _getItem(no);

    if(p)
    {
        m_pDat->deleteItem(p);

        if(p == m_pFocus)
            m_pFocus = (AXTabItem *)m_pDat->getTop();

        setReconfig();
    }
}

//! タブ全て削除

void AXTab::deleteAllItem()
{
    m_pDat->deleteAll();
    m_pFocus = NULL;

    redraw();
}

//! 現在の選択取得（-1でなし）

int AXTab::getCurSel()
{
    return m_pDat->getPos(m_pFocus);
}

//! 選択位置変更

void AXTab::setCurSel(int no)
{
    AXTabItem *p = _getItem(no);

    if(p != m_pFocus)
    {
        m_pFocus = p;
        redraw();
    }
}

//! アイテム数取得

int AXTab::getItemCnt()
{
    return m_pDat->getCnt();
}

//! アイテムのパラメータ取得

ULONG AXTab::getItemParam(int no)
{
    AXTabItem *p = _getItem(no);

    if(p)
        return p->m_lParam;
    else
        return 0;
}

//! アイテムのテキストセット

void AXTab::setItemText(int no,LPCUSTR pText)
{
    AXTabItem *p = _getItem(no);

    if(p)
    {
        p->m_strText = pText;

        _calcTabWidth(p);
        setReconfig();
    }
}

//! アイテムのパラメータセット

void AXTab::setItemParam(int no,ULONG lParam)
{
    AXTabItem *p = _getItem(no);

    if(p) p->m_lParam = lParam;
}


//==============================
//サブ
//==============================


//! 指定位置のアイテム取得
//! @param no 負の値で現在の選択

AXTabItem *AXTab::_getItem(int no)
{
    AXTabItem *p;

    if(no < 0)
        p = m_pFocus;
    else
        p = (AXTabItem *)m_pDat->getItem(no);

    return p;
}

//! タブの高さ計算（calcDefSize 時）

void AXTab::_calcTabH()
{
    if((m_uStyle & TABS_ICONHEIGHT) && m_pImgList)
        m_nTabH = m_pImgList->getHeight();
    else
    {
        m_nTabH = m_pFont->getHeight();
        if(m_nTabH < 14) m_nTabH = 14;
        if(m_pImgList && m_nTabH < m_pImgList->getHeight()) m_nTabH = m_pImgList->getHeight();
    }

    m_nTabH += SPACEY * 2;
}

//! タブの幅再計算

void AXTab::_calcTabWidth(AXTabItem *p)
{
    p->m_nWidth = m_pFont->getTextWidth(p->m_strText);
    if(m_pImgList && p->m_nIconNo >= 0) p->m_nWidth += SPACEICON + m_pImgList->getOneW();
    p->m_nWidth += SPACEX * 2;
}

//! 各タブの x 位置を計算

void AXTab::_calcTabX()
{
    AXTabItem *p,*p1;
    int x = 0,maxw = 1,remain,runflag = TRUE,flag,xx;

    //通常位置

    for(p = (AXTabItem *)m_pDat->getTop(); p; p = p->next())
    {
        p->m_nTabX = x;
        x += p->m_nWidth - 1;
        maxw += p->m_nWidth - 1;
    }

    //詰めるか

    if(!(m_uStyle & TABS_FIT)) return;

    remain = maxw - m_nW;
    if(remain <= 0) return;

    //詰める

    while(remain > 0 && runflag)
    {
        p = (AXTabItem *)m_pDat->getTop();
        if(p) p = p->next();

        for(runflag = FALSE; p && remain; p = p->next())
        {
            //左のタブとの左の隙間

            p1 = p->prev();

            if(p->m_nTabX - p1->m_nTabX <= MINSPACE) continue;

            //左側のタブとの右の隙間

            xx = p->m_nTabX + p->m_nWidth;

            for(p1 = p->prev(), flag = 0; p1; p1 = p1->prev())
            {
                if(xx - p1->m_nTabX - p1->m_nWidth <= MINSPACE) { flag = 1; break; }
            }

            if(flag) continue;

            //ずらす

            for(p1 = p; p1; p1 = p1->next())
                p1->m_nTabX--;

            remain--;
            runflag = TRUE;
        }
    }
}

//! カーソル位置からアイテム取得

AXTabItem *AXTab::_getCurPosItem(int x,int y)
{
    AXTabItem *p;

    if(!m_pFocus || y >= m_nTabH) return NULL;

    //フォーカス

    if(m_pFocus->m_nTabX <= x && x < m_pFocus->m_nTabX + m_pFocus->m_nWidth)
        return m_pFocus;

    //

    if(x < m_pFocus->m_nTabX)
    {
        //フォーカスより左

        for(p = m_pFocus->prev(); p; p = p->prev())
        {
            if(p->m_nTabX <= x && x < p->m_nTabX + p->m_nWidth)
                return p;
        }
    }
    else
    {
        //フォーカスより右

        for(p = m_pFocus->next(); p; p = p->next())
        {
            if(p->m_nTabX <= x && x < p->m_nTabX + p->m_nWidth)
                return p;
        }
    }

    return NULL;
}


//==============================
//ハンドラ
//==============================


//! サイズ変更時

BOOL AXTab::onSize()
{
    if(m_uStyle & TABS_FIT)
        _calcTabX();

    return TRUE;
}

//! ボタン押し時

BOOL AXTab::onButtonDown(AXHD_MOUSE *phd)
{
    AXTabItem *p;

    if(phd->button == BUTTON_LEFT)
    {
        p = _getCurPosItem(phd->x, phd->y);

        if(p && p != m_pFocus)
        {
            m_pFocus = p;
            redraw();

            getNotify()->sendNotify(this, TABN_SELCHANGE, 0);
        }
    }

    return TRUE;
}

//! 描画

BOOL AXTab::onPaint(AXHD_PAINT *phd)
{
    AXTabItem *p;
    int x,tcol;
    AXDrawable::DRAWPOINT pt[9];

    //背景

    if(!m_pFocus)
    {
        drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);
        return TRUE;
    }
    else
    {
        if(m_uStyle & TABS_TOPTAB)
            drawFillBox(0, 0, m_nW, m_nH, AXAppRes::FACE);
        else
        {
            drawFillBox(0, 0, m_nW, m_nTabH - 1, AXAppRes::FACE);
            drawFillBox(1, m_nTabH, m_nW - 1, m_nH - m_nTabH, AXAppRes::FACELIGHT);
        }
    }

    //------------

    tcol = (m_uFlags & FLAG_ENABLED)? AXAppRes::TC_NORMAL: AXAppRes::TC_DISABLE;

    AXDrawText dt(m_id);

    //タブ（フォーカスより左）

    for(p = (AXTabItem *)m_pDat->getTop(); p && p != m_pFocus; p = p->next())
        _drawTab(p, tcol, pt, &dt);

    //タブ（フォーカスより右）

    for(p = (AXTabItem *)m_pDat->getBottom(); p && p != m_pFocus; p = p->prev())
        _drawTab(p, tcol, pt, &dt);

    //フォーカスタブ

    p = m_pFocus;

    if(p)
    {
        x = p->m_nTabX;

        //背景

        drawFillBox(x + 1, 1, p->m_nWidth - 2, m_nTabH - 1, AXAppRes::FACELIGHT);

        //枠

        if(m_uStyle & TABS_TOPTAB)
        {
            //タブのみ

            pt[0].x = 0, pt[0].y = m_nTabH - 1;
            pt[1].x = x, pt[1].y = pt[0].y;
            pt[2].x = x, pt[2].y = 0;
            pt[3].x = x + p->m_nWidth - 1, pt[3].y = 0;
            pt[4].x = pt[3].x, pt[4].y = pt[0].y;
            pt[5].x = m_nW - 1, pt[5].y = pt[4].y;

            drawLines(pt, 6, AXAppRes::FRAMEOUTSIDE);
        }
        else
        {
            //囲む

            pt[0].x = x, pt[0].y = 0;
            pt[1].x = x + p->m_nWidth - 1, pt[1].y = 0;
            pt[2].x = pt[1].x, pt[2].y = m_nTabH - 1,
            pt[3].x = m_nW - 1, pt[3].y = pt[2].y;
            pt[4].x = pt[3].x, pt[4].y = m_nH - 1;
            pt[5].x = 0, pt[5].y = pt[4].y;
            pt[6].x = 0, pt[6].y = pt[2].y;
            pt[7].x = x, pt[7].y = pt[2].y;
            pt[8].x = x, pt[8].y = 0;

            drawLines(pt, 9, AXAppRes::FRAMEOUTSIDE);
        }

        //

        x += SPACEX;

        if(m_pImgList && p->m_nIconNo >= 0)
        {
            m_pImgList->put(m_id, x, (m_nTabH - m_pImgList->getHeight()) / 2, p->m_nIconNo);
            x += SPACEICON + m_pImgList->getOneW();
        }

        dt.draw(*m_pFont, x, (m_nTabH - m_pFont->getHeight()) / 2, p->m_strText, tcol);
    }

    dt.end();

    return TRUE;
}

//! フォーカス以外のタブ描画

void AXTab::_drawTab(AXTabItem *p,int tcol,DRAWPOINT *pt,AXDrawText *pdt)
{
    int x = p->m_nTabX;

    //背景

    drawFillBox(x + 1, 2, p->m_nWidth - 2, m_nTabH - 3, AXAppRes::FACEDARK);

    //枠

    pt[0].x = x, pt[0].y = m_nTabH - 1;
    pt[1].x = pt[0].x, pt[1].y = 1;
    pt[2].x = x + p->m_nWidth - 1, pt[2].y = 1;
    pt[3].x = pt[2].x, pt[3].y = pt[0].y;

    drawLines(pt, 4, AXAppRes::FRAMEOUTSIDE);

    //アイコン・テキスト

    x += SPACEX;

    if(m_pImgList && p->m_nIconNo >= 0)
    {
        m_pImgList->put(m_id, x, (m_nTabH - m_pImgList->getHeight() - 1) / 2 + 1, p->m_nIconNo);
        x += SPACEICON + m_pImgList->getOneW();
    }

    pdt->draw(*m_pFont, x, (m_nTabH - m_pFont->getHeight() - 1) / 2 + 1, p->m_strText, tcol);
}
