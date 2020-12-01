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
/*
    CDraw [layer] - レイヤ関連
*/

#include "CDraw.h"

#include "CLayerList.h"
#include "CUndo.h"
#include "CLayerWin.h"
#include "CMainWin.h"
#include "CProgressDlg.h"
#include "CConfig.h"
#include "defGlobal.h"


//==============================
// 取得
//==============================


//! レイヤ数取得

int CDraw::getLayerCnt()
{
    return m_player->getCnt();
}

//! レイヤアイテム取得

CLayerItem *CDraw::getLayer(int no)
{
    return (CLayerItem *)m_player->getItemBottom(no);
}

//! ポインタからレイヤ位置取得

int CDraw::getLayerPos(CLayerItem *p)
{
    return m_player->getPosBottom(p);
}

//! カレントレイヤのイメージ取得

CLayerImg& CDraw::getCurLayerImg()
{
    return m_pCurLayer->m_img;
}


//==============================
//
//==============================


//! カレントレイヤ変更
/*!
    @param bPrev プレビューを更新するか
*/

void CDraw::changeCurLayer(int no,BOOL bPrev)
{
    CLayerItem *pbk = m_pCurLayer;
    int cnt = m_player->getCnt();

    if(cnt == 0) return;

    if(no < 0 || no >= cnt) no = 0;

    //

    m_nCurLayerNo   = no;
    m_pCurLayer     = getLayer(no);

    //前と現在のカレントレイヤのプレビュー更新

    if(bPrev)
    {
        //※レイヤ削除後などだと、存在しないレイヤに対して実行することになるので注意

        if(pbk) updateLayerPrev(pbk);
        updateLayerPrev(m_pCurLayer);
    }
}

//! レイヤ表示/非表示反転

void CDraw::layer_revView(int no)
{
    CLayerItem *p = getLayer(no);

    if(p)
    {
        p->m_btFlag ^= CLayerItem::FLAG_VIEW;
        updateAll();
    }
}

//! レイヤマスクフラグ反転
/*!
    @param type [0]自動、[1]下レイヤマスク、[2]マスクレイヤ
*/

void CDraw::layer_changeMask(int no,int type)
{
    CLayerItem *p = getLayer(no);
    BYTE flag_off,flag;

    flag_off = p->m_btFlag & ~(CLayerItem::FLAG_MASK | CLayerItem::FLAG_MASKUNDER);

    if(type != 2 && (p->isMaskUnder() || type == 1))
    {
        //下レイヤをマスク ON/OFF

        if(p->isMaskUnder())
            p->m_btFlag = flag_off;
        else
            p->m_btFlag = flag_off | CLayerItem::FLAG_MASKUNDER;

        LAYERWIN->updateLayerNo(no);
    }
    else
    {
        //マスクレイヤ ON/OFF

        flag = !p->isMask();

        m_player->maskOffAll();

        if(flag) p->m_btFlag = flag_off | CLayerItem::FLAG_MASK;

        LAYERWIN->updateLayerAll(FALSE);
    }
}

//! 塗りつぶし判定元反転

void CDraw::layer_revPaintFlag(int no)
{
    CLayerItem *p = getLayer(no);

    p->m_btFlag ^= CLayerItem::FLAG_PAINT;
    LAYERWIN->updateLayerNo(no);
}

//! レイヤグループ化フラグ反転
/*!
    @param type [0]自動 [1]下レイヤグループ [2]グループ化
*/

void CDraw::layer_changeGroup(int no,int type)
{
    CLayerItem *p = getLayer(no);
    BYTE flag_off;

    if(type != 2 && (p->isGroupUnder() || type == 1))
    {
        //下レイヤグループ化 ON/OFF

        flag_off = p->m_btFlag & ~(CLayerItem::FLAG_GROUP | CLayerItem::FLAG_GROUPUNDER);

        if(p->isGroupUnder())
            p->m_btFlag = flag_off;
        else
            p->m_btFlag = flag_off | CLayerItem::FLAG_GROUPUNDER;
    }
    else
    {
        //グループ化 ON/OFF

        p->m_btFlag &= ~CLayerItem::FLAG_GROUPUNDER;
        p->m_btFlag ^= CLayerItem::FLAG_GROUP;
    }

    LAYERWIN->updateLayerNo(no);
}

//! チェックON/OFF

