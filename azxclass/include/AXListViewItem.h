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

#ifndef _AX_LISTVIEWITEM_H
#define _AX_LISTVIEWITEM_H

#include "AXList.h"
#include "AXString.h"

class AXListViewItem:public AXListItem
{
public:
    enum FLAGS
    {
        FLAG_SELECTED   = 1,
        FLAG_CHECKED    = 2
    };

public:
    AXString    m_strText;
    UINT        m_uFlags;
    int         m_nIconNo;
    ULONG       m_lParam;

public:
    AXListViewItem(LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam);

    AXListViewItem *prev() const { return (AXListViewItem *)m_pPrev; }
    AXListViewItem *next() const { return (AXListViewItem *)m_pNext; }

    BOOL isSelect() { return m_uFlags & FLAG_SELECTED; }
    BOOL isCheck() { return m_uFlags & FLAG_CHECKED; }
    void select() { m_uFlags |= FLAG_SELECTED; }
    void unselect() { m_uFlags &= ~FLAG_SELECTED; }
    void check() { m_uFlags |= FLAG_CHECKED; }
    void uncheck() { m_uFlags &= ~FLAG_CHECKED; }
};

#endif
