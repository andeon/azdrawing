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

#ifndef _AX_LAYOUT_H
#define _AX_LAYOUT_H

#include "AXLayoutItem.h"

class AXLayout:public AXLayoutItem
{
protected:
    AXLayoutItem    *m_pTop,
                    *m_pBottom;
    int     m_nSepW,
            m_nSpaceW;
    UINT    m_uFlags;

protected:
    enum FLAGS
    {
        FLAG_VISIBLE = 1
    };

public:
    virtual ~AXLayout();
    AXLayout();
    AXLayout(UINT uLayoutFlags,int sepw);

    AXLayoutItem *getTopItem() const { return m_pTop; }

    void setSepWidth(int w) { m_nSepW = w; }
    void setMatrixColumn(int cnt) { m_nSepW = cnt; }
    void setSpacing(int w) { m_nSpaceW = w; }

    void addItem(AXLayoutItem *p);
    void insertItem(AXLayoutItem *p,AXLayoutItem *pIns);
    void removeItem(AXLayoutItem *p);
    void removeItemAll();
    void replaceItem(AXLayoutItem *pSrc,AXLayoutItem *pReplace);

    virtual BOOL isVisible();
    virtual void getClientRect(AXRect *prc);
    virtual BOOL moveresize(int x,int y,int w,int h);
    virtual void show();
    virtual void hide();
};

//-------------------

class AXLayoutVert:public AXLayout
{
public:
    virtual ~AXLayoutVert();
    AXLayoutVert();
    AXLayoutVert(UINT uLayoutFlags,int sepw);

    virtual void layout();
    virtual void calcDefSize();
};

//-------------------

class AXLayoutHorz:public AXLayout
{
public:
    virtual ~AXLayoutHorz();
    AXLayoutHorz();
    AXLayoutHorz(UINT uLayoutFlags,int sepw);

    virtual void layout();
    virtual void calcDefSize();
};

//-------------------

class AXLayoutMatrix:public AXLayout
{
protected:
    AXLayoutItem *getRowNext(AXLayoutItem *p);
    int getColInfo(int width,LPINT pInfo);
    void getRowInfo(int height,LPINT pInfo);

public:
    virtual ~AXLayoutMatrix();
    AXLayoutMatrix(int colcnt);
    AXLayoutMatrix(int colcnt,UINT uLayoutFlags);

    virtual void layout();
    virtual void calcDefSize();
};

#endif
