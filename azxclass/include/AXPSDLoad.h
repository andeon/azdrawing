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

#ifndef _AX_PSDLOAD_H
#define _AX_PSDLOAD_H

#include "AXFile.h"
#include "AXMem.h"
#include "AXPSDdef.h"

class AXString;
class AXRectSize;

class AXPSDLoad
{
public:
    enum ERRCODE
    {
        ERR_SUCCESS,
        ERR_OPENFILE,
        ERR_MEMORY,
        ERR_FILEDAT,
        ERR_FORMAT,
        ERR_DECODE,
        ERR_NO_CHANNEL
    };

    enum CHANNELID
    {
        CHANNELID_R         = 0,
        CHANNELID_GRAYSCALE = 0,
        CHANNELID_G         = 1,
        CHANNELID_B         = 2,
        CHANNELID_ALPHA     = 0xffff,
        CHANNELID_LAYERMASK = 0xfffe
    };

    struct LAYERINFO
    {
        int     nTop,
                nLeft,
                nBottom,
                nRight;
        DWORD   dwBlend;
        BYTE    btOpacity,
                bHide;
        LPCSTR  szName;
    };

protected:
    struct LAYERWORKINFO
    {
        int     nLeft,
                nTop,
                nWidth,
                nHeight,
                nChCnt;
        WORD    wChID[5];
        DWORD   dwChSize[5];
    };

protected:
    AXFile  m_file;
    AXMem   m_memBuf,
            m_memLine,
            m_memLineSize,
            m_memLayerWork;

    int     m_nImgChCnt,
            m_nImgW,
            m_nImgH,
            m_nBits,
            m_nColMode,
            m_nLayerCnt;

    DWORD   m_dwBlockSize,
            m_dwFilePos,
            m_dwFilePosImg,
            m_dwBufSize,
            m_dwBufPos,
            m_dwChSize;
    int     m_nMaxWidth,
            m_nMaxHeight,
            m_nChNo,
            m_nNowY,
            m_nCompType;

    LAYERWORKINFO   *m_pLayerWork;

protected:
    int _read_header();
    BOOL _search_res(WORD wID,DWORD *pSize);
    int _read_ch_buf();
    BOOL _decPackBits(int dstsize,int srcsize);
    int _read_lineSize(int height,DWORD *pChSize=NULL);
    int _read_chLine(int width);

public:
    int getImgWidht() const { return m_nImgW; }
    int getImgHeight() const { return m_nImgH; }
    int getImgChCnt() const { return m_nImgChCnt; }
    int getBits() const { return m_nBits; }
    int getColMode() const { return m_nColMode; }
    int getLayerCnt() const { return m_nLayerCnt; }
    LPBYTE getLineBuf() { return (LPBYTE)m_memLine; }

    int openFile(const AXString &filename);
    void close();

    int beginRes();
    void endRes();
    BOOL readRes_resolution(int *pDPIH,int *pDPIV);
    int readRes_curlayer();

    int beginLayer();
    void endLayer();

    int readLayerInfo(LAYERINFO *pInfo);

    int beginLayerImageTop();
    BOOL beginLayerImageEach(AXRectSize *prcs);

    int beginLayerImageCh(WORD chID);
    int readLayerImageChLine();

    int beginImage();
    int beginImageCh();
    void skipImageCh();
    int readImageChLine();
};

#endif
