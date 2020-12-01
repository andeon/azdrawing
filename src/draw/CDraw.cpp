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

#include <math.h>

#include "CDraw.h"

#include "CCanvasWin.h"
#include "CStatusBar.h"
#include "CMainWin.h"
#include "CBrushWin.h"
#include "CToolWin.h"
#include "COptionWin.h"
#include "CProgressDlg.h"

#include "CLayerList.h"
#include "CConfig.h"
#include "CUndo.h"
#include "CPolygonPos.h"
#include "CSplinePos.h"
#include "CPosBuf.h"
#include "CImage8.h"
#include "CGradList.h"
#include "CBrushManager.h"
#include "CResizeImage.h"
#include "CFont.h"
#include "CApp.h"

#include "AXImage.h"
#include "AXConfRead.h"
#include "AXConfWrite.h"

#include "defGlobal.h"

#include "cursorimg.h"


//----------------------

void *thread_scaleCanvas(void *);

//----------------------


/*!
    @class CDraw
    @brief 描画関連のデータと処理
*/


CDraw::~CDraw()
{
    AXString str;

    //保存

    axapp->getConfigPath(&str, "grad.conf");
    m_pGrad->saveFile(str);

    BRUSHWIN->saveBrushFile();

    //削除

    delete m_pimgCanvas;
    delete m_pimgBlend;
    delete m_player;
    delete m_pimgUndo;
    delete m_pimgSel;
    delete m_pimgTmp;
    delete m_pimgCopy;
    delete m_pimg8Tex;
    delete m_pimg8BrushDef;
    delete m_pPolyPos;
    delete m_pSplinePos;
    delete m_pPosBuf;
    delete m_pGrad;
    delete m_pFont;
    delete m_pundo;

    delete BRUSHMAN;
}

//! 初期化（設定データ読み込み前）

CDraw::CDraw()
{
    m_pimgCanvas = new AXImage;
    m_pimgBlend  = new CImage32;
    m_player     = new CLayerList;
    m_pundo      = new CUndo;
    m_pimgUndo   = new CLayerImg;
    m_pimgSel    = new CLayerImg;
    m_pimgCopy   = new CLayerImg;
    m_pimgTmp    = new CLayerImg;
    m_pimg8Tex   = new CImage8;
    m_pimg8BrushDef = new CImage8;
    m_pPolyPos   = new CPolygonPos;
    m_pSplinePos = new CSplinePos;
    m_pPosBuf    = new CPosBuf;
    m_pGrad      = new CGradList;
    m_pFont      = new CFont;

    new CBrushManager;

    m_pimg1Work = NULL;

    //------- 値の初期化

    m_bCanvasHRev   = FALSE;
    m_nNowCtlNo     = NOWCTL_NONE;
    m_ruleType      = 0;
    m_nCanvasScale  = 100;
    m_nCanvasRot    = 0;

    m_rcfSel.flag = FALSE;

    AXMemZero(m_toolSubNo, sizeof(m_toolSubNo));

    //定規

    AXMemZero(&m_rule, sizeof(RULEDAT));

    m_rule.dAngleLine = m_rule.dAngleGrid = 90 * M_PI / 180;
    m_rule.dEllHV[0] = m_rule.dEllHV[1] = 1.0;
    m_rule.dEllTmp[0] = m_rule.dEllTmp[2] = 1.0;

    //描画用データ

    CLayerImg::m_pinfo = &m_datDraw;

    m_datDraw.pimgUndo = m_pimgUndo;

    //CLayerImg

    CLayerImg::init();

    //-------- 作成

    //カーソル

    createCursor(&m_cursorWait, CURSOR_WAIT);

    //ブラシ画像デフォルト

    m_pimg8BrushDef->createBrushImg_fillCircle(400);

    //--------- ファイル読み込み

    AXString str;

    //グラデーション

    axapp->getConfigPath(&str, "grad.conf");
    m_pGrad->readFile(str);
}

//! 設定ファイル読み込み後の初期化

void CDraw::initAfterConfig()
{
    //オプションテクスチャ画像

    loadTextureImg(m_pimg8Tex, m_strOptTex);

    //アンドゥ

    m_pundo->setMaxUndoCnt(g_pconf->nUndoCnt);
}

//! メインウィンドウ表示後の初期化

