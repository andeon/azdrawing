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
    CDraw [draw2]

    座標指定描画、フィルタ・キャンバスプレビュー、テキスト描画、選択範囲
*/

#include <math.h>

#include "CDraw.h"

#include "CCanvasWin.h"
#include "CDrawTextDlg.h"
#include "CLayerList.h"
#include "CUndo.h"
#include "CPolygonPos.h"
#include "CPosBuf.h"
#include "CImage8.h"
#include "CFont.h"

#include "AXFile.h"
#include "AXUtilZlib.h"
#include "AXApp.h"


//*********************************
// 座標指定描画
//*********************************


//! 座標指定描画

void CDraw::drawShape(UINT val,double *pParam)
{
    int type,sub,drawval;
    double dTmp[6];
    AXRect rc;

    drawval = val & 255;
    type    = (val >> 10) & 15;
    sub     = (val >> 14) & 15;

    //------ 描画準備

    switch(type)
    {
        //ブラシ・ドット線時
        case 0:
        case 1:
        case 2:
            setBeforeDraw((sub == 0)? TOOL_BRUSH: TOOL_DOT);

            if(sub == 1)
                m_datDraw.funcDrawPixel = &CLayerImg::setPixelDraw;
            else if(sub == 2)
                m_datDraw.funcDrawPixel = &CLayerImg::setPixel_dot2px;
            else if(sub == 3)
                m_datDraw.funcDrawPixel = &CLayerImg::setPixel_dot3px;
            break;
        //塗りつぶし時、マスクなどのみセット
        case 3:
        case 4:
            setBeforeDraw(-1);
            break;
    }

    //------ 描画

    setCursorWait();
    beginDrawUndo();

    switch(type)
    {
        //直線
        case 0:
            dTmp[0] = pParam[3] * M_PI / -180;
            dTmp[1] = pParam[2] * ::cos(dTmp[0]) + pParam[0];
            dTmp[2] = pParam[2] * ::sin(dTmp[0]) + pParam[1];

            if(sub == 0)
                m_pCurLayer->m_img.drawBrush_line(pParam[0], pParam[1], dTmp[1], dTmp[2]);
            else
            {
                m_pCurLayer->m_img.drawLineB((int)pParam[0], (int)pParam[1], (int)(dTmp[1] + 0.5), (int)(dTmp[2] + 0.5),
                                             drawval, FALSE);
            }
            break;
        //四角形
        case 1:
            dTmp[0] = pParam[0] + pParam[2];
            dTmp[1] = pParam[1] + pParam[3];

            if(sub == 0)
            {
                m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(pParam[0], pParam[1], dTmp[0], pParam[1]);
                m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(dTmp[0], pParam[1], dTmp[0], dTmp[1]);
                m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(dTmp[0], dTmp[1], pParam[0], dTmp[1]);
                m_datDraw.bparam.dT = 0; m_pCurLayer->m_img.drawBrush_line(pParam[0], dTmp[1], pParam[0], pParam[1]);
            }
            else
            {
                rc.left    = (int)pParam[0];
                rc.top     = (int)pParam[1];
                rc.right   = (int)dTmp[0] - 1;
                rc.bottom  = (int)dTmp[1] - 1;

                if(rc.right < rc.left) rc.right = rc.left;
                if(rc.bottom < rc.top) rc.bottom = rc.top;

                m_pCurLayer->m_img.drawLineB(rc.left, rc.top, rc.right, rc.top, 255, FALSE);
                m_pCurLayer->m_img.drawLineB(rc.right, rc.top, rc.right, rc.bottom, 255, TRUE);
                m_pCurLayer->m_img.drawLineB(rc.right, rc.bottom, rc.left, rc.bottom, 255, TRUE);
                m_pCurLayer->m_img.drawLineB(rc.left, rc.bottom, rc.left, rc.top, 255, TRUE);
            }
            break;
        //円
        case 2:
            if(pParam[2] <= 0) pParam[2] = 0.5;

            dTmp[2] = 1;
            dTmp[3] = 0;

            m_pCurLayer->m_img.drawCircle(pParam[0], pParam[1], pParam[2], pParam[2],
                                          dTmp, FALSE, (sub == 0));
            break;
        //四角形塗りつぶし
        case 3:
            m_datDraw.nColSetFunc = sub;

            rc.left   = (int)pParam[0];
            rc.top    = (int)pParam[1];
            rc.right  = (int)(pParam[0] + pParam[2]) - 1;
            rc.bottom = (int)(pParam[1] + pParam[3]) - 1;

            if(rc.right < rc.left) rc.right = rc.left;
            if(rc.bottom < rc.top) rc.bottom = rc.top;

            m_pCurLayer->m_img.fillBox(rc, drawval, &CLayerImg::setPixelDraw);
            break;
        //円塗りつぶし
        case 4:
            m_datDraw.nColSetFunc = sub;

            if(pParam[2] <= 0) pParam[2] = 0.5;

            dTmp[2] = dTmp[4] = 1;
            dTmp[3] = dTmp[5] = 0;

            m_pCurLayer->m_img.drawFillCircle(pParam[0], pParam[1], pParam[2], pParam[2],
                            dTmp, FALSE, drawval, val & (1<<8));
            break;
    }

    commonAfterDraw();
    restoreCursor();
}


