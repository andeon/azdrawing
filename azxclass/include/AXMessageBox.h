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

#ifndef _AX_MESSAGEBOX_H
#define _AX_MESSAGEBOX_H

#include "AXDialog.h"

class AXMessageBox:public AXDialog
{
public:
    enum MBBUTTONS
    {
        OK      = 0x0001,
        CANCEL  = 0x0002,
        YES     = 0x0004,
        NO      = 0x0008,
        SAVE    = 0x0010,
        SAVENO  = 0x0020,
        ABORT   = 0x0040,
        NOTSHOW = 0x40000000,

        OKCANCEL = OK | CANCEL,
        YESNO    = YES | NO
    };

protected:
    UINT    m_uBttFlags,
            m_uDownBtt,
            m_uOrFlags;

    void _createMessageBox(LPCUSTR pMessage,UINT uDefBtt);
    void _btt(AXLayout *pl,UINT btt,WORD wStrID,LPCSTR szDef,char key);

public:
    AXMessageBox(AXWindow *pOwner,LPCUSTR pTitle,LPCUSTR pMessage,UINT uBttFlags,UINT uDefBtt);

    UINT run();

    virtual BOOL onClose();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onKeyDown(AXHD_KEY *phd);
    virtual BOOL onKeyUp(AXHD_KEY *phd);

    static UINT message(AXWindow *pOwner,LPCUSTR pTitle,LPCUSTR pMessage,UINT uBttFlags,UINT uDefBtt);
    static void error(AXWindow *pOwner,LPCUSTR pMessage);
};

#endif