void CDraw::initShow()
{
    //イメージサイズから表示倍率計算

    m_szCanvas.w = CANVASAREA->getWidth();
    m_szCanvas.h = CANVASAREA->getHeight();

    m_nCanvasScale = calcCanvasScaleFromSize();

    changeScaleAndRotate(TRUE);
}

//! UNDOの更新フラグOFF

void CDraw::setUndoChangeOff()
{
    m_pundo->setChangeNone();
}

//! UNDO最大回数セット

void CDraw::setUndoMaxCnt(int cnt)
{
    m_pundo->setMaxUndoCnt(cnt);
}

//! イメージが変更されているか

BOOL CDraw::isChangeImage()
{
    return m_pundo->isChange();
}


//============================
//カーソル
//============================


//! カーソルデータ取得

LPBYTE CDraw::getCursorDat(int type)
{
    LPBYTE p = NULL;

    switch(type)
    {
        case CURSOR_DRAW: p = g_cursor_draw; break;
        case CURSOR_MOVE: p = g_cursor_move; break;
        case CURSOR_ROTATE: p = g_cursor_rotate; break;
        case CURSOR_RULE: p = g_cursor_rule; break;
        case CURSOR_SEL: p = g_cursor_sel; break;
        case CURSOR_BOXSEL: p = g_cursor_boxsel; break;
        case CURSOR_SPOIT: p = g_cursor_spoit; break;
        case CURSOR_TEXT: p = g_cursor_text; break;

        case CURSOR_WAIT: p = g_cursor_wait; break;
        case CURSOR_SELDRAG: p = g_cursor_seldrag; break;
        case CURSOR_LAYERMOVE: p = g_cursor_laymove; break;
    }

    return p;
}

//! カーソル作成

void CDraw::createCursor(AXCursor *pcur,int type)
{
    LPBYTE p = getCursorDat(type);

    if(p)
        pcur->create(p);
    else
        pcur->free();
}

//! 現在のツールのカーソルセット

void CDraw::setCursorTool()
{
    int no = CURSOR_DRAW;

    switch(m_toolno)
    {
        case TOOL_BRUSH:
        case TOOL_DOT:
            no = -1;
            break;
        case TOOL_MOVE:
            no = CURSOR_MOVE;
            break;
        case TOOL_ROTATE:
            no = CURSOR_ROTATE;
            break;
        case TOOL_RULE:
            no = CURSOR_RULE;
            break;
        case TOOL_BOXEDIT:
            no = CURSOR_BOXSEL;
            break;
        case TOOL_SEL:
            no = CURSOR_SEL;
            break;
        case TOOL_SPOIT:
            no = CURSOR_SPOIT;
            break;
        case TOOL_TEXT:
            no = CURSOR_TEXT;
            break;
    }

    //セット

    CANVASAREA->unsetCursor();

    if(no != -1)
        createCursor(&m_cursorTool, no);
    else
    {
        //描画用カーソル

        if(g_pconf->memDrawCursor.isExist())
            m_cursorTool.create(g_pconf->memDrawCursor);
        else
            createCursor(&m_cursorTool, CURSOR_DRAW);
    }

    CANVASAREA->setCursor(&m_cursorTool);
}

//! ドラッグ中用カーソルセット

void CDraw::setCursorDrag(int type)
{
    createCursor(&m_cursorDrag, type);

    CANVASAREA->setCursor(&m_cursorDrag);
}

//! カーソルを砂時計に

void CDraw::setCursorWait()
{
    CANVASAREA->setCursor(&m_cursorWait);
    CAPP->flush();
}

//! カーソルをツールのものに戻す（ドラッグ中・砂時計共通）

void CDraw::restoreCursor()
{
    CANVASAREA->setCursor(&m_cursorTool);

    m_cursorDrag.free();
}


//==========================
// イメージ
//==========================


//! 新規イメージ
/*!
    @param dpi       0以下でデフォルト
    @param bAddLayer 新規レイヤを作成するか（ADW読み込み時などはFALSE）
*/

BOOL CDraw::newImage(int w,int h,int dpi,BOOL bAddLayer)
{
    CLayerItem *p;

    if(w > IMGSIZE_MAX || h > IMGSIZE_MAX) return FALSE;

    if(w < 1) w = 1;
    if(h < 1) h = 1;

    //UNDO・レイヤ削除

    m_pundo->deleteAllDat();
    m_pundo->setChangeNone();

    m_player->deleteAll();

    //情報

    m_nImgW       = w;
    m_nImgH       = h;
    m_nImgDPI     = (dpi <= 0)? 96: dpi;
    m_nCurLayerNo = 0;
    m_pCurLayer   = NULL;

    //作業用

    if(!m_pimgBlend->create(w, h)) return FALSE;

    changeImageSize();

    //レイヤ

    if(bAddLayer)
    {
        //レイヤ追加

        p = m_player->addLayer(w, h);
        if(!p) return FALSE;

        //レイヤプレビュー

        p->m_img.drawLayerPrev(&p->m_imgPrev, m_nImgW, m_nImgH);

        //カレントレイヤ

        changeCurLayer(0);
    }

    return TRUE;
}

