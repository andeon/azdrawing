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

#include <stdlib.h>

#include "AXEditString.h"

#include "AXString.h"
#include "AXFont.h"
#include "AXApp.h"
#include "AXClipboard.h"
#include "AXUtilStr.h"
#include "AXKey.h"


/*!
    @class AXEditString
    @brief 一行/複数行エディット用文字列と情報

    Unicode(16bit) + 各文字幅 + カーソル位置 + 選択位置。

    @ingroup widgetdat
*/

/*
    m_nMallocLen : メモリ確保している文字数（NULL文字含む）
    m_nCurPos   : カーソル位置（文字数）
    m_nCurX     : カーソル位置（文字幅。px。先頭文字を 0 とする）
    m_nCurLine  : カーソルの行位置
    m_nMaxWidth : 最大幅
    m_nSelTop   : 選択の先頭位置（文字数）。-1 で選択なし
    m_nSelEnd   : 選択の終端位置（文字数）
*/


//-----------------------------


AXEditString::~AXEditString()
{
    if(m_pText) ::free(m_pText);
    if(m_pWidth) ::free(m_pWidth);
}

AXEditString::AXEditString()
{
    int len;

    m_nMallocLen = m_nLen = 0;
    m_nCurPos = m_nCurX =  m_nCurLine = 0;
    m_nMaxWidth = m_nMaxLine = 0;
    m_nSelTop = m_nSelEnd = -1;
    m_bMulti = FALSE;

    //確保

    len = _calcSize(31);

    m_pText = (LPUSTR)::malloc(len * sizeof(UNICHAR));
    if(!m_pText) return;

    m_pWidth = (LPWORD)::malloc(len * sizeof(WORD));
    if(!m_pWidth) return;

    //

    m_pText[0]  = 0;
    m_pWidth[0] = 0;

    m_nMallocLen = len;
}

//! 最大長さから確保長さ取得

int AXEditString::_calcSize(int len)
{
    int size;

    for(size = 32; size < (1 << 28); size <<= 1)
    {
        if(len < size) break;
    }

    return size;
}

//! バッファリサイズ

BOOL AXEditString::resize(int len)
{
    LPUSTR pNewText;
    LPWORD pNew;
    int newlen;

    //拡張の必要なし

    if(m_nMallocLen > len) return TRUE;

    //------ 拡張

    newlen = _calcSize(len);

    pNewText = (LPUSTR)::realloc(m_pText, newlen * sizeof(UNICHAR));
    if(!pNewText) return FALSE;

    m_pText = pNewText;

    //

    pNew = (LPWORD)::realloc(m_pWidth, newlen * sizeof(WORD));
    if(!pNew) return FALSE;

    m_pWidth = pNew;

    m_nMallocLen = newlen;

    return TRUE;
}

//! カーソル位置から XY 位置を計算

void AXEditString::_calcCurXY()
{
    int i;
    LPUSTR pt;
    LPWORD pw;

    pt  = m_pText;
    pw  = m_pWidth;

    m_nCurX = m_nCurLine = 0;

    for(i = m_nCurPos; i > 0; i--, pt++, pw++)
    {
        if(*pt == '\n')
        {
            m_nCurLine++;
            m_nCurX = 0;
        }
        else
            m_nCurX += *pw;
    }
}

//! 最大幅・行数を計算

void AXEditString::_calcMax()
{
    LPUSTR pt;
    LPWORD pw;
    int w,maxw;

    m_nMaxLine = 1;

    pt = m_pText;
    pw = m_pWidth;

    for(maxw = w = 0; *pt; pt++, pw++)
    {
        if(*pt == '\n')
        {
            if(w > maxw) maxw = w;
            w = 0;
            m_nMaxLine++;
        }
        else
            w += *pw;
    }

    if(w > maxw) maxw = w;

    m_nMaxWidth = maxw;
}

//! 上下移動時、X位置(px)と行位置からテキスト位置取得

int AXEditString::_getTextPos(int xpos,int line,BOOL bUp)
{
    int i,x,y;
    LPUSTR pt;
    LPWORD pw;

    pt = m_pText;
    pw = m_pWidth;

    for(i = 0, x = 0, y = 0; *pt; pt++, pw++, i++)
    {
        if(y == line)
        {
            if(bUp)
            {
                if(x + *pw >= xpos) break;
            }
            else
            {
                if(x >= xpos) break;
            }
        }

        if(*pt == '\n')
        {
            if(y == line) break;

            y++;
            x = 0;
        }
        else
            x += *pw;
    }

    return i;
}

//! セットするテキストを調整