void CDraw::layer_revCheck(int no)
{
    CLayerItem *p = getLayer(no);

    p->m_btFlag ^= CLayerItem::FLAG_CHECK;
    LAYERWIN->updateLayerNo(no);
}

//! 強調表示フラグの反転

void CDraw::layer_revHilight(int no)
{
    CLayerItem *p = getLayer(no);

    p->m_btFlag ^= CLayerItem::FLAG_HILIGHT;
    LAYERWIN->updateLayerNo(no);
}

//! 不透明度変更
/*!
    @param alpha 0-128
*/

void CDraw::layer_changeAlpha(int no,int alpha,BOOL bRedraw)
{
    CLayerItem *p = getLayer(no);

    if(p)
    {
        if(alpha < 0) alpha = 0; else if(alpha > 128) alpha = 128;

        p->m_btAlpha = alpha;
        if(bRedraw) updateAll();
    }
}

//! レイヤ位置移動（D&Dでの位置移動）

void CDraw::layer_moveDND(int pos,BOOL bUpdate)
{
    CLayerItem *p;
    int bkcur;

    if(pos == m_nCurLayerNo) return;

    bkcur = m_nCurLayerNo;

    //移動

    if(pos <= 0)
    {
        //一番上に

        if(m_pCurLayer->m_pNext == NULL) return;

        m_player->move(m_pCurLayer, NULL);
    }
    else if(pos >= m_player->getCnt())
    {
        //一番下に

        if(m_pCurLayer->m_pPrev == NULL) return;

        m_player->move(m_pCurLayer, m_player->getTop());
    }
    else
    {
        //指定位置

        p = getLayer(pos);
        p = p->next();
        if(m_pCurLayer == p) return;

        m_player->move(m_pCurLayer, p);
    }

    //カレントレイヤ

    m_nCurLayerNo = getLayerPos(m_pCurLayer);
    changeCurLayer(m_nCurLayerNo, bUpdate);

    //更新

    if(bUpdate)
    {
        m_pundo->add_layerMoveDND(bkcur);

        updateAllAndLayer(FALSE);
    }
}


//=======================
// コマンド 拡張
//=======================


//! カレントレイヤの選択を上下移動

void CDraw::layer_curUpDown(BOOL bDown)
{
    int no,bk;

    no = bk = m_nCurLayerNo;

    if(bDown)
    {
        if(no + 1 >= getLayerCnt()) return;
        no++;
    }
    else
    {
        if(no == 0) return;
        no--;
    }

    changeCurLayer(no, TRUE);

    LAYERWIN->updateLayerNo(bk);
    LAYERWIN->updateLayerNo(no);
}

//! カレントレイヤの不透明度を+1/-1

void CDraw::layer_curAlphaUpDown(BOOL bDown)
{
    int a;

    a = (int)(m_pCurLayer->m_btAlpha * 100.0 / 128.0 + 0.5);

    if(bDown)
    {
        a--;
        if(a < 0) a = 0;
    }
    else
    {
        a++;
        if(a > 100) a = 100;
    }

    m_pCurLayer->m_btAlpha = (int)(a * 128.0 / 100.0 + 0.5);

    LAYERWIN->updateLayerCurInfo();
    updateAll();
}

//! レイヤの線の色を変更

void CDraw::layer_changeCol(DWORD col)
{
    m_pCurLayer->m_dwCol = col & 0xffffff;

    LAYERWIN->updateLayerCurInfo();
    updateAll();
}


//=======================
// コマンド
//=======================


//! 新規レイヤ追加
/*!
    @param bUpdate   TRUEでUNDO処理・更新など行う。FALSEで追加のみ
    @param pUndoInfo アンドゥ時のレイヤイメージ情報（NULLで通常作成）
*/

BOOL CDraw::layer_new(BOOL bUpdate,LAYERINFO *pUndoInfo)
{
    CLayerItem *pNew;

    //追加

    pNew = m_player->addLayer(m_nImgW, m_nImgH, pUndoInfo);
    if(!pNew)return FALSE;

    //プレビュー

    if(!pUndoInfo)
        pNew->m_img.drawLayerPrev(&pNew->m_imgPrev, m_nImgW, m_nImgH);

    //カレントレイヤの前に移動

    if(m_pCurLayer->m_pNext != pNew)
        m_player->move(pNew, m_pCurLayer->m_pNext);

    //カレントレイヤ変更

    changeCurLayer(m_nCurLayerNo, bUpdate);

    //UNDO・更新

    if(bUpdate)
    {
        m_pundo->add_layerAdd();

        LAYERWIN->updateLayerAll(TRUE);
    }

    return TRUE;
}

