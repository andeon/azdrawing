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

#ifndef _AX_MENUWINDOW_H
#define _AX_MENUWINDOW_H

#include "AXTopWindow.h"

class AXMenuBar;
class AXMenu;
class AXMenuItem;

class AXMenuWindow:public AXTopWindow
{
    friend class AXMenuBar;

public:
    enum MENUFLAGS
    {
        FLAG_SCROLL     = FLAG_TEMP1,
        FLAG_SCROLLDOWN = FLAG_TEMP2,
        FLAG_BARCHANGE  = FLAG_TEMP3,
        FLAG_RESIZED    = FLAG_TEMP4
    };

protected:
    AXMenu          *m_pDat;
    AXMenuItem      *m_pSelItem;

    AXMenuBar       *m_pMenuBar;
    AXMenuWindow    *m_pTopPopup,
                    *m_pParentPopup,
                    *m_pCurPopup,
                    *m_pScrPopup;
    int             m_nMaxH,
                    m_nTextMaxW,
                    m_nScrY;
    ULONG           m_lReturn;

protected:
    AXMenuItem *showFromMenuBar(AXMenuBar *pMenuBar,int rootx,int rooty,UINT uParentID);

    void showTopPopup(AXWindow *pNotify,int rootx,int rooty,UINT uFlags);
    void showSubMenu(AXMenuWindow *pParent,int rootx,int rooty);
    void endPopup(ULONG ret);
    void _createSubMenu();
    void _hideSubMenu();
    void showSubMenuFromTimer();
    void _setWindowSize();

    AXMenuWindow *_getCurPosPopup(int rootx,int rooty,AXPoint *pPt);
    AXMenuItem *_getCurPosItem(int y);
    void _getShowItemPos(AXMenuItem *pItem,AXPoint *pPt);
    BOOL _judgeScroll(int rooty);
    void _scroll();

    void _enterItem(AXMenuItem *p);
    void _changeSelItem(AXMenuItem *pNewItem,BOOL bScrItem=FALSE);
    void _keyDown(UINT key);
    void _moveUpDownSel(BOOL bUp);

public:
    virtual ~AXMenuWindow();
    AXMenuWindow(AXWindow *pOwner,AXMenu *pMenu);

    AXMenuItem *showPopup(AXWindow *pSend,int rootx,int rooty,UINT uFlags);

    virtual BOOL onMap();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
};

#endif