//! 画像から読み込んで作成

int CDraw::loadImage(const AXString &filename)
{
    CImage32 img;
    CImage32::LOADINFO info;
    int w,h;

    //32BIT読み込み

    if(!img.loadImage(filename, &info)) return LOADERR_ETC;

    //サイズ確認

    w = img.getWidth();
    h = img.getHeight();

    if(w > IMGSIZE_MAX || h > IMGSIZE_MAX) return LOADERR_IMGSIZE;

    //新規キャンバス

    if(!newImage(w, h, info.nDPI, TRUE))
        return LOADERR_ETC;

    //PNG時アルファ値無効

    if((g_pconf->uFlags & CConfig::FLAG_OPENPNG_NO_ALPHA) && info.type == 'P')
		info.bAlpha = FALSE;

    //イメージ変換

    m_pCurLayer->m_img.convFrom32Bit(img, info.bAlpha);

    return LOADERR_SUCCESS;
}

//! キャンバスサイズ変更

BOOL CDraw::resizeCanvas(int w,int h,int topx,int topy)
{
    int bkw,bkh;

    bkw = m_nImgW, bkh = m_nImgH;

    if(w == bkw && h == bkh) return FALSE;

    //作業用変更

    if(!changeResizeCanvas(w, h)) return FALSE;

    //UNDO

    m_pundo->add_resizeCanvas(topx, topy, bkw, bkh);

    //処理（オフセット追加）

    m_player->resizeCanvas(topx, topy);

    //更新

    sel_release(FALSE);

    MAINWIN->updateNewCanvas(NULL);

    return TRUE;
}

//! キャンバス拡大縮小

BOOL CDraw::scaleCanvas(int w,int h,int dpi,int type)
{
    CProgressDlg *pDlg;
    int val[3];

    //スレッド処理

    val[0] = w;
    val[1] = h;
    val[2] = type;

    pDlg = new CProgressDlg(MAINWIN, thread_scaleCanvas, val, NULL);

    if(!pDlg->run()) return FALSE;

    //作業用変更

    if(!changeResizeCanvas(w, h)) return FALSE;

    //DPI変更

    if(dpi != -1)
        m_nImgDPI = dpi;

    //更新

    sel_release(FALSE);

    MAINWIN->updateNewCanvas(NULL);

    return TRUE;
}

void *thread_scaleCanvas(void *pParam)
{
    CProgressDlg *pDlg = (CProgressDlg *)pParam;
    BOOL bRet = FALSE;
    int *pVal;
    AXSize sizeOld,sizeNew;
    AXRectSize rcs;
    CImage8 img8Old,img8New;
    CLayerItem *p;
    CResizeImage resize;

    pVal = (int *)pDlg->m_pParam1;

    sizeOld.w = g_pdraw->m_nImgW;
    sizeOld.h = g_pdraw->m_nImgH;
    sizeNew.w = pVal[0];
    sizeNew.h = pVal[1];

    rcs.set(0, 0, sizeOld.w, sizeOld.h);

    //8bitイメージ作成

    if(!img8Old.create(sizeOld.w, sizeOld.h)) goto END;
    if(!img8New.create(sizeNew.w, sizeNew.h)) goto END;

    //アンドゥ

    g_pdraw->m_pundo->add_scaleCanvas();

    //各レイヤ処理

    pDlg->setProgMax(g_pdraw->getLayerCnt());

    for(p = g_pdraw->m_player->getTopItem(); p; p = p->next())
    {
        //元イメージを8bitに変換

        p->m_img.convTo8Bit(&img8Old, rcs, FALSE);

        //img8New に拡大縮小 -> レイヤイメージに変換

        if(resize.resize(img8Old.getBuf(), img8New.getBuf(), sizeOld, sizeNew, pVal[2], CResizeImage::IMGTYPE_8BIT))
        {
            if(p->m_img.create(sizeNew.w, sizeNew.h))
                p->m_img.convFrom8Bit(img8New.getBuf(), sizeNew.w, sizeNew.h, FALSE);
        }

        //

        pDlg->incProg();
    }

    bRet = TRUE;

END:
    pDlg->endThread(bRet);

    return NULL;
}


