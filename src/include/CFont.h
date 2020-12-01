/************************************************************************
*  Copyright (C) 2013-2015 Azel.
*
*  This file is part of AzDrawing.
*
*  AzDrawing is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  AzDrawing is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/

#ifndef _AZDW_FONT_H_
#define _AZDW_FONT_H_

#include "AXGSUBVertTbl.h"

class AXRect;
class AXString;
class CLayerImg;
class CFontCache;
class CFontCacheDat;

class CFont
{
public:
    typedef struct
    {
        int     nCharSpace,
                nLineSpace,
                nHinting;
        UINT    uFlags;
        void (CLayerImg::*pixfunc)(int,int,BYTE);
    }DRAWINFO;

    enum INFOFLAGS
    {
        INFOFLAG_2COL = 1,
        INFOFLAG_VERT = 2
    };

    enum HINTING
    {
        HINTING_NONE,
        HINTING_NORMAL,
        HINTING_LIGHT,
        HINTING_MAX
    };

protected:
    LPVOID          m_pFace;
    int             m_nPxSize;
    AXGSUBVertTbl   m_vtbl;
    CFontCache      *m_pCache;

protected:
    void _loadGSUB();
    CFontCacheDat *_getCharGlyph(UNICHAR code,int hinting,UINT uFlags);
    void _drawChar(CLayerImg *pimg,int x,int y,CFontCacheDat *pdat,DRAWINFO *pInfo);

public:
    CFont();
    ~CFont();

    BOOL isNone() const { return (m_pFace == NULL); }

    void free();
    void freeCache();

    BOOL load(const AXString &facename,const AXString &style,int pxsize);

    void drawString(CLayerImg *pimg,int x,int y,const AXString &str,DRAWINFO *pInfo);
};

#endif
