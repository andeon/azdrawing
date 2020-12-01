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

#ifndef _AX_APPATOM_H
#define _AX_APPATOM_H

#include "AXDef.h"

class AXAppAtom
{
public:
    enum
    {
        AXSELECTION,
        WM_DELETE_WINDOW,
        WM_STATE,
        WM_TAKE_FOCUS,
        WM_PROTOCOLS,

        _NET_WM_USER_TIME_WINDOW,
        _NET_WM_USER_TIME,
        _NET_WM_PING,
        _NET_WM_NAME,
        _NET_WM_ICON_NAME,
        _NET_WM_PID,
        _NET_WM_STATE,
        _NET_WM_STATE_MAXIMIZED_HORZ,
        _NET_WM_STATE_MAXIMIZED_VERT,
        _NET_WM_ICON,
        _NET_FRAME_EXTENTS,
        _NET_ACTIVE_WINDOW,
        _MOTIF_WM_HINTS,

        UTF8_STRING,
        COMPOUND_TEXT,
        TARGETS,
        CLIPBOARD,
        CLIPBOARD_MANAGER,
        SAVE_TARGETS,
        MULTIPLE,
        ATOM_PAIR,

        XdndAware,
        XdndEnter,
        XdndTypeList,
        XdndPosition,
        XdndLeave,
        XdndDrop,
        XdndStatus,
        XdndFinished,
        XdndActionCopy,
        XdndSelection,
        text_uri_list,

        ATOMCNT
    };

protected:
    ULONG   m_atom[ATOMCNT];

public:
    AXAppAtom();

    ULONG get(int no) const { return m_atom[no]; }
};

#endif
