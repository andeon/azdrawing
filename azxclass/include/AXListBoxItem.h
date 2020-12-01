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

#ifndef _AX_LISTBOXITEM_H
#define _AX_LISTBOXITEM_H

#include "AXList.h"
#include "AXString.h"

class AXListBoxItem:public AXListItem
{
public:
    AXString    m_strText;
    ULONG       m_lParam;

public:
    AXListBoxItem(LPCUSTR pstr,ULONG lParam);

    AXListBoxItem *prev() const { return (AXListBoxItem *)m_pPrev; }
    AXListBoxItem *next() const { return (AXListBoxItem *)m_pNext; }
};

#endif
