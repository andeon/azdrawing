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

#ifndef _AX_TABITEM_H
#define _AX_TABITEM_H

#include "AXList.h"
#include "AXString.h"

class AXTabItem:public AXListItem
{
public:
    AXString    m_strText;
    int         m_nIconNo,
                m_nWidth,
                m_nTabX;
    ULONG       m_lParam;

public:
    AXTabItem(LPCUSTR pText,int iconno,ULONG lParam,int width);

    AXTabItem *prev() const { return (AXTabItem *)m_pPrev; }
    AXTabItem *next() const { return (AXTabItem *)m_pNext; }
};

#endif
