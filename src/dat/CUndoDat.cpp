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

#include "CUndoDat.h"

#include "CUndo.h"
#include "CDraw.h"
#include "CLayerList.h"
#include "CTileImg.h"
#include "defGlobal.h"


/*!
    @class CUndoDat
    @brief アンドゥデータクラス
*/


AXMem CUndoDat::m_memWork(4096);  //圧縮作業用


//! 作成
/*!
    @param layerno マイナスでカレントレイヤの番号
*/

CUndoDat::CUndoDat(int cmd,int layerno)
{
    m_nCommand = cmd;
    m_nLayerNo = (layerno < 0)? g_pdraw->m_nCurLayerNo: layerno;
}

//! 実行
/*
    m_rcsUpdate.x == -1 で全体更新。layerno の値でさらに詳細。
      [0] 全体イメージ＆キャンバス更新
      [1] 全体＆レイヤ一覧更新
      [2] キャンバスサイズ変更 (w,h にサイズが入る)
      [3] キャンバスサイズ変更 (w,h にサイズが入る)＆DPI変更

    m_rcsUpdate.x が -1 でなければ、範囲更新。
    ただし、y が -1 の場合は更新はしない（キャンバス範囲外の場合）

    ※[指定レイヤ＆更新範囲][キャンバスサイズ変更]の場合はレイヤプレビューが更新されるが
    　他の場合は更新されないので、それぞれでフラグをONにしておくこと
*/

BOOL CUndoDat::run(AXUndo *pUndo,BOOL bUndo)
{
    CUndo *pundo = (CUndo *)pUndo;
    CLayerItem *p;

    pundo->m_rcsUpdate.x    = -1;
    pundo->m_nUpdateLayerNo = 1;

    switch(m_nCommand)
    {
        //アンドゥイメージ
        case CMD_UNDOIMG:
        case CMD_UNDOIMG_REDO:
            pundo->m_nUpdateLayerNo = m_nLayerNo;
            pundo->m_rcsUpdate = m_rcs;

            //x == -1 の場合は範囲外のため y = -1 とする

            if(m_rcs.x == -1)
                pundo->m_rcsUpdate.x = 0, pundo->m_rcsUpdate.y = -1;

            return readUndoImage((m_nCommand == CMD_UNDOIMG_REDO));
        //複数レイヤ
        case CMD_LAYER_MULTIPLE:
            return readMultipleLayer();

        //レイヤ追加
        case CMD_LAYER_ADD:
            return readLayer();
        //レイヤ削除
        case CMD_LAYER_DEL:
            g_pdraw->changeCurLayer(m_nLayerNo);
            g_pdraw->layer_del(FALSE);
            break;
        //レイヤ上移動
        case CMD_LAYER_MOVEUP:
            g_pdraw->changeCurLayer(m_nLayerNo);
            g_pdraw->layer_moveUp(FALSE);
            break;
        //レイヤ下移動
        case CMD_LAYER_MOVEDOWN:
            g_pdraw->changeCurLayer(m_nLayerNo);
            g_pdraw->layer_moveDown(FALSE);
            break;
		//レイヤ移動(D&D) [m_rcs.x : 移動位置]
		case CMD_LAYER_MOVEDND:
			g_pdraw->changeCurLayer(m_nLayerNo);
			g_pdraw->layer_moveDND(m_rcs.x, FALSE);
			break;
        //レイヤ結合（戻す）
        case CMD_LAYER_COMBINE:
            readRestoreCombine();
            break;
        //レイヤ結合（再結合）
        case CMD_LAYER_COMBINEREV:
            readReCombine();
            break;
        //レイヤオフセット移動
        case CMD_LAYER_OFFSET:
            pundo->m_nUpdateLayerNo = 0;
            return readLayerOffset();
        //レイヤ左右反転
        case CMD_LAYER_HREV:
            p = g_pdraw->getLayer(m_nLayerNo);
            if(p)
            {
                p->m_img.reverseHorzFull();
                p->m_bPrevUpdate = TRUE;
            }

            pundo->m_nUpdateLayerNo = 0;
            break;
        //全レイヤ左右反転
        case CMD_LAYER_ALLHREV:
            for(p = g_pdraw->m_player->getTopItem(); p; p = p->next())
                p->m_img.reverseHorzFull();

            //レイヤプレビューをすべて更新
            g_pdraw->updateLayerPrevAll(FALSE);
            break;
        //キャンバスサイズ変更
        case CMD_RESIZE_CANVAS:
            g_pdraw->m_player->resizeCanvas(m_rcs.x, m_rcs.y);

            pundo->m_nUpdateLayerNo = 2;
            pundo->m_rcsUpdate.w    = m_rcs.w;
            pundo->m_rcsUpdate.h    = m_rcs.h;
            break;
        //キャンバス拡大縮小
        case CMD_SCALE_CANVAS:
            g_pdraw->m_nImgDPI = m_rcs.x;

            pundo->m_nUpdateLayerNo = 3;
            pundo->m_rcsUpdate.w    = m_rcs.w;
            pundo->m_rcsUpdate.h    = m_rcs.h;

            return readMultipleLayer();
    }

    return TRUE;
}

