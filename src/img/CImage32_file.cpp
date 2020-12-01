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

#include "CImage32.h"

#include "AXBMPLoad.h"
#include "AXPNGLoad.h"
#include "AXGIFLoad.h"
#include "AXJPEG.h"
#include "AXMem.h"
#include "AXUtil.h"
#include "AXUtilImg.h"



//============================
//画像読み込み
//============================


/*! ヘッダで判別して読み込み
 *
 * @param pInfo NULL の場合あり */

BOOL CImage32::loadImage(const AXString &filename,CImage32::LOADINFO *pInfo)
{
    int type,ret = FALSE;

    type = AXGetImageFileType(filename);

    if(type == 'B')
        ret = loadBMP(filename, pInfo);
    else if(type == 'P')
        ret = loadPNG(filename, pInfo);
    else if(type == 'J')
        ret = loadJPEG(filename, pInfo);
    else if(type == 'G')
        ret = loadGIF(filename, pInfo);

    if(pInfo) pInfo->type = type;

    return ret;
}

//! BMP読み込み

BOOL CImage32::loadBMP(const AXString &filename,CImage32::LOADINFO *pInfo)
{
    AXBMPLoad bmp;
    int y,i;

    if(!bmp.openFile(filename)) return FALSE;

    if(!create(bmp.m_nWidth, bmp.m_nHeight)) return FALSE;

    for(i = m_nHeight; i > 0; i--)
    {
        y = bmp.readLine();
        if(y < 0) return FALSE;

        bmp.lineTo32bit(getBufPt(0, y));
    }

    bmp.close();

    //

    if(pInfo)
    {
        pInfo->nDPI   = AXDPMtoDPI(bmp.m_nResoH);
        pInfo->bAlpha = FALSE;
    }

    return TRUE;
}

//! PNG読み込み

BOOL CImage32::loadPNG(const AXString &filename,CImage32::LOADINFO *pInfo)
{
    AXPNGLoad png;

    if(!png.loadFile(filename)) return FALSE;

    if(!png.toImage32(this, FALSE)) return FALSE;

    //

    if(pInfo)
    {
        if(png.m_nResoH == -1)
            pInfo->nDPI = 96;
        else
            pInfo->nDPI = AXDPMtoDPI(png.m_nResoH);

        pInfo->bAlpha = (png.m_nBits == 32);
    }

    return TRUE;
}

//! GIF読み込み

BOOL CImage32::loadGIF(const AXString &filename,CImage32::LOADINFO *pInfo)
{
    AXGIFLoad gif;

    if(!gif.loadFile(filename)) return FALSE;

    if(!gif.getNextImage()) return FALSE;

    if(!create(gif.m_nWidth, gif.m_nHeight)) return FALSE;

    gif.to32bit(m_pBuf, FALSE);

    //

    if(pInfo)
    {
        pInfo->nDPI   = 96;
        pInfo->bAlpha = FALSE;
    }

    return TRUE;
}

//! JPEG読み込み

BOOL CImage32::loadJPEG(const AXString &filename,CImage32::LOADINFO *pInfo)
{
    AXJPEG jpg;
    int i,y;

    if(!jpg.openFileLoad(filename)) return FALSE;

    if(!create(jpg.m_nWidth, jpg.m_nHeight)) return FALSE;

    for(i = m_nHeight; i > 0; i--)
    {
        y = jpg.readLine();
        if(y == -1) break;

        jpg.lineTo32bit(getBufPt(0, y));
    }

    jpg.close();

    //

    if(pInfo)
    {
        pInfo->nDPI   = (jpg.m_nDPI_H == -1)? 96: jpg.m_nDPI_H;
        pInfo->bAlpha = FALSE;
    }

    return TRUE;
}


//******************************
//
//******************************


//! 画像からカーソルイメージ作成

BOOL CImage32::createCursor(const AXString &filename,AXMem *pmem)
{
    AXImage32::PIXEL *pSrc;
    LPBYTE pDst,pDstCol,pDstMask;
    int pitchDst,ix,iy,f,xpos;
    DWORD col;

    //読み込み

    if(!loadImage(filename, NULL)) return FALSE;

    //サイズ

    if(m_nWidth > 32 || m_nHeight > 32) return FALSE;

    //

    pitchDst = (m_nWidth + 7) >> 3;

    //確保

    if(!pmem->allocClear(4 + pitchDst * m_nHeight * 2))
        return FALSE;

    pDst = *pmem;

    pDst[0] = m_nWidth;
    pDst[1] = m_nHeight;

    //----- 変換

    pSrc = (AXImage32::PIXEL *)m_pBuf;

    pDstCol  = pDst + 4;
    pDstMask = pDst + 4 + pitchDst * m_nHeight;

    for(iy = 0; iy < m_nHeight; iy++)
    {
        for(ix = 0, f = 1, xpos = 0; ix < m_nWidth; ix++, pSrc++)
        {
            col = (pSrc->c & 0xffffff);

            if(col == 0 || col == 0xffffff)
            {
                if(col == 0) pDstCol[xpos] |= f;
                pDstMask[xpos] |= f;
            }

            f <<= 1;
            if(f == 256) f = 1, xpos++;
        }

        pDstCol  += pitchDst;
        pDstMask += pitchDst;
    }

    return TRUE;
}
