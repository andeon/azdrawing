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
    CDraw [update] - 更新
*/

#include "CDraw.h"

#include "CLayerList.h"
#include "CConfig.h"
#include "CCanvasWin.h"
#include "CPrevWin.h"
#include "CLayerWin.h"

#include "AXImage.h"

#include "defGlobal.h"


//=============================
//まとめて更新
//=============================


//! 全体更新（イメージ＆キャンバス＆プレビューウィンドウ）

void CDraw::updateAll()
{
    updateImage();
    updateCanvas(TRUE);

    PREVWIN->draw();
}

//! 全体更新＆レイヤ一覧更新
/*!
    @param bChangeCnt レイヤ数が変わった
*/

void CDraw::updateAllAndLayer(BOOL bChangeCnt)
{
    updateAll();
    LAYERWIN->updateLayerAll(bChangeCnt);
}

//! アンドゥ後の範囲更新

void CDraw::updateAfterUndo(int layerno,const AXRectSize &rcs)
{
    CLayerItem *p;

    //y == -1 で更新なし（更新部分がキャンバス範囲外の場合）

    if(rcs.y != -1)
    {
        updateRect(rcs);
        PREVWIN->draw();

        //プレビュー更新フラグ

        p = getLayer(layerno);
        if(p) p->m_bPrevUpdate = TRUE;
    }
}

//! 範囲更新（イメージとキャンバス）
/*!
    @param rcs イメージ範囲
*/

void CDraw::updateRect(const AXRectSize &rcs)
{
    AXRectSize rcsCanvas;

    //------- イメージ合成

    //クリア

    m_pimgBlend->clear(rcs, g_pconf->dwImgBkCol);

    //各レイヤ合成

    update_blendImage(rcs);

    //特殊

    if(m_nNowCtlNo)
    {
        switch(m_nNowCtlNo)
        {
            //閉領域・ブラシ描画
            case NOWCTL_PAINTCLOSE_BRUSH:
                m_pimgTmp->blend32Bit(m_pimgBlend, rcs, 0x00ff00, 64);
                break;
            //テキスト
            case NOWCTL_TEXT:
                m_pimgTmp->blend32Bit(m_pimgBlend, rcs, m_pCurLayer->m_dwCol, m_pCurLayer->m_btAlpha);
                break;
            //フィルタ・キャンバスプレビュー
            case NOWCTL_FILTER_CANVASPREV:
                m_pimgTmp->blend32Bit(m_pimgBlend, rcs, 0xff0000, 128);
                break;
            //矩形編集・拡大縮小＆回転
            case NOWCTL_BOXEDIT_SCALEROT:
                m_pimgTmp->blend32Bit(m_pimgBlend, rcs, m_optBEScaleRot, 128);
                break;
        }
    }

    //選択範囲

    if(m_rcfSel.flag)
        m_pimgSel->blend32Bit(m_pimgBlend, rcs, m_optToolSel, m_optToolSel >> 24);

    //------- キャンバス更新

    if(calcImageToWindowRect(&rcsCanvas, rcs))
    {
        //キャンバス描画

        update_drawCanvas(rcsCanvas, TRUE);

        //更新

        CANVASAREA->redraw(rcsCanvas);
    }
}


//=============================
// 各更新
//=============================


//! レイヤプレビュー更新
/*!
    @param pLayer NULLでカレントレイヤ
    @param bForce フラグに関係なく強制更新
*/

void CDraw::updateLayerPrev(CLayerItem *p,BOOL bForce)
{
    if(!p) p = m_pCurLayer;

    if(bForce || p->m_bPrevUpdate)
    {
        //プレビュー描画

        p->m_img.drawLayerPrev(&p->m_imgPrev, m_nImgW, m_nImgH);

        //レイヤ一覧更新

        LAYERWIN->updateLayerNoPrev(getLayerPos(p));

        //更新クリア

        p->m_bPrevUpdate = FALSE;
    }
}

//! 全レイヤのプレビュー更新

void CDraw::updateLayerPrevAll(BOOL bRedraw)
{
    CLayerItem *p;

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        p->m_img.drawLayerPrev(&p->m_imgPrev, m_nImgW, m_nImgH);
        p->m_bPrevUpdate = FALSE;
    }

    if(bRedraw) LAYERWIN->updateLayerAll(FALSE);
}

//! 描画後の範囲更新（レイヤプレビュー更新フラグON＆プレビュー更新）
/*!
    @param rcs イメージ範囲（x == -1 で範囲なし）
*/

void CDraw::updateAfterDraw(const AXRectSize &rcs)
{
    if(rcs.x == -1) return;

    m_pCurLayer->m_bPrevUpdate = TRUE;

    PREVWIN->draw(rcs);
}

//! FLAGRECT 範囲（イメージ座標）のプレビューウィンドウ更新
/*!
    選択範囲時に使う
*/

void CDraw::updatePrevWin(const FLAGRECT &rcf)
{
    AXRectSize rcs;

    if(rcf.flag)
    {
        if(calcImgRectInCanvas(&rcs, rcf))
            PREVWIN->draw(rcs);
    }
}

//! FLAGRECT の範囲（イメージ座標）のキャンバスとプレビューウィンドウ更新
/*!
    閉領域ブラシ描画、選択範囲の更新で使う
*/

void CDraw::updateCanvasAndPrevWin(const FLAGRECT &rcf,BOOL bPrevWin)
{
    AXRectSize rcs;

    if(rcf.flag)
    {
        if(calcImgRectInCanvas(&rcs, rcf))
        {
            updateRect(rcs);

            //プレビューウィンドウ更新

            if(bPrevWin)
                PREVWIN->draw(rcs);
        }
    }
}