//! 逆のデータセット

BOOL CUndoDat::setReverseDat(AXUndo *pUndo,AXUndoDat *pSrc,BOOL bUndo)
{
    int n;

    copy((CUndoDat *)pSrc);

    switch(m_nCommand)
    {
        //アンドゥイメージ
        case CMD_UNDOIMG:
            m_nCommand = CMD_UNDOIMG_REDO;
            return writeRevUndoImage((CUndoDat *)pSrc, TRUE);
        case CMD_UNDOIMG_REDO:
            m_nCommand = CMD_UNDOIMG;
            return writeRevUndoImage((CUndoDat *)pSrc, FALSE);
        //複数レイヤ
        case CMD_LAYER_MULTIPLE:
            return writeRevMultipleLayer((CUndoDat *)pSrc);

        //レイヤ追加
        case CMD_LAYER_ADD:
            m_nCommand = CMD_LAYER_DEL;
            break;
        //レイヤ削除
        case CMD_LAYER_DEL:
            m_nCommand = CMD_LAYER_ADD;

            n = m_nLayerNo;
            return writeMultipleLayer(1, &n, FALSE);

        //レイヤ上移動
        case CMD_LAYER_MOVEUP:
            m_nCommand = CMD_LAYER_MOVEDOWN;
            m_nLayerNo--;
            break;
        //レイヤ下移動
        case CMD_LAYER_MOVEDOWN:
            m_nCommand = CMD_LAYER_MOVEUP;
            m_nLayerNo++;
            break;
		//レイヤ移動(D&D)
		case CMD_LAYER_MOVEDND:
            n = m_rcs.x;

            if(m_nLayerNo > n)
            {
                m_rcs.x     = m_nLayerNo + 1;
                m_nLayerNo  = n;
            }
            else
            {
                m_rcs.x     = m_nLayerNo;
                m_nLayerNo  = n - 1;
            }
            break;
        //レイヤ結合（戻す）
        case CMD_LAYER_COMBINE:
            m_nCommand = CMD_LAYER_COMBINEREV;
            return writeCombToReComb((CUndoDat *)pSrc);
        //レイヤ結合（再結合）
        case CMD_LAYER_COMBINEREV:
            m_nCommand = CMD_LAYER_COMBINE;
            return writeReCombToComb((CUndoDat *)pSrc);
        //レイヤオフセット移動
        case CMD_LAYER_OFFSET:
            return writeRevLayerOffset((CUndoDat *)pSrc);
        //キャンバスサイズ変更
        case CMD_RESIZE_CANVAS:
            m_rcs.x = -m_rcs.x;
            m_rcs.y = -m_rcs.y;
            m_rcs.w = g_pdraw->m_nImgW;
            m_rcs.h = g_pdraw->m_nImgH;
            break;
        //キャンバス拡大縮小
        case CMD_SCALE_CANVAS:
            m_rcs.w = g_pdraw->m_nImgW;
            m_rcs.h = g_pdraw->m_nImgH;
            m_rcs.x = g_pdraw->m_nImgDPI;

            return writeRevMultipleLayer((CUndoDat *)pSrc);
    }

    return TRUE;
}

//! 情報コピー

void CUndoDat::copy(CUndoDat *psrc)
{
    m_nCommand = psrc->m_nCommand;
    m_nLayerNo = psrc->m_nLayerNo;
    m_rcs      = psrc->m_rcs;
}



