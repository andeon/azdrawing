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

#ifndef _AX_MENUITEM_H
#define _AX_MENUITEM_H

#include "AXList.h"
#include "AXString.h"

class AXFont;
class AXMenu;
class AXMenuWindow;

class AXMenuItem:public AXListItem
{
    friend class AXMenuBar;
    friend class AXMenu;
    friend class AXMenuWindow;

public:
    UINT        m_uID,
                m_uFlags;
    ULONG       m_lParam;
    AXString    m_strText;
    AXMenu      *m_pSubMenu;

    int         m_nTextW,
                m_nAccW,
                m_nAccTextPos,
                m_nItemH,
                m_nHotKeyPos;
    WORD        m_wHotKeyX,
                m_wHotKeyW;
    char        m_cHotKey;

protected:
    void init(AXFont *pFont);
    void checkRadio(BOOL bCheck=TRUE);
    int hotKey();

public:
    virtual ~AXMenuItem();
    AXMenuItem(UINT uID,UINT uFlags,LPCUSTR pstr,AXMenu *pSubMenu,ULONG lParam);

    AXMenuItem *prev() const { return (AXMenuItem *)m_pPrev; }
    AXMenuItem *next() const { return (AXMenuItem *)m_pNext; }

    BOOL isDisableItem();
    BOOL isEnableSubMenu();
};

#endif
