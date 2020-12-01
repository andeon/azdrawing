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

#ifndef _AX_PIXMAP_H
#define _AX_PIXMAP_H

#include "AXDrawable.h"

class AXImage32;
class AXGC;

class AXPixmap:public AXDrawable
{
protected:
    int     m_nWidth,
            m_nHeight;

public:
    AXPixmap();
    virtual ~AXPixmap();

    int getWidth() const { return m_nWidth; }
    int getHeight() const { return m_nHeight; }

    void free();
    BOOL create(int w,int h);
    BOOL recreate(int w,int h,int hblock,int vblock);
    BOOL createMask(const AXImage32 &imgSrc,DWORD dwCol);

    void put(AXDrawable *pdst,int x,int y,int w,int h) const;
    void put(AXDrawable *pdst,int dx,int dy,int sx,int sy,int w,int h) const;
    void put(AXDrawable *pdst,const AXGC &gc,int dx,int dy,int sx,int sy,int w,int h) const;
};

#endif