//=============================
// レイヤ・オフセット位置移動
//=============================
/*
    int,int	: 相対移動数(x,y)
    WORD...	: レイヤ番号(0xffffで終了)
*/


//! 書き込み

BOOL CUndoDat::writeLayerOffset(int movx,int movy)
{
    CLayerItem *p;
    int cnt;
    WORD no;

    cnt = g_pdraw->m_player->getWorkFlagOnCnt();

    //確保

    if(!m_dat.alloc(8 + (cnt + 1) * 2)) return FALSE;

    //------- データセット

    if(!m_dat.openWrite()) return FALSE;

    //相対移動数

    movx = -movx;
    movy = -movy;

    m_dat.write(&movx, 4);
    m_dat.write(&movy, 4);

    //レイヤ番号

    for(p = g_pdraw->m_player->getBottomItem(), no = 0; p; p = p->prev(), no++)
    {
        if(p->m_bWorkFlag)
            m_dat.write(&no, 2);
    }

    //終了

    no = 0xffff;
    m_dat.write(&no, 2);

    m_dat.closeWrite(FALSE);

    return TRUE;
}

//! オフセット移動・復元

BOOL CUndoDat::readLayerOffset()
{
    CLayerItem *p;
    int movx,movy;
    WORD no;

    if(!m_dat.openRead()) return FALSE;

    //相対移動数

    m_dat.read(&movx, 4);
    m_dat.read(&movy, 4);

    //レイヤ番号

    while(1)
    {
        m_dat.read(&no, 2);
        if(no == 0xffff) break;

        p = g_pdraw->getLayer(no);
        if(p)
        {
            p->m_img.setOffset(movx, movy, TRUE);
            p->m_bPrevUpdate = TRUE;
        }
    }

    m_dat.closeRead();

    return TRUE;
}

//! オフセット移動・反転書き込み

BOOL CUndoDat::writeRevLayerOffset(CUndoDat *pSrc)
{
	int movx,movy;
	WORD no;

	//確保

	if(!m_dat.alloc(pSrc->m_dat.getSize())) return FALSE;

	//---------

	if(!m_dat.openWrite()) return FALSE;
	if(!pSrc->m_dat.openRead()) { m_dat.closeWrite(FALSE); return FALSE; }

	//相対移動数

	pSrc->m_dat.read(&movx, 4);
	pSrc->m_dat.read(&movy, 4);

	movx = -movx;
	movy = -movy;

	m_dat.write(&movx, 4);
	m_dat.write(&movy, 4);

	//レイヤ番号

	while(1)
	{
		pSrc->m_dat.read(&no, 2);
		m_dat.write(&no, 2);

		if(no == 0xffff) break;
	}

	pSrc->m_dat.closeRead();
	m_dat.closeWrite(FALSE);

	return TRUE;
}


//=============================
// アンドゥ用イメージ
//=============================
/*
    [LAYERINFO] 描画前の画像情報
    [DWORD] 元が空でないタイル数
    [DWORD] 元が空のタイル数
    [タイル...]
        ※サイズは(REDO時)最上位ビットONで元が空
*/


//! アンドゥイメージ書き込み

BOOL CUndoDat::writeUndoImage(const LAYERINFO &info)
{
    CLayerImg *pimg = g_pdraw->m_pimgUndo;
    DWORD allsize,tcnt[2];
    CTileImg **ppTile;
    WORD size,x,y,xcnt,ycnt;
    LPBYTE pWorkBuf = m_memWork;

    //--------- サイズ計算

    allsize	= sizeof(LAYERINFO) + 8;
    allsize	+= pimg->getUndoWriteSize(tcnt);

    //--------- 書き込み

    if(!m_dat.alloc(allsize)) return FALSE;
    if(!m_dat.openWrite()) return FALSE;

    //画像情報（描画前）

    m_dat.write(&info, sizeof(LAYERINFO));

    //タイル数

    m_dat.write(tcnt, 8);

    //イメージ

    ppTile  = pimg->getTileBuf();
    xcnt    = pimg->getTileXCnt();
    ycnt    = pimg->getTileYCnt();

    for(y = 0; y < ycnt; y++)
    {
        for(x = 0; x < xcnt; x++, ppTile++)
        {
            if(*ppTile)
            {
                m_dat.write(&x, 2);
                m_dat.write(&y, 2);

                if(*ppTile == (CTileImg *)CLayerImg::TILEPT_EMPTY)
                {
                    //ポインタが TILEPT_EMPTY で元データが空 -> サイズを0

                    size = 0;
                    m_dat.write(&size, 2);
                }
                else
                {
                    size = (*ppTile)->encodePackBits(pWorkBuf);

                    m_dat.write(&size, 2);
                    m_dat.write(pWorkBuf, size);
                }
            }
        }
    }

    m_dat.closeWrite(TRUE);

    return TRUE;
}

