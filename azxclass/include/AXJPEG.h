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

#ifndef _AX_JPEG_H
#define _AX_JPEG_H

#include "AXDef.h"

class AXString;

class AXJPEG
{
public:
    enum SAMPLINGTYPE
    {
        SAMP_444,
        SAMP_422,
        SAMP_411,
        SAMP_DEFAULT = SAMP_411
    };

    struct SAVEINFO
    {
        int     nWidth,
                nHeight,
                nQuality,
                nSamplingType,
                nDPI_H,
                nDPI_V;
        UINT    uFlags;
    };

public:
    int     m_nWidth,
            m_nHeight,
            m_nDPI_H,
            m_nDPI_V;

protected:
    BOOL    m_bEncode;
    LPVOID  m_pInfo,
            m_pErr,
            m_pFile;
    LPBYTE  m_pRawBuf;

protected:
    void _close();
    void _closeErr();
    BOOL _init();
    BOOL _mallocBuf(DWORD size);
    void _setDownsamp(int type);

public:
    ~AXJPEG();
    AXJPEG();

    LPBYTE getImgBuf() const { return m_pRawBuf; }

    void close();

    BOOL openFileLoad(const AXString &filename);
    int readLine();
    void lineTo32bit(LPDWORD pDst);
    BOOL isGrayscale();

    BOOL openFileSave(const AXString &filename,AXJPEG::SAVEINFO *pInfo);
    void putLine();
    void putLineFrom32bit(const DWORD *pSrc);
};

#endif
