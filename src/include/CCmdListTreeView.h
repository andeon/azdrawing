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

#ifndef _AZDRAW_CMDLISTTREEVIEW_H_
#define _AZDRAW_CMDLISTTREEVIEW_H_

#include "AXTreeView.h"

class AXTreeViewItem;
class AXMenu;

class CCmdListTreeView:public AXTreeView
{
public:
    enum
    {
        STYLE_KEY       = TVS_EXTRA,
        STYLE_DEVICE    = TVS_EXTRA << 1
    };

protected:
    BOOL    m_bKey;

    void _add_item(AXTreeViewItem *pParent,UINT cmdid,int strid=-1);
    void _add_itemMul(WORD wTopStrID,WORD wGroupID,int strid,int cmdid,int cnt);

    void _set_mainmenu();
    void _set_mainmenu_sub(AXMenu *pMenu,AXTreeViewItem *pParent);
    void _set_layermenu();
    void _set_registBrush(WORD wTopStrID,int cmdid);

    BOOL _check_sameKey(UINT key);

public:
    CCmdListTreeView(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    void setCmdList();

    void setKeyDat();
    void clearAll();

    void setSelCmd(int cmdid);

    BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