//! アンドゥイメージから復元

BOOL CUndoDat::readUndoImage(BOOL bRedo)
{
    LAYERINFO info;
    CLayerItem *p;
    DWORD i,tcnt[2];
    WORD x,y,size;
    BOOL ret = FALSE;
    LPBYTE pWorkBuf = m_memWork;

    p = g_pdraw->getLayer(m_nLayerNo);

    if(!m_dat.openRead()) return FALSE;

    //画像情報

    m_dat.read(&info, sizeof(LAYERINFO));

    //タイル数

    m_dat.read(tcnt, 8);

    //配列リサイズ（リドゥ時は先に行う）

    if(bRedo)
    {
        if(!p->m_img.resizeTile(info.nTileTopX, info.nTileTopY, info.nTileXCnt, info.nTileYCnt))
            goto END;
    }

    //イメージ

    for(i = tcnt[0] + tcnt[1]; i > 0; i--)
    {
        m_dat.read(&x, 2);
        m_dat.read(&y, 2);
        m_dat.read(&size, 2);

        size &= 0x7fff;

        if(size == 0)
            p->m_img.freeTilePos(x, y);
        else
        {
            m_dat.read(pWorkBuf, size);
            p->m_img.setTileFromUndo(x, y, pWorkBuf, size);
        }
    }

    //配列リサイズ（アンドゥ時は読み込み後に）

    if(!bRedo)
        p->m_img.resizeTile(info.nTileTopX, info.nTileTopY, info.nTileXCnt, info.nTileYCnt);

    ret = TRUE;

END:
    m_dat.closeRead();

    return ret;
}

//! アンドゥイメージ反転書き込み

BOOL CUndoDat::writeRevUndoImage(CUndoDat *pSrc,BOOL bRedo)
{
    LAYERINFO info;
    CLayerItem *p;
    CTileImg *ptile;
    DWORD i,tcnt[2],allsize;
    WORD x,y,size,size2;
    int stopx,stopy;
    BOOL ret = FALSE;
    LPBYTE pWorkBuf = m_memWork;

    p = g_pdraw->getLayer(m_nLayerNo);

    //------------- サイズ計算

    if(!pSrc->m_dat.openRead()) return FALSE;

    pSrc->m_dat.read(&info, sizeof(LAYERINFO));
    pSrc->m_dat.read(tcnt, 8);

    allsize = sizeof(LAYERINFO) + 8;

    if(bRedo)
        allsize += (tcnt[0] + tcnt[1]) * ONETILESIZE;
    else
        allsize += tcnt[0] * ONETILESIZE + tcnt[1] * 6;

    stopx = info.nTileTopX;
    stopy = info.nTileTopY;

    //------------- 書き込み

    if(!m_dat.alloc(allsize)) goto END;
    if(!m_dat.openWrite()) goto END;

    //情報

    p->m_img.getInfo(&info);

    m_dat.write(&info, sizeof(LAYERINFO));
    m_dat.write(tcnt, 8);

    //イメージ

    stopx = info.nTileTopX - stopx;
    stopy = info.nTileTopY - stopy;

    for(i = tcnt[0] + tcnt[1]; i > 0; i--)
    {
        pSrc->m_dat.read(&x, 2);
        pSrc->m_dat.read(&y, 2);
        pSrc->m_dat.read(&size, 2);
        pSrc->m_dat.seekRead(size & 0x7fff);

        m_dat.write(&x, 2);
        m_dat.write(&y, 2);

        if(bRedo)
        {
            //------- UNDO -> REDO
            //※イメージはまだUNDOされていない

            ptile = p->m_img.getTile(x, y);

            if(!ptile)
                size2 = 0;      //消しゴム時は空の場合あり
            else
                size2 = ptile->encodePackBits(pWorkBuf);

            //元が空の場合はフラグON
            size = size2 | ((size == 0)? 0x8000: 0);

            m_dat.write(&size, 2);
            if(size2) m_dat.write(pWorkBuf, size2);
        }
        else
        {
            //------ REDO -> UNDO : 元が空なら空データ
            //※イメージはまだUNDO後のままの状態

            if(size & 0x8000)
            {
                size2 = 0;
                m_dat.write(&size2, 2);
            }
            else
            {
                ptile = p->m_img.getTile(x - stopx, y - stopy);

                if(!ptile)
                    size2 = 0;
                else
                    size2 = ptile->encodePackBits(pWorkBuf);

                m_dat.write(&size2, 2);
                if(size2) m_dat.write(pWorkBuf, size2);
            }
        }
    }

    m_dat.closeWrite(TRUE);

    ret = TRUE;

END:
    pSrc->m_dat.closeRead();

    return ret;
}


