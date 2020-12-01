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
    CMainWin [command] - コマンド処理
*/

#include "CMainWin.h"

#include "CDraw.h"
#include "CConfig.h"
#include "CStatusBar.h"
#include "CToolWin.h"
#include "CLayerWin.h"
#include "CBrushWin.h"

#include "CResizeCanvasDlg.h"
#include "CScaleCanvasDlg.h"
#include "CDrawShapeDlg.h"
#include "CGridOptDlg.h"
#include "CKeyOptDlg.h"
#include "CDeviceOptDlg.h"
#include "COptionDlg.h"

#include "defGlobal.h"
#include "defStrID.h"

#include "AXFileDialog.h"
#include "AXStrDialog.h"
#include "AXApp.h"


//==============================
// 表示
//==============================


//! 全ウィンドウ非表示/復元

void CMainWin::allWindowHideShow()
{
    int i;
    UINT flag;

    if(g_pconf->uViewFlags & CConfig::VIEWFLAG_ALLHIDE)
    {
        //復元（閉じる前と表示状態が違う時のみ）

        for(i = 0, flag = 1; i < 6; i++, flag <<= 1)
        {
            if((g_pconf->uViewFlags & flag) != (g_pconf->uViewBkFlags & flag))
                onCommand(STRID_MENU_VIEW_TOOLWIN + i, 0, 0);
        }
    }
    else
    {
        //非表示

        g_pconf->uViewBkFlags = g_pconf->uViewFlags;

        for(i = 0; i < 6; i++)
        {
            if(g_pconf->uViewFlags & (1 << i))
                onCommand(STRID_MENU_VIEW_TOOLWIN + i, 0, 0);
        }
    }

    g_pconf->uViewFlags ^= CConfig::VIEWFLAG_ALLHIDE;
}


//==============================
// 編集
//==============================


//! アンドゥ・リドゥ

void CMainWin::undoRedo(BOOL bRedo)
{
    AXRectSize rcs;
    int layerno;

    //アンドゥ・リドゥ

    if(!g_pdraw->undoRedo(bRedo, &layerno, &rcs))
        return;

    //更新

    if(rcs.x != -1)
    {
        //指定レイヤの更新範囲のみ
        g_pdraw->updateAfterUndo(layerno, rcs);
    }
    else
    {
        //全体を更新

        switch(layerno)
        {
            //イメージ・キャンバス
            case 0:
                g_pdraw->updateAll();
                break;
            //イメージ・キャンバス・レイヤ一覧
            case 1:
                g_pdraw->updateAllAndLayer(TRUE);
                break;
            //サイズ変更
            case 2:
            case 3:
                g_pdraw->changeResizeCanvas(rcs.w, rcs.h);
                updateNewCanvas(NULL);
                break;
        }
    }
}

//! DPI値変更

void CMainWin::changeDPI()
{
    AXString str;
    int n;

    _trgroup(STRGID_DLG_CHANGEDPI);

    str.setInt(g_pdraw->m_nImgDPI);

    if(AXStrDialog::getString(this, _str(STRID_CHANGEDPI_TITLE), _str(STRID_CHANGEDPI_MESSAGE), &str))
    {
        n = str.toInt();
        if(n < 1) n = 1; else if(n > 10000) n = 10000;

        g_pdraw->m_nImgDPI = n;

        STATUSBAR->setInfoLabel();
    }
}

//! キャンバスサイズ変更

void CMainWin::resizeCanvas()
{
    CResizeCanvasDlg *pdlg;
    int val[3],n,topx,topy;

    //ダイアログ

    val[0] = g_pdraw->m_nImgW;
    val[1] = g_pdraw->m_nImgH;

    pdlg = new CResizeCanvasDlg(this, val);

    if(!pdlg->runDialog()) return;

    //左上位置

    n = val[2] % 3;

    if(n == 0) topx = 0;
    else if(n == 1) topx = (val[0] - g_pdraw->m_nImgW) / 2;
    else topx = val[0] - g_pdraw->m_nImgW;

    n = val[2] / 3;

    if(n == 0) topy = 0;
    else if(n == 1) topy = (val[1] - g_pdraw->m_nImgH) / 2;
    else topy = val[1] - g_pdraw->m_nImgH;

    //実行

    g_pdraw->resizeCanvas(val[0], val[1], topx, topy);
}

//! キャンバス拡大縮小

void CMainWin::scaleCanvas()
{
    CScaleCanvasDlg *pdlg;
    CScaleCanvasDlg::VALUE val;

    //ダイアログ

    val.nWidth  = g_pdraw->m_nImgW;
    val.nHeight = g_pdraw->m_nImgH;
    val.nDPI    = g_pdraw->m_nImgDPI;
    val.nType   = g_pconf->btScaleType;

    pdlg = new CScaleCanvasDlg(this, &val);

    if(!pdlg->runDialog()) return;

    //補間方法保存

    g_pconf->btScaleType = val.nType;

    //サイズが同じ

    if(g_pdraw->m_nImgW == val.nWidth && g_pdraw->m_nImgH == val.nHeight)
        return;

    //処理

    if(!g_pdraw->scaleCanvas(val.nWidth, val.nHeight, val.nDPI, val.nType))
        errMes(STRGID_MESSAGE, STRID_MES_ERR_FAILED);
}

