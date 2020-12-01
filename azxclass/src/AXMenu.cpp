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

#include "AXMenu.h"
#include "AXMenuItem.h"

#include "AXList.h"
#include "AXFont.h"
#include "AXMenuWindow.h"
#include "AXApp.h"


#define ITEMSPACE_Y     3
#define ITEMSEP_H       5


//***********************************
// AXMenu
//***********************************

/*!
    @class AXMenu
    @brief メニューアイテムのデータ（ポップアップの１つ分）

    - new で作成すること。
    - 削除時は、サブメニューも含めて削除される。
    - 同じメニューデータを複数の項目でサブメニューとして指定してはいけない。
    - メニューのウィンドウは AXMenuWindow （通常、直接使うことはない）
    - メニューバーの場合は AXMenuBar で項目セット。@n
      独立ポップアップ表示の場合は AXMenu で項目をセットして popup() を実行。
    - テキストは、'&' の次がホットキー（"&&" で '&'）。@n
      '\\t' でアクセラレータの区切り。

    @ingroup widgetdat
*/
/*!
    @var AXMenu::POPF_RIGHT
    @brief 右寄せ
    @var AXMenu::POPF_NOSEND
    @brief 通知先ウィンドウに通知しない
*/
/*
    m_pwinPopup : AXMenuWindow で使われる。ポップアップウィンドウのポインタ
*/


AXMenu::~AXMenu()
{
    delete m_pList;
}

AXMenu::AXMenu()
{
    m_pList     = new AXList;
    m_pwinPopup = NULL;
}

//! 先頭アイテム取得

AXMenuItem *AXMenu::getTopItem() const
{
    return (AXMenuItem *)m_pList->getTop();
}

//! アイテム数取得

int AXMenu::getCnt() const
{
    return m_pList->getCnt();
}

//------------------

//! アイテム追加

void AXMenu::add(UINT uID,UINT uFlags,LPCUSTR pstr,AXMenu *pSubMenu,ULONG lParam)
{
    m_pList->add(new AXMenuItem(uID, uFlags, pstr, pSubMenu, lParam));
}

void AXMenu::add(UINT uID,UINT uFlags,LPCUSTR pstr)
{
    m_pList->add(new AXMenuItem(uID, uFlags, pstr, NULL, 0));
}

void AXMenu::add(UINT uID,LPCUSTR pstr,AXMenu *pSubMenu)
{
    m_pList->add(new AXMenuItem(uID, 0, pstr, pSubMenu, 0));
}

void AXMenu::add(UINT uID,LPCUSTR pstr)
{
    m_pList->add(new AXMenuItem(uID, 0, pstr, NULL, 0));
}

//! セパレータ追加

void AXMenu::addSep()
{
    m_pList->add(new AXMenuItem((UINT)-1, AXMenu::MIF_SEP, NULL, NULL, 0));
}

//! 自動チェックアイテム追加

void AXMenu::addCheck(UINT uID,LPCUSTR pstr,BOOL bCheck)
{
    m_pList->add(new AXMenuItem(uID, AXMenu::MIF_AUTOCHECK | (bCheck? AXMenu::MIF_CHECK: 0), pstr, NULL, 0));
}

//! ラジオチェックアイテム追加

void AXMenu::addRadio(UINT uID,LPCUSTR pstr)
{
    m_pList->add(new AXMenuItem(uID, AXMenu::MIF_AUTOCHECK | AXMenu::MIF_RADIO, pstr, NULL, 0));
}

//! オーナードローアイテム追加

void AXMenu::addOD(UINT uID,LPCUSTR pstr,ULONG lParam,int nItemW,int nItemH)
{
    AXMenuItem *p = new AXMenuItem(uID, AXMenu::MIF_OWNERDRAW, pstr, NULL, lParam);

    p->m_nTextW = nItemW;
    p->m_nItemH = nItemH;
    p->m_nAccW  = 0;

    m_pList->add(p);
}

//------------------------

//! 翻訳文字列からアイテム追加