//============================
// 複数レイヤ
//============================


//! 複数レイヤ復元
//[m_nLayerNo] レイヤ数

BOOL CUndoDat::readMultipleLayer()
{
    CLayerItem *p;
    UNDOLAYERINFO info;
    int i;
    BOOL ret = FALSE;

    if(!m_dat.openRead()) return FALSE;

    for(i = 0; i < m_nLayerNo; i++)
    {
        //情報読み込み

        readLayerInfo(&info);

        //レイヤ取得＆再作成

        p = g_pdraw->getLayer(info.nLayerNo);

        if(!p->m_img.createFromInfo(info.imginfo)) goto END;

        //イメージ

        readOneLayer(p, info);
    }

    ret = TRUE;

END:
    m_dat.closeRead();

    return ret;
}

//! 複数レイヤ・逆のデータセット

BOOL CUndoDat::writeRevMultipleLayer(CUndoDat *pSrc)
{
    CLayerItem *p;
    UNDOLAYERINFO info;
    int i;
    DWORD allsize = 0;
    BOOL ret = FALSE;

    //----------- サイズ計算

    if(!pSrc->m_dat.openRead()) return FALSE;

    for(i = 0; i < m_nLayerNo; i++)
    {
        //情報読み込み

        pSrc->readLayerInfo(&info);
        pSrc->seekTileImg(info.dwTileCnt);

        //サイズ

        p = g_pdraw->getLayer(info.nLayerNo);

        allsize += getLayerDatSize(p);
    }

    pSrc->m_dat.closeRead();

    //----------- 書き込み

    if(!pSrc->m_dat.openRead()) return FALSE;

    if(!m_dat.alloc(allsize)) goto END;
    if(!m_dat.openWrite()) goto END;

    for(i = 0; i < m_nLayerNo; i++)
    {
        //情報読み込み

        pSrc->readLayerInfo(&info);
        pSrc->seekTileImg(info.dwTileCnt);

        //レイヤ取得

        p = g_pdraw->getLayer(info.nLayerNo);

        //書き込み

        getLayerInfo(&info, p);

        writeOneLayer(p, info);
    }

    m_dat.closeWrite(TRUE);
    ret = TRUE;

END:
    pSrc->m_dat.closeRead();

    return ret;
}


//============================
// レイヤ情報＆イメージ
//============================


//! 複数レイヤの情報とイメージを書き込み
/*
    cnt  : レイヤ数
    pNo  : レイヤ位置（NULL で全レイヤ）
          [0] は絶対位置（レイヤウィンドウから見て一番下のレイヤの番号）、
          以降は飛ばす数（0で前の位置の次のレイヤ、1で次のレイヤを飛ばしその次の位置）
    bCombine : 結合時の情報としてセット
               FALSE 時は、nLayerNo はすべて絶対位置で書き込む

    [レイヤ情報]
    [各タイル]
        WORD(タイルX・Y番号、圧縮サイズ[4096=無圧縮])
        データ
    ...
*/

