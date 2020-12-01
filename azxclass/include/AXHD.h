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

#ifndef _AX_HD_H
#define _AX_HD_H

class AXWindow;
class AXDrawText;

//-----------------

struct AXHD_PAINT
{
    int x,y,w,h;
};

struct AXHD_OWNERDRAW
{
    AXWindow    *pwinFrom;
    int         nItemNo;
    UINT        uState;
    AXRectSize  rcs;
    ULONG       lParam1,lParam2;
    LPVOID      pItem;
    AXDrawText  *pDrawText;
};

struct AXHD_MOUSE
{
    int x,y,rootx,rooty,
        button, //!< AXWindow::MOUSEBUTTON
        type;   //!< AXWindow::EVENTTYPE
    UINT state; //!< AXWindow::BUTTONSTATE
    ULONG time;
};

struct AXHD_KEY
{
    UINT state,code,keysym;
    ULONG time;
    LPVOID pEvent;
};

struct AXHD_CONFIGURE
{
    int x,y,w,h;
    BOOL bSendEvent;
};

struct AXHD_ENTERLEAVE
{
    int x,y,rootx,rooty,detail;
    ULONG time;
};


//-------------------

typedef union
{
    AXHD_PAINT      paint;
    AXHD_OWNERDRAW  od;
    AXHD_MOUSE      mouse;
    AXHD_KEY        key;
    AXHD_CONFIGURE  configure;
    AXHD_ENTERLEAVE enter;
}AXHD;

#endif
