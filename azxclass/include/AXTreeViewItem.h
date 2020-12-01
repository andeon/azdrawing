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

#ifndef _AX_TREEVIEWITEM_H
#define _AX_TREEVIEWITEM_H

#include "AXTree.h"
#include "AXString.h"

class AXTreeViewItem:public AXTreeItem
{
public:
    enum FLAGS
    {
        FLAG_CHECKBOX   = 0x0001,
        FLAG_EXPAND     = 0x0002,
        FLAG_CHECKED    = 0x0004,
        FLAG_GRAY       = 0x0008,
        FLAG_TEMP1      = 0x80000000
    };

public:
    AXString    m_strText;
    int         m_nIconNo;
    UINT        m_uFlags;
    ULONG       m_lParam;

    int         m_nW,m_nX;

public:
    AXTreeViewItem(LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam);

    AXTreeViewItem *parent() const { return (AXTreeViewItem *)m_pParent; }
    AXTreeViewItem *first() const { return (AXTreeViewItem *)m_pFirst; }
    AXTreeViewItem *last() const { return (AXTreeViewItem *)m_pLast; }
    AXTreeViewItem *prev() const { return (AXTreeViewItem *)m_pPrev; }
    AXTreeViewItem *next() const { return (AXTreeViewItem *)m_pNext; }

    BOOL isExpand() { return m_uFlags & FLAG_EXPAND; }
    BOOL isCheckBox() { return m_uFlags & FLAG_CHECKBOX; }
    BOOL isChecked() { return m_uFlags & FLAG_CHECKED; }
    BOOL isGray() { return m_uFlags & FLAG_GRAY; }

    void check() { m_uFlags |= FLAG_CHECKED; }
    void uncheck() { m_uFlags &= ~FLAG_CHECKED; }
    void expand() { m_uFlags |= FLAG_EXPAND; }

    AXTreeViewItem *nextVisible();
    AXTreeViewItem *prevVisible();
};

#endif
