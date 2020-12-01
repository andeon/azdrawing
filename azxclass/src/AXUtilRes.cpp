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
#include <stdio.h>

#include "AXUtilRes.h"

#include "AXString.h"
#include "AXByteString.h"
#include "AXFile.h"
#include "AXPNGLoad.h"
#include "AXImageList.h"
#include "AXTarBall.h"
#include "AXApp.h"


//---------------------

void axutilres_error(AXString &filename)
{
    AXByteString str;

    fprintf(stderr, "! can't load \"%s\"\n", filename.toLocal(&str));
    exit(1);
}

void axutilres_tar_error(LPCSTR szFileName)
{
    fprintf(stderr, "! can't load \"%s\" in tar\n", szFileName);
    exit(1);
}

//---------------------

/*!
    @defgroup axutilres AXUtilRes
    @brief リソースファイル（画像など）読み込みユーティリティ

    - ファイル名は、AXApp でセットされているリソースディレクトリから検索される。
    - 画像形式は PNG。
    - 読み込めなかった場合（アイコン画像は除く）は exit(1) で終了する。

    @ingroup util
    @{
*/


//! アプリケーションアイコンイメージを読み込み

BOOL AXLoadResAppIcon(AXMem *pmem,LPCSTR szFileName)
{
    AXPNGLoad png;
    AXString str;

    axapp->getResourcePath(&str, szFileName);

    if(!png.loadFile(str)) return FALSE;

    if(!png.toAppIconImg(pmem)) return FALSE;

    return TRUE;
}

//! イメージリスト画像読み込み

void AXLoadResImageList(AXImageList *pimg,LPCSTR szFileName,int onew,int colMask,BOOL bDisable)
{
    AXString str;

    axapp->getResourcePath(&str, szFileName);

    if(!pimg->loadPNG(str, onew, colMask, bDisable))
        axutilres_error(str);
}

//! AXImage32 に画像読み込み

void AXLoadResImage32(AXImage32 *pimg,LPCSTR szFileName)
{
    AXString str;
    AXPNGLoad png;
    BOOL bErr = TRUE;

    axapp->getResourcePath(&str, szFileName);

    if(png.loadFile(str))
    {
        if(png.toImage32(pimg, FALSE))
            bErr = FALSE;
    }

    if(bErr) axutilres_error(str);
}


//============================


//! tar からイメージリスト画像読み込み

void AXLoadTarResImageList(AXTarBall *ptar,AXImageList *pimg,LPCSTR szFileName,int onew,int colMask,BOOL bDisable)
{
    LPBYTE pbuf;
    DWORD size;
    BOOL bErr = TRUE;

    pbuf = ptar->findFile(szFileName, &size);

    if(pbuf)
    {
        if(pimg->loadPNG(pbuf, size, onew, colMask, bDisable))
            bErr = FALSE;
    }

    if(bErr)
        axutilres_tar_error(szFileName);
}

//! tarから AXImage32 に画像読み込み

void AXLoadTarResImage32(AXTarBall *ptar,AXImage32 *pimg,LPCSTR szFileName)
{
    LPBYTE pbuf;
    DWORD size;
    AXPNGLoad png;
    BOOL bErr = TRUE;

    pbuf = ptar->findFile(szFileName, &size);

    if(pbuf)
    {
        if(png.loadBuf(pbuf, size))
        {
            if(png.toImage32(pimg, FALSE))
                bErr = FALSE;
        }
    }

    if(bErr)
        axutilres_tar_error(szFileName);
}

//@}