BOOL CUndoDat::writeMultipleLayer(int cnt,int *pNo,BOOL bCombine)
{
    CLayerItem *p;
    UNDOLAYERINFO info;
    int i,no;
    DWORD allsize;

    //------------ サイズ計算（無圧縮で）

    allsize = 0;
    no      = (pNo)? pNo[0]: cnt - 1;

    for(i = 0; i < cnt; i++)
    {
        if(i) no -= (pNo)? pNo[i] + 1: 1;

        p = g_pdraw->getLayer(no);

        allsize += getLayerDatSize(p);
    }

    //------------ 書き込み

    if(!m_dat.alloc(allsize)) return FALSE;
    if(!m_dat.openWrite()) return FALSE;

    //

    no = (pNo)? pNo[0]: cnt - 1;

    for(i = 0; i < cnt; i++)
    {
        if(i) no -= (pNo)? pNo[i] + 1: 1;

        p = g_pdraw->getLayer(no);

        //レイヤ番号

        if(bCombine)
            info.nLayerNo = (i == 0)? pNo[i] - (cnt - 1): pNo[i];
        else
            info.nLayerNo = no;

        //書き込み

        getLayerInfo(&info, p);

        writeOneLayer(p, info);
    }

    m_dat.closeWrite(TRUE); //メモリリサイズ

    return TRUE;
}

//! 単体レイヤを読み込み、復元（m_nLayerNo のレイヤ）

BOOL CUndoDat::readLayer()
{
    CLayerItem *p;
    UNDOLAYERINFO info;
    BOOL ret = FALSE;

    if(!m_dat.openRead()) return FALSE;

    //情報読み込み

    readLayerInfo(&info);

    //レイヤ作成

    p = g_pdraw->getLayer(m_nLayerNo);

    if(!p)
    {
        //一番下（リストでは一番上）に追加する時
        g_pdraw->changeCurLayer(m_nLayerNo - 1);
        if(!g_pdraw->layer_new(FALSE, &info.imginfo)) goto END;
        g_pdraw->layer_moveDown(FALSE);
    }
    else
    {
        //通常時
        g_pdraw->changeCurLayer(m_nLayerNo);
        if(!g_pdraw->layer_new(FALSE, &info.imginfo)) goto END;
    }

    //イメージ

    readOneLayer(g_pdraw->m_pCurLayer, info);

    ret = TRUE;

END:
    m_dat.closeRead();

    return ret;
}


//============================
// 結合
//============================


//! 結合前に戻す
//[m_nLayerNo] レイヤ数

BOOL CUndoDat::readRestoreCombine()
{
    CLayerItem *p;
    UNDOLAYERINFO info;
    int i,j;
    BOOL ret = FALSE;

    if(!m_dat.openRead()) return FALSE;

    for(i = 0; i < m_nLayerNo; i++)
    {
        //情報読み込み

        readLayerInfo(&info);

        //レイヤ取得 or 作成

        if(i == 0)
        {
            g_pdraw->changeCurLayer(info.nLayerNo);

            //再作成

            if(!g_pdraw->m_pCurLayer->m_img.createFromInfo(info.imginfo)) goto END;
        }
        else
        {
            //相対位置移動

            p = g_pdraw->m_pCurLayer;
            for(j = 0; j < info.nLayerNo; j++) p = p->next();

            j = g_pdraw->getLayerPos(p);
            g_pdraw->changeCurLayer(j);

            //レイヤ追加

            if(!g_pdraw->layer_new(FALSE, &info.imginfo)) goto END;
        }

        //イメージ

        readOneLayer(g_pdraw->m_pCurLayer, info);
    }

    ret = TRUE;

END:
    m_dat.closeRead();

    return ret;
}

//! レイヤ再結合

BOOL CUndoDat::readReCombine()
{
    CLayerItem *p;
    UNDOLAYERINFO info;
    int i,j,pos;

    if(!m_dat.openRead()) return FALSE;

    //--------- 結合後レイヤ -> 結合後のイメージへ戻す

    readLayerInfo(&info);

    g_pdraw->changeCurLayer(info.nLayerNo);

    if(!g_pdraw->m_pCurLayer->m_img.createFromInfo(info.imginfo))
    {
        m_dat.closeRead();
        return FALSE;
    }

    readOneLayer(g_pdraw->m_pCurLayer, info);

    //--------- 他レイヤ -> レイヤ削除

    for(i = 1; i < m_nLayerNo; i++)
    {
        m_dat.read(&pos, 4);

        //相対位置移動

        p = g_pdraw->m_pCurLayer;
        for(j = 0; j <= pos; j++) p = p->next();

        j = g_pdraw->getLayerPos(p);
        g_pdraw->changeCurLayer(j);

        g_pdraw->layer_del(FALSE);
    }

    m_dat.closeRead();

    return TRUE;
}