//*********************************
// フィルタ・キャンバスプレビュー
//*********************************
/*
    m_ptTmp[0]  : 描画位置（イメージ位置）
    m_rcfDrawUp : プレビュー前回の描画範囲
    m_pTmp      : フィルタダイアログ(AXWindow*)
*/


//! プレビュー開始

void CDraw::beginFilterCanvasPrev(LPVOID pDlg)
{
    //プレビュー用イメージ

    if(!m_pimgTmp->create(m_nImgW, m_nImgH)) return;

    //

    m_nNowCtlNo = NOWCTL_FILTER_CANVASPREV;

    m_pTmp = pDlg;

    m_rcfDrawUp.flag = FALSE;

    setBeforeDraw_filter(TRUE);
    setBeforeDraw_filterDrawFlash();

    //描画位置（キャンバスの中心）

    calcWindowToImage(&m_ptTmp[0], m_szCanvas.w / 2, m_szCanvas.h / 2);
}

//! プレビュー終了

void CDraw::endFilterCanvasPrev()
{
    m_nNowCtlNo = NOWCTL_NONE;

    //プレビュー用イメージ削除

    m_pimgTmp->free();

    //プレビュー範囲を元に戻す

    updateCanvasAndPrevWin(m_rcfDrawUp, FALSE);
}

//! 描画位置変更時

void CDraw::onDown_filterPrevInDlg(int winx,int winy)
{
    calcWindowToImage(&m_ptTmp[0], winx, winy);

    ((AXWindow *)m_pTmp)->onCommand(0, 0, 0);
}


//*********************************
// テキスト描画
//*********************************
/*
    m_pimgTmp   : プレビュー表示用（updateRect 時に直接描画する）
    m_ptTmp[0]  : 描画位置（イメージ位置）
    m_rcfDrawUp : プレビュー前回の描画範囲
*/


//! 押し時

void CDraw::onDown_text()
{
    CDrawTextDlg *pDlg;
    BOOL ret;

	//描画位置

	getDrawPointInt(&m_ptTmp[0], FALSE);

    //プレビュー用イメージ

    if(!m_pimgTmp->create(m_nImgW, m_nImgH)) return;

    //

    m_nNowCtlNo      = NOWCTL_TEXT;
    m_rcfDrawUp.flag = FALSE;

    //-------- ダイアログ

    pDlg = new CDrawTextDlg;

    ret = pDlg->runDialog();

    //-------- 終了

    m_nNowCtlNo = NOWCTL_NONE;

    //プレビュー用イメージ削除

    m_pimgTmp->free();

    //プレビュー範囲を元に戻す

    updateCanvasAndPrevWin(m_rcfDrawUp, FALSE);

    //描画

    if(ret)
    {
        CFont::DRAWINFO info;

        setBeforeDraw(TOOL_TEXT);
        beginDrawUndo();

        //描画情報

        info.nCharSpace = m_nTextCharSP;
        info.nLineSpace = m_nTextLineSP;
        info.nHinting   = m_nTextHinting;
        info.pixfunc    = &CLayerImg::setPixelDraw;

        info.uFlags = 0;
        if(m_uTextFlags & TEXTF_2COL) info.uFlags |= CFont::INFOFLAG_2COL;
        if(m_uTextFlags & TEXTF_VERT) info.uFlags |= CFont::INFOFLAG_VERT;

        //

        m_pFont->drawString(&m_pCurLayer->m_img, m_ptTmp[0].x, m_ptTmp[0].y, m_strDrawText, &info);

        commonAfterDraw();
    }
}

//! ダイアログ中の押し時（描画位置変更）

