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
    CMainWin [filter] - フィルタ関連
*/

#include "CMainWin.h"

#include "CDraw.h"
#include "CImage8.h"
#include "CLayerImg.h"
#include "CFilterDlg.h"
#include "CProgressDlg.h"

#include "defGlobal.h"
#include "defStrID.h"

#include "filterdat.h"

//--------------------

typedef void (*NODLGFUNC)(const RECTANDSIZE &,CProgressDlg *);
typedef BOOL (*DLGFUNC)(CFilterDlg::DRAWINFO *);

void runFilter_noDlg(NODLGFUNC);
void runFilter_dlg(UINT uTitleID,const BYTE *,DLGFUNC,BOOL bCopySrc);

void *thread_filter_func(void *);

//--------------------


//=============================
//各フィルタ関数
//=============================


//! 色反転

void filterFunc_col_inverse(const RECTANDSIZE &rs,CProgressDlg *pdlg)
{
    g_pdraw->getCurLayerImg().filter_col_inverse(rs, pdlg);
}

//! テクスチャ適用

void filterFunc_setTexture(const RECTANDSIZE &rs,CProgressDlg *pdlg)
{
    g_pdraw->getCurLayerImg().filter_setTexture(rs, pdlg, g_pdraw->m_pimg8Tex);
}

//! レベル補正

BOOL filterFunc_col_level(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_col_level(*pinfo->pimgSrc,
                                *pinfo->prs, pinfo->pProgDlg, pinfo->nValBar);
}

//! ガンマ補正

BOOL filterFunc_col_gamma(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_col_gamma(*pinfo->pimgSrc,
                                *pinfo->prs, pinfo->pProgDlg, pinfo->nValBar[0]);
}

//! 2値化

BOOL filterFunc_2col(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_2col(*pinfo->pimgSrc,
                                *pinfo->prs, pinfo->pProgDlg, pinfo->nValBar[0]);
}

//! 2値化(ディザ)

BOOL filterFunc_2col_dither(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_2col_dither(*pinfo->pimgSrc,
                                *pinfo->prs, pinfo->pProgDlg, pinfo->nValCombo[0]);
}

//-----------

//! ぼかし

BOOL filterFunc_blur(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_blur(*pinfo->pimgSrc, *pinfo->prs, pinfo->pProgDlg,
                                       pinfo->nValBar[0], pinfo->bClipping);
}

//! ガウスぼかし

BOOL filterFunc_gaussblur(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_gaussBlur(*pinfo->pimgSrc, *pinfo->prs, pinfo->pProgDlg,
                                       pinfo->nValBar[0], pinfo->bClipping);
}

//! モーションブラー

BOOL filterFunc_motionblur(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_motionBlur(*pinfo->pimgSrc, *pinfo->prs, pinfo->pProgDlg,
                                       pinfo->nValBar[0], pinfo->nValBar[1], pinfo->bClipping);
}

//-----------

//! アミトーン描画[1]

BOOL filterFunc_drawAmitone1(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_drawAmiTone(*pinfo->prs, pinfo->pProgDlg,
                            pinfo->nValBar[0], pinfo->nValBar[1], pinfo->nValBar[2], pinfo->nValBar[3],
                            pinfo->nValCheck[0]);
}

//! アミトーン描画[2]

BOOL filterFunc_drawAmitone2(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_drawAmiTone(*pinfo->prs, pinfo->pProgDlg,
                            pinfo->nValBar[0], pinfo->nValBar[1], pinfo->nValBar[2], -1,
                            pinfo->nValCheck[0]);
}

//! ドットライン描画

BOOL filterFunc_drawline(CFilterDlg::DRAWINFO *pinfo)
{
    UINT flags = 0;

    if(pinfo->nValCheck[0]) flags |= 1;
    if(pinfo->nValCheck[1]) flags |= 2;

    return pinfo->pimgDst->filter_drawDotLine(*pinfo->prs, pinfo->pProgDlg,
                                       pinfo->nValBar[0], pinfo->nValBar[1], pinfo->nValBar[2], pinfo->nValBar[3],
                                       flags);
}

