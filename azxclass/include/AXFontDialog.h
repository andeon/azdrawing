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

#ifndef _AX_FONTDIALOG_H
#define _AX_FONTDIALOG_H

#include "AXDialog.h"
#include "AXString.h"

class AXListBox;
class AXLineEdit;

class AXFontDialog:public AXDialog
{
public:
    typedef struct
    {
        AXString    strName,
                    strStyle;
        double      dSize;
    }FONTINFO;

    enum
    {
        FDFLAG_NOSIZE = 1
    };

protected:
    FONTINFO    *m_pInfo;

    AXListBox   *m_plbName,
                *m_plbStyle;
    AXLineEdit  *m_peditSize;

protected:
    void _setFontFace();
    void _setFontStyle(BOOL bInit);

public:
    AXFontDialog(AXWindow *pOwner,FONTINFO *pinfo,UINT uFlags);

    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);

    static BOOL getFontInfo(AXWindow *pOwner,FONTINFO *pinfo,UINT uFlags=0);
    static BOOL getFontInfo(AXWindow *pOwner,AXString *pstr,UINT uFlags=0);

    static void xftFormatToFontInfo(FONTINFO *pinfo,const AXString &strText);
    static void fontInfoToXftFormat(AXString *pstr,const FONTINFO &info);
};

#endif
