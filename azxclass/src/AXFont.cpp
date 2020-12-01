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

#define _AX_X11_XFT
#include "AXX11.h"

#include "AXFont.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXString.h"
#include "AXByteString.h"
#include "AXRect.h"
#include "AXUtilStr.h"


#define FONT ((XftFont *)m_pFont)


//---------------------------


void ax_setXftColor(XftColor *pd,DWORD col)
{
    pd->color.red   = ((col & 0xff0000) >> 8) + 255;
    pd->color.green = (col & 0x00ff00) + 255;
    pd->color.blue  = ((col & 0xff) << 8) + 255;
    pd->color.alpha = 0xffff;
    pd->pixel       = axapp->rgbToPix(col);
}


//---------------------------


//***************************
// AXFont
//***************************

/*!
    @class AXFont
    @brief Xftフォントクラス

    @ingroup draw
*/


AXFont::AXFont()
{
    m_pFont = NULL;
}

AXFont::~AXFont()
{
    free();
}

//! 削除

void AXFont::free()
{
    if(m_pFont)
    {
        ::XftFontClose(axdisp(), FONT);
        m_pFont = NULL;
    }
}


//======================================
// 作成
//======================================


//! 各値を指定して作成
/*!
    @param pname  NULL または空文字で指定なし
    @param pstyle NULL または空文字で指定なし
    @param size   正の値でptサイズ。負の値でpxサイズ。
*/

BOOL AXFont::create(const AXString *pname,const AXString *pstyle,double size)
{
    XftPattern *p,*match;
    XftResult res;
    AXByteString strb;

    free();

    //---------------

    p = XftPatternCreate();
    if(!p) return FALSE;

    //フォント名

    if(pname && pname->isNoEmpty())
    {
        pname->toLocal(&strb);
        XftPatternAddString(p, XFT_FAMILY, (LPSTR)strb);
    }

    XftPatternAddString(p, XFT_FAMILY, "sans-serif");

    //スタイル

    if(pstyle && pstyle->isNoEmpty())
    {
        pstyle->toLocal(&strb);
        XftPatternAddString(p, XFT_STYLE, (LPSTR)strb);
    }
    else
    {
        XftPatternAddInteger(p, XFT_WEIGHT, XFT_WEIGHT_MEDIUM);
        XftPatternAddInteger(p, XFT_SLANT, XFT_SLANT_ROMAN);
    }

    //サイズ

    if(size < 0)
        XftPatternAddDouble(p, XFT_PIXEL_SIZE, -size);
    else
        XftPatternAddDouble(p, XFT_SIZE, size);

    //作成
    /* match は削除しなくて良い */

    match = XftFontMatch(axdisp(), axapp->getDefScreen(), p, &res);

    if(match)
        m_pFont = (LPVOID)XftFontOpenPattern(axdisp(), match);

    XftPatternDestroy(p);

    //---------------

    if(m_pFont)
    {
        m_nHeight = FONT->ascent + FONT->descent;
        m_nAscent = FONT->ascent;
    }
    else
    {
        m_nHeight = 12;
        m_nAscent = 0;
    }

    return (m_pFont != NULL);
}

//! パターン文字列から作成
/*!
    フォント名に '-' が含まれると正しく作成されないので注意。
*/

BOOL AXFont::createPattern(LPCSTR szPattern)
{
    free();

    m_nHeight = 12;
    m_nAscent = 0;

    //

    m_pFont = (LPVOID)::XftFontOpenName(axdisp(), axapp->getDefScreen(), szPattern);
    if(!m_pFont) return FALSE;

    m_nHeight = FONT->ascent + FONT->descent;
    m_nAscent = FONT->ascent;

    return TRUE;
}

//! パターン文字列から作成

BOOL AXFont::createPattern(const AXString &pattern)
{
    AXByteString strb;

    pattern.toLocal(&strb);

    return createPattern(strb);
}

//! "フォント名:スタイル" の文字列とサイズを指定して作成