void AXEditString::_checkText(AXString *pstr)
{
    LPUSTR p;
    UNICHAR c;
    AXString str;

    for(p = *pstr; *p; )
    {
        c = *(p++);

        if(c == '\n' || c == '\r')
        {
            if(!m_bMulti)
                continue;
            else
            {
                //改行は \n に統一
                if(c == '\r' && *p == '\n')
                    p++;

                str += '\n';
            }
        }
        else if(c < 0x20)
            continue;
        else
            str += c;
    }

    *pstr = str;
}


//==============================
//他
//==============================


//! 文字列取得

void AXEditString::getString(AXString *pstr)
{
    pstr->set(m_pText, m_nLen);
}

//! テキストを数値として取得

int AXEditString::getInt()
{
    return AXUStrToInt(m_pText);
}

//! テキストをdouble値として取得

double AXEditString::getDouble()
{
    return AXUStrToDouble(m_pText);
}

//! カーソル位置の一つ前の文字の幅を取得

int AXEditString::getCurBackWidth()
{
    if(m_nCurPos == 0)
        return 0;
    else
        return m_pWidth[m_nCurPos - 1];
}


//==============================
//キー処理
//==============================


//! キー処理
/*!
    @param str      入力された文字列
    @param key      キー識別子
    @param state    装飾キー
    @param bEdit    編集可能状態か
    @param idWin    クリップボード処理時のウィンドウXID
    @param pFont    フォント
    @param pScr     処理後のスクロール状態を返す（AXEditString::SCRRET）
    @return 処理後のタイプ（AXEditString::CMDRET）
*/

int AXEditString::keyCommand(const AXString &str,UINT key,UINT state,
                             BOOL bEdit,ULONG idWin,const AXFont *pFont,LPINT pScr)
{
    int bCtrl,bUpdate = FALSE,ret = CMDRET_NONE,scr = SCRRET_NONE;
    UNICHAR enter[2] = {'\n', 0};

    bCtrl = ((state & (SHIFT|CTRL|ALT)) == CTRL);

    /*
        - ret は操作キーかどうかの判定にも使う
        - 実際に更新するかどうかは bUpdate で判定
    */

    switch(key)
    {
        //左
        case KEY_LEFT:
        case KEY_NUM_LEFT:
            bUpdate = moveCursor(LEFT, state & SHIFT);
            ret = CMDRET_UPDATE;
            break;
        //右
        case KEY_RIGHT:
        case KEY_NUM_RIGHT:
            bUpdate = moveCursor(RIGHT, state & SHIFT);
            ret = CMDRET_UPDATE;
            break;
        //上
        case KEY_UP:
        case KEY_NUM_UP:
            bUpdate = moveCursor(UP, state & SHIFT);
            ret = CMDRET_UPDATE;
            break;
        //下
        case KEY_DOWN:
        case KEY_NUM_DOWN:
            bUpdate = moveCursor(DOWN, state & SHIFT);
            ret = CMDRET_UPDATE;
            break;
        //ENTER
        case KEY_ENTER:
        case KEY_NUM_ENTER:
            if(m_bMulti && bEdit)
            {
                bUpdate = insert(enter, pFont);
                ret = CMDRET_CHANGE;
            }
            break;
        //バックスペース - 削除
        case KEY_BACKSPACE:
            if(bEdit)
            {
                bUpdate = backspace();
                scr = SCRRET_BACKSPACE;
            }
            ret = CMDRET_CHANGE;
            break;
        //削除
        case KEY_DELETE:
        case KEY_NUM_DELETE:
            if(bEdit) bUpdate = deleteCur();
            ret = CMDRET_CHANGE;
            break;
        //Home
        case KEY_HOME:
        case KEY_NUM_HOME:
            bUpdate = moveCurTop();
            ret = CMDRET_UPDATE;
            break;
        //End
        case KEY_END:
        case KEY_NUM_END:
            bUpdate = moveCurEnd();
            ret = CMDRET_UPDATE;
            break;

        //CTRL+V（貼り付け）
        case 'v':
        case 'V':
            if(bCtrl)
            {
                if(bEdit) bUpdate = paste(idWin, pFont);
                ret = CMDRET_CHANGE;
            }
            break;
        //CTRL+C（コピー）
        case 'c':
        case 'C':
            if(bCtrl)
            {
                copy(idWin);
                ret = CMDRET_UPDATE;
            }
            break;
        //CTRL+X（切り取り）
        case 'x':
        case 'X':
            if(bCtrl)
            {
                if(bEdit)
                {
                    bUpdate = cut(idWin);
                    ret = CMDRET_CHANGE;
                }
                else
                {
                    copy(idWin);
                    ret = CMDRET_UPDATE;
                }
            }
            break;
        //CTRL+A（すべて選択）
        case 'a':
        case 'A':
            if(bCtrl)
            {
                bUpdate = selectAll();
                ret = CMDRET_UPDATE;
            }
            break;
    }

    //文字列挿入

    if(bEdit && ret == CMDRET_NONE && str.isNoEmpty())
    {
        bUpdate = insert(str, pFont);
        ret = CMDRET_CHANGE;
    }

    //

    if(!bUpdate)
    {
        ret = CMDRET_NONE;
        scr = SCRRET_NONE;
    }

    *pScr = scr;

    return ret;
}


