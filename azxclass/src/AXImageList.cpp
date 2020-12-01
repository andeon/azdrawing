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

#include "AXImageList.h"

#include "AXImage32.h"
#include "AXImage.h"
#include "AXPixmap.h"
#include "AXGC.h"
#include "AXPNGLoad.h"


/*!
    @class AXImageList
    @brief イメージリスト

    - 横に複数のイメージを並べた一枚絵を扱う。

    @ingroup draw
*/


AXImageList::AXImageList()
{
    m_pimgSrc = m_pimgDisable = NULL;
    m_pimgMask  = NULL;
    m_pGC       = NULL;
    m_nOneW     = 1;
    m_nMaxCnt   = 0;
}

AXImageList::~AXImageList()
{
    free();
}

//! 解放

void AXImageList::free()
{
    _DELETE(m_pimgSrc);
    _DELETE(m_pimgMask);
    _DELETE(m_pimgDisable);
    _DELETE(m_pGC);
}

//! 高さ取得

int AXImageList::getHeight() const
{
    return (m_pimgSrc)? m_pimgSrc->getHeight(): 0;
}

//! AXImage32 を元に作成

BOOL AXImageList::create(const AXImage32 &imgSrc,int onew,int colMask,BOOL bDisable)
{
    free();

    //マスク (1bit)

    if(colMask >= 0)
    {
        m_pimgMask = new AXPixmap;

        if(!m_pimgMask->createMask(imgSrc, colMask)) return FALSE;

        //GC

        m_pGC = new AXGC;
        m_pGC->createClipMask(m_pimgMask->getid());
    }

    //無効用

    if(bDisable)
    {
        m_pimgDisable = new AXImage;

        if(!m_pimgDisable->createDisableImg(imgSrc)) return FALSE;
    }

    //ソース

    m_pimgSrc = new AXImage;

    if(!m_pimgSrc->createFromImage32(imgSrc)) return FALSE;

    //

    m_nOneW   = (onew < 1)? 1: onew;
    m_nMaxCnt = m_pimgSrc->getWidth() / m_nOneW;

    return TRUE;
}

//! PNGファイルから読み込み
/*!
    @param onew 一つの幅
    @param colMask マスク色（マイナスでなし）
    @param bDisable 無効時のイメージを作るか
*/

BOOL AXImageList::loadPNG(const AXString &filename,int onew,int colMask,BOOL bDisable)
{
    AXPNGLoad png;
    AXImage32 img;

    if(!png.loadFile(filename)) return FALSE;

    if(!png.toImage32(&img, FALSE)) return FALSE;

    png.free();

    return create(img, onew, colMask, bDisable);
}

//! PNGをバッファから読み込み

BOOL AXImageList::loadPNG(LPBYTE pBuf,DWORD dwSize,int onew,int colMask,BOOL bDisable)
{
    AXPNGLoad png;
    AXImage32 img;

    if(!png.loadBuf(pBuf, dwSize)) return FALSE;

    if(!png.toImage32(&img, FALSE)) return FALSE;

    png.free();

    return create(img, onew, colMask, bDisable);
}

//! 転送

void AXImageList::put(UINT dstID,int dx,int dy,int no,BOOL bDisable)
{
    AXImage *pimg;

    if(no >= 0 && no < m_nMaxCnt)
    {
        pimg = (bDisable && m_pimgDisable)? m_pimgDisable: m_pimgSrc;

        if(m_pGC)
        {
            //マスクあり

            m_pGC->setClipOrigin(dx - no * m_nOneW, dy);
            pimg->put(dstID, *m_pGC, dx, dy, no * m_nOneW, 0, m_nOneW, m_pimgSrc->getHeight());
        }
        else
            pimg->put(dstID, dx, dy, no * m_nOneW, 0, m_nOneW, m_pimgSrc->getHeight());
    }
}
