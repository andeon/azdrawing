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

#ifndef _AX_GC_H
#define _AX_GC_H

#include "AXDef.h"

class AXGC
{
protected:
    LPVOID  m_gc;

protected:
    BOOL _create(ULONG mask,LPVOID pVal);

public:
    AXGC();
    virtual ~AXGC();

    LPVOID getGC() const { return m_gc; }

    void free();

    BOOL createDefault();
    BOOL createColor(DWORD col);
    BOOL createXor();
    BOOL createClipMask(ULONG pixmapID);

    void setColorPix(ULONG pix);
    void setColorRGB(DWORD col);
    void setBkColorPix(ULONG pix);
    void setExposureFlag(BOOL flag);
    void setClipNone();
    void setClipRect(int x,int y,int w,int h);
    void setClipOrigin(int x,int y);
};

#endif
