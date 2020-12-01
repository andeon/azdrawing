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

#ifndef _AX_GIFSAVE_H
#define _AX_GIFSAVE_H

#include "AXMem.h"
#include "AXFile.h"

class AXString;

class AXGIFSave
{
public:
    enum DISPOSALTYPE
    {
        DISPOSAL_NONE    = 0,
        DISPOSAL_STAY    = 1,
        DISPOSAL_FILLBK  = 2,
        DISPOSAL_BACKIMG = 3
    };

    typedef struct
    {
        WORD    wWidth,
                wHeight;
        int     nBits,
                nBKIndex,
                nGlobalPalCnt;
        AXMem   memGlobalPal;
    }GLOBALINFO;

    typedef struct
    {
        WORD    wOffX,
                wOffY,
                wWidth,
                wHeight,
                wDelayTime;
        int     nTPIndex,
                nDisposal,
                nLocalPalCnt;
        AXMem   memLocalPal;
    }IMGINFO;

protected:
    AXFile  m_file;
    AXGIFSave::GLOBALINFO   *m_pGInfo;

protected:
    BOOL _header();
    BOOL _palette(LPDWORD pSrc,int cnt);
    void _block_grpctl(const AXGIFSave::IMGINFO &info);
    BOOL _block_image(const AXGIFSave::IMGINFO &info,LPBYTE pImgBuf);

public:
    ~AXGIFSave();

    BOOL openFile(const AXString &filename,AXGIFSave::GLOBALINFO *pInfo);
    void close();

    BOOL putImg(const AXGIFSave::IMGINFO &info,LPBYTE pImgBuf,BOOL bGrpCtl=FALSE);
    void putLoopBlock(WORD wLoopCnt=0);

    int calcBits(int pcnt);
    void initIMGINFO(AXGIFSave::IMGINFO *pInfo,int w,int h);
    int getColIndex(DWORD col,LPDWORD pPalBuf,int pcnt);
    BOOL convFrom32bit(LPDWORD pSrc,AXGIFSave::IMGINFO *pInfo,AXMem *pmem,LPDWORD pPalette=NULL,int palcnt=0);
};

#endif
