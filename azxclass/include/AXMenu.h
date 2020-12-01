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

#ifndef _AX_MENU_H
#define _AX_MENU_H

#include "AXDef.h"

class AXList;
class AXFont;
class AXMenuWindow;
class AXMenuItem;
class AXString;
class AXWindow;

class AXMenu
{
    friend class AXMenuBar;
    friend class AXMenuWindow;

public:
    enum MENUITEMFLAGS
    {
        MIF_SEP         = 0x0001,
        MIF_OWNERDRAW   = 0x0002,
        MIF_DISABLE     = 0x0004,
        MIF_CHECK       = 0x0008,
        MIF_RADIO       = 0x0010,
        MIF_AUTOCHECK   = 0x0020
    };

    enum POPUPFLAGS
    {
        POPF_RIGHT      = 1,
        POPF_NOSEND     = 2
    };

protected:
    AXList          *m_pList;
    AXMenuWindow    *m_pwinPopup;

protected:
    void initAllItem(AXFont *pFont);
    AXMenuItem *getFirstItem();
    AXMenuItem *findHotKey(char cKey);

public:
    ~AXMenu();
    AXMenu();

    AXMenuItem *getTopItem() const;
    int getCnt() const;

    void add(UINT uID,UINT uFlags,LPCUSTR pstr,AXMenu *pSubMenu,ULONG lParam);
    void add(UINT uID,UINT uFlags,LPCUSTR pstr);
    void add(UINT uID,LPCUSTR pstr,AXMenu *pSubMenu);
    void add(UINT uID,LPCUSTR pstr);
    void addSep();
    void addCheck(UINT uID,LPCUSTR pstr,BOOL bCheck);
    void addRadio(UINT uID,LPCUSTR pstr);
    void addOD(UINT uID,LPCUSTR pstr,ULONG lParam,int nItemW,int nItemH);

    void addTr(WORD wStrID,UINT uFlags,AXMenu *pSubMenu,ULONG lParam);
    void addTr(WORD wStrID);
    void addTr(WORD wStrID,UINT uFlags);
    void addTr(WORD wStrID,AXMenu *pSubMenu);
    void addTrCheck(WORD wStrID,BOOL bCheck);
    void addTrRadio(WORD wStrID);

    void addTrMul(UINT uStartID,int cnt);
    void addTrArray(const LPWORD pIDArray,int cnt);
    void setStrArray(UINT uIDStart,AXString *pstr,int cnt);

    void deleteAll();
    void del(UINT uID);
    void delPos(int pos);

    ULONG getParam(UINT uID);

    void enable(UINT uID,BOOL bEnable);
    void check(UINT uID,BOOL bCheck);
    void setSubMenu(UINT uID,AXMenu *pSubMenu);

    AXMenuItem *getItem(UINT uID) const;
    AXMenuItem *getItemAll(UINT uID,AXMenu **ppOwnerMenu=NULL);
    AXMenuItem *getLastItem();

    int popup(AXWindow *pSend,int rootx,int rooty,UINT uFlags);
};

#endif