//=============================
// 変更時
//=============================


//! ファイル変更時（開く/新規作成時）

void CDraw::changeFile()
{
    //選択解除

    sel_release(FALSE);

    //表示倍率・回転

    m_nCanvasScale = calcCanvasScaleFromSize();
    m_nCanvasRot   = 0;

    calcViewParam();

    TOOLWIN->changeScale();
    TOOLWIN->changeRotate();
}

//! イメージサイズ変更時（新規/開く/サイズ変更 時)

void CDraw::changeImageSize()
{
    m_datDraw.nImgW = m_nImgW;
    m_datDraw.nImgH = m_nImgH;
}

//! 途中でのキャンバスサイズ変更時

BOOL CDraw::changeResizeCanvas(int w,int h)
{
    //作業用サイズ変更

    if(!m_pimgBlend->create(w, h))
    {
        m_pimgBlend->create(m_nImgW, m_nImgH);
        return FALSE;
    }

    //

    m_nImgW = w;
    m_nImgH = h;

    return TRUE;
}

//! ツール変更

void CDraw::changeTool(int no)
{
    m_toolno = no;

    STATUSBAR->setLabel_help();

    TOOLWIN->changeTool();
    OPTWIN->changeTool();

    //ツールのカーソルセット

    setCursorTool();
}

//! ツールサブタイプ変更

void CDraw::changeToolSubType(int no)
{
    m_toolSubNo[m_toolno] = no;

    STATUSBAR->setLabel_help();
}

//! キャンバスウィンドウのサイズが変わった時
/*!
    @param pct キャンバス中央のイメージ位置(NULLで現在の位置取得)
*/

void CDraw::changeCanvasWinSize(BOOL bReDraw,AXPoint *pct)
{
    AXPoint pt;

    //中央のイメージ位置

    if(pct)
        pt = *pct;
    else
        calcWindowToImage(&pt, m_szCanvas.w / 2, m_szCanvas.h / 2);

    //サイズ取得

    m_szCanvas.w = CANVASAREA->getWidth();
    m_szCanvas.h = CANVASAREA->getHeight();

    //イメージ再作成

    m_pimgCanvas->recreate(m_szCanvas.w, m_szCanvas.h, 32, 32);

    //スクロール位置

    m_ptBaseImg = pt;
    m_ptScr.x = m_ptScr.y = 0;

    //

    CANVASWIN->setScroll();

    if(bReDraw) updateCanvas(TRUE);
}

//! 拡大率が変わった時
/*!
    ※キャンバスのスクロールバー範囲が変更される。@n
    ※イメージ基準位置は現在の中央位置にリセット。
*/

void CDraw::changeScale(int scale,BOOL bRedraw,BOOL bHiQuality,BOOL bScrReset)
{
    AXPoint pt;

    //中央のイメージ位置計算

    calcWindowToImage(&pt, m_szCanvas.w / 2, m_szCanvas.h / 2);

    //セット

    if(scale < SCALE_MIN) scale = SCALE_MIN;
    else if(scale > SCALE_MAX) scale = SCALE_MAX;

    m_nCanvasScale = scale;

    //

    if(bScrReset) setScroll_reset();

    calcViewParam();
    changeCanvasWinSize(FALSE, &pt);

    if(bRedraw)
        updateCanvas(TRUE, bHiQuality);

    STATUSBAR->setInfoLabel();
    TOOLWIN->changeScale();
}

//! キャンバス回転が変わった時

void CDraw::changeRotate(int rotate,BOOL bRedraw,BOOL bHiQuality,BOOL bScrReset)
{
    //セット

    if(rotate < -18000) rotate += 36000;
    else if(rotate > 18000) rotate -= 36000;

    m_nCanvasRot = rotate;

    //

    if(bScrReset) setScroll_reset();

    calcViewParam();

    TOOLWIN->changeRotate();

    //更新

    if(bRedraw) updateCanvas(TRUE, bHiQuality);
}

//! 他ウィンドウからスクロール位置変更
/*!
    キャンバス左右反転/プレビューからの位置変更 時

    @param pct 中央イメージ位置
*/

