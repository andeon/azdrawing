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

#ifndef _AZDRAW_UNDODAT_H_
#define _AZDRAW_UNDODAT_H_

#include "AXUndoDat.h"
#include "AXRect.h"
#include "AXMem.h"
#include "AXString.h"
#include "CUndoDatBuf.h"
#include "defStruct.h"

class CLayerItem;
class CLayerImg;

class CUndoDat:public AXUndoDat
{
    friend class CUndo;

protected:
    enum
    {
        ONETILESIZE = 4096 + 6
    };

    enum UNDOCOMMAND
    {
        CMD_UNDOIMG,
        CMD_UNDOIMG_REDO,
        CMD_LAYER_MULTIPLE,
        CMD_LAYER_ADD,
        CMD_LAYER_DEL,
        CMD_LAYER_MOVEUP,
        CMD_LAYER_MOVEDOWN,
        CMD_LAYER_MOVEDND,
        CMD_LAYER_COMBINE,
        CMD_LAYER_COMBINEREV,
        CMD_LAYER_OFFSET,
        CMD_LAYER_HREV,
        CMD_LAYER_ALLHREV,
        CMD_RESIZE_CANVAS,
        CMD_SCALE_CANVAS
    };

    struct UNDOLAYERINFO
    {
        int     nLayerNo;

        AXString strName;
        DWORD   dwTileCnt,
                dwCol;
        BYTE    btAlpha,
                btFlag;

        LAYERINFO   imginfo;
    };

protected:
    static AXMem m_memWork;     //!< 圧縮作業用

    int         m_nCommand,     //!< コマンドNo
                m_nLayerNo;     //!< レイヤNoまたはレイヤ数
    AXRectSize  m_rcs;          //!< 範囲
    CUndoDatBuf m_dat;

protected:
    CUndoDat(int cmd,int layerno);

    virtual BOOL run(AXUndo *pUndo,BOOL bUndo);
    virtual BOOL setReverseDat(AXUndo *pUndo,AXUndoDat *pSrc,BOOL bUndo);

    void copy(CUndoDat *psrc);

    //

    BOOL writeLayerOffset(int movx,int movy);
    BOOL readLayerOffset();
    BOOL writeRevLayerOffset(CUndoDat *pSrc);

    BOOL writeUndoImage(const LAYERINFO &info);
    BOOL readUndoImage(BOOL bRedo);
    BOOL writeRevUndoImage(CUndoDat *pSrc,BOOL bRedo);

    BOOL readMultipleLayer();
    BOOL writeRevMultipleLayer(CUndoDat *pSrc);

    BOOL writeMultipleLayer(int cnt,int *pNo,BOOL bCombine);
    BOOL readLayer();

    BOOL readRestoreCombine();
    BOOL readReCombine();

    BOOL writeCombToReComb(CUndoDat *pSrc);
    BOOL writeReCombToComb(CUndoDat *pSrc);

    //

    void getLayerInfo(UNDOLAYERINFO *pd,CLayerItem *pItem);
    DWORD getLayerDatSize(CLayerItem *p);

    void readLayerInfo(UNDOLAYERINFO *pinfo);
    void writeOneLayer(CLayerItem *p,const UNDOLAYERINFO &info);
    void readOneLayer(CLayerItem *p,const UNDOLAYERINFO &info);
    void seekTileImg(DWORD tilecnt);
};

#endif
