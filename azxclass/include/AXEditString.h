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

#ifndef _AX_EDITSTRING_H
#define _AX_EDITSTRING_H

#include "AXDef.h"

class AXFont;
class AXString;

class AXEditString
{
public:
    enum DIR
    {
        LEFT    = 0,
        RIGHT   = 1,
        UP      = 2,
        DOWN    = 3
    };

    enum KEYSTATE
    {
        SHIFT   = 1,
        CTRL    = 2,
        ALT     = 4
    };

    enum CMDRET
    {
        CMDRET_NONE     = 0,
        CMDRET_UPDATE   = 1,
        CMDRET_CHANGE   = 2
    };

    enum SCRRET
    {
        SCRRET_NONE         = 0,
        SCRRET_BACKSPACE    = 1,
    };

protected:
    LPUSTR  m_pText;
    LPWORD  m_pWidth;
    int     m_nLen,
            m_nMallocLen,
            m_nCurPos,
            m_nCurX,
            m_nCurLine,
            m_nMaxWidth,
            m_nMaxLine,
            m_nSelTop,
            m_nSelEnd;
    BOOL    m_bMulti;

protected:
    int _calcSize(int len);
    void _calcCurXY();
    void _calcMax();
    int _getTextPos(int xpos,int line,BOOL bUp);
    void _checkText(AXString *pstr);

public:
    AXEditString();
    ~AXEditString();

    LPCUSTR getText() const { return m_pText; }
    LPWORD getWidthBuf() const { return m_pWidth; }
    int getLen() const { return m_nLen; }
    int getCurX() const { return m_nCurX; }
    int getCurLine() const { return m_nCurLine; }
    int getMaxWidth() const { return m_nMaxWidth; }
    int getMaxLine() const { return m_nMaxLine; }
    int getSelTop() const { return m_nSelTop; }
    int getSelEnd() const { return m_nSelEnd; }
    BOOL isEmpty() const { return (m_nLen == 0); }
    BOOL isSel() const { return (m_nSelTop >= 0); }
    BOOL isPosSel(int pos) const { return (m_nSelTop >= 0 && m_nSelTop <= pos && pos < m_nSelEnd); }

    void clearSel() { m_nSelTop = -1; }
    void setMultiLine() { m_bMulti = TRUE; }

    BOOL resize(int len);
    int getCurBackWidth();

    void getString(AXString *pstr);
    int getInt();
    double getDouble();

    void setInt(int val,const AXFont *pFont);
    void setIntFloat(int val,int dig,const AXFont *pFont);

    int keyCommand(const AXString &str,UINT key,UINT state,BOOL bEdit,ULONG idWin,const AXFont *pFont,LPINT pScr);

    void setText(LPCUSTR pText,const AXFont *pFont);
    BOOL insert(LPCUSTR pText,const AXFont *pFont);
    void del(int pos,int len);
    BOOL backspace();
    BOOL deleteCur();
    BOOL deleteSelText();
    void moveCursorPos(int pos,BOOL bSel);
    void setCursorPos(int pos);
    BOOL moveCursor(int dir,BOOL bSel);
    BOOL moveCurTop();
    BOOL moveCurEnd();
    void expandSel(int posto);
    BOOL selectAll();
    void copy(ULONG idWin);
    BOOL cut(ULONG idWin);
    BOOL paste(ULONG idWin,const AXFont *pFont);

    int getPosFromMouse(int x,int y,int texth);
};

#endif
