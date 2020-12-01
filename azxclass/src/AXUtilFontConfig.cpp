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

#include <fontconfig/fontconfig.h>

#include "AXString.h"
#include "AXByteString.h"


/*!
    @defgroup AXUtilFontConfig AXUtilFontConfig
    fontconfig 関連ユーティリティ

    @ingroup util

    @{
*/


//! フォントファミリ名リストを取得
/*!
    FcFontSetDestroy() で解放する。
*/

FcFontSet *AXFCGetFamilyList()
{
    FcObjectSet *os;
    FcPattern *pat;
    FcFontSet *fs;

    os  = FcObjectSetBuild(FC_FAMILY, NULLP);
    pat = FcPatternCreate();

    fs = FcFontList(0, pat, os);

    FcPatternDestroy(pat);
    FcObjectSetDestroy(os);

    return fs;
}

//! 指定ファミリ名のスタイルリスト取得
/*!
    FcFontSetDestroy() で解放する。
*/

FcFontSet *AXFCGetStyleList(const AXString &strFamily)
{
    FcObjectSet *os;
    FcPattern *pat;
    FcFontSet *fs;
    AXByteString strb;

    strFamily.toLocal(&strb);

    os  = FcObjectSetBuild(FC_STYLE, NULLP);
    pat = FcPatternBuild(0, FC_FAMILY, FcTypeString, (LPSTR)strb, NULLP);

    fs = FcFontList(NULL, pat, os);

    FcPatternDestroy(pat);
    FcObjectSetDestroy(os);

    return fs;
}

//! パターンから文字列取得

BOOL AXFCGetPatternString(const FcPattern *pattern,const char *object,AXString *pstr)
{
    FcChar8 *name;

    if(FcPatternGetString(pattern, object, 0, &name) == FcResultMatch)
    {
        pstr->setLocal((LPSTR)name);
        return TRUE;
    }
    else
        return FALSE;
}

//@}
