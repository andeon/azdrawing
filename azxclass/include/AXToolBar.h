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

#ifndef _AX_TOOLBAR_H
#define _AX_TOOLBAR_H

#include "AXWindow.h"
#include "AXString.h"

class AXList;
class AXImageList;
class AXToolTipWin;
class AXToolBarItem;
class AXPixmap;

class AXToolBar:public AXWindow
{
public:
    enum TOOLBATSTYLE
    {
        TBS_FRAME       = WS_EXTRA,
        TBS_TOOLTIP     = WS_EXTRA << 1,
        TBS_SEPBOTTOM   = WS_EXTRA << 2,
        TBS_SPACEBOTTOM = WS_EXTRA << 3,
        TBS_TOOLTIP_TR  = WS_EXTRA << 4
    };

    enum TOOLBARBUTTONFLAG
    {
        BF_BUTTON       = 0,
        BF_CHECKBUTTON  = 0x0001,
        BF_CHECKGROUP   = 0x0002,
        BF_SEP          = 0x0004,
        BF_DROPDOWN     = 0x0008,
        BF_WRAP         = 0x0010,
        BF_DISABLE      = 0x0020,
        BF_CHECKED      = 0x0040
    };

protected:
    AXList      *m_pDat;
    AXImageList *m_pImgList;
    AXPixmap    *m_pImg;
    AXString    m_strToolTip;

    AXToolTipWin    *m_pToolTip;
    AXToolBarItem   *m_pOnItem;

    int     m_fDown,
            m_nIconW,m_nIconH,
            m_trGroupID;

protected:
    void _createToolBar();
    void _draw();
    int _getMaxWidth(LPINT pLineCnt);
    AXToolBarItem *_getCurItem(int x,int y);
    void _cursorOnBtt(int x,int y);
    void _showToolTip(AXToolBarItem *p);
    void _cancelToolTip();
    void _checkGroup(AXToolBarItem *pSel);
    AXToolBarItem *_getItem(UINT uID);

public:
    virtual ~AXToolBar();

    AXToolBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXToolBar(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    void setImageList(AXImageList *pImg);
    void setToolTipTrGroup(WORD wGroupID);
    void setToolTipText(LPCUSTR pText);
    void setToolTipTextTr(const LPWORD pStrIDArray,int cnt,WORD wMask=0xffff);
    void setToolTipTextTr(int nStartID,int cnt);

    void addItem(UINT uID,int nImgNo,int nToolTipNo,UINT uFlags,ULONG lParam);
    void addItemSep();
    void checkItem(UINT uID,BOOL bCheck);
    BOOL isCheckItem(UINT uID);
    void enableItem(UINT uID,BOOL bEnable);
    void enableItemNo(int no,BOOL bEnable);
    void getItemRect(UINT uID,AXRect *prc,BOOL bRoot);

    virtual void calcDefSize();
    virtual void reconfig();

    virtual BOOL onSize();
    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
    virtual BOOL onEnter(AXHD_ENTERLEAVE *phd);
    virtual BOOL onLeave(AXHD_ENTERLEAVE *phd);
};

#endif