void CDraw::onDown_textInDlg(int winx,int winy)
{
    //位置

    calcWindowToImage(&m_ptTmp[0], winx, winy);

    //プレビュー

    draw_textPrev();
}

//! テキストプレビュー描画

void CDraw::draw_textPrev()
{
    CFont::DRAWINFO info;
    FLAGRECT rcf = m_rcfDrawUp;

    //プレビューイメージ・範囲クリア

    if(m_rcfDrawUp.flag)
    {
        m_pimgTmp->freeAllTile();
        m_rcfDrawUp.flag = FALSE;
    }

    //プレビューなしの時、前回の範囲を消す

    if(!(m_uTextFlags & TEXTF_PREVIEW))
    {
        updateCanvasAndPrevWin(rcf, FALSE);
        return;
    }

    //描画情報

    info.nCharSpace = m_nTextCharSP;
    info.nLineSpace = m_nTextLineSP;
    info.nHinting   = m_nTextHinting;
    info.pixfunc    = &CLayerImg::setPixel_blendCalcRect;

    info.uFlags = 0;
    if(m_uTextFlags & TEXTF_2COL) info.uFlags |= CFont::INFOFLAG_2COL;
    if(m_uTextFlags & TEXTF_VERT) info.uFlags |= CFont::INFOFLAG_VERT;

    //描画

    m_datDraw.rcfDraw.flag = FALSE;

    m_pFont->drawString(m_pimgTmp, m_ptTmp[0].x, m_ptTmp[0].y, m_strDrawText, &info);

    //更新（前回の範囲と合わせた範囲）

    calcUnionFlagRect(&rcf, m_datDraw.rcfDraw);

    updateCanvasAndPrevWin(rcf, FALSE);
    axapp->update();

    //m_rcfDrawUp = 今回の範囲

    m_rcfDrawUp = m_datDraw.rcfDraw;
}

//! 描画位置1px移動

void CDraw::text_move1px(int dir)
{
    switch(dir)
    {
        case 0: m_ptTmp[0].x--; break;
        case 1: m_ptTmp[0].y--; break;
        case 2: m_ptTmp[0].x++; break;
        case 3: m_ptTmp[0].y++; break;
    }
}

//! フォント作成（ダイアログ表示時）

void CDraw::text_createFontInit()
{
    if(m_pFont->isNone())
        text_createFont();
}

//! フォント作成

void CDraw::text_createFont()
{
    m_pFont->load(m_strFontFace, m_strFontStyle, m_nTextSize);
}

//! キャッシュクリア

void CDraw::text_clearCache()
{
    m_pFont->freeCache();
}


//*********************************
//選択範囲
//*********************************
/*
    - 選択範囲イメージは m_pimgSel。
    - 範囲情報は m_rcfSel。
    - 選択範囲イメージはキャンバス（m_pimgBlend）に描画される。
    - プレビューウィンドウ上にも選択範囲は表示される。
    - 選択範囲を移動する場合は m_rcfSel の値も移動すること。
*/



//! 選択範囲・タイル配列確保

BOOL CDraw::allocSelArray()
{
    if(m_pimgSel->isExist())
        return TRUE;
    else
        return m_pimgSel->allocTileArray_incImgAndCanvas(&m_pCurLayer->m_img);
}

//! 選択範囲・透明部分のタイルを解放（全て透明なら削除）

void CDraw::freeSelTransparent()
{
    if(m_pimgSel->freeTileTransparent())
    {
        m_pimgSel->free();
        m_rcfSel.flag = FALSE;
    }
}

//! 選択範囲があるか

BOOL CDraw::isSelExist()
{
    return (m_rcfSel.flag && m_pimgSel->isExist());
}


//=================================
//選択範囲 コマンド
//=================================


//! 選択解除

void CDraw::sel_release(BOOL bUpdate)
{
    //イメージ解放

    m_pimgSel->free();

    //キャンバス更新

    if(bUpdate)
        updateCanvasAndPrevWin(m_rcfSel, TRUE);

    m_rcfSel.flag = FALSE;
}

//! すべて選択

void CDraw::sel_all()
{
    AXRect rc;

    m_pimgSel->free();

    if(allocSelArray())
    {
        m_pCurLayer->m_img.getEnableDrawRectPixel(&rc);

        m_pimgSel->fillBox(rc, 255, &CLayerImg::setPixel_create);

        m_rcfSel.set(rc.left, rc.top, rc.right, rc.bottom);

        updateCanvasAndPrevWin(m_rcfSel, TRUE);
    }
}

