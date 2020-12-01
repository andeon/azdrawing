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

#ifndef _AZDRAW_LAYERLIST_H_
#define _AZDRAW_LAYERLIST_H_

#include "AXList.h"
#include "AXString.h"
#include "CLayerImg.h"
#include "CImage32.h"

struct LAYERINFO;


class CLayerItem:public AXListItem
{
public:
    enum
    {
        PREVIMG_W   = 26,
        PREVIMG_H   = 29,

        FLAG_VIEW       = 0x01,
        FLAG_PAINT      = 0x02,
        FLAG_MASK       = 0x04,
        FLAG_MASKUNDER  = 0x08,
        FLAG_GROUP      = 0x10,
        FLAG_GROUPUNDER = 0x20,
        FLAG_HILIGHT    = 0x40,
        FLAG_CHECK      = 0x80
    };

public:
    CLayerImg   m_img;
    CImage32    m_imgPrev;

    AXString    m_strName;
    DWORD       m_dwCol;        //! 色
    WORD        m_wAnimCnt;     //! アニメ表示フレーム数

    BYTE        m_btAlpha,      //! 不透明度(0-128)
                m_btFlag,       //! フラグ
                m_btAType,      //! アニメタイプ
                m_bPrevUpdate,  //! プレビュー更新フラグ
                m_bWorkFlag;    //! UNDO用などの作業フラグ

public:
    CLayerItem *next() { return (CLayerItem *)m_pNext; }
    CLayerItem *prev() { return (CLayerItem *)m_pPrev; }

    BOOL isView() { return m_btFlag & FLAG_VIEW; }
    BOOL isMask() { return m_btFlag & FLAG_MASK; }
    BOOL isMaskUnder() { return m_btFlag & FLAG_MASKUNDER; }
    BOOL isGroup() { return m_btFlag & FLAG_GROUP; }
    BOOL isGroupUnder() { return m_btFlag & FLAG_GROUPUNDER; }
    BOOL isCheck() { return m_btFlag & FLAG_CHECK; }
    BOOL isPaint() { return m_btFlag & FLAG_PAINT; }
    BOOL isHilight() { return m_btFlag & FLAG_HILIGHT; }

    BOOL create(int w,int h,int no,LAYERINFO *pInfo);
    BOOL createFromImg(const CLayerImg &src,int no);
    BOOL copy(CLayerItem *pSrc);
    void init(int no);
};

//---------

class CLayerList:public AXList
{
public:
    CLayerItem *getTopItem() { return (CLayerItem *)m_pTop; }
    CLayerItem *getBottomItem() { return (CLayerItem *)m_pBottom; }

    CLayerItem *addLayer(int w,int h,LAYERINFO *pInfo=NULL);
    CLayerItem *copyLayer(CLayerItem *pSrc);
    CLayerItem *addLayerFromImg(const CLayerImg &src);

    void maskOffAll();
    int getWorkFlagOnCnt();
    void clearWorkFlag();
    CLayerItem *getCombineSomeInfo(int type,int *pCnt);
    void toggleShowGroup(CLayerItem *pCur);
    void addOffset_group(CLayerItem *pCur,int addx,int addy,int type);
    CLayerImg *getMaskLayerImg(CLayerItem *pCur);
    CLayerImg *setPaintRefLink(CLayerImg *pCurrent,BOOL bDisable);
    void resizeCanvas(int addx,int addy);
};

#endif
