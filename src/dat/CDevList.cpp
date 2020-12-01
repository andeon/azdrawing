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

#include "CDevList.h"

#include "AXMem.h"


//**************************************
// CDevList
//**************************************

/*!
    @class CDevList
    @brief 各入力デバイスの設定データリスト
*/


CDevList::CDevList()
{
    m_pDefItem = NULL;
}

//! 初期時用、デバイス追加

void CDevList::addDeviceInit(int devid,const AXString &strName,int bttnum,
                             int pressno,double pressmin,double pressmax)
{
    CDevItem *p;

    AXList::add(p = new CDevItem);

    p->m_devid      = devid;
    p->m_strName    = strName;
    p->m_nBttNum    = bttnum;
    p->m_nPressNo   = pressno;
    p->m_dPressMin  = pressmin;
    p->m_dPressMax  = pressmax;

    //デフォルトのボタン動作データセット

    if(devid == 2)
    {
        m_pDefItem = p;

        p->setDefaultAction();
    }
}

//! デバイス追加（設定からの読み込み時）
/*!
    すでに setDevice() でセットされている場合はそのままポインタを返す。@n
    現時点で接続されていないデバイスの場合に追加。
*/

CDevItem *CDevList::addDevice(const AXString &strName)
{
    CDevItem *p;

    p = getDevFromName(strName);

    if(!p)
    {
        AXList::add(p = new CDevItem);

        p->m_strName = strName;
    }

    return p;
}

//! デバイス名からアイテム取得

CDevItem *CDevList::getDevFromName(const AXString &strName)
{
    CDevItem *p;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_strName == strName) return p;
    }

    return NULL;
}

//! デバイスIDからアイテム取得

CDevItem *CDevList::getDevFromID(int devid)
{
    CDevItem *p;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_devid == devid) return p;
    }

    return NULL;
}

//! ボタン情報からボタン＋動作データ取得

UINT CDevList::getButtonAction(int devid,int bttno,BOOL bCtrl,BOOL bShift,BOOL bAlt)
{
    CDevItem *p = NULL;
    LPDWORD pdw;
    DWORD val,valMain;
    int i;

    //ボタン値

    val = bttno - 1;
    if(bCtrl)  val |= CDevItem::FLAG_CTRL;
    if(bShift) val |= CDevItem::FLAG_SHIFT;
    if(bAlt)   val |= CDevItem::FLAG_ALT;

    valMain = (bttno - 1) | (CDevItem::ACTION_MAIN << CDevItem::SHIFT_ACTION);

    //デバイス取得

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_devid == devid) break;
    }

    if(!p) p = m_pDefItem;

    //(指定デバイス) -> デフォルトの順で検索

    while(p)
    {
        //ボタンから検索

        for(i = p->m_nActCnt, pdw = p->m_pActDat; i > 0; i--, pdw++)
        {
            if((*pdw & CDevItem::MASK_BUTTONMOD) == val)
                return *pdw;
            else if(*pdw == valMain)
                //メイン操作のボタンは装飾キーなしの状態で判定
                /* メイン操作+装飾キーは各ツールごとの独自の処理を行わせるため */
                return val | (CDevItem::ACTION_MAIN << CDevItem::SHIFT_ACTION);
        }

        //見つからなかった場合、デフォルトを検索

        if(p == m_pDefItem)
            break;
        else
            p = m_pDefItem;
    }

    return val | (CDevItem::ACTION_NONE << CDevItem::SHIFT_ACTION);
}


//**************************************
// CDevItem
//**************************************

/*!
    @class CDevItem
    @brief 入力デバイスのアイテム

    - 名前が空の場合はデフォルトデータ。
    - デバイスIDが 0 で空データ（同名のデバイスが見つからなかった）
*/


CDevItem::~CDevItem()
{
    free();
}

CDevItem::CDevItem()
{
    m_devid     = 0;
    m_nActCnt   = 0;
    m_pActDat   = NULL;
    m_nBttNum   = 0;
    m_nPressNo  = PRESS_NONE;
    m_dPressMin = m_dPressMax = 1;
}

//! 解放

void CDevItem::free()
{
    AXFree((void **)&m_pActDat);

    m_nActCnt = 0;
}

//! ボタン動作バッファ確保

void CDevItem::allocAction(int cnt)
{
    free();

    m_pActDat = (LPDWORD)AXMalloc(cnt * sizeof(DWORD));
    m_nActCnt = cnt;
}

//! デフォルトのボタン動作データセット

void CDevItem::setDefaultAction()
{
    allocAction(4);

    m_pActDat[0] = ACTION_MAIN << SHIFT_ACTION;
    m_pActDat[1] = 1 | (2300 << SHIFT_ACTION);  //キャンバス移動
    m_pActDat[2] = 3 | (1300 << SHIFT_ACTION);  //1段階拡大
    m_pActDat[3] = 4 | (1301 << SHIFT_ACTION);  //1段階縮小
}

//! ボタン動作データセット
/*!
    @param cnt データ数。-1 でサイズから自動計算。
*/

void CDevItem::setAction(AXMem *pmem,int cnt)
{
    if(cnt < 0)
        cnt = pmem->getSize() / sizeof(DWORD);

    if(cnt)
    {
        allocAction(cnt);

        pmem->copyTo(m_pActDat, cnt * sizeof(DWORD));
    }
}

//! 筆圧取得（0.0-1.0）

double CDevItem::getPress(double press)
{
    return (press - m_dPressMin) / (m_dPressMax - m_dPressMin);
}

//! 編集用に動作データを AXMem にコピー

void CDevItem::setActEditDat(AXMem *pmem)
{
    if(pmem->alloc(m_nActCnt * sizeof(DWORD)))
        pmem->copyFrom(m_pActDat, m_nActCnt * sizeof(DWORD));
}
