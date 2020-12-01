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

#ifndef _AX_IMAGELIST_H
#define _AX_IMAGELIST_H

#include "AXDef.h"

class AXString;
class AXImage;
class AXPixmap;
class AXImage32;
class AXGC;

class AXImageList
{
protected:
    AXImage     *m_pimgSrc,
                *m_pimgDisable;
    AXPixmap    *m_pimgMask;
    AXGC        *m_pGC;
    int         m_nOneW,
                m_nMaxCnt;

public:
    AXImageList();
    virtual ~AXImageList();

    int getOneW() const { return m_nOneW; }
    int getMaxCnt() const { return m_nMaxCnt; }
    int getHeight() const;

    void free();
    BOOL create(const AXImage32 &imgSrc,int onew,int colMask,BOOL bDisable);

    BOOL loadPNG(const AXString &filename,int onew,int colMask,BOOL bDisable=FALSE);
    BOOL loadPNG(LPBYTE pBuf,DWORD dwSize,int onew,int colMask,BOOL bDisable=FALSE);

    void put(UINT dstID,int dx,int dy,int no,BOOL bDisable=FALSE);
};

#endif
