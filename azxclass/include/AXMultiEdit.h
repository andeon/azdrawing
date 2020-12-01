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

#ifndef _AX_MULTIEDIT_H
#define _AX_MULTIEDIT_H

#include "AXScrollView.h"

class AXString;
class AXEditString;

class AXMultiEdit:public AXScrollView
{
public:
    enum MULTIEDITSTYLE
    {
        MES_READONLY = SVS_EXTRA
    };

    enum MULTIEDITNOTIFY
    {
        MEN_CHANGE
    };

protected:
    AXEditString    *m_pString;

protected:
    void _createMultiEdit();
    virtual void getICPos(AXPoint *pPos);
    virtual BOOL isAcceptKey(UINT keytype);

public:
    virtual ~AXMultiEdit();

    AXMultiEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags);
    AXMultiEdit(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding);

    void setText(LPCUSTR pText);
    void getText(AXString *pstr);
    int getTextLen();
    void selectAll();

    virtual void reconfig();
    virtual BOOL onFocusIn(int detail);
    virtual BOOL onFocusOut(int detail);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
};

#endif
