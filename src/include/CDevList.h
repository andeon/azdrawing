/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#ifndef _AZDW_DEVLIST_H_
#define _AZDW_DEVLIST_H_

#include "AXList.h"
#include "AXString.h"

class AXMem;

class CDevItem:public AXListItem
{
    friend class CDevList;

public:
    enum
    {
        MASK_BUTTON     = (1<<13) - 1,
        MASK_BUTTONMOD  = 0xffff,
        FLAG_SHIFT      = 1<<13,
        FLAG_CTRL       = 1<<14,
        FLAG_ALT        = 1<<15,
        MASK_MOD        = FLAG_CTRL|FLAG_SHIFT|FLAG_ALT,
        SHIFT_ACTION    = 16,

        PRESS_NONE      = -1,
        BUTTON_MAXCNT   = 32
    };

    enum ACTION
    {
        ACTION_NONE = 0,
        ACTION_MAIN = 1
    };

protected:
    AXString    m_strName;
    int         m_devid,
                m_nBttNum,
                m_nActCnt,
                m_nPressNo;
    double      m_dPressMin,
                m_dPressMax;
    LPDWORD     m_pActDat;

public:
    virtual ~CDevItem();
    CDevItem();

    CDevItem *next() const { return (CDevItem *)m_pNext; }

    const AXString& getName() const { return m_strName; }
    int getDevID() const { return m_devid; }
    int getPressNo() const { return m_nPressNo; }
    int getBttNum() const { return m_nBttNum; }
    LPDWORD getActBuf() const { return m_pActDat; }
    DWORD getActBufSize() const { return m_nActCnt * sizeof(DWORD); }

    void free();
    void allocAction(int cnt);
    void setDefaultAction();
    void setAction(AXMem *pmem,int cnt=-1);
    double getPress(double press);
    void setActEditDat(AXMem *pmem);
};

//

class CDevList:public AXList
{
protected:
    CDevItem    *m_pDefItem;

public:
    CDevList();

    CDevItem *getDefaultItem() const { return m_pDefItem; }
    CDevItem *getTopItem() const { return (CDevItem *)m_pTop; }

    void addDeviceInit(int devid,const AXString &strName,int bttnum,int pressno,double pressmin,double pressmax);
    CDevItem *addDevice(const AXString &strName);

    CDevItem *getDevFromName(const AXString &strName);
    CDevItem *getDevFromID(int devid);

    UINT getButtonAction(int devid,int bttno,BOOL bCtrl,BOOL bShift,BOOL bAlt);
};

#endif