//============================
// 結合 逆データセット
//============================


//! 結合戻す-> 再結合 のデータをセット
/*
    最初（結合後）のレイヤのみ情報とイメージ。他は相対位置のみ
*/

BOOL CUndoDat::writeCombToReComb(CUndoDat *pSrc)
{
    UNDOLAYERINFO info;
    CLayerItem *p;
    DWORD allsize;
    int i;
    BOOL ret = FALSE;

    if(!pSrc->m_dat.openRead()) return FALSE;

    //--------- 最初のレイヤ＆データサイズ計算

    pSrc->readLayerInfo(&info);
    pSrc->seekTileImg(info.dwTileCnt);

    p = g_pdraw->getLayer(info.nLayerNo);

    allsize = getLayerDatSize(p);
    allsize += (m_nLayerNo - 1) * 4;

    //-------- 書き込み

    if(!m_dat.alloc(allsize)) goto END;
    if(!m_dat.openWrite()) goto END;

    //最初のレイヤ

    info.nLayerNo += m_nLayerNo - 1;

    p->m_img.getInfo(&info.imginfo);

    info.dwTileCnt = p->m_img.getAllocTileCnt();

    writeOneLayer(p, info);

    //以降のレイヤは相対位置のみ

    for(i = 1; i < m_nLayerNo; i++)
    {
        pSrc->readLayerInfo(&info);
        pSrc->seekTileImg(info.dwTileCnt);

        m_dat.write(&info.nLayerNo, 4);
    }

    m_dat.closeWrite(TRUE);

    ret = TRUE;

END:
    pSrc->m_dat.closeRead();

    return ret;
}

//! 再結合 -> 結合前に戻す セット

BOOL CUndoDat::writeReCombToComb(CUndoDat *pSrc)
{
    UNDOLAYERINFO info;
    int i,no,pos;
    CLayerItem *p;
    DWORD allsize;
    BOOL ret = FALSE;

    //--------- サイズ計算

    if(!pSrc->m_dat.openRead()) return FALSE;

    allsize = 0;

    for(i = 0; i < m_nLayerNo; i++)
    {
        if(i == 0)
        {
            pSrc->readLayerInfo(&info);
            pSrc->seekTileImg(info.dwTileCnt);

            no = info.nLayerNo;
        }
        else
        {
            pSrc->m_dat.read(&pos, 4);
            no -= pos + 1;
        }

        p = g_pdraw->getLayer(no);

        allsize += getLayerDatSize(p);
    }

    pSrc->m_dat.closeRead();

    //---------- 書き込み

    if(!pSrc->m_dat.openRead()) return FALSE;

    if(!m_dat.alloc(allsize)) goto END;
    if(!m_dat.openWrite()) goto END;

    //

    for(i = 0; i < m_nLayerNo; i++)
    {
        //レイヤ番号

        if(i == 0)
        {
            pSrc->readLayerInfo(&info);
            pSrc->seekTileImg(info.dwTileCnt);

            no  = info.nLayerNo;
            pos = info.nLayerNo - (m_nLayerNo - 1);
        }
        else
        {
            pSrc->m_dat.read(&pos, 4);
            no -= pos + 1;
        }

        //------- レイヤ情報

        p = g_pdraw->getLayer(no);

        //情報取得

        getLayerInfo(&info, p);

        info.nLayerNo = pos;

        //書き込み

        writeOneLayer(p, info);
    }

    m_dat.closeWrite(TRUE);
    ret = TRUE;

END:
    pSrc->m_dat.closeRead();

    return ret;
}


//============================
// サブ
//============================


//! レイヤ情報取得

