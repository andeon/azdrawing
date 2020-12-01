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

#ifndef _AX_APPRES_H
#define _AX_APPRES_H

#include "AXFont.h"

class AXGC;

class AXAppRes
{
public:
    enum RESCOL
    {
        WHITE,
        BLACK,
        FACE,
        FACEDARK,
        FACELIGHT,
        FACEDOC,
        FACEFOCUS,
        FACEMENU,
        FRAMEDARK,
        FRAMELIGHT,
        FRAMEOUTSIDE,
        FRAMEDEFBTT,
        FRAMEMENU,
        TEXTNORMAL,
        TEXTREV,
        TEXTDISABLE,
        TEXTMENU,
        BACKREVTEXT,
        BACKREVTEXTLT,
        BACKMENUSEL,

        RESCOL_NUM
    };

    enum TEXTCOL
    {
        TC_NORMAL,
        TC_DISABLE,
        TC_REV,
        TC_REVBACK,
        TC_MENU,
        TC_MENUDISABLE,
        TC_WHITE,
        TC_BLACK,

        TEXTCOL_NUM
    };

    enum RESFONT
    {
        FONT_NORMAL,

        FONT_NUM
    };

protected:
    DWORD   m_dwRGBCol[RESCOL_NUM];
    DWORD   m_pixCol[RESCOL_NUM];
    LPVOID  m_gcCol[RESCOL_NUM],
            m_pTextCol;
    AXGC    *m_pgcXor;
    AXFont  m_font[FONT_NUM];

protected:
    void _setRGBCol();
    void _setGC();
    void _setTextCol();
    void _setFont(LPCSTR szFontPattern);
    void _setDefFont();

    void setTextCol(int no,DWORD rgb);

public:
    ~AXAppRes();
    AXAppRes(LPCSTR szFontPattern);

    DWORD colRGB(int no) const { return m_dwRGBCol[no]; }
    DWORD colPix(int no) const { return m_pixCol[no]; }
    const void *colGC(int no) const { return m_gcCol[no]; }
    const AXGC &gcXor() const { return *m_pgcXor; }
    const void *textcol(int no) const;
    AXFont *font(int no) { return m_font + no; }
};

#endif