void AXMenu::addTr(WORD wStrID,UINT uFlags,AXMenu *pSubMenu,ULONG lParam)
{
    m_pList->add(new AXMenuItem(wStrID, uFlags, _str(wStrID), pSubMenu, lParam));
}

void AXMenu::addTr(WORD wStrID)
{
    m_pList->add(new AXMenuItem(wStrID, 0, _str(wStrID), NULL, 0));
}

void AXMenu::addTr(WORD wStrID,UINT uFlags)
{
    m_pList->add(new AXMenuItem(wStrID, uFlags, _str(wStrID), NULL, 0));
}

void AXMenu::addTr(WORD wStrID,AXMenu *pSubMenu)
{
    m_pList->add(new AXMenuItem(wStrID, 0, _str(wStrID), pSubMenu, 0));
}

void AXMenu::addTrCheck(WORD wStrID,BOOL bCheck)
{
    m_pList->add(new AXMenuItem(wStrID, AXMenu::MIF_AUTOCHECK | (bCheck? AXMenu::MIF_CHECK: 0), _str(wStrID), NULL, 0));
}

void AXMenu::addTrRadio(WORD wStrID)
{
    m_pList->add(new AXMenuItem(wStrID, AXMenu::MIF_AUTOCHECK | AXMenu::MIF_RADIO, _str(wStrID), NULL, 0));
}

//----------------------

//! 指定文字列IDから連続したIDのアイテム追加

void AXMenu::addTrMul(UINT uStartID,int cnt)
{
    for(; cnt > 0; cnt--)
        addTr(uStartID++);
}

//! 文字列IDの配列から複数追加
/*!
    0xffff でセパレータ。0x8000 のフラグが ON で自動チェックタイプ。@n
    ※文字列IDは 0x8000 未満の値であること。
*/

void AXMenu::addTrArray(const LPWORD pIDArray,int cnt)
{
    int i,id;

    for(i = 0; i < cnt; i++)
    {
        id = pIDArray[i];

        if(id == 0xffff)
            addSep();
        else if(id & 0x8000)
            addTrCheck(id & 0x7fff, FALSE);
        else
            addTr(id);
    }
}

//! 文字列の配列からアイテムセット
/*!
    ・追加する前にすべて削除される @n
    ・空の文字列はセットされない

    @param uIDStart IDの開始番号
    @param pstr     文字列配列
    @param cnt      配列数
*/

void AXMenu::setStrArray(UINT uIDStart,AXString *pstr,int cnt)
{
    int i;

    deleteAll();

    for(i = 0; i < cnt; i++)
    {
        if(pstr[i].isNoEmpty())
            add(uIDStart + i, 0, pstr[i]);
    }
}

//----------------------

//! 全て削除

void AXMenu::deleteAll()
{
    m_pList->deleteAll();
}

//! 指定IDアイテム削除（サブメニュー含む）

void AXMenu::del(UINT uID)
{
    AXMenuItem *p;
    AXMenu *pOwner;

    p = getItemAll(uID, &pOwner);
    if(p) (pOwner->m_pList)->deleteItem(p);
}

//! 位置から削除

void AXMenu::delPos(int pos)
{
    m_pList->deleteItem(pos);
}

//-----------------------

//! 指定IDのパラメータ値取得

ULONG AXMenu::getParam(UINT uID)
{
    AXMenuItem *p = getItemAll(uID);
    if(p)
        return p->m_lParam;
    else
        return 0;
}

//------------------------

//! 有効/無効セット（サブメニュー含む）

void AXMenu::enable(UINT uID,BOOL bEnable)
{
    AXMenuItem *p = getItemAll(uID);

    if(p)
    {
        if(bEnable)
            p->m_uFlags &= ~MIF_DISABLE;
        else
            p->m_uFlags |= MIF_DISABLE;
    }
}

//! チェックON/OFF（サブメニュー含む）

void AXMenu::check(UINT uID,BOOL bCheck)
{
    AXMenuItem *p = getItemAll(uID);

    if(p)
    {
        //グループのチェックOFF
        if(p->m_uFlags & MIF_RADIO)
            p->checkRadio(FALSE);

        if(bCheck)
            p->m_uFlags |= MIF_CHECK;
        else
            p->m_uFlags &= ~MIF_CHECK;
    }
}