//! 選択反転

void CDraw::sel_inverse()
{
    AXRect rc;

    if(allocSelArray())
    {
        //反転

        m_datDraw.nColSetFunc  = CLayerImg::COLSETSUB_REV;
        m_datDraw.rcfDraw.flag = FALSE;

        m_pCurLayer->m_img.getEnableDrawRectPixel(&rc);
        m_pimgSel->fillBox(rc, 255, &CLayerImg::setPixel_subDraw);

        //範囲

        m_rcfSel = m_datDraw.rcfDraw;

        //透明部分を解放

        freeSelTransparent();

        //更新

        updateCanvasAndPrevWin(m_datDraw.rcfDraw, TRUE);
    }
}

//! 塗りつぶし/消去

void CDraw::sel_fillOrErase(BOOL bErase)
{
    AXRect rc;

    setCursorWait();
    setBeforeDraw(-1);  //マスクなどだけセット
    beginDrawUndo();

    m_datDraw.nColSetFunc = (bErase)? CLayerImg::COLSETF_ERASE: CLayerImg::COLSETF_BLEND;

    //範囲なしの場合は全体

    if(isSelExist())
        rc.set(m_rcfSel.x1, m_rcfSel.y1, m_rcfSel.x2, m_rcfSel.y2);
    else
        m_pCurLayer->m_img.getEnableDrawRectPixel(&rc);

    //描画

    m_pCurLayer->m_img.fillBox(rc, 255, &CLayerImg::setPixelDraw);

    commonAfterDraw();
    restoreCursor();
}

//! コピー/切り取り

void CDraw::sel_copy(BOOL bCut)
{
    AXRect rc;

    //選択範囲なし

    if(!isSelExist()) return;

    //イメージ作成

    calcSelCopyRect(&rc);

    if(!m_pimgCopy->allocTileArrayFromPx(rc)) return;

    //コピー (setPixel_subDraw)

    m_datDraw.nColSetFunc = CLayerImg::COLSETSUB_SET;

    m_pimgCopy->copySelectImage(m_pCurLayer->m_img, *m_pimgSel);

    //切り取り (setPixelDraw - 0で上書き)

    if(bCut)
    {
        setCursorWait();
        setBeforeDraw(TOOL_SEL);
        beginDrawUndo();

        m_pCurLayer->m_img.cutSelectImage(*m_pimgSel);

        commonAfterDraw();
        restoreCursor();
    }
}

//! 新規レイヤに貼り付け

void CDraw::sel_paste()
{
    if(m_pimgCopy->isExist())
        layer_newPaste();
}

//! 選択範囲ファイル保存

BOOL CDraw::sel_saveFile(const AXString &filename,const AXRectSize &rcs,BOOL bImg)
{
    AXFile file;
    CImage8 img8;
    DWORD pos,size;

    //8bitイメージ

    if(!m_pimgSel->convTo8Bit(&img8, rcs, TRUE)) return FALSE;

    //---------

    if(!file.openWrite(filename)) return FALSE;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    //ヘッダ

    file.writeStr("AZDWLSEL");
    file.writeBYTE(0);

    //フラグ

    file.writeBYTE((bImg)? 1: 0);

    //選択範囲イメージ

    file.writeDWORD(rcs.x);
    file.writeDWORD(rcs.y);
    file.writeDWORD(rcs.w);
    file.writeDWORD(rcs.h);

    pos = file.getPosition();
    file.writeDWORD((DWORD)0);

    size = AXZlib_EncodeToFile(&file, img8.getBuf(), rcs.w * rcs.h, 6);
    if(size == 0) return FALSE;

    file.seekTop(pos);
    file.writeDWORD(size);
    file.seekEnd(0);

    //レイヤイメージ

    if(bImg)
    {
        m_pCurLayer->m_img.convTo8Bit(&img8, rcs, FALSE);

        pos = file.getPosition();
        file.writeDWORD((DWORD)0);

        size = AXZlib_EncodeToFile(&file, img8.getBuf(), rcs.w * rcs.h, 6);
        if(size == 0) return FALSE;

        file.seekTop(pos);
        file.writeDWORD(size);
        file.seekEnd(0);
    }

    file.close();

    return TRUE;
}

//! 選択範囲ファイル読み込み

