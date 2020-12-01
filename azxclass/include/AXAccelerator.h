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

#ifndef _AX_ACCELERATOR_H
#define _AX_ACCELERATOR_H

#include "AXList.h"

#define ACKEY_SHIFT     0x01000000
#define ACKEY_CTRL      0x02000000
#define ACKEY_ALT       0x04000000


class AXWindow;
struct AXHD_KEY;

class AXAccelerator
{
protected:
    AXList      m_dat;
    AXWindow    *m_pwinDef;

public:
    AXAccelerator();

    void setDefaultWindow(AXWindow *pwin) { m_pwinDef = pwin; }

    void add(UINT uCmdID,UINT key);
    void add(UINT uCmdID,UINT key,AXWindow *pwin);

    BOOL onKey(const AXHD_KEY *phd,BOOL bUp);
};

#endif