//! 指定IDの項目のサブメニューセット

void AXMenu::setSubMenu(UINT uID,AXMenu *pSubMenu)
{
    AXMenuItem *p = getItemAll(uID);

    if(p) p->m_pSubMenu = pSubMenu;
}

//----------------------

//! IDからアイテム取得（サブメニュー含まない）

AXMenuItem *AXMenu::getItem(UINT uID) const
{
    AXMenuItem *p;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_uID == uID) return p;
    }

    return NULL;
}

//! IDからアイテム取得（サブメニューも含む）
/*!
    @param ppOwnerMenu 指定アイテムの実際のオーナーが入る（NULL可）
*/

AXMenuItem *AXMenu::getItemAll(UINT uID,AXMenu **ppOwnerMenu)
{
    AXMenuItem *p,*psub;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_uID == uID)
        {
            if(ppOwnerMenu) *ppOwnerMenu = this;
            return p;
        }

        if(p->m_pSubMenu)
        {
            psub = (p->m_pSubMenu)->getItemAll(uID, ppOwnerMenu);
            if(psub) return psub;
        }
    }

    return NULL;
}

//! 最後のアイテム取得

AXMenuItem *AXMenu::getLastItem()
{
    return (AXMenuItem *)m_pList->getBottom();
}

//----------------------

//! 独立ポップアップ表示
/*!
    @param pSend 通知先ウィンドウ（NULL で通知しない）
    @param uFlags AXMenu::POPUPFLAGS
    @return 選択された項目のID（-1 でキャンセル）
*/

int AXMenu::popup(AXWindow *pSend,int rootx,int rooty,UINT uFlags)
{
    AXMenuWindow *pPop;
    AXMenuItem *pItem;

    if(m_pList->getCnt() == 0) return -1;

    pPop = new AXMenuWindow(NULL, this);

    pItem = pPop->showPopup(pSend, rootx, rooty, uFlags);

    return (pItem)? pItem->m_uID: -1;
}

//----------------------

//! メニュー表示前の全アイテム初期化

void AXMenu::initAllItem(AXFont *pFont)
{
    AXMenuItem *p;

    for(p = getTopItem(); p; p = p->next())
        p->init(pFont);
}

//! 一番最初の有効アイテム取得

AXMenuItem *AXMenu::getFirstItem()
{
    AXMenuItem *p;

    for(p = getTopItem(); p && p->isDisableItem(); p = p->next());

    return p;
}

//! ホットキーからアイテム検索

AXMenuItem *AXMenu::findHotKey(char cKey)
{
    AXMenuItem *p;

    if(cKey >= 'a' && cKey <= 'z') cKey -= 0x20;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_cHotKey == cKey && !p->isDisableItem())
            return p;
    }

    return NULL;
}



//***********************************
// AXMenuItem
//***********************************

/*!
    @class AXMenuItem
    @brief メニューアイテム

    - アイテム削除時は、サブメニューも含めて削除される。

    @ingroup widgetdat
*/
/*!
    @var AXMenuItem::m_uFlags
    @brief フラグ。 AXMenu::MENUITEMFLAGS
    @var AXMenuItem::m_nItemH
    @brief アイテムの高さ
*/

/*
    m_nAccTextPos   : テキストのアクセラレータ部分の開始位置（'\t'の位置）
    m_nTextW        : アクセラレータ部分を除くテキストの幅
    m_nAccW         : アクセラレータ部分のテキストの幅
    m_nHotKeyPos    : ホットキーのテキスト位置
    m_wHotKeyX      : 描画時のホットキーX位置
    m_wHotkeyW      : 描画時のホットキー文字幅
    m_cHotKey       : ホットキー文字（アルファベットは大文字）。0でなし。
*/


AXMenuItem::~AXMenuItem()
{
    if(m_pSubMenu)
        delete m_pSubMenu;
}

//! 作成
//! @param pstr NULLで空文字

