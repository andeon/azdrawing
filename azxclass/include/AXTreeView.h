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

#ifndef _AX_TREEVIEW_H
#define _AX_TREEVIEW_H

#include "AXScrollView.h"
#include "AXTreeViewItem.h"

class AXTreeViewItemManager;
class AXImageList;

class AXTreeView:public AXScrollView
{
    friend class AXTreeViewArea;

public:
    enum TREEVIEWSTYLE
    {
        TVS_DND     = SVS_EXTRA,

        TVS_EXTRA   = SVS_EXTRA << 1
    };

    enum TREEVIEWNOTIFY
    {
        TVN_SELCHANGE,
        TVN_EXPAND,
        TVN_CHECKITEM,
        TVN_RBUTTON,
        TVN_DBLCLK,
        TVN_DRAG_BEGIN,
        TVN_DRAG_DST,
        TVN_DRAG_END
    };

protected:
    AXTreeViewItemManager   *m_pDat;
    AXImageList     *m_pImgList;

    int     m_nMaxWidth,
            m_nMaxHeight,
            m_nItemH;

    enum TREEVIEWFLAGS
    {
        FLAG_DATRECONFIG = FLAG_TEMP1
    };

protected:
    void _createTreeView();
    void _initItem(AXTreeViewItem *pItem);
    void _setScrollInfo();
    void _setItemH();
    void _calcReconfig();
    void _updateDat();

public:
    virtual ~AXTreeView();

    AXTreeView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXTreeView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    AXTreeViewItem *getTopItem();
    AXTreeViewItem *getFocusItem();
    AXTreeViewItem *getDragSrcItem();

    void setImageList(AXImageList *pImg);
    void redrawArea();
    void updateItem(AXTreeViewItem *pItem);

    AXTreeViewItem *addItem(AXTreeItem *pParent,LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam);
    AXTreeViewItem *insertItem(AXTreeItem *pInsert,LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam);
    AXTreeViewItem *addItem(AXTreeItem *pParent,AXTreeViewItem *pItem);
    AXTreeViewItem *insertItem(AXTreeItem *pInsert,AXTreeViewItem *pItem);
    void deleteAllItem();
    void delItem(AXTreeViewItem *pItem);
    void expandItem(AXTreeViewItem *pItem,int expand);
    void setFocusItem(AXTreeViewItem *pItem);
    void moveItem(AXTreeViewItem *pItem,AXTreeViewItem *pDst);
    void moveItemLast(AXTreeViewItem *pItem,AXTreeViewItem *pParent);
    void setItemText(AXTreeViewItem *pItem,LPCUSTR pText);
    void checkItem(AXTreeViewItem *pItem,int check);
    BOOL isItemChild(AXTreeViewItem *pItem,AXTreeViewItem *pParent);
    AXTreeViewItem *findItemParam(ULONG lParam);
    void setScrollItem(AXTreeViewItem *pItem,int align);

    virtual void reconfig();
    virtual BOOL onFocusIn(int detail);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
