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

#ifndef _AX_LAYOUTITEM_H
#define _AX_LAYOUTITEM_H

#include "AXDef.h"

class AXRect;
class AXRectSize;

class AXLayoutItem
{
    friend class AXWindow;
    friend class AXLayout;
    friend class AXLayoutVert;
    friend class AXLayoutHorz;
    friend class AXLayoutMatrix;

protected:
    AXLayoutItem    *m_pLParent,
                    *m_pLNext,
                    *m_pLPrev;
    int     m_nX, m_nY,
            m_nW, m_nH;
    int     m_nDefW,
            m_nDefH,
            m_nMinW,
            m_nMinH;
    int     m_padLeft,
            m_padTop,
            m_padRight,
            m_padBottom;
    UINT    m_uLFlags,
            m_uType;

public:
    virtual ~AXLayoutItem();
    AXLayoutItem();

    AXLayoutItem *getLayoutParent() const { return m_pLParent; }
    AXLayoutItem *getLayoutPrev() const { return m_pLPrev; }
    AXLayoutItem *getLayoutNext() const { return m_pLNext; }

    int getX() const { return m_nX; }
    int getY() const { return m_nY; }
    int getWidth() const { return m_nW; }
    int getHeight() const { return m_nH; }
    int getDefW() const { return m_nDefW; }
    int getDefH() const { return m_nDefH; }
    int getMinW() const { return m_nMinW; }
    int getMinH() const { return m_nMinH; }
    UINT getType() const { return m_uType; }
    UINT getLayoutFlags() const { return m_uLFlags; }

    int getLayoutW();
    int getLayoutH();
    int getWidthWithPadding();
    int getHeightWithPadding();
    int getLayoutMinW();
    int getLayoutMinH();

    void setLayoutParent(AXLayoutItem *p) { m_pLParent = p; }
    void setLayoutFlags(UINT uFlags) { m_uLFlags = uFlags; }
    void setMinWidth(int w) { m_nMinW = w; }
    void setMinHeight(int h) { m_nMinH = h; }
    void setMinSize(int w,int h);
    void setPadding(int width);
    void setPadding(int left,int top,int right,int bottom);
    void setPaddingLeft(int left) { m_padLeft = left; }
    void setPaddingTop(int top) { m_padTop = top; }
    void setPaddingRight(int right) { m_padRight = right; }
    void setPaddingBottom(int bottom) { m_padBottom = bottom; }

    void getPadding(AXRect *prc);
    void getRectSize(AXRectSize *prcs);
    void getRect(AXRect *prc);
    virtual void getClientRect(AXRect *prc);

    virtual void layout();
    virtual void calcDefSize();

    virtual BOOL isVisible();
    virtual void show();
    virtual void hide();
    virtual BOOL moveresize(int x,int y,int w,int h);
    void moveresize(const AXRect &rc);

public:
    enum LAYOUTFLAGS
    {
        LF_CENTER_X     = 0x00000001,
        LF_RIGHT        = 0x00000002,
        LF_CENTER_Y     = 0x00000004,
        LF_BOTTOM       = 0x00000008,
        LF_EXPAND_X     = 0x00000010,
        LF_EXPAND_Y     = 0x00000020,
        LF_EXPAND_W     = 0x00000040,
        LF_EXPAND_H     = 0x00000080,
        LF_FIX_W        = 0x00000100,
        LF_FIX_H        = 0x00000200,
        LF_COL_W        = 0x00000400,
        LF_ROW_H        = 0x00000800,

        LF_EXPAND_XY    = LF_EXPAND_X | LF_EXPAND_Y,
        LF_EXPAND_WH    = LF_EXPAND_W | LF_EXPAND_H,
        LF_CENTER_XY    = LF_CENTER_X | LF_CENTER_Y,
        LF_FIX_WH       = LF_FIX_W | LF_FIX_H
    };

    enum TYPE
    {
        TYPE_NONE,
        TYPE_ROOT,
        TYPE_TOPLEVEL,
        TYPE_LAYOUT,
        TYPE_SPACER,
        TYPE_EDGEBOX,
        TYPE_SCROLLVIEW,
        TYPE_GROUPBOX,
        TYPE_LABEL,
        TYPE_BUTTON,
        TYPE_CHECKBUTTON,
        TYPE_SCROLLBAR,
        TYPE_LISTBOX,
        TYPE_COMBOBOX,
        TYPE_PROGRESSBAR,
        TYPE_SLIDERBAR,
        TYPE_MENUBAR,
        TYPE_LINEEDIT,
        TYPE_MULTIEDIT,
        TYPE_HSVPICKER,
        TYPE_COLORPREV,
        TYPE_COLORBUTTON,
        TYPE_HEADER,
        TYPE_LISTVIEW,
        TYPE_FILELISTVIEW,
        TYPE_TAB,
        TYPE_TOOLBAR,
        TYPE_TREEVIEW,
        TYPE_SPLITTER
    };
};

#endif
