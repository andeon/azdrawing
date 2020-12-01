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

#ifndef _AX_FTFONT_H
#define _AX_FTFONT_H

#include "AXDef.h"

class AXString;

class AXFTFont
{
protected:
    LPVOID  m_pFace;

public:
    virtual ~AXFTFont();
    AXFTFont();

    LPVOID getFTFace() const { return m_pFace; }

    void free();
    BOOL loadFile(AXString &filename,int index);

    BOOL setSizePt(double ptsize,int dpi);
    BOOL setSizePx(int pxsize);
};

#endif
