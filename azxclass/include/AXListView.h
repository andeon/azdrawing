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

#ifndef _AX_LISTVIEW_H
#define _AX_LISTVIEW_H

#include "AXScrollView.h"
#include "AXListViewItem.h"

class AXListViewItemManager;
class AXHeader;
class AXImageList;

class AXListView:public AXScrollView
{
    friend class AXListViewArea;

public:
    enum LISTVIEWSTYLE
    {
        LVS_COLLINE     = SVS_EXTRA,
        LVS_ROWLINE     = SVS_EXTRA << 1,
        LVS_CHECKBOX    = SVS_EXTRA << 2,
        LVS_MULTISEL    = SVS_EXTRA << 3,
        LVS_NOHEADER    = SVS_EXTRA << 4,

        LVS_EXTRA       = SVS_EXTRA << 5,

        LVS_GRIDLINE    = LVS_COLLINE | LVS_ROWLINE
    };

    enum LISTVIEWNOTIFY
    {
        LVN_CHANGEFOCUS,
        LVN_CHECKITEM,
        LVN_RIGHTCLICK,
        LVN_DBLCLK
    };

    enum COLUMNFLAGS
    {
        CF_RIGHT    = 1,
        CF_FIX      = 2,
        CF_EXPAND   = 4 | CF_FIX,
        CF_OWNERDRAW = 8
    };

protected:
    AXListViewItemManager   *m_pDat;
    AXHeader    *m_pHeader;
    AXImageList *m_pImgList;
    int         m_nItemH,
                m_nCustumH;

protected:
    void _createListView();
    void _setScrollInfo();
    void _setItemH();

public:
    virtual ~AXListView();

    AXListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXListView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    int getItemH() const { return m_nItemH; }
    AXImageList *getImageList() const { return m_pImgList; }
    int getLastClkColumn();
    void setImageList(AXImageList *pimg);
    void setItemHeight(int h);

    void addColumn(LPCUSTR pText,int width,UINT uFlags);
    int getColWidth(int no);
    void setColumnWidth(int colno,int width);

    AXListViewItem *addItem(LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam);
    AXListViewItem *addItem(LPCUSTR pText);
    AXListViewItem *addItem(AXListViewItem *pItem);
    AXListViewItem *insertItem(AXListViewItem *pInsert,LPCUSTR pText,int nIconNo,UINT uFlags,ULONG lParam);
    AXListViewItem *insertItem(AXListViewItem *pInsert,AXListViewItem *pItem);
    void addItemMulTr(int startID,int cnt);
    void deleteAllItem();
    void delItem(AXListViewItem *pItem);
    void delItem(int pos);
    void delItemFocus();
    void setItemText(AXListViewItem *pItem,int colno,LPCUSTR pText);
    void setItemText(int pos,int colno,LPCUSTR pText);
    void getItemText(AXListViewItem *pItem,int colno,AXString *pstr);
    ULONG getItemParam(int pos);
    void setItemParam(int pos,ULONG lParam);
    void setItemParam(AXListViewItem *p,ULONG lParam);
    AXListViewItem *getTopItem();
    AXListViewItem *getItem(int pos);
    AXListViewItem *getFocusItem();
    AXListViewItem *getSelNext(AXListViewItem *pTop=NULL);
    int getItemCnt();
    void scrollEnd();
    void setFocusItem(AXListViewItem *pItem);
    void setFocusItem(int pos);
    void sortItem(int (*func)(AXListItem*,AXListItem*,ULONG),ULONG lParam);
    void setAutoWidth(int colno);
    void updatePaint();

    virtual void calcDefSize();
    virtual void reconfig();
    virtual BOOL onFocusIn(int detail);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
