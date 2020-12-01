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

#include "AXUtilImg.h"

#include "AXFile.h"


/*!
    @defgroup axutilimg AXUtilImg
    @brief 画像関連ユーティリティ

    @ingroup util
    @{
*/


//! ヘッダから画像形式判別
/*!
    @return [-1]エラー ['B']BMP ['P']PNG ['G']GIF ['J']JPEG
*/

int AXGetImageFileType(const AXString &filename)
{
    BYTE dat[8];

    //先頭8バイト読み込み

    if(!AXFile::readFile(filename, dat, 8)) return -1;

    //判別

    return AXGetImageFileType(dat);
}

//! ヘッダのデータから画像タイプ判別（最低8バイト必要）

int AXGetImageFileType(const LPBYTE pBuf)
{
    int ret = -1;

    if(pBuf[0] == 'B' && pBuf[1] == 'M')
        //BMP
        ret = 'B';
    else if(pBuf[0] == 0x89 && pBuf[1] == 0x50 && pBuf[2] == 0x4e && pBuf[3] == 0x47 &&
            pBuf[4] == 0x0d && pBuf[5] == 0x0a && pBuf[6] == 0x1a && pBuf[7] == 0x0a)
        //PNG
        ret = 'P';
    else if(pBuf[0] == 'G' && pBuf[1] == 'I' && pBuf[2] == 'F')
        //GIF
        ret = 'G';
    else if(pBuf[0] == 0xff && pBuf[1] == 0xd8)
        //JPEG
        ret = 'J';

    return ret;
}

//@}