//==============================
//編集
//==============================


//! int値をテキストとしてセット

void AXEditString::setInt(int val,const AXFont *pFont)
{
    AXString str;

    str.setInt(val);
    setText(str, pFont);
}

//! 小数点ありの数値としてテキストセット

void AXEditString::setIntFloat(int val,int dig,const AXFont *pFont)
{
    AXString str;

    str.setIntFloat(val, dig);
    setText(str, pFont);
}

//! テキストセット（カーソル位置は終端＆選択リセット）

void AXEditString::setText(LPCUSTR pText,const AXFont *pFont)
{
    LPUSTR ps,pd;
    LPWORD pw;
    AXString str(pText);

    _checkText(&str);

    if(resize(str.getLen()))
    {
        ps = str;
        pd = m_pText;
        pw = m_pWidth;

        for(; *ps; ps++)
        {
            *(pd++) = *ps;
            *(pw++) = pFont->getTextWidth(ps, 1);
        }

        *pd = 0;

        m_nLen = str.getLen();
    }

    //

    m_nCurPos   = m_nLen;
    m_nSelTop   = -1;

    _calcMax();
    _calcCurXY();
}

//! 現在位置に文字列挿入

BOOL AXEditString::insert(LPCUSTR pText,const AXFont *pFont)
{
    LPUSTR ps,pd;
    LPWORD pw;
    int i,len;
    AXString str(pText);

    _checkText(&str);

    len = str.getLen();
    if(len == 0) return FALSE;

    //選択範囲削除

    deleteSelText();

    //サイズ拡張

    if(!resize(m_nLen + len))
        return FALSE;

    //挿入位置を空ける : テキスト

    ps = m_pText + m_nLen;
    pd = ps + len;

    for(i = m_nLen - m_nCurPos; i >= 0; i--)
        *(pd--) = *(ps--);

    //挿入位置を空ける : 文字幅

    ps = m_pWidth + m_nLen;
    pd = ps + len;

    for(i = m_nLen - m_nCurPos; i >= 0; i--)
        *(pd--) = *(ps--);

    //セット

    for(ps = str, pd = m_pText + m_nCurPos, pw = m_pWidth + m_nCurPos; *ps; ps++)
    {
        *(pd++) = *ps;
        *(pw++) = pFont->getTextWidth(ps, 1);
    }

    //

    m_nLen      += len;
    m_nCurPos   += len;

    _calcMax();
    _calcCurXY();

    return TRUE;
}

//! 指定範囲削除

void AXEditString::del(int pos,int len)
{
    LPUSTR psT,pdT;
    LPWORD psW,pdW;
    int i;

    //テキスト

    pdT = m_pText + pos;
    psT = pdT + len;

    for(i = pos + len; i < m_nLen; i++)
        *(pdT++) = *(psT++);

    *pdT = 0;

    //幅

    pdW = m_pWidth + pos;
    psW = pdW + len;

    for(i = pos + len; i < m_nLen; i++)
        *(pdW++) = *(psW++);

    *pdW = 0;

    //

    m_nLen    -= len;
    m_nCurPos = pos;

    _calcMax();
    _calcCurXY();
}

//! バックスペースで1文字削除

BOOL AXEditString::backspace()
{
    if(m_nSelTop >= 0)
        return deleteSelText();
    else if(m_nCurPos == 0)
        return FALSE;
    else
    {
        del(m_nCurPos - 1, 1);
        return TRUE;
    }
}

//! 現在位置の1文字削除

BOOL AXEditString::deleteCur()
{
    if(m_nSelTop >= 0)
        return deleteSelText();
    else if(m_nCurPos >= m_nLen)
        return FALSE;
    else
    {
        del(m_nCurPos, 1);
        return TRUE;
    }
}

//! 選択範囲の文字列削除

BOOL AXEditString::deleteSelText()
{
    if(m_nSelTop < 0) return FALSE;

    del(m_nSelTop, m_nSelEnd - m_nSelTop);

    m_nSelTop = -1;

    return TRUE;
}

//! カーソルを指定位置に移動

void AXEditString::moveCursorPos(int pos,BOOL bSel)
{
    if(bSel)
        expandSel(pos);
    else
        m_nSelTop = -1;

    m_nCurPos = pos;
    _calcCurXY();
}

//! カーソルを指定位置に移動（選択範囲処理なし）

void AXEditString::setCursorPos(int pos)
{
    m_nCurPos = pos;
    _calcCurXY();
}