void CDraw::changeScrPos(AXPoint *pct,BOOL bReDraw)
{
    m_ptBaseImg = *pct;
    m_ptScr.x = m_ptScr.y = 0;

    CANVASWIN->setScroll();

    if(bReDraw) updateCanvas(TRUE);
}

//! 表示倍率と回転変更時
/*!
    初期表示時・キャンバスリセット時
*/

void CDraw::changeScaleAndRotate(BOOL bRedraw)
{
    AXPoint pt;

    pt.x = m_nImgW / 2;
    pt.y = m_nImgH / 2;

    calcViewParam();
    changeCanvasWinSize(FALSE, &pt);

    STATUSBAR->setInfoLabel();
    TOOLWIN->changeScale();
    TOOLWIN->changeRotate();

    if(bRedraw)
        updateCanvas(TRUE, TRUE);
}


//=============================
// コマンド
//=============================


//! キャンバス左右反転

void CDraw::canvasHRev()
{
    AXPoint pt;

    calcWindowToImage(&pt, m_szCanvas.w / 2, m_szCanvas.h / 2);

    m_bCanvasHRev ^= 1;

    changeScrPos(&pt, FALSE);                   //反転後も現在のキャンバス中央になるように
    changeRotate(-m_nCanvasRot, TRUE, TRUE, FALSE);   //回転も反転
}

//! 表示倍率１段階上げる/下げる

void CDraw::scaleUpDown(BOOL bUp)
{
    int n = m_nCanvasScale;

    if((!bUp && n <= 100) || (bUp && n < 100))
    {
        //〜100%

        if(bUp)
        {
            n = 100 - (100 - n - 1) / g_pconf->nScaleWidthDown * g_pconf->nScaleWidthDown;
            if(n > 100) n = 100;
        }
        else
            n = 100 - (100 - n + g_pconf->nScaleWidthDown) / g_pconf->nScaleWidthDown * g_pconf->nScaleWidthDown;
    }
    else
    {
        //100%〜

        if(bUp)
            n = (n + g_pconf->nScaleWidthUp) / g_pconf->nScaleWidthUp * g_pconf->nScaleWidthUp;
        else
        {
            n = (n - 1) / g_pconf->nScaleWidthUp * g_pconf->nScaleWidthUp;
            if(n < 100) n = 100;
        }
    }

    changeScale(n);
}

//! イメージ全体が見えるように表示倍率変更・回転リセット

void CDraw::resetCanvas()
{
    m_nCanvasScale = calcCanvasScaleFromSize();
    m_nCanvasRot   = 0;

    setScroll_default();

    changeScaleAndRotate(TRUE);
}

//! アンドゥ・リドゥ

BOOL CDraw::undoRedo(BOOL bRedo,LPINT pLayerNo,AXRectSize *prcs)
{
    AXUndo::RETURN ret;

    if(bRedo)
        ret = m_pundo->redo();
    else
        ret = m_pundo->undo();

    m_pundo->getUpdateParam(pLayerNo, prcs);

    return (ret == AXUndo::RET_OK);
}


//=============================
//
//=============================


//! スクロール位置をデフォルトに（画像の中央を基準とする）

void CDraw::setScroll_default()
{
    m_ptBaseImg.x = m_nImgW / 2;
    m_ptBaseImg.y = m_nImgH / 2;
    m_ptScr.x = m_ptScr.y = 0;
}

//! スクロール位置を現在のキャンバス中央にリセット

void CDraw::setScroll_reset()
{
    calcWindowToImage(&m_ptBaseImg, m_szCanvas.w / 2, m_szCanvas.h / 2);

    m_ptScr.x = m_ptScr.y = 0;

    CANVASWIN->setScroll();
}


//=============================
//
//=============================


//! 全レイヤを保存用に合成（m_pimgBlend を使う）
/*!
    @param bAlpha アルファ計算あり
*/

BOOL CDraw::blendAllLayer(BOOL bAlpha)
{
    CLayerItem *p;
    AXRectSize rcs;

    //クリア

    m_pimgBlend->clear(g_pconf->dwImgBkCol & 0xffffff);

    //レイヤ

    rcs.set(0, 0, m_nImgW, m_nImgH);

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        if(p->isView())
            p->m_img.blend32Bit_real(m_pimgBlend, rcs, p->m_dwCol, p->m_btAlpha, bAlpha);
    }

    return TRUE;
}

//! テクスチャ画像読み込み（オプションウィンドウ）
/*!
    @param pName NULLまたは空文字で削除
*/

