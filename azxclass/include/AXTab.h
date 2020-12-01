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

#ifndef _AX_TAB_H
#define _AX_TAB_H

#include "AXWindow.h"

class AXList;
class AXTabItem;
class AXImageList;
class AXDrawText;

class AXTab:public AXWindow
{
public:
    enum TABSTYLE
    {
        TABS_TOPTAB     = WS_EXTRA,
        TABS_FIT        = WS_EXTRA << 1,
        TABS_ICONHEIGHT = WS_EXTRA << 2
    };

    enum TABNOTIFY
    {
        TABN_SELCHANGE
    };

protected:
    AXList      *m_pDat;
    AXTabItem   *m_pFocus;
    AXImageList *m_pImgList;
    int         m_nTabH;

protected:
    void _createTab();
    void _calcTabH();
    void _calcTabX();
    AXTabItem *_getCurPosItem(int x,int y);
    void _drawTab(AXTabItem *p,int tcol,DRAWPOINT *pt,AXDrawText *pdt);
    AXTabItem *_getItem(int no);
    void _calcTabWidth(AXTabItem *p);

public:
    virtual ~AXTab();

    AXTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXTab(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    void setImageList(AXImageList *pimg) { m_pImgList = pimg; }

    void addItem(LPCUSTR pText,int nIconNo,ULONG lParam,int width);
    void addItem(LPCUSTR pText);
    void addItem(LPCUSTR pText,int nIconNo,ULONG lParam);
    void addItem(WORD wStrID);
    void delItem(int no);
    void deleteAllItem();
    int getCurSel();
    void setCurSel(int no);
    int getItemCnt();
    ULONG getItemParam(int no);
    void setItemText(int no,LPCUSTR pText);
    void setItemParam(int no,ULONG lParam);

    virtual void getClientRect(AXRect *prc);
    virtual void calcDefSize();
    virtual void reconfig();

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
};

#endif