//! カーソル位置を指定方向に移動
/*!
    @param bSel TRUEで選択範囲位置の移動
*/

BOOL AXEditString::moveCursor(int dir,BOOL bSel)
{
    int pos;

    switch(dir)
    {
        //左
        case LEFT:
            if(m_nCurPos == 0) return FALSE;

            pos = m_nCurPos - 1;
            break;
        //右
        case RIGHT:
            if(m_nCurPos >= m_nLen) return FALSE;

            pos = m_nCurPos + 1;
            break;
        //上
        case UP:
            if(!m_bMulti || m_nCurLine == 0) return FALSE;

            pos = _getTextPos(m_nCurX, m_nCurLine - 1, TRUE);
            break;
        //下
        case DOWN:
            if(!m_bMulti || m_nCurLine == m_nMaxLine - 1) return FALSE;

            pos = _getTextPos(m_nCurX, m_nCurLine + 1, FALSE);
            break;
    }

    //

    if(bSel)
        expandSel(pos);
    else
        m_nSelTop = -1;

    m_nCurPos = pos;
    _calcCurXY();

    return TRUE;
}

//! 先頭へ移動

BOOL AXEditString::moveCurTop()
{
    if(!m_bMulti)
        m_nCurPos = m_nCurX = 0;
    else
    {
        LPUSTR pt = m_pText + m_nCurPos;

        for(; m_nCurPos > 0; pt--, m_nCurPos--)
        {
            if(pt[-1] == '\n') break;
        }

        _calcCurXY();
    }

    m_nSelTop = -1;

    return TRUE;
}

//! 終端へ移動

BOOL AXEditString::moveCurEnd()
{
    if(!m_bMulti)
    {
        m_nCurPos   = m_nLen;
        m_nCurX     = m_nMaxWidth;
    }
    else
    {
        LPUSTR pt = m_pText + m_nCurPos;

        for(; *pt && *pt != '\n'; pt++, m_nCurPos++);

        _calcCurXY();
    }

    m_nSelTop = -1;

    return TRUE;
}

//! すべて選択

BOOL AXEditString::selectAll()
{
    if(m_nLen == 0)
        return FALSE;
    else
    {
        m_nSelTop = 0;
        m_nSelEnd = m_nLen;
        return TRUE;
    }
}

//! クリップボードにコピー

void AXEditString::copy(ULONG idWin)
{
    if(m_nSelTop >= 0)
        axclipb->setText(idWin, m_pText + m_nSelTop, m_nSelEnd - m_nSelTop);
}

//! 切り取り

BOOL AXEditString::cut(ULONG idWin)
{
    if(m_nSelTop < 0)
        return FALSE;
    else
    {
        axclipb->setText(idWin, m_pText + m_nSelTop, m_nSelEnd - m_nSelTop);
        deleteSelText();
        return TRUE;
    }
}

//! 貼り付け

BOOL AXEditString::paste(ULONG idWin,const AXFont *pFont)
{
    AXString str;

    if(!axclipb->getText(idWin, &str))
        return FALSE;
    else
        return insert(str, pFont);
}

//! 選択範囲拡張（現在位置から指定位置まで）

void AXEditString::expandSel(int posto)
{
    if(m_nSelTop < 0)
    {
        //選択なし時 -> 選択開始

        if(posto < m_nCurPos)
            m_nSelTop = posto, m_nSelEnd = m_nCurPos;
        else
            m_nSelTop = m_nCurPos, m_nSelEnd = posto;
    }
    else
    {
        //選択あり時 -> 拡張

        if(posto < m_nCurPos)
        {
            if(posto < m_nSelTop)
                m_nSelTop = posto;
            else
                m_nSelEnd = posto;
        }
        else if(posto > m_nCurPos)
        {
            if(posto > m_nSelEnd)
                m_nSelEnd = posto;
            else
                m_nSelTop = posto;
        }

        if(m_nSelTop == m_nSelEnd) m_nSelTop = -1;
    }
}


//=============================
//
//=============================


//! マウスでの操作時のテキスト位置取得

int AXEditString::getPosFromMouse(int x,int y,int texth)
{
    int xx,yy,pos;
    LPUSTR pt;
    LPWORD pw;

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    pt = m_pText;
    pw = m_pWidth;

    xx = yy = pos = 0;

    if(m_bMulti)
    {
        //複数行

        for(; *pt; pt++, pw++, pos++)
        {
            if(xx <= x && x < xx + *pw && yy <= y && y < yy + texth) break;

            if(*pt == '\n')
            {
                if(y < yy + texth) break;

                xx = 0;
                yy += texth;
            }
            else
                xx += *pw;
        }
    }
    else
    {
        //単一行

        for(; *pt; pt++, pw++, pos++)
        {
            if(xx <= x && x < xx + *pw) break;

            xx += *pw;
        }
    }

    return pos;
}
