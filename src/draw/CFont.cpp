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

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H
#include FT_BITMAP_H
#include FT_OPENTYPE_VALIDATE_H

#include <fontconfig/fontconfig.h>

#include "CFont.h"

#include "CLayerImg.h"

#include "AXFreeType.h"
#include "AXString.h"
#include "AXByteString.h"
#include "AXList.h"
#include "AXRect.h"


//---------------

#define FTLIB   (FT_Library)AXFreeType::m_pSelf->getFTLib()
#define FTFACE  (FT_Face)m_pFace

//---------------


//******************************
// キャッシュ
//******************************


//! 文字キャッシュデータ

class CFontCacheDat:public AXListItem
{
public:
    UINT        m_uIndex;   //!< グリフindex
    FT_Glyph    m_glyph;    //!< グリフデータ
    int         m_nHorzX, m_nHorzY, m_nHorzNext,
                m_nVertX, m_nVertY, m_nVertNext;

public:
    virtual ~CFontCacheDat();
    CFontCacheDat(UINT index,FT_Glyph glyph,FT_Glyph_Metrics *mt);
};

//! キャッシュ

class CFontCache:public AXList
{
public:
    CFontCacheDat *find(UINT uIndex);
};


//******************************
// CFont
//******************************


/*!
    @class CFont
    @brief フォントクラス
*/


CFont::CFont()
{
    m_pFace  = NULL;
    m_pCache = new CFontCache;
}

CFont::~CFont()
{
    free();

    delete m_pCache;
}

//! 解放

void CFont::free()
{
    m_pCache->deleteAll();

    if(m_pFace)
    {
        FT_Done_Face(FTFACE);
        m_pFace = NULL;
    }
}

//! キャッシュすべて削除

void CFont::freeCache()
{
    m_pCache->deleteAll();
}

//! 読み込み

BOOL CFont::load(const AXString &facename,const AXString &style,int pxsize)
{
    BOOL result = FALSE;
    int index = 0;
    FcPattern *pat,*match;
    FcResult ret;
    char *fname;
    AXByteString strFace,strStyle;

    if(!FTLIB) return FALSE;

    free();

    //---------- フォント名からファイル名取得

    facename.toLocal(&strFace);
    style.toLocal(&strStyle);

    //マッチするパターン取得

    pat = FcPatternCreate();

    FcPatternAddString(pat, FC_FAMILY, (unsigned char *)(LPSTR)strFace);
    FcPatternAddString(pat, FC_STYLE, (unsigned char *)(LPSTR)strStyle);
    FcPatternAddDouble(pat, FC_PIXEL_SIZE, (double)pxsize);

    if(!FcConfigSubstitute(NULL, pat, FcMatchPattern))
    {
        FcPatternDestroy(pat);
        return FALSE;
    }

    match = FcFontMatch(0, pat, &ret);

    FcPatternDestroy(pat);

    if(!match) return FALSE;

    //パターンからファイル名取得

    ret = FcPatternGetString(match, FC_FILE, 0, (FcChar8 **)&fname);

    FcPatternGetInteger(match, FC_INDEX, 0, &index);

    if(ret == FcResultMatch)
    {
        //ファイルから読み込み

        FT_Face face;
        double size;

        if(FT_New_Face(FTLIB, fname, index, &face) == 0)
        {
            m_pFace = (LPVOID)face;
            result  = TRUE;

            //GSUBテーブル読み込み

            _loadGSUB();

            //実際のpxサイズ

            FcPatternGetDouble(match, FC_PIXEL_SIZE, 0, &size);
            m_nPxSize = (int)(size + 0.5);

            //サイズセット

            FT_Set_Pixel_Sizes(face, 0, m_nPxSize);
        }
    }

    FcPatternDestroy(match);

    return result;
}

//! GUSBテーブル読み込み

void CFont::_loadGSUB()
{
    FT_Bytes base,gdef,gpos,gsub,jstf;

    if(FT_OpenType_Validate(FTFACE, FT_VALIDATE_GSUB, &base,&gdef,&gpos,&gsub,&jstf) == 0)
    {
        if(gsub)
        {
            m_vtbl.loadGSUB((LPBYTE)gsub);

            FT_OpenType_Free(FTFACE, gsub);
        }
    }
}

//! 文字列描画

void CFont::drawString(CLayerImg *pimg,int x,int y,const AXString &str,DRAWINFO *pInfo)
{
    LPCUSTR pc;
    CFontCacheDat *pdat;
    int dx,dy;

    if(!m_pFace) return;

    dx = x << 6;
    dy = y << 6;

    if(pInfo->uFlags & INFOFLAG_VERT)
    {
        //------- 縦書き

        for(pc = str; *pc; pc++)
        {
            if(*pc == '\n')
            {
                //改行

                dx -= (m_nPxSize + pInfo->nLineSpace) << 6;
                dy = y << 6;
            }
            else
            {
                //文字

                pdat = _getCharGlyph(*pc, pInfo->nHinting, pInfo->uFlags);

                if(pdat)
                {
                    _drawChar(pimg, (dx + pdat->m_nVertX) >> 6, (dy + pdat->m_nVertY) >> 6, pdat, pInfo);

                    dy += pdat->m_nVertNext + (pInfo->nCharSpace << 6);
                }
            }
        }
    }
    else
    {
        //------- 横書き

        for(pc = str; *pc; pc++)
        {
            if(*pc == '\n')
            {
                //改行

                dx = x << 6;
                dy += (m_nPxSize + pInfo->nLineSpace) << 6;
            }
            else
            {
                //文字

                pdat = _getCharGlyph(*pc, pInfo->nHinting, pInfo->uFlags);

                if(pdat)
                {
                    _drawChar(pimg, (dx + pdat->m_nHorzX) >> 6, (dy + pdat->m_nHorzY) >> 6, pdat, pInfo);

                    dx += pdat->m_nHorzNext + (pInfo->nCharSpace << 6);
                }
            }
        }
    }
}


