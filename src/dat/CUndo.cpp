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

#include "CUndo.h"

#include "CUndoDat.h"
#include "CDraw.h"
#include "defGlobal.h"


/*!
    @class CUndo
    @brief アンドゥ管理クラス
*/


CUndo::CUndo()
{
    m_bChange = FALSE;
}

//! データ作成

AXUndoDat *CUndo::createDat()
{
    return new CUndoDat(0, 0);
}

//! アンドゥデータ追加後

void CUndo::afterAddUndo()
{
    m_bChange = TRUE;
}


//=========================


//! カレントレイヤのUNDO用イメージ
/*!
    @param pinfo 描画前の画像情報
    @param rcs   更新範囲
*/

void CUndo::add_curUndoImage(const LAYERINFO &info,const AXRectSize &rcs)
{
    CUndoDat *p;

    addDat(p = new CUndoDat(CUndoDat::CMD_UNDOIMG, -1));

    p->m_rcs = rcs;
    p->writeUndoImage(info);
}

//! カレントレイヤ全体

void CUndo::add_curLayer()
{
    CUndoDat *p;
    int no;

    addDat(p = new CUndoDat(CUndoDat::CMD_LAYER_MULTIPLE, 1));

    no = g_pdraw->m_nCurLayerNo;
    p->writeMultipleLayer(1, &no, FALSE);
}

//! 複数レイヤイメージ

void CUndo::add_multipleLayer(int *pNo,int cnt)
{
    CUndoDat *p;

    addDat(p = new CUndoDat(CUndoDat::CMD_LAYER_MULTIPLE, cnt));

    p->writeMultipleLayer(cnt, pNo, FALSE);
}


//! レイヤ追加時（※追加後に実行）

void CUndo::add_layerAdd()
{
    addDat(new CUndoDat(CUndoDat::CMD_LAYER_DEL, -1));
}

//! レイヤ削除時

void CUndo::add_layerDel()
{
    CUndoDat *p;
    int n;

    addDat(p = new CUndoDat(CUndoDat::CMD_LAYER_ADD, -1));

    n = p->m_nLayerNo;
    p->writeMultipleLayer(1, &n, FALSE);
}

//! レイヤ移動時

void CUndo::add_layerMove(BOOL bUp)
{
    addDat(new CUndoDat(bUp? CUndoDat::CMD_LAYER_MOVEDOWN: CUndoDat::CMD_LAYER_MOVEUP, -1));
}

//! レイヤD&D移動（移動した後）

void CUndo::add_layerMoveDND(int bkcur)
{
    CUndoDat *p;

    addDat(p = new CUndoDat(CUndoDat::CMD_LAYER_MOVEDND, -1));

    p->m_rcs.x = bkcur + ((bkcur > g_pdraw->m_nCurLayerNo)? 1: 0);
}

//! レイヤ結合

void CUndo::add_layerComb(int *pNo,int cnt)
{
    CUndoDat *p;

    addDat(p = new CUndoDat(CUndoDat::CMD_LAYER_COMBINE, cnt));

    p->writeMultipleLayer(cnt, pNo, TRUE);
}

//! レイヤオフセット位置移動
/*!
    レイヤの作業用フラグがONのレイヤが対象
*/

void CUndo::add_layerOffset(int movx,int movy)
{
    CUndoDat *p;

    addDat(p = new CUndoDat(CUndoDat::CMD_LAYER_OFFSET, 0));

    p->writeLayerOffset(movx, movy);
}

//! カレントレイヤ左右反転

void CUndo::add_layerHRev()
{
    addDat(new CUndoDat(CUndoDat::CMD_LAYER_HREV, -1));
}

//! 全レイヤ左右反転

void CUndo::add_layerAllHRev()
{
    addDat(new CUndoDat(CUndoDat::CMD_LAYER_ALLHREV, 0));
}

//! キャンバスサイズ変更

void CUndo::add_resizeCanvas(int addx,int addy,int imgw,int imgh)
{
    CUndoDat *p;

    addDat(p = new CUndoDat(CUndoDat::CMD_RESIZE_CANVAS, 0));

    p->m_rcs.x = -addx;
    p->m_rcs.y = -addy;
    p->m_rcs.w = imgw;
    p->m_rcs.h = imgh;
}

//! キャンバス拡大縮小

void CUndo::add_scaleCanvas()
{
    CUndoDat *p;

    addDat(p = new CUndoDat(CUndoDat::CMD_SCALE_CANVAS, g_pdraw->getLayerCnt()));

    p->m_rcs.w = g_pdraw->m_nImgW;
    p->m_rcs.h = g_pdraw->m_nImgH;
    p->m_rcs.x = g_pdraw->m_nImgDPI;

    p->writeMultipleLayer(p->m_nLayerNo, NULL, FALSE);
}
