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

#ifndef _AX_PSDSAVE_H
#define _AX_PSDSAVE_H

#include "AXFile.h"
#include "AXMem.h"
#include "AXPSDdef.h"

class AXString;
class AXRectSize;

class AXPSDSave
{
public:
    struct INFO
    {
        int     nWidth,
                nHeight,
                nImgCh,
                nBits,
                nColMode;
    };

    struct LAYERINFO
    {
        int     nTop,
                nLeft,
                nBottom,
                nRight;
        DWORD   dwBlend;
        LPCSTR  szName;
        BYTE    btOpacity,
                bHide;
    };

    enum IMGCH
    {
        IMGCH_GRAY  = 1,
        IMGCH_RGB   = 3,
        IMGCH_RGBA  = 4
    };

protected:
    struct LAYERWORKINFO
    {
        DWORD   dwChSizeFilePos[4];
        int     nLeft,
                nTop,
                nWidth,
                nHeight;
    };

protected:
    AXFile  m_file;
    INFO    *m_pinfo;

    int     m_nLayerCnt,
            m_nChNo,
            m_nNowY,
            m_nMaxWidth,
            m_nMaxHeight;
    DWORD   m_dwFilePos,
            m_dwFilePosCh,
            m_dwOutSize;
    AXMem   m_memLayerInfo,
            m_memLineSize,
            m_memLineSrc,
            m_memOut;

    LAYERWORKINFO   *m_pLayerWork;

protected:
    void _write_header();

    int _encPackBits(int length);
    int _writeRLE(LPBYTE pDat,int cnt,int type);
    void _writeOutBuf();

public:
    LPBYTE getLineBuf() { return (LPBYTE)m_memLineSrc; }

    BOOL openFile(const AXString &filename,INFO *pInfo);
    void close();

    void beginRes();
    void endRes();
    void writeRes_resolution(int nDPIH,int nDPIV);
    void writeRes_curlayer(int no);

    //

    BOOL beginLayer(int nCnt);
    void endLayer();

    void writeLayerInfo(LAYERINFO *pInfo);

    BOOL beginLayerImageTop();
    BOOL beginLayerImageEach(AXRectSize *prcs=NULL);

    void beginLayerImageCh();
    void writeLayerImageChLine();
    void endLayerImageCh();

    //

    BOOL beginImage();
    void beginImageCh();
    void writeImageChLine();
    void endImageCh();
};

#endif
