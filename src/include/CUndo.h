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

#ifndef _AZDRAW_UNDO_H_
#define _AZDRAW_UNDO_H_

#include "AXUndo.h"
#include "AXRect.h"

class CLayerImg;
struct LAYERINFO;


class CUndo:public AXUndo
{
    friend class CUndoDat;

protected:
    BOOL        m_bChange;
    AXRectSize  m_rcsUpdate;
    int         m_nUpdateLayerNo;

protected:
    virtual AXUndoDat *createDat();
    virtual void afterAddUndo();

public:
    CUndo();

    BOOL isChange() const { return m_bChange; }
    void setChangeNone() { m_bChange = FALSE; }

    void getUpdateParam(LPINT pLayerNo,AXRectSize *prcs)
    {
        *pLayerNo = m_nUpdateLayerNo;
        *prcs = m_rcsUpdate;
    }

    void add_curUndoImage(const LAYERINFO &info,const AXRectSize &rcs);
    void add_curLayer();
    void add_multipleLayer(int *pNo,int cnt);

    void add_layerAdd();
    void add_layerDel();
    void add_layerMove(BOOL bUp);
    void add_layerMoveDND(int bkcur);
    void add_layerComb(int *pNo,int cnt);
    void add_layerOffset(int movx,int movy);
    void add_layerHRev();
    void add_layerAllHRev();

    void add_resizeCanvas(int addx,int addy,int imgw,int imgh);
    void add_scaleCanvas();
};

#endif
