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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "AXFreeType.h"
#include "AXFTFont.h"

#include "AXString.h"
#include "AXByteString.h"

#define FTLIB   (FT_Library)(AXFreeType::m_pSelf->getFTLib())
#define FTFACE  (FT_Face)m_pFace


//********************************
// AXFreeType
//********************************


/*!
    @class AXFreeType
    @brief freetype クラス

    @ingroup etc
*/


AXFreeType *AXFreeType::m_pSelf = NULL;


AXFreeType::AXFreeType()
{
    FT_Library lib;

    m_pSelf = this;

    //初期化

    if(FT_Init_FreeType(&lib))
        m_pFT = NULL;
    else
        m_pFT = (void *)lib;
}

AXFreeType::~AXFreeType()
{
    if(m_pFT)
    {
        FT_Done_FreeType((FT_Library)m_pFT);
        m_pFT = NULL;
    }
}



//********************************
// AXFTFont
//********************************

/*!
    @class AXFTFont
    @brief freetype フォントクラス

    @ingroup etc
*/


AXFTFont::~AXFTFont()
{
    free();
}

AXFTFont::AXFTFont()
{
    m_pFace = NULL;
}

//! 解放

void AXFTFont::free()
{
    if(m_pFace)
    {
        FT_Done_Face((FT_Face)m_pFace);
        m_pFace = NULL;
    }
}

//! ファイルからフォント読込

BOOL AXFTFont::loadFile(AXString &filename,int index)
{
    FT_Face face;
    AXByteString str;

    free();

    //

    filename.toLocal(&str);

    if(FT_New_Face(FTLIB, str, index, &face))
        return FALSE;

    m_pFace = (LPVOID)face;

    return TRUE;
}

//! ptサイズで高さ指定

BOOL AXFTFont::setSizePt(double ptsize,int dpi)
{
    if(m_pFace)
        return (FT_Set_Char_Size(FTFACE, 0, (int)(ptsize * 64 + 0.5), dpi, dpi) == 0);
    else
        return FALSE;
}

//! pxサイズで高さ指定

BOOL AXFTFont::setSizePx(int pxsize)
{
    if(m_pFace)
        return (FT_Set_Pixel_Sizes(FTFACE, 0, pxsize) == 0);
    else
        return FALSE;
}