//! チェック柄描画

BOOL filterFunc_drawcheck(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_drawCheck(*pinfo->prs, pinfo->pProgDlg,
                                       pinfo->nValBar[0], pinfo->nValBar[1], pinfo->nValCheck[0]);
}

//-----------

//! 集中線・フラッシュ

BOOL filterFunc_comicFlash(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_drawFlash(pinfo->pProgDlg,
                                       0, g_pdraw->m_ptTmp[0], pinfo->nValBar, pinfo->nValCombo[0], pinfo->nValCheck[0]);
}

//! ベタフラッシュ

BOOL filterFunc_comicBetaFlash(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_drawFlash(pinfo->pProgDlg,
                                       1, g_pdraw->m_ptTmp[0], pinfo->nValBar, pinfo->nValCombo[0], pinfo->nValCheck[0]);
}

//! ウニフラッシュ

BOOL filterFunc_comicUniFlash(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_drawFlash(pinfo->pProgDlg,
                                       2, g_pdraw->m_ptTmp[0], pinfo->nValBar, pinfo->nValCombo[0], pinfo->nValCheck[0]);
}

//! ウニフラッシュ(波)

BOOL filterFunc_comicUniFlashWave(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_drawUniFlashWave(pinfo->pProgDlg,
                                    g_pdraw->m_ptTmp[0], pinfo->nValBar, pinfo->nValCombo[0], pinfo->nValCheck[0]);
}

//-----------

//! 他のレイヤと操作

BOOL filterFunc_layercol(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_layercol(*pinfo->prs, pinfo->pProgDlg,
                                *pinfo->pimgRef, pinfo->nValCombo[0]);
}

//! モザイク

BOOL filterFunc_mozaic(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_mozaic(*pinfo->pimgSrc,
                                *pinfo->prs, pinfo->pProgDlg, pinfo->nValBar[0], pinfo->bClipping);
}

//! モザイク（網目）

BOOL filterFunc_mozaic2(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_mozaic2(*pinfo->pimgSrc,
                                *pinfo->prs, pinfo->pProgDlg, pinfo->nValBar[0], pinfo->bClipping);
}

//! 水晶

BOOL filterFunc_crystal(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_crystal(*pinfo->pimgSrc, *pinfo->prs, pinfo->pProgDlg,
                                pinfo->nValBar[0], pinfo->nValCheck[0], pinfo->bClipping);
}

//! ノイズ

BOOL filterFunc_noise(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_noise(*pinfo->pimgSrc, *pinfo->prs, pinfo->pProgDlg,
                                pinfo->nValBar[0], pinfo->nValBar[1], pinfo->nValCheck[0]);
}

//! 縁取り

BOOL filterFunc_edge(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_edge(*pinfo->pimgSrc, *pinfo->prs, pinfo->pProgDlg,
                                *pinfo->pimgRef, pinfo->nValBar[0], pinfo->nValCheck[0]);
}

//! ハーフトーン

BOOL filterFunc_halftone(CFilterDlg::DRAWINFO *pinfo)
{
    return pinfo->pimgDst->filter_halftone(*pinfo->pimgSrc, *pinfo->prs, pinfo->pProgDlg,
                                pinfo->nValBar[0], pinfo->nValBar[1], pinfo->nValCheck[0]);
}

//! 1pxドット線の補正

void filterFunc_thinning(const RECTANDSIZE &rs,CProgressDlg *pdlg)
{
    g_pdraw->getCurLayerImg().filter_thinning(rs, pdlg);
}


//=============================
//コマンド
//=============================


//! フィルタコマンド