//! 画像ファイルまたは指定画像から新規レイヤ
/*!
    @param pSrcImg NULL以外で、このイメージから作成。NULL で pStr のファイル名から。
    @param bUpdate D&Dで複数読み込みの場合、更新は最後に行う
*/

BOOL CDraw::layer_newFromImg(AXString *pStr,CImage32 *pSrcImg,BOOL bUpdate,BOOL bSrcAlpha)
{
    CLayerItem *pNew;
    CImage32 img,*pImg;
    CImage32::LOADINFO info;
    int bAlpha,w,h;

    //ファイル読み込み

    if(pSrcImg)
    {
        pImg    = pSrcImg;
        bAlpha  = bSrcAlpha;
    }
    else
    {
        if(!img.loadImage(*pStr, &info))
            return FALSE;

        pImg   = &img;
        bAlpha = info.bAlpha;

        if((g_pconf->uFlags & CConfig::FLAG_OPENPNG_NO_ALPHA) && info.type == 'P')
            bAlpha = FALSE;
    }

    //レイヤ追加
    //（キャンバスより大きければそのサイズ、小さければキャンバスサイズ）

    w = pImg->getWidth();  if(w < m_nImgW) w = m_nImgW;
    h = pImg->getHeight(); if(h < m_nImgH) h = m_nImgH;

    pNew = m_player->addLayer(w, h);
    if(!pNew) return FALSE;

    //カレントレイヤの前に移動

    if(m_pCurLayer->m_pNext != pNew)
        m_player->move(pNew, m_pCurLayer->m_pNext);

    //変換

    pNew->m_img.convFrom32Bit(*pImg, bAlpha);

    //プレビュー

    pNew->m_img.drawLayerPrev(&pNew->m_imgPrev, m_nImgW, m_nImgH);

    //レイヤ名

    if(pStr)
        pNew->m_strName.path_filenameNoExt(*pStr);

    //-----------------

    //カレントレイヤ変更

    changeCurLayer(m_nCurLayerNo, bUpdate);

    //UNDO・更新

    m_pundo->add_layerAdd();

    if(bUpdate) updateAllAndLayer(TRUE);

    return TRUE;
}

//! 選択選択でコピーされたイメージから作成（貼り付け）

void CDraw::layer_newPaste()
{
    CLayerItem *pNew;

    //コピー作成

    pNew = m_player->addLayerFromImg(*m_pimgCopy);
    if(!pNew) return;

    //プレビュー

    pNew->m_img.drawLayerPrev(&pNew->m_imgPrev, m_nImgW, m_nImgH);

    //カレントレイヤの前に移動

    if(m_pCurLayer->next() != pNew)
        m_player->move(pNew, m_pCurLayer->m_pNext);

    //カレントレイヤ変更

    changeCurLayer(m_nCurLayerNo, TRUE);

    //UNDO・更新

    m_pundo->add_layerAdd();

    updateAllAndLayer(TRUE);
}

//! カレントレイヤを複製

void CDraw::layer_copy()
{
    CLayerItem *pNew;

    //コピー作成

    pNew = m_player->copyLayer(m_pCurLayer);
    if(!pNew) return;

    //プレビュー

    pNew->m_img.drawLayerPrev(&pNew->m_imgPrev, m_nImgW, m_nImgH);

    //カレントレイヤの前に移動

    if(m_pCurLayer->m_pNext != pNew)
        m_player->move(pNew, m_pCurLayer->m_pNext);

    //カレントレイヤ変更

    changeCurLayer(m_nCurLayerNo, TRUE);

    //UNDO・更新

    m_pundo->add_layerAdd();
    updateAllAndLayer(TRUE);
}

//! カレントレイヤ削除

void CDraw::layer_del(BOOL bUpdate)
{
    //レイヤが1つの時は削除しない

    if(m_player->getCnt() == 1) return;

    //UNDO

    if(bUpdate) m_pundo->add_layerDel();

    //削除

    m_player->deleteItem(m_pCurLayer);

    //カレントレイヤ変更
    //※プレビューは更新しない（カレントレイヤが削除されているため）

    if(m_nCurLayerNo >= m_player->getCnt())
        m_nCurLayerNo = m_player->getCnt() - 1;

    changeCurLayer(m_nCurLayerNo, FALSE);

    //更新

    if(bUpdate) updateAllAndLayer(TRUE);
}

