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

#ifndef _AX_TOPWINDOW_H
#define _AX_TOPWINDOW_H

#include "AXWindow.h"

class AXString;
class AXAppIC;
class AXAccelerator;
class AXMenuBar;


class AXTopWindow:public AXWindow
{
    friend class AXWindow;
    friend class AXButton;

protected:
    AXWindow        *m_pFocus,
                    *m_pDefButton;
    AXAccelerator   *m_pAccel;
    AXMenuBar       *m_pMenuBar;

#ifndef _AX_OPT_IM
    AXAppIC     *m_pIC;
#endif

protected:
    void _setTitle(LPCSTR pBuf,int len);
    void _changeFocusWindow(AXWindow *p,int detail);
    BOOL _isFocusProcKey(UINT keysym,UINT *pType);
    void _setDecoration();
    void _setDefButtonNone() { m_pDefButton = NULL; }

public:
    virtual ~AXTopWindow();
    AXTopWindow(AXWindow *pOwner,UINT uStyle);

    AXWindow *getDefaultButton() const { return m_pDefButton; }
    AXAccelerator *getAccel() const { return m_pAccel; }
    AXMenuBar *getMenuBar() const { return m_pMenuBar; }

    void attachAccel(AXAccelerator *pAccel) { m_pAccel = pAccel; }
    void attachMenuBar(AXMenuBar *pMenuBar) { m_pMenuBar = pMenuBar; }

    void setTitle(const AXString &str);
    void setTitle(LPCUSTR pText);
    void setTitle(LPCSTR pText);
    void setTitle(WORD wStrID);
    void setAppIcon(const void *pBuf,BOOL bClear=FALSE);
    void enableDND();
    void changeDecoration(UINT style,BOOL bAdd);
    void getFrameWidth(AXRect *prc);
    void getWindowSize(AXSize *psize);
    void getTopWindowRect(AXRectSize *prcs);
    void getTopWindowPos(AXPoint *ppt);
    void setDefaultButton(AXWindow *pbtt);
    int getKeyString(LPVOID pEvent,AXString *pStr,UINT *pKeySym);

    virtual void getICPos(AXPoint *pPos);
    virtual void layout();

    //

    void showRestore();
    void showInit(const AXRectSize &rcs,const AXRectSize &rcsDef,int initVal,BOOL bShow);
    void showInit(const AXPoint &pt,int defx,int defy,int initVal,BOOL bShow);
    void moveCenter(AXWindow *pwin=NULL);
    void moveInRoot(int x,int y);

    virtual void show();
    virtual void hide();
    virtual void move(int x,int y);
    virtual BOOL resize(int w,int h);
    virtual BOOL moveresize(int x,int y,int w,int h);

    BOOL isMinimized();
    BOOL isMaximized();
    BOOL minimize();
    BOOL maximize();
    BOOL restore();

    //

    AXWindow *getDefaultFocus();
    AXWindow *getFirstTakeFocus();
    BOOL moveNextFocus();

    void adjustPosInRoot(LPINT px,LPINT py);
    void errMes(LPCUSTR pText);
    void errMes(WORD wStrGroupID,WORD wStrID);

    //

    virtual BOOL onConfigure(AXHD_CONFIGURE *phd);
    virtual BOOL onFocusIn(int detail);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onKeyUp(AXHD_KEY *phd);
};

#endif