BOOL CDraw::sel_loadFile(const AXString &filename)
{
    AXFile file;
    BYTE ver,flag;
    DWORD size;
    AXRectSize rcs;
    CImage8 imgSel,imgLayer;

    //---------- 読み込み

    if(!file.openRead(filename)) return FALSE;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    //ヘッダ

    if(!file.readCompare("AZDWLSEL")) return FALSE;

    //バージョン

    file.read(&ver, 1);
    if(ver != 0) return FALSE;

    //フラグ

    file.read(&flag, 1);

    //範囲

    file.readDWORD(&rcs.x);
    file.readDWORD(&rcs.y);
    file.readDWORD(&rcs.w);
    file.readDWORD(&rcs.h);

    //選択範囲イメージ

    file.readDWORD(&size);
    if(size == 0) return FALSE;

    if(!imgSel.create(rcs.w, rcs.h)) return FALSE;

    if(!AXZlib_DecodeFromFile(imgSel.getBuf(), rcs.w * rcs.h, &file, size)) return FALSE;

    //レイヤイメージ

    if(flag & 1)
    {
        file.readDWORD(&size);
        if(size == 0) return FALSE;

        if(!imgLayer.create(rcs.w, rcs.h)) return FALSE;

        if(!AXZlib_DecodeFromFile(imgLayer.getBuf(), rcs.w * rcs.h, &file, size)) return FALSE;
    }

    file.close();

    //----------- セット

    //選択解除

    sel_release(TRUE);

    //確保

    if(!allocSelArray()) return FALSE;

    //セット

    if(flag & 1)
    {
        setCursorWait();
        setBeforeDraw_clearMask();
        beginDrawUndo();

        m_datDraw.nColSetFunc = CLayerImg::COLSETF_OVERWRITE;
    }

    m_pCurLayer->m_img.inportSel(imgSel, imgLayer, m_pimgSel, rcs);

    m_rcfSel.set(rcs.x, rcs.y, rcs.x + rcs.w - 1, rcs.y + rcs.h - 1);

    if(flag & 1)
    {
        commonAfterDraw();
        restoreCursor();
    }

    updateCanvasAndPrevWin(m_rcfSel, TRUE);

    return TRUE;
}


//===========================
//選択範囲 描画
//===========================


//! 多角形 共通

void CDraw::draw_sel_polygonCommon()
{
    BOOL bDel;

    setCursorWait();

    if(allocSelArray())
    {
        bDel = isDownKeyCtrlOnly();

        //多角形

        m_datDraw.nColSetFunc  = CLayerImg::COLSETSUB_SET;
        m_datDraw.rcfDraw.flag = FALSE;

        m_pimgSel->drawFillPolygonTmp(m_pPolyPos, (bDel)? 0: 255, &CLayerImg::setPixel_subDraw);

        //範囲

        calcUnionFlagRect(&m_rcfSel, m_datDraw.rcfDraw);

        //削除時は透明部分解放

        if(bDel) freeSelTransparent();

        //更新

        updateCanvasAndPrevWin(m_datDraw.rcfDraw, TRUE);
    }

    restoreCursor();
}

//! 四角

void CDraw::draw_sel_box()
{
    AXPoint pt[4];
    int i;

    //描画位置

    calcDrawBoxPointInt(pt);

    //点セット

    if(!m_pPolyPos->alloc(5)) return;

    for(i = 0; i < 4; i++)
        m_pPolyPos->add(pt[i].x, pt[i].y);

    m_pPolyPos->endPos();

    //描画

    draw_sel_polygonCommon();

    //終了

    m_pPolyPos->free();
}


//=============================
//選択範囲 ブラシ描画
//=============================


//! 押し時

BOOL CDraw::onDown_sel_brush()
{
    //配列準備

    if(!allocSelArray()) return FALSE;

    //描画準備

    setBeforeDraw_selBrush();

    m_rcfDrawUp.flag = FALSE;

    //

    m_nNowCtlNo = NOWCTL_SEL_BRUSH;

    return TRUE;
}

//! 移動時

void CDraw::onMove_sel_brush()
{
    DRAWPOINT pt;
    CPosBuf::POSDAT *p;

    //位置 （定規無効）

    getDrawPoint(&pt, FALSE);

    p = m_pPosBuf->addPos(pt.x, pt.y, pt.press, 0, 0);

    //描画

    m_datDraw.rcfDraw.flag = FALSE;

    m_pimgSel->drawBrush_free(p->x, p->y, p->press);

    calcUnionFlagRect(&m_rcfSel, m_datDraw.rcfDraw);

    commonFreeDraw();
}