//! カレントレイヤクリア

void CDraw::layer_curClear()
{
    //UNDO

    m_pundo->add_curLayer();

    //クリア
    //（配列削除してキャンバス範囲で再確保）

    m_pCurLayer->m_img.create(m_nImgW, m_nImgH);

    //更新

    updateAll();
    updateLayerPrev(NULL, TRUE);
}

//! 下のレイヤに結合してクリア

void CDraw::layer_combineClear()
{
    int no[2];

    //カレントレイヤが一番下の場合

    if(m_pCurLayer->m_pPrev == NULL) return;

    //UNDO

    no[0] = m_nCurLayerNo + 1;
    no[1] = 0;

    m_pundo->add_multipleLayer(no, 2);

    //結合

    (m_pCurLayer->prev())->m_img.combine(m_pCurLayer->m_img, m_pCurLayer->m_btAlpha, 0);

    //クリア（タイル解放）

    m_pCurLayer->m_img.freeAllTile();

    //更新

    updateAll();

    updateLayerPrev(NULL, TRUE);
    updateLayerPrev(m_pCurLayer->prev(), TRUE);
}

//! 下のレイヤと結合して削除
/*!
    @param type [0]通常合成 [1]減算
*/

void CDraw::layer_combineDel(int type)
{
    int no[2];

    //カレントレイヤが一番下の場合

    if(m_pCurLayer->m_pPrev == NULL) return;

    //UNDO

    no[0] = m_nCurLayerNo + 1;
    no[1] = 0;

    m_pundo->add_layerComb(no, 2);

    //結合

    (m_pCurLayer->prev())->m_img.combine(m_pCurLayer->m_img, m_pCurLayer->m_btAlpha, type);

    //カレントレイヤを削除

    layer_del(FALSE);

    //更新

    updateAllAndLayer(TRUE);
    updateLayerPrev(NULL, TRUE);
}

//! 複数レイヤ結合スレッド
/*
    m_pParam1 : 結合先レイヤ(CLayerItem*)
*/

void *thread_combineSome(void *pParam)
{
    CProgressDlg *pdlg = (CProgressDlg *)pParam;
	CLayerItem *pDst,*p,*pNext;

	pDst = (CLayerItem *)pdlg->m_pParam1;

	for(p = pDst->next(); p; p = pNext)
	{
		pNext = p->next();

		if(p->m_bWorkFlag)
		{
			pDst->m_img.combine(p->m_img, p->m_btAlpha);

			g_pdraw->m_player->deleteItem(p);

			pdlg->incProg();
		}
	}

    pdlg->endThread(0);

    return NULL;
}

//! 複数レイヤ結合
/*!
    @param bCheck TRUE でチェックされているレイヤ。FALSE で表示レイヤ。
*/

void CDraw::layer_combineSome(BOOL bCheck)
{
    CLayerItem *pDst,*p;
    int i,cnt,pos = 0;
    int *pNo;
    CProgressDlg *pDlg;

    //結合先・結合数を取得(結合するレイヤは作業用フラグがON)

    pDst = m_player->getCombineSomeInfo((bCheck)? 1: 0, &cnt);
    if(!pDst) return;

    //UNDO

    pNo = new int[cnt];

    for(i = 0, p = pDst; i < cnt; p = p->next())
    {
        if(!p->m_bWorkFlag)
            pos++;
        else
        {
            if(i == 0)
                pNo[i] = getLayerPos(p);
            else
                pNo[i] = pos;

            pos = 0;
            i++;
        }
    }

    m_pundo->add_layerComb(pNo, cnt);

    delete []pNo;

    //結合＆削除

    pDlg = new CProgressDlg(MAINWIN, thread_combineSome, pDst);
    pDlg->setProgMaxOutThread(cnt);

    pDlg->run();

    //チェックレイヤの場合、結合先のチェックをはずす

    if(bCheck)
        pDst->m_btFlag &= ~CLayerItem::FLAG_CHECK;

    //カレントレイヤ変更

    changeCurLayer(getLayerPos(pDst));

    //更新

    updateAllAndLayer(TRUE);
    updateLayerPrev(NULL, TRUE);
}

