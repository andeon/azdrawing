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

#ifndef _AZDRAW_BRUSHWIN_H_
#define _AZDRAW_BRUSHWIN_H_

#include "AXTopWindow.h"

class AXToolBar;
class CBrushWin_sizeList;
class CBrushWin_tree;
class CBrushWin_value;
class AXTreeViewItem;

class CBrushWin:public AXTopWindow
{
public:
    static CBrushWin   *m_pSelf;

protected:
    AXToolBar           *m_ptbTree;
    CBrushWin_sizeList  *m_pSizeList;
    CBrushWin_tree      *m_pTree;
    CBrushWin_value     *m_pValue;

protected:
    void _setTB();
    void _changeBrushSel(AXTreeViewItem *pItem);

public:
    CBrushWin(AXWindow *pOwner,UINT addstyle);

    void showChange();
    void getWinH();
    void setBrushSize(int size);
    void setBrushVal(int val);
    void saveBrushFile();
    void initAfterShow();
    void moveBrushSelUpDown(BOOL bDown);
    void setSelRegBrush(int no);

    virtual BOOL onClose();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);
};

#define BRUSHWIN  (CBrushWin::m_pSelf)

#endif