//! 座標を指定して図形描画

void CMainWin::drawShape()
{
    CDrawShapeDlg *pdlg;
    CDrawShapeDlg::VALUE val;

    val.uValue = g_pconf->uDrawShapeVal;
    val.nDPI   = g_pdraw->m_nImgDPI;

    pdlg = new CDrawShapeDlg(this, &val);

    if(pdlg->runDialog())
    {
        g_pconf->uDrawShapeVal = val.uValue;

        g_pdraw->drawShape(val.uValue, val.dParam);
    }
}


//==============================
// 選択範囲
//==============================


//! 選択範囲インポート

void CMainWin::sel_inport()
{
    AXString filter,str,dir;

    //ファイル名取得

    filter = "*.sel\t*.sel\t";

    getFileInitDir(&dir);

    if(!AXFileDialog::openFile(this, filter, 0, dir, 0, &str)) return;

    //読み込み

    if(!g_pdraw->sel_loadFile(str))
        errMes(STRGID_MESSAGE, STRID_MES_ERR_LOAD);
}

//! 選択範囲エクスポート

void CMainWin::sel_export(BOOL bImg)
{
    AXRectSize rcs;
    AXString filter,str,dir;

    //選択範囲がない

    if(!g_pdraw->isSelExist()) return;

    //範囲（キャンバス範囲内）

    if(!g_pdraw->calcImgRectInCanvas(&rcs, g_pdraw->m_rcfSel)) return;

    //ファイル名取得

    filter = "*.sel\t*.sel\t";

    getFileInitDir(&dir);

    if(!AXFileDialog::saveFile(this, filter, 0, dir, 0, &str)) return;

    str.path_setExt("sel");

    //保存

    if(!g_pdraw->sel_saveFile(str, rcs, bImg))
        errMes(STRGID_MESSAGE, STRID_MES_ERR_SAVE);
}


//==============================
// 設定
//==============================


//! グリッド設定

void CMainWin::gridOption()
{
    CGridOptDlg *pdlg = new CGridOptDlg(this);

    if(pdlg->runDialog())
    {
        if(g_pconf->isGridView())
            g_pdraw->updateCanvas();
    }
}

//! キー設定

void CMainWin::keyOption()
{
    CKeyOptDlg *pdlg = new CKeyOptDlg(this);

    pdlg->runDialog();
}

//! デバイス設定

void CMainWin::deviceOption()
{
    CDeviceOptDlg *pdlg = new CDeviceOptDlg(this);

    pdlg->runDialog();
}

//! 環境設定

void CMainWin::envOption()
{
    COptionDlg *pdlg = new COptionDlg(this);

    if(pdlg->runDialog() == 2)
        g_pdraw->updateAll();
}


//==================================
// ショートカットキーなどの拡張コマンド
//==================================


//! ショートカットキーなどの拡張コマンド

void CMainWin::_command_ex(UINT id)
{
    if(id < 1100)
        //ツール選択 (1000-)
        g_pdraw->changeTool(id - 1000);
    else if(id < 1200)
    {
        //描画タイプ変更 (1100-) * ブラシ・ドット描画時

        if(g_pdraw->m_toolno == CDraw::TOOL_BRUSH || g_pdraw->m_toolno == CDraw::TOOL_DOT)
        {
            g_pdraw->changeToolSubType(id - 1100);
            TOOLWIN->checkTB_toolsub_pen(id - 1100);
        }
    }
    else if(id < 1300)
    {
        //レイヤ拡張コマンド (1200-)

        switch(id - 1200)
        {
            case 0: g_pdraw->layer_curUpDown(FALSE); break;     //レイヤ選択↑
            case 1: g_pdraw->layer_curUpDown(TRUE); break;      //レイヤ選択↓
            case 2: g_pdraw->layer_curAlphaUpDown(TRUE); break;     //レイヤ不透明度-1
            case 3: g_pdraw->layer_curAlphaUpDown(FALSE); break;    //レイヤ不透明度+1
            //カレントレイヤ表示/非表示
            case 4:
                g_pdraw->layer_revView(g_pdraw->m_nCurLayerNo);
                LAYERWIN->updateLayerCurInfo();
                break;
        }
    }
    else if(id < 1400)
    {
        //他コマンド (1300-)

        switch(id - 1300)
        {
            case 0: g_pdraw->scaleUpDown(TRUE); break;      //1段階拡大
            case 1: g_pdraw->scaleUpDown(FALSE); break;     //1段階縮小
            case 2: g_pdraw->changeScale(100); break;       //100%
            case 3: g_pdraw->changeRotate(g_pdraw->m_nCanvasRot - g_pconf->nRotateWidth * 100); break; //1段階左に回転
            case 4: g_pdraw->changeRotate(g_pdraw->m_nCanvasRot + g_pconf->nRotateWidth * 100); break; //1段階右に回転
            case 5: g_pdraw->changeRotate(0); break;            //回転0
            case 6: BRUSHWIN->moveBrushSelUpDown(FALSE); break; //ブラシ選択↑
            case 7: BRUSHWIN->moveBrushSelUpDown(TRUE); break;  //ブラシ選択↓
        }
    }
    else if(id < 1500)
        //登録ブラシ選択 (1400-)
        BRUSHWIN->setSelRegBrush(id - 1400);
}