//==============================
//サブ
//==============================


//! 1文字描画

void CFont::_drawChar(CLayerImg *pimg,int x,int y,CFontCacheDat *pdat,DRAWINFO *pInfo)
{
    FT_Bitmap *pbm;
    LPBYTE pbuf,pb;
    int ix,iy,w,h,pitch,flag;
    void (CLayerImg::*funcPix)(int,int,BYTE) = pInfo->pixfunc;

    pbm = &((FT_BitmapGlyph)pdat->m_glyph)->bitmap;

    w     = pbm->width;
    h     = pbm->rows;
    pbuf  = pbm->buffer;
    pitch = pbm->pitch;

    if(pitch < 0) pbuf += -pitch * (h - 1);

    //描画

    if(pbm->pixel_mode == FT_PIXEL_MODE_MONO)
    {
        //-------- 1bitモノクロ

        for(iy = 0; iy < h; iy++, pbuf += pitch)
        {
            for(ix = 0, flag = 0x80, pb = pbuf; ix < w; ix++)
            {
                if(*pb & flag)
                    (pimg->*funcPix)(x + ix, y + iy, 255);

                flag >>= 1;
                if(flag == 0) { flag = 0x80; pb++; }
            }
        }
    }
    else if(pbm->pixel_mode == FT_PIXEL_MODE_GRAY)
    {
        //-------- 8bitグレイスケール

        for(iy = 0; iy < h; iy++, pbuf += pitch)
        {
            for(ix = 0, pb = pbuf; ix < w; ix++, pb++)
            {
                if(*pb)
                    (pimg->*funcPix)(x + ix, y + iy, *pb);
            }
        }
    }
}

//! ビットマップのグリフ取得

CFontCacheDat *CFont::_getCharGlyph(UNICHAR code,int hinting,UINT uFlags)
{
    FT_Face face = FTFACE;
    FT_UInt gindex;
    FT_Glyph glyph;
    CFontCacheDat *pdat;
    UINT flag;

    //UNICODEからグリフインデックス取得

    gindex = FT_Get_Char_Index(face, code);
    if(gindex == 0) return NULL;

    //縦書きの場合、置き換え

    if(uFlags & INFOFLAG_VERT)
        gindex = m_vtbl.getGlyphIndex(gindex);

    //キャッシュ検索

    pdat = m_pCache->find(gindex);

    //キャッシュにない場合

    if(!pdat)
    {
        //グリフスロットにロード

        flag = FT_LOAD_DEFAULT;

        if(!(uFlags & INFOFLAG_2COL))
            flag |= FT_LOAD_NO_BITMAP;

        if(hinting == HINTING_NONE)
            flag |= FT_LOAD_NO_HINTING;
        else if(hinting == HINTING_LIGHT)
            flag |= FT_LOAD_TARGET_LIGHT;
        else if(hinting == HINTING_MAX)
            flag |= FT_LOAD_FORCE_AUTOHINT;

        if(FT_Load_Glyph(face, gindex, flag))
            return NULL;

        //グリフのコピー取得

        if(FT_Get_Glyph(face->glyph, &glyph))
            return NULL;

/*
        //太字変換

        if(uFlags & INFOFLAG_BOLD)
        {
            if(glyph->format == FT_GLYPH_FORMAT_OUTLINE)
                FT_Outline_Embolden(&((FT_OutlineGlyph)glyph)->outline, 1 << 5);
            else if(glyph->format == FT_GLYPH_FORMAT_BITMAP)
                FT_Bitmap_Embolden(FTLIB, &((FT_BitmapGlyph)glyph)->bitmap, 1 << 6, 0);
        }
*/

        //グリフからビットマップに変換

        if(glyph->format != FT_GLYPH_FORMAT_BITMAP)
        {
            if(FT_Glyph_To_Bitmap(&glyph,
                                  (uFlags & INFOFLAG_2COL)? FT_RENDER_MODE_MONO: FT_RENDER_MODE_NORMAL,
                                  NULL, TRUE))
                return NULL;
        }

        //古いキャッシュ削除

        if(m_pCache->getCnt() >= 40)
            m_pCache->deleteItem(0);

        //キャッシュに追加

        m_pCache->add(pdat = new CFontCacheDat(gindex, glyph, &face->glyph->metrics));
    }

    return pdat;
}


//******************************
// CFontCache
//******************************


//! 検索

CFontCacheDat *CFontCache::find(UINT uIndex)
{
    CFontCacheDat *p;

    for(p = (CFontCacheDat *)m_pTop; p; p = (CFontCacheDat *)p->m_pNext)
    {
        if(p->m_uIndex == uIndex) return p;
    }

    return NULL;
}


//******************************
// CFontCacheDat
//******************************


CFontCacheDat::~CFontCacheDat()
{
    FT_Done_Glyph(m_glyph);
}

CFontCacheDat::CFontCacheDat(UINT index,FT_Glyph glyph,FT_Glyph_Metrics *mt)
{
    m_uIndex    = index;
    m_glyph     = glyph;

    //レイアウト情報

    m_nHorzX    = mt->horiBearingX;
    m_nHorzY    = -mt->horiBearingY;
    m_nHorzNext = mt->horiAdvance;

    m_nVertX    = mt->vertBearingX;
    m_nVertY    = mt->vertBearingY;
    m_nVertNext = mt->vertAdvance;
}