void CUndoDat::getLayerInfo(UNDOLAYERINFO *pd,CLayerItem *pItem)
{
    pd->strName = pItem->m_strName;
    pd->dwCol   = pItem->m_dwCol;
    pd->btAlpha = pItem->m_btAlpha;
    pd->btFlag  = pItem->m_btFlag;

    pItem->m_img.getInfo(&pd->imginfo);

    pd->dwTileCnt = pItem->m_img.getAllocTileCnt();
}

//! レイヤの情報＋イメージのサイズ取得

DWORD CUndoDat::getLayerDatSize(CLayerItem *p)
{
    DWORD size;
    int len;

    len = p->m_strName.getLen() + 1;

    size = 4 + 1 + len * 2 + 4 + 2 + sizeof(LAYERINFO) + 4;
    size += p->m_img.getAllocTileCnt() * ONETILESIZE;

    return size;
}

//! レイヤ情報読み込み

void CUndoDat::readLayerInfo(UNDOLAYERINFO *pinfo)
{
    BYTE len;

    m_dat.read(&pinfo->nLayerNo, 4);

    m_dat.read(&len, 1);
    pinfo->strName.resize(len);
    m_dat.read((LPUSTR)pinfo->strName, len * 2);
    pinfo->strName.recalcLen();

    m_dat.read(&pinfo->dwCol, 4);
    m_dat.read(&pinfo->btAlpha, 1);
    m_dat.read(&pinfo->btFlag, 1);
    m_dat.read(&pinfo->imginfo, sizeof(LAYERINFO));
    m_dat.read(&pinfo->dwTileCnt, 4);
}

//! 一つのレイヤを書き込み

void CUndoDat::writeOneLayer(CLayerItem *p,const UNDOLAYERINFO &info)
{
    CTileImg **ppTile;
    LPBYTE pWorkBuf = m_memWork;
    WORD size,x,y,xcnt,ycnt;
    BYTE len;

    //------ 情報書き込み

    len = info.strName.getLen() + 1;

    m_dat.write(&info.nLayerNo, 4);
    m_dat.write(&len, 1);
    m_dat.write((LPUSTR)info.strName, len * 2);
    m_dat.write(&info.dwCol, 4);
    m_dat.write(&info.btAlpha, 1);
    m_dat.write(&info.btFlag, 1);
    m_dat.write(&info.imginfo, sizeof(LAYERINFO));
    m_dat.write(&info.dwTileCnt, 4);

    //------ 各タイル

    ppTile  = p->m_img.getTileBuf();
    xcnt    = info.imginfo.nTileXCnt;
    ycnt    = info.imginfo.nTileYCnt;

    for(y = 0; y < ycnt; y++)
    {
        for(x = 0; x < xcnt; x++, ppTile++)
        {
            if(*ppTile)
            {
                size = (*ppTile)->encodePackBits(pWorkBuf);

                m_dat.write(&x, 2);
                m_dat.write(&y, 2);
                m_dat.write(&size, 2);
                m_dat.write(pWorkBuf, size);
            }
        }
    }
}

//! 一つのレイヤ読み込み（イメージの作成はそれぞれで行う）

void CUndoDat::readOneLayer(CLayerItem *p,const UNDOLAYERINFO &info)
{
    DWORD i;
    WORD x,y,size;
    LPBYTE pWorkBuf = m_memWork;

    //レイヤ情報セット（※レイヤマスクはOFFにする）

    p->m_strName = info.strName;
    p->m_dwCol   = info.dwCol;
    p->m_btAlpha = info.btAlpha;
    p->m_btFlag  = info.btFlag & (~CLayerItem::FLAG_MASK);

    //各タイル

    for(i = info.dwTileCnt; i > 0; i--)
    {
        m_dat.read(&x, 2);
        m_dat.read(&y, 2);
        m_dat.read(&size, 2);
        m_dat.read(pWorkBuf, size);

        p->m_img.setTileFromUndo(x, y, pWorkBuf, size);
    }

    //プレビュー

    p->m_img.drawLayerPrev(&p->m_imgPrev, g_pdraw->m_nImgW, g_pdraw->m_nImgH);
}

//! タイル画像のデータを飛ばす

void CUndoDat::seekTileImg(DWORD tilecnt)
{
    WORD size;
    DWORD xy,i;

    for(i = tilecnt; i > 0; i--)
    {
        m_dat.read(&xy, 4);
        m_dat.read(&size, 2);

        m_dat.seekRead(size);
    }
}
