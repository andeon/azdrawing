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

#ifndef _AX_MENUBAR_H
#define _AX_MENUBAR_H

#include "AXWindow.h"

class AXMenu;
class AXMenuItem;
class AXMenuWindow;

class AXMenuBar:public AXWindow
{
    friend class AXMenuWindow;
    friend class AXTopWindow;

public:
    enum MENUBARSTYLE
    {
        MBS_SEP_BOTTOM  = WS_EXTRA
    };

protected:
    AXMenu      *m_pDat;
    AXMenuItem  *m_pSelItem;

protected:
    void _createMenuBar();
    AXMenuItem *_getCurPosItem(int x);
    void _getShowItemPos(AXMenuItem *pItem,AXPoint *ppt);
    void _showSubMenu(AXMenuItem *pItem);
    AXMenuItem *_movePopup(AXMenuWindow *pTopPopup,int x,int y);
    BOOL _showFromHotKey(UINT key,UINT state);

public:
    virtual ~AXMenuBar();

    AXMenuBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXMenuBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    virtual void calcDefSize();

    AXMenu *getMenu() const { return m_pDat; }
    AXMenu *getSubMenu(UINT uID);

    void addItem(UINT uID,UINT uFlags,LPCUSTR pstr,AXMenu *pSubMenu);
    void addItemTr(WORD wStrID,AXMenu *pSubMenu);
    void addItemTrMultiple(const WORD *pIDArray,int cnt,int cmdIDMin);
    void checkItem(UINT uID,BOOL bCheck);
    void setItemSubMenu(UINT uID,AXMenu *pSubMenu);

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
};

#endif
