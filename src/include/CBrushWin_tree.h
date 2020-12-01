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

#ifndef _AZDRAW_BRUSHWINTREE_H_
#define _AZDRAW_BRUSHWINTREE_H_

#include "AXTreeView.h"

class CBrushTreeItem;

class CBrushWin_tree:public AXTreeView
{
public:
    enum
    {
        NOTIFY_SELCHANGE
    };

protected:
    BOOL    m_bDragFolder;

protected:
    void _notifyChange(AXTreeViewItem *p);

    void _showMenu(AXTreeViewItem *pItem);

    void _addBrush(AXTreeViewItem *pItem);
    void _addFolder(AXTreeViewItem *pItem);
    void _copyItem(CBrushTreeItem *pItem);
    void _delItem(AXTreeViewItem *pItem);
    void _rename(AXTreeViewItem *pItem);
    BOOL _paste(AXTreeViewItem *pItem);
    void _copyCBText(CBrushTreeItem *pItem);

public:
    CBrushWin_tree(AXWindow *pParent);

    virtual AXWindow *getNotify();

    void funcCommand(int id);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
};

#endif