AXMenuItem::AXMenuItem(UINT uID,UINT uFlags,LPCUSTR pstr,AXMenu *pSubMenu,ULONG lParam)
{
    m_uID       = uID;
    m_uFlags    = uFlags;
    m_pSubMenu  = pSubMenu;
    m_lParam    = lParam;
    m_strText   = pstr;

    //ホットキー(&)

    m_nHotKeyPos = hotKey();

    if(m_nHotKeyPos == -1)
    {
        m_wHotKeyX = m_wHotKeyW = 0;
        m_cHotKey = 0;
    }
    else
    {
        m_cHotKey = (char)m_strText[m_nHotKeyPos];

        if(m_cHotKey >= 'a' && m_cHotKey <= 'z')
            m_cHotKey -= 0x20;
    }
}

//! 無効なアイテムか（無効またはセパレータ）

BOOL AXMenuItem::isDisableItem()
{
    return (m_uFlags & (AXMenu::MIF_DISABLE | AXMenu::MIF_SEP));
}

//! サブメニューが有効か

BOOL AXMenuItem::isEnableSubMenu()
{
    return (!(m_uFlags & AXMenu::MIF_DISABLE) && m_pSubMenu && m_pSubMenu->getCnt());
}

//! 初期化

void AXMenuItem::init(AXFont *pFont)
{
    int pos;

    if(m_uFlags & AXMenu::MIF_OWNERDRAW) return;

    if(m_uFlags & AXMenu::MIF_SEP)
    {
        //セパレータ

        m_nTextW = 1;
        m_nAccW  = 0;
        m_nItemH = ITEMSEP_H;
    }
    else
    {
        //-------- テキスト項目

        //ホットキー(&)

        if(m_nHotKeyPos != -1)
        {
            m_wHotKeyX  = pFont->getTextWidth(m_strText, m_nHotKeyPos);
            m_wHotKeyW  = pFont->getTextWidth(m_strText.at(m_nHotKeyPos), 1);
        }

        //

        pos = m_strText.find('\t');

        if(pos == -1)
        {
            m_nTextW        = pFont->getTextWidth(m_strText);
            m_nAccW         = 0;
            m_nAccTextPos   = m_strText.getLen();
        }
        else
        {
            m_nTextW        = pFont->getTextWidth(m_strText, pos);
            m_nAccW         = pFont->getTextWidth(m_strText.at(pos + 1), m_strText.getLen() - pos - 1);
            m_nAccTextPos   = pos;
        }

        m_nItemH = pFont->getHeight();
        if(m_nItemH < 9) m_nItemH = 9;
        m_nItemH += ITEMSPACE_Y * 2;
    }
}

//! ホットキーの位置取得＆ホットキー文字削除

int AXMenuItem::hotKey()
{
    LPUSTR p;
    AXString str(m_strText.getLen());
    int pos = -1;

    for(p = m_strText; *p; p++)
    {
        if(*p == '&' && p[1] > 32 && p[1] < 127)
        {
            //"&&" は '&'

            if(p[1] != '&' && pos == -1)
                pos = str.getLen();

            str += p[1];
            p++;
        }
        else
            str += *p;
    }

    m_strText = str;

    return pos;
}

//! ラジオグループのチェック
/*!
    グループ内の他のアイテムのチェックをOFFにする。
    @param bCheck このアイテムのチェックをONにするか
*/

void AXMenuItem::checkRadio(BOOL bCheck)
{
    AXMenuItem *p;

    //前

    for(p = prev(); p; p = p->prev())
    {
        if(p->m_uFlags & AXMenu::MIF_SEP) break;

        p->m_uFlags &= ~AXMenu::MIF_CHECK;
    }

    //後ろ

    for(p = next(); p; p = p->next())
    {
        if(p->m_uFlags & AXMenu::MIF_SEP) break;

        p->m_uFlags &= ~AXMenu::MIF_CHECK;
    }

    //チェック

    if(bCheck)
        m_uFlags |= AXMenu::MIF_CHECK;
    else
        m_uFlags &= ~AXMenu::MIF_CHECK;
}
