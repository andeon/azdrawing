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

#ifndef _AZDRAW_FILTERCONFIGLIST_H_
#define _AZDRAW_FILTERCONFIGLIST_H_

#include "AXList.h"
#include "AXByteString.h"

class AXConfWrite;

class CFilterConfigItem:public AXListItem
{
public:
    UINT    m_uID;
    AXByteString    m_str;

public:
    CFilterConfigItem(UINT id,const AXByteString &str);
    CFilterConfigItem *next() const { return (CFilterConfigItem *)m_pNext; }

    BOOL getVal(char sig,int *pRet);
};

//

class CFilterConfigList:public AXList
{
protected:

public:
    CFilterConfigItem *getTopItem() const { return (CFilterConfigItem *)m_pTop; }

    void setDat(UINT id,const AXByteString &str);
    CFilterConfigItem *findID(UINT id);
    void writeConf(AXConfWrite *pcf);
};

#endif
