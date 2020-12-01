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

#ifndef _AX_WINDOW_H
#define _AX_WINDOW_H

#include "AXDrawable.h"
#include "AXLayoutItem.h"
#include "AXRect.h"
#include "AXHD.h"

class AXFont;
class AXLayout;
class AXMenu;
class AXMenuBar;
class AXDND;
class AXCursor;

class AXWindow:public AXDrawable,public AXLayoutItem
{
    friend class AXApp;
    friend class AXAppWinHash;
    friend class AXTopWindow;
    friend class AXAppIC;

protected:

    AXWindow    *m_pParent,
                *m_pOwner,
                *m_pFirst,
                *m_pLast,
                *m_pPrev,
                *m_pNext,
                *m_pTopLevel,
                *m_pNotify,
                *m_pHashPrev,
                *m_pHashNext,
                *m_pDrawPrev,
                *m_pDrawNext,
                *m_pReConfPrev,
                *m_pReConfNext;

    AXLayout    *m_pLayout;

    UINT        m_uStyle,
                m_uFlags,
                m_uItemID;
    ULONG       m_lEventMask,
                m_idUserTimeWin;

    AXRect      m_rcDraw;

    AXFont      *m_pFont;
    AXCursor    *m_pCursor;

protected:

    enum FLAGS
    {
        FLAG_EXIST          = 0x00000001,
        FLAG_VISIBLE        = 0x00000002,
        FLAG_ENABLED        = 0x00000004,
        FLAG_TAKE_FOCUS     = 0x00000008,
        FLAG_DEFAULT_FOCUS  = 0x00000010,
        FLAG_FOCUSED        = 0x00000020,
        FLAG_CALCULATED     = 0x00000040,
        FLAG_LAYOUTED       = 0x00000080,
        FLAG_CURSOR_NEW     = 0x00000100,
        FLAG_REDRAW         = 0x00000200,
        FLAG_CHILD_NOTIFY_PARENT = 0x00000400,
        FLAG_WHEELEVENT_NORMAL   = 0x00000800,
        FLAG_TOPLEVEL            = 0x00001000,
        FLAG_FOCUS_DISABLE_KEY   = 0x00002000,

        FLAG_TEMP4          = 0x10000000,
        FLAG_TEMP3          = 0x20000000,
        FLAG_TEMP2          = 0x40000000,
        FLAG_TEMP1          = 0x80000000
    };

public:

    enum WINDOWSTYLE
    {
        WS_HIDE             = 0x00000001,
        WS_DISABLE          = 0x00000002,
        WS_DROP             = 0x00000004,
        WS_BK_FACE          = 0x00000008,

        WS_EXTRA            = 0x00000010,

        WS_DISABLE_WM       = WS_EXTRA,
        WS_TITLE            = WS_EXTRA << 1,
        WS_MINIMIZE         = WS_EXTRA << 2,
        WS_MAXIMIZE         = WS_EXTRA << 3,
        WS_CLOSE            = WS_EXTRA << 4,
        WS_BORDER           = WS_EXTRA << 5,
        WS_MENUBTT          = WS_EXTRA << 6,
        WS_DISABLE_IM       = WS_EXTRA << 7,
        WS_TABMOVE          = WS_EXTRA << 8,
        WS_TRANSIENT_FOR    = WS_EXTRA << 9,
        WS_DISABLE_RESIZE   = WS_EXTRA << 10,
        WS_NOT_TAKEFOCUS    = WS_EXTRA << 11,

        WS_TOP_NORMAL       = WS_TITLE | WS_MINIMIZE | WS_MAXIMIZE | WS_CLOSE | WS_BORDER | WS_MENUBTT,
        WS_DIALOG_NORMAL    = WS_TOP_NORMAL | WS_HIDE | WS_TABMOVE | WS_TRANSIENT_FOR
    };