void CMainWin::_command_filter(UINT id)
{
    switch(id)
    {
        //色反転
        case STRID_MENU_FILTER_COL_INVERSE:
            runFilter_noDlg(filterFunc_col_inverse);
            break;
        //テクスチャ適用
        case STRID_MENU_FILTER_COL_TEXTURE:
            if(g_pdraw->m_pimg8Tex->isExist())
                runFilter_noDlg(filterFunc_setTexture);
            else
                errMes(STRGID_MESSAGE, STRID_MES_ERR_FILTER_SETTEXTURE);
            break;
        //レベル補正
        case STRID_MENU_FILTER_COL_LEVEL:
            runFilter_dlg(id, g_filter_level, filterFunc_col_level, FALSE);
            break;
        //ガンマ補正
        case STRID_MENU_FILTER_COL_GAMMA:
            runFilter_dlg(id, g_filter_gamma, filterFunc_col_gamma, FALSE);
            break;
        //2値化
        case STRID_MENU_FILTER_COL_2COL:
            runFilter_dlg(id, g_filter_2col, filterFunc_2col, FALSE);
            break;
        //2値化(ディザ)
        case STRID_MENU_FILTER_COL_DITHER:
            runFilter_dlg(id, g_filter_2col_dither, filterFunc_2col_dither, FALSE);
            break;

        //ぼかし
        case STRID_MENU_FILTER_BLUR_BLUR:
            runFilter_dlg(id, g_filter_blur, filterFunc_blur, TRUE);
            break;
        //ガウスぼかし
        case STRID_MENU_FILTER_BLUR_GAUSS:
            runFilter_dlg(id, g_filter_gaussblur, filterFunc_gaussblur, TRUE);
            break;
        //モーションブラー
        case STRID_MENU_FILTER_BLUR_MOTION:
            runFilter_dlg(id, g_filter_motionblur, filterFunc_motionblur, TRUE);
            break;

        //アミトーン描画[1]
        case STRID_MENU_FILTER_DRAW_AMITONE1:
            runFilter_dlg(id, g_filter_drawamitone1, filterFunc_drawAmitone1, FALSE);
            break;
        //アミトーン描画[2]
        case STRID_MENU_FILTER_DRAW_AMITONE2:
            runFilter_dlg(id, g_filter_drawamitone2, filterFunc_drawAmitone2, FALSE);
            break;
        //ドットライン描画
        case STRID_MENU_FILTER_DRAW_LINE:
            runFilter_dlg(id, g_filter_drawline, filterFunc_drawline, FALSE);
            break;
        //チェック柄描画
        case STRID_MENU_FILTER_DRAW_CHECK:
            runFilter_dlg(id, g_filter_drawcheck, filterFunc_drawcheck, FALSE);
            break;

        //集中線
        case STRID_MENU_FILTER_COMIC_CONCLINE:
            runFilter_dlg(id, g_filter_comicConcLine, filterFunc_comicFlash, FALSE);
            break;
        //フラッシュ
        case STRID_MENU_FILTER_COMIC_FLASH:
            runFilter_dlg(id, g_filter_comicFlash, filterFunc_comicFlash, FALSE);
            break;
        //ベタフラッシュ
        case STRID_MENU_FILTER_COMIC_BETAFLASH:
            runFilter_dlg(id, g_filter_comicBetaFlash, filterFunc_comicBetaFlash, FALSE);
            break;
        //ウニフラッシュ
        case STRID_MENU_FILTER_COMIC_UNIFLASH:
            runFilter_dlg(id, g_filter_comicUniFlash, filterFunc_comicUniFlash, FALSE);
            break;
        //ウニフラッシュ(波)
        case STRID_MENU_FILTER_COMIC_UNIFLASH_WAVE:
            runFilter_dlg(id, g_filter_comicUniFlashWave, filterFunc_comicUniFlashWave, FALSE);
            break;

        //他のレイヤと操作
        case STRID_MENU_FILTER_OTHERLAYER:
            runFilter_dlg(id, g_filter_layercol, filterFunc_layercol, FALSE);
            break;
        //モザイク
        case STRID_MENU_FILTER_MOZAIC:
            runFilter_dlg(id, g_filter_mozaic, filterFunc_mozaic, TRUE);
            break;
        //モザイク(網目)
        case STRID_MENU_FILTER_MOZAIC2:
            runFilter_dlg(id, g_filter_mozaic, filterFunc_mozaic2, TRUE);
            break;
        //水晶
        case STRID_MENU_FILTER_CRYSTAL:
            runFilter_dlg(id, g_filter_crystal, filterFunc_crystal, TRUE);
            break;
        //ノイズ
        case STRID_MENU_FILTER_NOISE:
            runFilter_dlg(id, g_filter_noise, filterFunc_noise, FALSE);
            break;
        //縁取り
        case STRID_MENU_FILTER_EDGE:
            runFilter_dlg(id, g_filter_edge, filterFunc_edge, TRUE);
            break;
        //ハーフトーン
        case STRID_MENU_FILTER_HALFTONE:
            runFilter_dlg(id, g_filter_halftone, filterFunc_halftone, TRUE);
            break;
        //1pxドット線の補正
        case STRID_MENU_FILTER_1DOTLINE:
            runFilter_noDlg(filterFunc_thinning);
            break;
    }
}