//! 離し

void CDraw::onUp_sel_brush()
{
    //範囲削除時は透明部分解放

    if(m_datDraw.nColSetFunc == CLayerImg::COLSETSUB_CLEAR)
        freeSelTransparent();

    //プレビューウィンドウ更新

    updatePrevWin(m_rcfDrawUp);
}


//=============================
//選択範囲 ドラッグ移動
//=============================
/*
    update_blendImage() で m_pimgTmp を直接描画。
    移動はレイヤのオフセット移動で行う。

    m_pimgTmp : ドラッグイメージ

    m_ptTmp[1]  : 押し時のレイヤオフセット位置
    m_ptTmp[2]  : レイヤオフセット位置総移動px数
    m_rcfDrawUp : 押し時の選択範囲
*/


//! 押し

BOOL CDraw::onDown_sel_dragMove()
{
    AXRect rc;

    //m_pimgTmp にドラッグ用イメージ作成

    calcSelCopyRect(&rc);

    if(!m_pimgTmp->allocTileArrayFromPx(rc)) return FALSE;

    m_datDraw.nColSetFunc = CLayerImg::COLSETSUB_SET;
    m_pimgTmp->copySelectImage(m_pCurLayer->m_img, *m_pimgSel); //setPixel_subDraw

    //元範囲を切り取り＆UNDO開始

    setBeforeDraw(TOOL_SEL);
    beginDrawUndo();

    m_pCurLayer->m_img.cutSelectImage(*m_pimgSel);   //setPixelDraw - 0で上書き

    //作業用

    m_pimgTmp->getOffset(&m_ptTmp[1]);

    m_ptTmp[2].zero();

    m_nNowCtlNo = NOWCTL_SEL_DRAGMOVE;
    m_rcfDrawUp = m_rcfSel;

    //カーソル

    setCursorDrag(CURSOR_SELDRAG);

    return TRUE;
}

//! 移動

void CDraw::onMove_sel_dragMove()
{
    AXPoint pt,pt2,ptMov;
    FLAGRECT rcf;
    AXRectSize rcs;

    pt2.x = (int)(m_ptNowWinPos.x - m_ptDownWinPos.x);
    pt2.y = (int)(m_ptNowWinPos.y - m_ptDownWinPos.y);

    //総移動数 変換

    calcMoveDstPos(&pt, pt2);

    pt.x += m_ptTmp[1].x;
    pt.y += m_ptTmp[1].y;

    //前回からの変化数

    m_pimgTmp->getOffset(&ptMov);

    ptMov.x = pt.x - ptMov.x;
    ptMov.y = pt.y - ptMov.y;

    if(ptMov.x == 0 && ptMov.y == 0) return;

    //--------- オフセット移動

    m_pimgTmp->setOffset(ptMov.x, ptMov.y, TRUE);
    m_pimgSel->setOffset(ptMov.x, ptMov.y, TRUE);

    m_ptTmp[2].x += ptMov.x;
    m_ptTmp[2].y += ptMov.y;

    //--------- 更新

    rcf = m_rcfSel;

    calcAddFlagRect(&m_rcfSel, ptMov.x, ptMov.y);   //範囲情報も移動
    calcUnionFlagRect(&rcf, m_rcfSel);

    if(calcImgRectInCanvas(&rcs, rcf))
        CANVASAREA->setTimer_updateRect(rcs, 5);
}

//! 離し

void CDraw::onUp_sel_dragMove()
{
    BOOL bOverwrite;

    CANVASAREA->delTimer(CCanvasWinArea::TIMERID_UPDATERECT);

    //押し時 Ctrl+Shift で上書き

    bOverwrite = isDownKeyCtrlShiftOnly();

    //ドラッグイメージを合成 (setPixelDraw - 合成/上書き)

    setBeforeDraw_selDragMove(bOverwrite);
    m_datDraw.rcfDraw.flag = FALSE;

    m_pCurLayer->m_img.blendSelImage(*m_pimgTmp, *m_pimgSel, bOverwrite);

    //

    m_pimgTmp->free();

    //更新
    /* rcfDrawUp = 移動前の選択範囲。
       現在の範囲と元の範囲を含めた範囲で更新  */

    m_datDraw.rcfDraw = m_rcfSel;

    calcUnionFlagRect(&m_datDraw.rcfDraw, m_rcfDrawUp);
    commonAfterDraw();

    //カーソル

    restoreCursor();
}