    enum MOUSEBUTTON
    {
        BUTTON_LEFT   = 1,
        BUTTON_MIDDLE = 2,
        BUTTON_RIGHT  = 3
    };

    enum BUTTONSTATE
    {
        STATE_SHIFT     = 1,
        STATE_CTRL      = 2,
        STATE_ALT       = 4,
        STATE_LBUTTON   = 8,
        STATE_MBUTTON   = 16,
        STATE_RBUTTON   = 32,

        STATE_MODMASK   = STATE_CTRL | STATE_SHIFT | STATE_ALT
    };

    enum EVENTTYPE
    {
        EVENT_BUTTONDOWN = 1,
        EVENT_BUTTONUP   = 2,
        EVENT_MOUSEMOVE  = 3
    };

    enum EVENTDETAIL
    {
        EVENTDETAIL_ANCESTOR           = 0,
        EVENTDETAIL_VIRTUAL            = 1,
        EVENTDETAIL_INFERIOR           = 2,
        EVENTDETAIL_NONELINER          = 3,
        EVENTDETAIL_NONELINERVIRTUAL   = 4,
        FOCUSDETAIL_ACTIVE      = -1,
        FOCUSDETAIL_TABMOVE     = -2,
        FOCUSDETAIL_MANUAL      = -3
    };

    enum OWNERDRAWSTATE
    {
        ODS_SELECTED    = 1,
        ODS_FOCUSED     = 2,
        ODS_ENABLED     = 4
    };

    enum COMMANDFROM
    {
        COMFROM_NONE,
        COMFROM_MENU,
        COMFROM_ACCEL,
        COMFROM_TOOLBAR,
        COMFROM_TOOLBAR_DROP
    };

    enum SEARCHTYPE
    {
        SEARCHTYPE_FLAGS,
        SEARCHTYPE_ITEMID
    };

    enum ACCEPTKEYTYPE
    {
        ACCEPTKEYTYPE_TAB,
        ACCEPTKEYTYPE_ENTER,
        ACCEPTKEYTYPE_ESCAPE
    };

protected:

    void _init();
    void _createwin(AXWindow *pParent,AXWindow *pOwner,UINT uStyle);
    void _setUserTimeWindow();
    void _deleteSub();

    AXWindow *_getTopLevel();
    void _setFocus(int detail);
    void _killFocus(int detail);
    void _removeFocusMe();
    void _updateUserTime(ULONG time);
    virtual BOOL isAcceptKey(UINT keytype);
    virtual void getICPos(AXPoint *pPos);

public:

    virtual ~AXWindow();

