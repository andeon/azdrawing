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

#include <unistd.h>
#include <string.h>

#include "AXAppRes.h"
#include "AXApp.h"
#include "AXString.h"
#include "AXByteString.h"
#include "AXGC.h"
#include "AXConfRead.h"


//--------------------

/*!
    @class AXAppRes
    @brief GUI関連のリソース（AXApp で作成される）

    @ingroup core
*/


//! 解放

AXAppRes::~AXAppRes()
{
    int i;

    //GC

    for(i = 0; i < RESCOL_NUM; i++)
    {
        if(m_gcCol[i])
            ::XFreeGC(axdisp(), (GC)m_gcCol[i]);
    }

    delete m_pgcXor;

    //テキスト色

    if(m_pTextCol)
        ::free(m_pTextCol);
}

//! 作成

AXAppRes::AXAppRes(LPCSTR szFontPattern)
{
    int i;

    //RGB値

    _setRGBCol();

    //色ピクセル値

    for(i = 0; i < RESCOL_NUM; i++)
        m_pixCol[i] = axapp->rgbToPix(m_dwRGBCol[i]);

    //GC

    _setGC();

    //テキスト色

    _setTextCol();

    //フォント

    _setFont(szFontPattern);
}

//! テキスト色取得
/*!
    @return XftColor *
*/

const void *AXAppRes::textcol(int no) const
{
    return (void *)((XftColor *)m_pTextCol + no);
}


//============================
//セット
//============================


//! m_dwRGBColセット

void AXAppRes::_setRGBCol()
{
    m_dwRGBCol[WHITE]        = 0xffffff;
    m_dwRGBCol[BLACK]        = 0;

    m_dwRGBCol[FACE]         = _RGB(217,217,217);
    m_dwRGBCol[FACEDARK]     = _RGB(180,180,180);
    m_dwRGBCol[FACELIGHT]    = _RGB(230,230,230);
    m_dwRGBCol[FACEDOC]      = 0xffffff;
    m_dwRGBCol[FACEFOCUS]    = _RGB(203,208,226);
    m_dwRGBCol[FACEMENU]     = _RGB(112,112,112);

    m_dwRGBCol[FRAMEDARK]    = 0x808080;
    m_dwRGBCol[FRAMELIGHT]   = 0xffffff;
    m_dwRGBCol[FRAMEOUTSIDE] = _RGB(32,32,32);
    m_dwRGBCol[FRAMEDEFBTT]  = _RGB(57,124,187);
    m_dwRGBCol[FRAMEMENU]    = _RGB(0,0,0);

    m_dwRGBCol[TEXTNORMAL]   = 0;
    m_dwRGBCol[TEXTREV]      = 0xffffff;
    m_dwRGBCol[TEXTDISABLE]  = 0x808080;
    m_dwRGBCol[TEXTMENU]     = 0xffffff;

    m_dwRGBCol[BACKREVTEXT]   = _RGB(94,132,219);
    m_dwRGBCol[BACKREVTEXTLT] = _RGB(138,169,239);
    m_dwRGBCol[BACKMENUSEL]   = _RGB(150,165,255);
}

//! GCセット

void AXAppRes::_setGC()
{
    int i;
    ULONG mask;
    XGCValues gcval;

    //色に対応するGC

    mask = GCForeground | GCBackground | GCFillStyle | GCGraphicsExposures;

    gcval.fill_style         = FillSolid;
    gcval.graphics_exposures = FALSE;
    gcval.background         = m_pixCol[FACE];

    for(i = 0; i < RESCOL_NUM; i++)
    {
        gcval.foreground = m_pixCol[i];
        m_gcCol[i] = (LPVOID)::XCreateGC(axdisp(), axapp->getRootID(), mask, &gcval);
    }

    //XOR GC

    m_pgcXor = new AXGC;
    m_pgcXor->createXor();
}

//! テキスト色セット

void AXAppRes::_setTextCol()
{
    m_pTextCol = ::malloc(sizeof(XftColor) * TEXTCOL_NUM);

    setTextCol(TC_NORMAL, 0);
    setTextCol(TC_DISABLE, 0x808080);
    setTextCol(TC_REV, 0xffffff);
    setTextCol(TC_REVBACK, m_dwRGBCol[BACKREVTEXT]);
    setTextCol(TC_MENU, 0xffffff);
    setTextCol(TC_MENUDISABLE, _RGB(48,48,48));
    setTextCol(TC_WHITE, 0xffffff);
    setTextCol(TC_BLACK, 0);
}

//! フォントセット

void AXAppRes::_setFont(LPCSTR szFontPattern)
{
    if(szFontPattern)
        //コマンドラインから指定
        m_font[FONT_NORMAL].createPattern(szFontPattern);
    else
        _setDefFont();
}

//! デフォルトフォントセット

void AXAppRes::_setDefFont()
{
    AXConfRead conf;
    AXString str;
    int size;

    //共通の設定ファイル

    conf.loadFileHome(".azxclass/default.conf");
    conf.setGroup("font");

    conf.getStr("name", &str, "");
    size = conf.getInt("size", 9);

    conf.free();

    //

    if(str.isNoEmpty())
    {
        //設定ファイルのフォントから

        m_font[FONT_NORMAL].create(&str, NULL, size);
    }
    else
    {
        //デフォルト

        AXByteString str2;

        LPSTR pcLang = ::getenv("LANG");

        if(pcLang && ::strncmp(pcLang, "ja_JP", 5) == 0)
            str.setUTF8("Takao Pゴシック,IPA Pゴシック,梅Pゴシック,VL Pゴシック,Sans:size=9");
        else
            str.setUTF8("Dejavu Sans,Sans,serif:size=9");

        str.toLocal(&str2);

        m_font[FONT_NORMAL].createPattern(str2);
    }
}


//==========================


//! 各テキスト色セット

void AXAppRes::setTextCol(int no,DWORD rgb)
{
    XftColor *p = (XftColor *)m_pTextCol + no;

    p->color.red   = ((rgb & 0xff0000) >> 8) + 255;
    p->color.green = (rgb & 0x00ff00) + 255;
    p->color.blue  = ((rgb & 0xff) << 8) + 255;
    p->color.alpha = 0xffff;

    p->pixel = axapp->rgbToPix(rgb);
}