BOOL AXFont::createStyleSize(const AXString &str,double size)
{
    AXString name,style;

    str.getSplit(&name, &style, ':');

    return create(&name, &style, size);
}

//! アプリケーションのデフォルトフォントにpxサイズを指定して作成

BOOL AXFont::createAppFontPxSize(int nPxSize)
{
    AXString str;

    (axres->font(0))->getFontName(&str);

    return create(&str, NULL, -nPxSize);
}


//========================


//! フォント名取得

void AXFont::getFontName(AXString *pstr) const
{
    pstr->empty();

    if(m_pFont)
    {
        FcChar8 *name;

        if(::FcPatternGetString(FONT->pattern, FC_FAMILY, 0, &name) == FcResultMatch)
            pstr->setLocal((LPSTR)name);
    }
}

//========================

//! 文字列のpx幅サイズ取得

int AXFont::getTextWidth(const AXString &str) const
{
    if(!m_pFont)
        return 0;
    else if(str.isEmpty())
        return 0;
    else
    {
        XGlyphInfo gi;

        ::XftTextExtents16(axdisp(), FONT, (XftChar16 *)(LPUSTR)str, str.getLen(), &gi);

        return gi.xOff;
    }
}

//! 文字列のpx幅サイズ取得
//! @param len 負の値で全体の長さ

int AXFont::getTextWidth(LPCUSTR pstr,int len) const
{
    if(!m_pFont)
        return 0;
    else
    {
        XGlyphInfo gi;

        if(len < 0) len = AXUStrLen(pstr);

        ::XftTextExtents16(axdisp(), FONT, (XftChar16 *)pstr, len, &gi);

        return gi.xOff;
    }
}

//! 文字列のpx幅サイズ取得（ASCII文字列）

int AXFont::getTextWidth(LPCSTR pstr,int len) const
{
    if(!m_pFont)
        return 0;
    else
    {
        XGlyphInfo gi;

        ::XftTextExtents8(axdisp(), FONT, (XftChar8 *)pstr, len, &gi);

        return gi.xOff;
    }
}

//! 文字列のサイズ取得

void AXFont::getTextSize(const AXString &str,AXSize *psize) const
{
    psize->w = getTextWidth(str);
    psize->h = m_nHeight;
}


//***************************
// AXDrawText
//***************************


/*!
    @class AXDrawText
    @brief フォント描画クラス

    @ingroup draw
*/


AXDrawText::~AXDrawText()
{
    end();
}

AXDrawText::AXDrawText()
{
    m_pDraw = NULL;
}

AXDrawText::AXDrawText(ULONG dst)
{
    begin(dst);
}

//! 終了

void AXDrawText::end()
{
    if(m_pDraw)
    {
        ::XftDrawDestroy((XftDraw *)m_pDraw);
        m_pDraw = NULL;
    }
}

//! 開始

void AXDrawText::begin(ULONG dst)
{
    m_pDraw = (LPVOID)::XftDrawCreate(axdisp(), dst,
                        (Visual *)axapp->getVisual(), axapp->getColmap());
}

//! 開始（クリッピング範囲指定）

void AXDrawText::begin(ULONG dst,const AXRectSize &rcs)
{
    begin(dst);
    setClipRect(rcs);
}

//! クリッピング範囲変更

void AXDrawText::setClipRect(const AXRectSize &rcs)
{
	XRectangle rc;

	if(m_pDraw)
	{
        rc.x = rcs.x;
        rc.y = rcs.y;
        rc.width  = (rcs.w < 0)? 0: rcs.w;
        rc.height = (rcs.h < 0)? 0: rcs.h;

        ::XftDrawSetClipRectangles((XftDraw *)m_pDraw, 0, 0, &rc, 1);
	}
}

void AXDrawText::setClipRect(int x,int y,int w,int h)
{
	XRectangle rc;

	if(m_pDraw)
	{
        rc.x = x;
        rc.y = y;
        rc.width  = (w < 0)? 0: w;
        rc.height = (h < 0)? 0: h;

        ::XftDrawSetClipRectangles((XftDraw *)m_pDraw, 0, 0, &rc, 1);
	}
}