//=============================
//共通処理
//=============================


//! フィルタ処理、ダイアログなし

void runFilter_noDlg(NODLGFUNC func)
{
    CProgressDlg *pdlg;

    pdlg = new CProgressDlg(MAINWIN, thread_filter_func, (LPVOID)func);

    pdlg->run();
}

//! フィルタ処理、ダイアログあり
/*!
    @param bCopySrc 実際の処理時、元画像のコピーを作成する
*/

void runFilter_dlg(UINT uTitleID,const BYTE *pDat,DLGFUNC func,BOOL bCopySrc)
{
    CFilterDlg *pFilterDlg;
    CProgressDlg *pProgDlg;
    CFilterDlg::DRAWINFO info;

    AXMemZero(&info, sizeof(CFilterDlg::DRAWINFO));

    //フィルタダイアログ

    pFilterDlg = new CFilterDlg(MAINWIN, uTitleID, pDat, func, &info);
    if(!pFilterDlg->runDialog()) return;

    //処理

    info.bCopySrc = bCopySrc;

    pProgDlg = new CProgressDlg(MAINWIN, thread_filter_func, (LPVOID)func, (LPVOID)&info);

    if(!pProgDlg->run())
        MAINWIN->errMes(STRGID_MESSAGE, STRID_MES_ERR_FAILED);
}

//! フィルタ処理スレッド
/*
    m_pParam1 : 処理関数
    m_pParam2 : CFilterDlg::DRAWINFO* 描画用情報(NULL でなし)
*/

void *thread_filter_func(void *pParam)
{
    CProgressDlg *pProgDlg = (CProgressDlg *)pParam;
    CFilterDlg::DRAWINFO *pinfo;
    CLayerImg imgSrc;
    AXRect rc;
    RECTANDSIZE rs;
    BOOL ret = FALSE;

    pinfo = (CFilterDlg::DRAWINFO *)pProgDlg->m_pParam2;

    //UNDO開始

    g_pdraw->beginDrawUndo();

    //描画準備

    g_pdraw->setBeforeDraw_filter(FALSE);

    //処理範囲

    g_pdraw->getDrawRectSel(&rc);

    rs.set(rc);

    //処理

    pProgDlg->beginProgSub(50, rs.h, TRUE);

    if(pinfo)
    {
        //ダイアログ設定あり

        pinfo->prs      = &rs;
        pinfo->pProgDlg = pProgDlg;
        pinfo->pimgDst  = &g_pdraw->getCurLayerImg();

        if(!pinfo->bCopySrc)
            pinfo->pimgSrc = pinfo->pimgDst;
        else
        {
            //コピーを作成してソースとする

            if(!imgSrc.copy(g_pdraw->getCurLayerImg()))
                goto END;

            pinfo->pimgSrc = &imgSrc;
        }

        ret = ((DLGFUNC)pProgDlg->m_pParam1)(pinfo);
    }
    else
    {
        //ダイアログ設定なし

        ((NODLGFUNC)pProgDlg->m_pParam1)(rs, pProgDlg);
        ret = TRUE;
    }

    //UNDO後処理

    g_pdraw->commonAfterDraw();

    //終了

END:
    pProgDlg->endThread(ret);

    return NULL;
}