    AXWindow();
    AXWindow(AXWindow *pOwner,UINT uStyle);
    AXWindow(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXWindow(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    //

    UINT getItemID() const { return m_uItemID; }
    UINT getStyle() const { return m_uStyle; }

    AXWindow *getParent() const { return m_pParent; }
    AXWindow *getTopLevel() const { return m_pTopLevel; }
    AXLayout *getLayout() const { return m_pLayout; }
    AXFont *getFont() const { return m_pFont; }
    AXCursor *getCursor() const { return m_pCursor; }
    virtual AXWindow *getNotify();

    void setChildNotifyParent() { m_uFlags |= FLAG_CHILD_NOTIFY_PARENT; }
    void setFlag_focusDisableKey() { m_uFlags |= FLAG_FOCUS_DISABLE_KEY; }
    void setNotify(AXWindow *pwin) { m_pNotify = pwin; }
    void setFont(AXFont *pFont) { m_pFont = pFont; }

    //

    virtual BOOL isVisible();
    BOOL isVisibleReal();
    BOOL isEnabled();
    BOOL isFocused();
    BOOL isContain(int x,int y);
    BOOL isLayouted() const { return m_uFlags & FLAG_LAYOUTED; }
    BOOL isTopLevel() const { return m_uFlags & FLAG_TOPLEVEL; }

    void setTakeFocus() { m_uFlags |= FLAG_TAKE_FOCUS; }

    void getWindowRect(AXRectSize *prcs);
    AXWindow *searchChild(SEARCHTYPE type,ULONG data);
    AXWindow *searchTree(SEARCHTYPE type,ULONG data,AXWindow *pTop=NULL);

    //

    void setEventMask(ULONG mask,BOOL bAdd=TRUE);
    void selectInput(ULONG mask);
    void selectInputExposure();
    void setWindowType(ULONG atom);
    void setBackgroundFaceLight();

    //

    void redraw();
    void redraw(const AXRectSize &rcs);
    void redraw(int x,int y,int w,int h);
    void redrawUpdate();

    void setReconfig();

    //

    void setLayout(AXLayout *p);
    virtual void layout();
    virtual void calcDefSize();
    void removeFromLayout();
    void relayoutAndResize();

    //

    void show(BOOL bShow);
    void enable(BOOL bEnable);

    virtual void show();
    virtual void hide();
    virtual void enable();
    virtual void disable();

    void raise();
    void lower();
    void above(BOOL bSet);
    void setActive();
    void setSkipTaskbar();

    virtual void move(int x,int y);
    virtual BOOL resize(int w,int h);
    virtual BOOL moveresize(int x,int y,int w,int h);
    void resizeDefSize();
    void resizeSwitch(int w,int h,int defw,int defh);

    //

    void changeStyle(UINT style,BOOL bAdd);

    void setFocus();
    void setXFocus();
    void killXFocus();

    void setCursor(AXCursor *pCursor);
    void setCursor(int type);
    void setCursor(const unsigned char *pDat);
    void unsetCursor();

    void getCursorPos(AXPoint *ppt);
    BOOL isCursorIn();
    void translateTo(AXWindow *pwinTo,LPINT px,LPINT py);
    void translateFrom(AXWindow *pwinFrom,LPINT px,LPINT py);
    void translateRectTo(AXWindow *pwinTo,AXRect *prc);

    BOOL grabPointer();
    BOOL grabPointer(AXCursor *pCursor);
    void ungrabPointer();
    void changeGrabCursor(AXCursor *pCursor,BOOL bEvent);

    void addTimer(UINT uTimerID,UINT uInterval,ULONG lParam=0);
    void delTimer(UINT uTimerID);
    void delTimerAll();
    BOOL isTimerExist(UINT uTimerID);
    BOOL isTimerExistWin();

    AXWindow *getWidget(UINT uID);
    void enableWidget(UINT uID,BOOL bEnable);

    void sendNotify(AXWindow *pwinFrom,UINT uNotify,ULONG lParam);
    void sendCommand(UINT uID,ULONG lParam,int from);

    //

    virtual void reconfig();

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onOwnerDraw(AXDrawable *pdraw,AXHD_OWNERDRAW *phd);
    virtual BOOL onClose();
    virtual BOOL onMove();
    virtual BOOL onSize();
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
    virtual BOOL onMenuPopup(AXMenuBar *pMenuBar,AXMenu *pMenu,UINT uID);
    virtual BOOL onDND_Drop(AXDND *pDND);

    virtual BOOL onMap();
    virtual BOOL onFocusIn(int detail);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onConfigure(AXHD_CONFIGURE *phd);
    virtual BOOL onEnter(AXHD_ENTERLEAVE *phd);
    virtual BOOL onLeave(AXHD_ENTERLEAVE *phd);
    virtual BOOL onUngrab(AXHD_ENTERLEAVE *phd);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onKeyUp(AXHD_KEY *phd);

    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
    virtual BOOL onMouseMove(AXHD_MOUSE *phd);
    virtual BOOL onDblClk(AXHD_MOUSE *phd);
    virtual BOOL onMouseWheel(AXHD_MOUSE *phd,BOOL bUp);
    virtual BOOL onMouseInSkip(AXHD_MOUSE *phd);
};

#endif