//! すべての表示レイヤを新規レイヤに結合

void *thread_combineAllViewNew(void *pParam)
{
    CProgressDlg *pDlg = (CProgressDlg *)pParam;
    CLayerItem *pNew,*p;

    pNew = (CLayerItem *)pDlg->m_pParam1;

    pDlg->setProgMax(*((LPINT)pDlg->m_pParam2));

    for(p = g_pdraw->m_player->getTopItem(); p; p = p->next())
    {
        if(p->isView())
        {
            pNew->m_img.combine(p->m_img, p->m_btAlpha);

            pDlg->incProg();
        }
    }

    pDlg->endThread(0);

    return NULL;
}

void CDraw::layer_combineAllViewNew()
{
    CLayerItem *p,*pNew,*pEnd;
    int cnt = 0;
    CProgressDlg *pDlg;

    //結合数が1以下なら処理なし

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        if(p->isView()) { pEnd = p; cnt++; }
    }

    if(cnt <= 1) return;

    //レイヤ追加

    pNew = m_player->addLayer(m_nImgW, m_nImgH);
    if(!pNew) return;

    //結合の最後のレイヤの前に移動

    if(pEnd->next() != pNew)
        m_player->move(pNew, pEnd->next());

    //各レイヤ合成

    pDlg = new CProgressDlg(MAINWIN, thread_combineAllViewNew, pNew, &cnt);
    pDlg->run();

    //カレントレイヤ変更

    changeCurLayer(getLayerPos(pNew));

    //UNDO

    m_pundo->add_layerAdd();

    //更新

    updateAllAndLayer(TRUE);
    updateLayerPrev(NULL, TRUE);
}

//! カレントレイヤを上へ移動

void CDraw::layer_moveUp(BOOL bUpdate)
{
    if(!m_pCurLayer->m_pNext) return;

    //移動

    m_player->move(m_pCurLayer->m_pNext, m_pCurLayer);

    m_nCurLayerNo--;
    changeCurLayer(m_nCurLayerNo, bUpdate);

    //更新

    if(bUpdate)
    {
        m_pundo->add_layerMove(TRUE);
        updateAllAndLayer(FALSE);
    }
}

//! カレントレイヤを下へ移動

void CDraw::layer_moveDown(BOOL bUpdate)
{
    if(!m_pCurLayer->m_pPrev) return;

    //移動

    m_player->move(m_pCurLayer, m_pCurLayer->m_pPrev);

    m_nCurLayerNo++;
    changeCurLayer(m_nCurLayerNo, bUpdate);

    //更新

    if(bUpdate)
    {
        m_pundo->add_layerMove(FALSE);
        updateAllAndLayer(FALSE);
    }
}

//! すべて表示/非表示
/*!
    @param type [0]非表示 [1]表示 [2]カレントのみ
*/

void CDraw::layer_viewAll(int type)
{
    CLayerItem *p;

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        p->m_btFlag &= ~CLayerItem::FLAG_VIEW;

        if(type == 1 || (type == 2 && p == m_pCurLayer))
            p->m_btFlag |= CLayerItem::FLAG_VIEW;
    }

    updateAllAndLayer(FALSE);
}

//! グループレイヤを表示/非表示

void CDraw::layer_toggleShowGroup()
{
    m_player->toggleShowGroup(m_pCurLayer);
    updateAllAndLayer(FALSE);
}

//! チェックレイヤを表示/非表示

void CDraw::layer_toggleShowCheck()
{
    CLayerItem *p;

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        if(p->isCheck())
            p->m_btFlag ^= CLayerItem::FLAG_VIEW;
    }

    updateAllAndLayer(FALSE);
}

//! カレントレイヤを左右反転

void CDraw::layer_curHRev()
{
    //UNDO

    m_pundo->add_layerHRev();

    //実行

    m_pCurLayer->m_img.reverseHorzFull();

    //更新

    updateAll();
    updateLayerPrev(NULL, TRUE);
}

//! 全レイヤを左右反転

void CDraw::layer_allHRev()
{
    CLayerItem *p;

    setCursorWait();

    //UNDO

    m_pundo->add_layerAllHRev();

    //処理

    for(p = m_player->getTopItem(); p; p = p->next())
        p->m_img.reverseHorzFull();

    //更新

    updateAll();
    updateLayerPrevAll(TRUE);

    restoreCursor();
}
