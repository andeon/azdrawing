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

#include "AXX11.h"

#include "AXCursor.h"

#include "AXApp.h"
#include "AXAppRes.h"

#include "cursorimg.h"

/*!
    @class AXCursor
    @brief カーソルクラス

    - 色は白・黒＋マスク。

    @ingroup etc
*/


AXCursor::~AXCursor()
{
    free();
}

AXCursor::AXCursor()
{
    m_id = 0;
}

//! 指定タイプのカーソル作成
//! @param type AXCursor::CURSORTYPE

AXCursor::AXCursor(int type)
{
    m_id = 0;

    create(type);
}

//! 解放

void AXCursor::free()
{
    if(m_id)
    {
        ::XFreeCursor(axdisp(), m_id);
        m_id = 0;
    }
}

//! 指定タイプから作成
//! @param type AXCursor::CURSORTYPE

BOOL AXCursor::create(int type)
{
    LPBYTE p = NULL;

    switch(type)
    {
        case SPLIT_H: p = g_cur_hsplit; break;
        case SPLIT_V: p = g_cur_vsplit; break;
    }

    if(p)
        return create(p);
    else
        return FALSE;
}

//! データから作成
/*!
    [0byte] 幅@n
    [1byte] 高さ@n
    [2byte] hotx@n
    [3byte] hoty@n
    [4-]    img(1bit)@n
    [4+(w+7)/8*h] mask(1bit)
*/

BOOL AXCursor::create(const unsigned char *pDat)
{
    Pixmap img,mask;
    XColor col[2];
    int w,h;
    BOOL ret = FALSE;

    free();

    //

    w = pDat[0];
    h = pDat[1];

    //Pixmap作成

    img  = ::XCreateBitmapFromData(axdisp(), axapp->getRootID(), (LPSTR)pDat + 4, w, h);
    mask = ::XCreateBitmapFromData(axdisp(), axapp->getRootID(), (LPSTR)pDat + 4 + ((w + 7) / 8) * h, w, h);

    if(img == None || mask == None) goto END;

    //カーソル作成

    col[0].pixel = axres->colPix(AXAppRes::BLACK);
    col[0].flags = DoRed | DoGreen | DoBlue;
    col[0].red = col[0].green = col[0].blue = 0;

    col[1].pixel = axres->colPix(AXAppRes::WHITE);
    col[1].flags = DoRed | DoGreen | DoBlue;
    col[1].red = col[1].green = col[1]. blue = 0xffff;

    m_id = ::XCreatePixmapCursor(axdisp(), img, mask, col, col + 1, pDat[2], pDat[3]);

    if(m_id != None)
        ret = TRUE;

END:
    if(img != None) ::XFreePixmap(axdisp(), img);
    if(mask != None) ::XFreePixmap(axdisp(), mask);

    return ret;
}