//! テキスト描画（AXString）

void AXDrawText::draw(const AXFont &font,int x,int y,const AXString &str,int colno)
{
    if(font.m_pFont && str.isNoEmpty() && m_pDraw)
    {
        ::XftDrawString16((XftDraw *)m_pDraw, (XftColor *)axres->textcol(colno),
            (XftFont *)font.m_pFont, x, y + font.m_nAscent, (XftChar16 *)(LPUSTR)str, str.getLen());
    }
}

//! テキスト描画（Unicode:16bit）

void AXDrawText::draw(const AXFont &font,int x,int y,LPCUSTR pstr,int len,int colno)
{
    if(font.m_pFont && pstr && m_pDraw)
    {
        ::XftDrawString16((XftDraw *)m_pDraw, (XftColor *)axres->textcol(colno),
            (XftFont *)font.m_pFont, x, y + font.m_nAscent, (XftChar16 *)pstr, len);
    }
}

//! テキスト描画（ASCII文字列）

void AXDrawText::draw(const AXFont &font,int x,int y,LPCSTR pText,int len,int colno)
{
    if(font.m_pFont && m_pDraw)
    {
        ::XftDrawString8((XftDraw *)m_pDraw, (XftColor *)axres->textcol(colno),
            (XftFont *)font.m_pFont, x, y + font.m_nAscent, (XftChar8 *)pText, len);
    }
}

//! RGB指定でテキスト描画

void AXDrawText::drawRGB(const AXFont &font,int x,int y,LPCUSTR pstr,int len,DWORD col)
{
    XftColor xc;

    if(font.m_pFont && pstr && m_pDraw)
    {
        ax_setXftColor(&xc, col);

        ::XftDrawString16((XftDraw *)m_pDraw, &xc,
            (XftFont *)font.m_pFont, x, y + font.m_nAscent, (XftChar16 *)pstr, len);
    }
}

//! RGB指定でテキスト描画(ASCII)

void AXDrawText::drawRGB(const AXFont &font,int x,int y,LPCSTR pstr,int len,DWORD col)
{
    XftColor xc;

    if(font.m_pFont && pstr && m_pDraw)
    {
        ax_setXftColor(&xc, col);

        ::XftDrawString8((XftDraw *)m_pDraw, &xc,
            (XftFont *)font.m_pFont, x, y + font.m_nAscent, (XftChar8 *)pstr, len);
    }
}

//! 折り返しあり・RGB指定でテキスト描画

void AXDrawText::drawWrapRGB(const AXFont &font,int x,int y,int w,int h,LPCUSTR pstr,int len,DWORD col)
{
    XftColor xc;
    XGlyphInfo gi;
    int xx,yy;

    if(font.m_pFont && pstr && m_pDraw)
    {
        ax_setXftColor(&xc, col);

        xx = x, yy = y;

        for(; *pstr && len && yy < y + h; pstr++, len--)
        {
            ::XftTextExtents16(axdisp(), (XftFont *)font.m_pFont, (XftChar16 *)pstr, 1, &gi);

            if(xx != x && xx + gi.xOff > x + w)
            {
                xx = x;
                yy += font.m_nHeight;
            }

            ::XftDrawString16((XftDraw *)m_pDraw, &xc,
                (XftFont *)font.m_pFont, xx, yy + font.m_nAscent, (XftChar16 *)pstr, 1);

            xx += gi.xOff;
        }
    }
}

//! 四角形塗りつぶし

void AXDrawText::drawRect(int x,int y,int w,int h,int colno)
{
    if(m_pDraw)
        ::XftDrawRect((XftDraw *)m_pDraw, (XftColor *)axres->textcol(colno), x, y, w, h);
}


//***************************
// 関数
//***************************


//! フォント初期化（AXApp で呼ばれる）

BOOL AXFontInit()
{
    if(!::XftInit(NULL)) return FALSE;

    if(::XftGetVersion() < 2) return FALSE;

    return TRUE;
}