//=============================
// イメージ更新
//=============================


//! イメージ全体更新

void CDraw::updateImage()
{
    AXRectSize rcs;

    rcs.set(0, 0, m_nImgW, m_nImgH);

    //背景

    m_pimgBlend->clear(g_pconf->dwImgBkCol);

    //イメージ合成

    update_blendImage(rcs);

    //選択範囲

    if(m_rcfSel.flag)
        m_pimgSel->blend32Bit(m_pimgBlend, rcs, m_optToolSel, m_optToolSel >> 24);
}

//! レイヤイメージ合成処理
/*!
    prc : イメージ範囲
*/

void CDraw::update_blendImage(const AXRectSize &rcs)
{
    CLayerItem *p;

    if(m_nNowCtlNo == NOWCTL_SEL_DRAGMOVE)
    {
        //選択範囲ドラッグ移動時
        //（カレントレイヤの後にドラッグイメージを合成）

        for(p = m_player->getTopItem(); p; p = p->next())
        {
            if(p->isView())
            {
                p->m_img.blend32Bit(m_pimgBlend, rcs, p->m_dwCol, p->m_btAlpha);

                if(p == m_pCurLayer)
                    m_pimgTmp->blend32Bit(m_pimgBlend, rcs, p->m_dwCol, p->m_btAlpha);
            }
        }
    }
    else
    {
        //通常時

        for(p = m_player->getTopItem(); p; p = p->next())
        {
            if(p->isView())
                p->m_img.blend32Bit(m_pimgBlend, rcs, p->m_dwCol, p->m_btAlpha);
        }
    }
}


//=============================
//キャンバス更新
//=============================


//! キャンバス全体更新

void CDraw::updateCanvas(BOOL bReDraw,BOOL bHiQuality)
{
    AXRectSize rcs;

    //キャンバス描画

    rcs.set(0, 0, m_szCanvas.w, m_szCanvas.h);

    update_drawCanvas(rcs, bHiQuality);

    //更新

    if(bReDraw)
        CANVASAREA->redraw();
}

//! キャンバス描画

void CDraw::update_drawCanvas(const AXRectSize &rcsDst,BOOL bHiQuality)
{
    CImage32::CANVASDRAW info;

    if(!m_pimgCanvas->isExist()) return;

    info.rcsDst     = rcsDst;
    info.nBaseX     = m_ptBaseImg.x;
    info.nBaseY     = m_ptBaseImg.y;
    info.nScrollX   = m_szCanvas.w / 2 - m_ptScr.x;
    info.nScrollY   = m_szCanvas.h / 2 - m_ptScr.y;
    info.bHRev      = m_bCanvasHRev;
    info.dwExCol    = g_pconf->dwCanvasBkCol;
    info.dScaleDiv  = m_dViewParam[VIEWPARAM_SCALEDIV];
    info.dCos       = m_dViewParam[VIEWPARAM_COSREV];
    info.dSin       = m_dViewParam[VIEWPARAM_SINREV];

    //

    if(g_pconf->uViewFlags & (CConfig::VIEWFLAG_GRID | CConfig::VIEWFLAG_GRIDSPLIT))
    {
        //----------- グリッドあり

        info.dwGridFlags = (g_pconf->isGrid())? 1: 0;
        info.nGridW     = g_pconf->nGridW;
        info.nGridH     = g_pconf->nGridH;
        info.dwGridCol  = g_pconf->dwGridCol;
        info.dScale     = m_dViewParam[VIEWPARAM_SCALE];

        if(g_pconf->isGridSplit())
        {
            info.dwGridFlags |= 2;
            info.dwGridSplitCol = g_pconf->dwGridSplitCol;
            info.nGridSplitW = m_nImgW / g_pconf->nGridSplitX;
            info.nGridSplitH = m_nImgH / g_pconf->nGridSplitY;
            if(info.nGridSplitW < 2) info.nGridSplitW = 2;
            if(info.nGridSplitH < 2) info.nGridSplitH = 2;
        }

        //

        if(m_nCanvasRot == 0)
        {
            //回転なし

            if(bHiQuality && m_nCanvasScale != 100 && m_nCanvasScale < 200)
                m_pimgBlend->drawCanvasScaleDown(m_pimgCanvas, &info);
            else
                m_pimgBlend->drawCanvasNormal(m_pimgCanvas, &info);

            if(g_pconf->isGrid())
                m_pimgBlend->drawCanvasGrid(m_pimgCanvas, &info, info.nGridW, info.nGridH, info.dwGridCol);

            if(g_pconf->isGridSplit())
                m_pimgBlend->drawCanvasGrid(m_pimgCanvas, &info, info.nGridSplitW, info.nGridSplitH, info.dwGridSplitCol);
        }
        else
        {
            //回転あり

            if(bHiQuality)
                m_pimgBlend->drawCanvasRotHiQualityGrid(m_pimgCanvas, &info);
            else
                m_pimgBlend->drawCanvasRotNormalGrid(m_pimgCanvas, &info);
        }
    }
    else
    {
        //----------- グリッドなし

        if(m_nCanvasRot == 0)
        {
            //回転なし（200%以下[100%は除く] は高品質で）

            if(bHiQuality && m_nCanvasScale != 100 && m_nCanvasScale < 200)
                m_pimgBlend->drawCanvasScaleDown(m_pimgCanvas, &info);
            else
                m_pimgBlend->drawCanvasNormal(m_pimgCanvas, &info);
        }
        else
        {
            //回転あり

            if(bHiQuality)
                m_pimgBlend->drawCanvasRotHiQuality(m_pimgCanvas, &info);
            else
                m_pimgBlend->drawCanvasRotNormal(m_pimgCanvas, &info);
        }
    }
}