BOOL CDraw::loadTextureImg(CImage8 *pimg,LPCUSTR pName)
{
    AXString str;
    CImage32 img;
    CImage32::LOADINFO info;

    pimg->free();

    if(!pName) return FALSE;
    if(pName[0] == 0) return FALSE;

    //読み込み(32bit)

    str = g_pconf->strTextureDir;
    str.path_add(pName);

    if(!img.loadImage(str, &info))
    {
        axapp->getResourcePath(&str, "texture");
        str.path_add(pName);

        if(!img.loadImage(str, &info))
            return FALSE;
    }

    //8bit 作成＆変換

    return pimg->createFrom32bit(img, info.bAlpha);
}


//=============================
// 設定ファイル
//=============================


//! 読み込み

void CDraw::loadConfig(AXConfRead *pcf)
{
    pcf->setGroup("draw");

    m_toolno = pcf->getInt("toolno", 0);
    m_pGrad->setSelItemFromNo(pcf->getInt("gradsel", 0));

    m_optToolDot  = pcf->getInt("tool_dot", 0);
    m_optToolPolyPaint = pcf->getInt("tool_polypaint", 255 | TOOLPOLY_F_ANTI);
    m_optToolPolyErase = pcf->getInt("tool_polyerase", 255 | TOOLPOLY_F_ANTI);
    m_optToolPaint = pcf->getInt("tool_paint", 255|(255<<TOOLPAINT_VALCMP_SHIFT));
    m_optToolPaintErase = pcf->getInt("tool_painterase", 0);
    m_optToolPaintClose = pcf->getInt("tool_paintclose", 255);
    m_optToolGrad = pcf->getInt("tool_grad", 255);
    m_optToolMove = pcf->getInt("tool_move", 0);
    m_optToolSel = pcf->getHex("tool_sel", 0x338000ff);
    m_optBEScaleRot = pcf->getHex("be_scalerot", 0xff00cc);

    m_wHeadTailLine = pcf->getInt("headtailLine", 0);
    m_wHeadTailBezir = pcf->getInt("headtailBezier", 0);

    pcf->getArrayVal("toolsub", m_toolSubNo, TOOL_NUM, 1);

    pcf->getStr("texture", &m_strOptTex);

    //テキスト

    pcf->setGroup("text");

    pcf->getStr("face", &m_strFontFace);
    pcf->getStr("style", &m_strFontStyle);

    m_nTextSize    = pcf->getInt("size", 12);
    m_nTextCharSP  = pcf->getInt("cspace", 0);
    m_nTextLineSP  = pcf->getInt("lspace", 0);
    m_nTextHinting = pcf->getInt("hinting", 1);
    m_uTextFlags   = pcf->getInt("flags", TEXTF_PREVIEW);
}

//! 書き込み

void CDraw::saveConfig(AXConfWrite *pcf)
{
    pcf->putGroup("draw");

    pcf->putInt("toolno", m_toolno);
    pcf->putInt("gradsel", m_pGrad->getSelNo());

    pcf->putInt("tool_dot", m_optToolDot);
    pcf->putInt("tool_polypaint", m_optToolPolyPaint);
    pcf->putInt("tool_polyerase", m_optToolPolyErase);
    pcf->putInt("tool_paint", m_optToolPaint);
    pcf->putInt("tool_painterase", m_optToolPaintErase);
    pcf->putInt("tool_paintclose", m_optToolPaintClose);
    pcf->putInt("tool_grad", m_optToolGrad);
    pcf->putInt("tool_move", m_optToolMove);
    pcf->putHex("tool_sel", m_optToolSel);
    pcf->putHex("be_scalerot", m_optBEScaleRot);

    pcf->putInt("headtailLine", m_wHeadTailLine);
    pcf->putInt("headtailBezier", m_wHeadTailBezir);

    pcf->putArrayVal("toolsub", m_toolSubNo, TOOL_NUM, 1);

    pcf->putStr("texture", m_strOptTex);

    //テキスト

    pcf->putGroup("text");

    pcf->putStr("face", m_strFontFace);
    pcf->putStr("style", m_strFontStyle);
    pcf->putInt("size", m_nTextSize);
    pcf->putInt("cspace", m_nTextCharSP);
    pcf->putInt("lspace", m_nTextLineSP);
    pcf->putInt("hinting", m_nTextHinting);
    pcf->putInt("flags", m_uTextFlags);
}
