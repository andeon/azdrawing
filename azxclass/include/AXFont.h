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

#ifndef _AX_FONT_H
#define _AX_FONT_H

#include "AXDef.h"

class AXString;
class AXRectSize;


class AXFont
{
    friend class AXDrawText;

protected:
    LPVOID  m_pFont;
    int     m_nHeight,
            m_nAscent;

public:
    ~AXFont();
    AXFont();

    LPVOID getFont() const { return m_pFont; }
    int getHeight() const { return m_nHeight; }

    void free();

    BOOL create(const AXString *pname,const AXString *pstyle,double size);
    BOOL createPattern(LPCSTR szPattern);
    BOOL createPattern(const AXString &pattern);
    BOOL createStyleSize(const AXString &str,double size);
    BOOL createAppFontPxSize(int nPxSize);

    void getFontName(AXString *pstr) const;

    int getTextWidth(const AXString &str) const;
    int getTextWidth(LPCUSTR pstr,int len) const;
    int getTextWidth(LPCSTR pstr,int len) const;
    void getTextSize(const AXString &str,AXSize *psize) const;
};

//------------------

class AXDrawText
{
protected:
    LPVOID  m_pDraw;

public:
    ~AXDrawText();
    AXDrawText();
    AXDrawText(ULONG dst);

    void begin(ULONG dst);
    void begin(ULONG dst,const AXRectSize &rcs);
    void end();
    void setClipRect(const AXRectSize &rcs);
    void setClipRect(int x,int y,int w,int h);

    void draw(const AXFont &font,int x,int y,const AXString &str,int colno=0);
    void draw(const AXFont &font,int x,int y,LPCUSTR pstr,int len,int colno=0);
    void draw(const AXFont &font,int x,int y,LPCSTR pText,int len,int colno=0);

    void drawRGB(const AXFont &font,int x,int y,LPCUSTR pstr,int len,DWORD col);
    void drawRGB(const AXFont &font,int x,int y,LPCSTR pstr,int len,DWORD col);
    void drawWrapRGB(const AXFont &font,int x,int y,int w,int h,LPCUSTR pstr,int len,DWORD col);

    void drawRect(int x,int y,int w,int h,int colno);
};


BOOL AXFontInit();

#endif
