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

#include "AXLayoutItem.h"
#include "AXSpacerItem.h"

#include "AXRect.h"


//********************************
// AXLayoutItem
//********************************

/*!
    @class AXLayoutItem
    @brief レイアウトアイテムの基底クラス

    - AXWindow や AXLayout に派生する。
    - AXLayoutItem は、レイアウトの addItem() でレイアウトにセットできる。
    - m_nDefW/H は calcDefSize() で計算される。
    - レイアウト時の最小サイズは、基本的に m_nDefW/H。@n
      m_nMinW/H が m_nDefW/H より大きい場合は、m_nMinW/H が最小値となる。@n
      m_nMinW/H が負の値の場合は、その絶対値が最小値となる（m_nDefW/H より小さいサイズを最小値としたい場合に使う）

    @ingroup layout
*/

/*!
    @enum AXLayoutItem::LAYOUTFLAGS
    @brief レイアウトフラグ

    デフォルトで位置は左上・サイズは標準サイズを使う。

    @var AXLayoutItem::LF_CENTER_X
    @brief 水平方向の中央
    @var AXLayoutItem::LF_RIGHT
    @brief 右端
    @var AXLayoutItem::LF_CENTER_Y
    @brief 垂直方向の中央
    @var AXLayoutItem::LF_BOTTOM
    @brief 下端
    @var AXLayoutItem::LF_EXPAND_X
    @brief サイズは拡張せずに、領域のみ水平方向に拡張する
    @var AXLayoutItem::LF_EXPAND_Y
    @brief サイズは拡張せずに、領域のみ垂直方向に拡張する
    @var AXLayoutItem::LF_EXPAND_W
    @brief アイテムのサイズを水平方向に拡張する
    @var AXLayoutItem::LF_EXPAND_H
    @brief アイテムのサイズを垂直方向に拡張する
    @var AXLayoutItem::LF_FIX_W
    @brief サイズをアイテムの実際の幅で固定する
    @var AXLayoutItem::LF_FIX_H
    @brief サイズをアイテムの実際の高さで固定する
    @var AXLayoutItem::LF_COL_W
    @brief matrix時、縦列の最大幅でサイズ拡張
    @var AXLayoutItem::LF_ROW_H
    @brief matrix時、横列の最大高さでサイズ拡張
*/

//------------------------


AXLayoutItem::~AXLayoutItem()
{

}

//! 初期化

AXLayoutItem::AXLayoutItem()
{
    m_pLParent = m_pLNext = m_pLPrev = NULL;

    m_nX = m_nY = m_nW = m_nH = 0;
    m_nDefW = m_nDefH = 0;
    m_nMinW = m_nMinH = 0;

    m_padLeft = m_padTop = m_padRight = m_padBottom = 0;

    m_uLFlags = 0;
    m_uType = TYPE_NONE;
}



//! レイアウト時の幅取得

int AXLayoutItem::getLayoutW()
{
    if(m_uLFlags & LF_FIX_W)
        return m_nW;
    else if(m_nMinW < 0)
        return -m_nMinW;
    else
        return (m_nMinW > m_nDefW)? m_nMinW: m_nDefW;
}

//! レイアウト時の高さ取得

int AXLayoutItem::getLayoutH()
{
    if(m_uLFlags & LF_FIX_H)
        return m_nH;
    else if(m_nMinH < 0)
        return -m_nMinH;
    else
        return (m_nMinH > m_nDefH)? m_nMinH: m_nDefH;
}

//! レイアウト時用、幅と余白取得

int AXLayoutItem::getWidthWithPadding()
{
    return getLayoutW() + m_padLeft + m_padRight;
}

//! レイアウト時用、高さと余白取得

int AXLayoutItem::getHeightWithPadding()
{
    return getLayoutH() + m_padTop + m_padBottom;
}

//! レイアウト時の最小幅取得

int AXLayoutItem::getLayoutMinW()
{
    if(m_nMinW < 0)
        return -m_nMinW;
    else
        return (m_nMinW > m_nDefW)? m_nMinW: m_nDefW;
}

//! レイアウト時の最小高さ取得

int AXLayoutItem::getLayoutMinH()
{
    if(m_nMinH < 0)
        return -m_nMinH;
    else
        return (m_nMinH > m_nDefH)? m_nMinH: m_nDefH;
}

//! 最小サイズセット

void AXLayoutItem::setMinSize(int w,int h)
{
    m_nMinW = w;
    m_nMinH = h;
}

//! 外側の余白セット

void AXLayoutItem::setPadding(int width)
{
    m_padLeft = m_padTop = m_padRight = m_padBottom = width;
}

//! 外側の余白セット

void AXLayoutItem::setPadding(int left,int top,int right,int bottom)
{
    m_padLeft   = left;
    m_padTop    = top;
    m_padRight  = right;
    m_padBottom = bottom;
}

//! 余白取得

void AXLayoutItem::getPadding(AXRect *prc)
{
    prc->left   = m_padLeft;
    prc->top    = m_padTop;
    prc->right  = m_padRight;
    prc->bottom = m_padBottom;
}

//! x,y,w,h の値を AXRectSize で取得

void AXLayoutItem::getRectSize(AXRectSize *prcs)
{
    prcs->set(m_nX, m_nY, m_nW, m_nH);
}

//! x,y,w,h の範囲を AXRect として取得

void AXLayoutItem::getRect(AXRect *prc)
{
    prc->setFromSize(m_nX, m_nY, m_nW, m_nH);
}

//! 範囲取得（自身を基準とする）

void AXLayoutItem::getClientRect(AXRect *prc)
{
    prc->left   = 0;
    prc->top    = 0;
    prc->right  = m_nW - 1;
    prc->bottom = m_nH - 1;
}


//========================


//! レイアウト（デフォルト）

void AXLayoutItem::layout()
{

}

//! 標準サイズ計算（デフォルト）

void AXLayoutItem::calcDefSize()
{
    m_nDefW = m_nDefH = 0;
}

//! 表示状態（デフォルト）

BOOL AXLayoutItem::isVisible()
{
    return TRUE;
}

//! 表示

void AXLayoutItem::show()
{

}

//! 非表示

void AXLayoutItem::hide()
{

}

//! 移動＆リサイズ（デフォルト）

BOOL AXLayoutItem::moveresize(int x,int y,int w,int h)
{
    if(x != m_nX || y != m_nY || w != m_nW || h != m_nH)
    {
        m_nX = x, m_nY = y;
        m_nW = w, m_nH = h;

        layout();

        return TRUE;
    }

    return FALSE;
}

//! AXRect の範囲で移動・リサイズ

void AXLayoutItem::moveresize(const AXRect &rc)
{
    moveresize(rc.left, rc.top, rc.width(), rc.height());
}


//****************************************
// AXSpacerItem
//****************************************

/*!
    @class AXSpacerItem
    @brief 空白用レイアウトアイテム

    matrixレイアウトなどで空白をセットしたい時に使う。

    @ingroup layout
*/

AXSpacerItem::AXSpacerItem()
    : AXLayoutItem()
{
    m_uType = TYPE_SPACER;
}
