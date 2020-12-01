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

#include "CConfig.h"

#include "CDevList.h"
#include "CKeyDat.h"
#include "CFilterConfigList.h"
#include "CApp.h"

#include "AXConfRead.h"
#include "AXConfWrite.h"


/*!
    @class CConfig
    @brief 設定データ
*/


CConfig::~CConfig()
{
    delete pDev;
    delete pKey;
    delete pFilterConf;
}

CConfig::CConfig()
{
    int i;

    pDev = new CDevList;
    pKey = new CKeyDat;
    pFilterConf = new CFilterConfigList;

    //初期化

    for(i = 0; i < GRIDLIST_NUM; i++)
        dwGridList[i] = (100 << 16) | 100;

    for(i = 0; i < NEWREG_NUM; i++)
    {
        dwNewRegSize[i] = (300 << 16) | 300;
        wNewRegDPI[i]   = 300;
    }

    for(i = 0; i < NEWRECENT_NUM; i++)
    {
        dwNewRecentSize[i] = (300 << 16) | 300;
        wNewRecentDPI[i]   = 300;
    }

    for(i = 0; i < IMGBKCOL_NUM; i++)
        dwImgBkColList[i] = 0xffffff;

    AXMemZero(dwLayerPal, sizeof(DWORD) * LAYERPAL_NUM);
    AXMemZero(wRuleAngle, sizeof(WORD) * RULEANGLE_NUM);
    AXMemZero(sRulePos, sizeof(short) * RULEPOS_NUM * 2);
    AXMemZero(wHeadTailList, sizeof(WORD) * HEADTAIL_NUM);

    wHeadTailList[1] = 100;

    //ブラシサイズ・濃度リスト

    memBrushSize.alloc(sizeof(WORD) * 20, sizeof(WORD) * 10);
}


//==========================
//
//==========================


//! ブラシサイズ・濃度リストの数取得

int CConfig::getBrushSizeCnt()
{
    return memBrushSize.getNowSize() / sizeof(WORD);
}

//! 新規作成の履歴を追加

void CConfig::addNewRecent(int w,int h,int dpi)
{
    DWORD val;
    int i,top;

    val = ((DWORD)w << 16) | h;
    top = NEWRECENT_NUM - 1;

    //すでに存在するか

    for(i = 0; i < NEWRECENT_NUM; i++)
    {
        if(dwNewRecentSize[i] == val && wNewRecentDPI[i] == dpi)
        {
            top = i;
            break;
        }
    }

    //先頭を空ける

    for(i = top; i > 0; i--)
    {
        dwNewRecentSize[i] = dwNewRecentSize[i - 1];
        wNewRecentDPI[i]   = wNewRecentDPI[i - 1];
    }

    dwNewRecentSize[0] = val;
    wNewRecentDPI[0]   = dpi;
}


//==========================
//設定ファイル
//==========================


//! 設定ファイルから読み込み

void CConfig::load(AXConfRead *pcf)
{
    AXByteString strb;
    AXString str;
    AXMem mem;
    int i,devcnt;

    //-------- window

    pcf->setGroup("window");

    pcf->getRectSize("main", &rcsMainWin, WININITVAL);
    pcf->getPoint("tool", &ptToolWin, WININITVAL);
    pcf->getRectSize("brush", &rcsBrushWin, WININITVAL);
    pcf->getRectSize("layer", &rcsLayerWin, WININITVAL);
    pcf->getRectSize("option", &rcsOptWin, WININITVAL);
    pcf->getRectSize("imgview", &rcsImgViewWin, WININITVAL);
    pcf->getRectSize("prev", &rcsPrevWin, WININITVAL);
    pcf->getRectSize("text", &rcsTextWin, WININITVAL);

    bMaximized = pcf->getInt("maximized", FALSE);

    nBrushWinH[0] = pcf->getInt("brushH1", 58);
    nBrushWinH[1] = pcf->getInt("brushH2", -1);

    //-------- env

    pcf->setGroup("env");

    uFlags      = pcf->getHex("flags", FLAG_MES_SAVE_OVERWRITE|FLAG_MES_SAVE_ADW);
    uViewFlags  = pcf->getHex("viewflags", VIEWFLAG_TOOLWIN|VIEWFLAG_LAYERWIN|VIEWFLAG_OPTIONWIN|VIEWFLAG_BRUSHWIN|VIEWFLAG_CANVASSCROLL);
    uViewBkFlags  = pcf->getHex("viewbkflags", 0);
    uPrevWinFlags = pcf->getHex("prevflags", PREVF_LEFT_CANVAS|PREVF_FULLVIEW);
    uImgVFlags  = pcf->getHex("imgvflags", IMGVF_FULLVIEW);
    uEtcFlags   = pcf->getHex("etcflags", ETCFLAG_FILTER_PREVIEW);
    uDrawShapeVal = pcf->getHex("drawshape", 255);

    nInitImgW   = pcf->getInt("initw", 400);
    nInitImgH   = pcf->getInt("inith", 400);
    nUndoCnt    = pcf->getInt("undocnt", 60);
    nPrevWinScale   = pcf->getInt("prevscale", 10000);
    nPrevWinLoupeScale = pcf->getInt("prevlpscale", 100);
    nScaleWidthUp   = pcf->getInt("scalewup", 100);
    nScaleWidthDown = pcf->getInt("scalewdown", 15);
    nRotateWidth    = pcf->getInt("rotwidth", 15);
    nGridW          = pcf->getInt("gridw", 100);
    nGridH          = pcf->getInt("gridh", 100);
    nGridSplitX     = pcf->getInt("gridspx", 2);
    nGridSplitY     = pcf->getInt("gridspy", 2);

    sDragBrushSizeW = pcf->getInt("dragbrushsizeW", 10);

    btOptWinTab     = pcf->getInt("optwintab", 0);
    btImgBkColSel   = pcf->getInt("imgbkcolsel", 0);
    btBrushWinTab   = pcf->getInt("brushwintab", 0);
    btOptHeadTail   = pcf->getInt("optheadtail", 0);
    btScaleType     = pcf->getInt("scaletype", 0);
    btPNGLevel      = pcf->getInt("pnglevel", 6);
    btJPEGQua       = pcf->getInt("jpegqua", 85);
    wJPEGSamp       = pcf->getInt("jpegsamp", 411);
    btExportFormat  = pcf->getInt("exportFormat", 0);
    btToolboxType   = pcf->getInt("toolboxtype", 0);

    dwCanvasBkCol   = pcf->getHex("canvasbkcol", 0xc0c0c0);
    dwImgBkCol      = pcf->getHex("imgbkcol", 0xffffff);
    dwGridCol       = pcf->getHex("gridcol", 0x400040ff);
    dwGridSplitCol  = pcf->getHex("gridspcol", 0x40ff00ff);

    pcf->getStr("opendir", &strOpenDir);
    pcf->getStr("savedir", &strSaveDir);
    pcf->getStr("exportdir", &strExportDir);
    pcf->getStr("imgviewdir", &strImgViewDir);
    pcf->getStr("texturedir", &strTextureDir);
    pcf->getStr("brushdir", &strBrushDir);
    pcf->getStr("subwinfont", &strSubWinFont);

    //テクスチャディレクトリ

    if(strTextureDir.isEmpty())
        axapp->getConfigPath(&strTextureDir, "texture");

    //ブラシディレクトリ

    if(strBrushDir.isEmpty())
        axapp->getConfigPath(&strBrushDir, "brush");

    //---------- dat

    pcf->setGroup("dat");

    pcf->getArrayVal("imgbkcollist", dwImgBkColList, IMGBKCOL_NUM, 4, TRUE);
    pcf->getArrayVal("gridlist", dwGridList, GRIDLIST_NUM, 4, TRUE);
    pcf->getArrayVal("newregsize", dwNewRegSize, NEWREG_NUM, 4, TRUE);
    pcf->getArrayVal("newrecsize", dwNewRecentSize, NEWRECENT_NUM, 4, TRUE);
    pcf->getArrayVal("newregdpi", wNewRegDPI, NEWREG_NUM, 2, FALSE);
    pcf->getArrayVal("newrecdpi", wNewRecentDPI, NEWRECENT_NUM, 2, FALSE);
    pcf->getArrayVal("layerpal", dwLayerPal, LAYERPAL_NUM, 4, TRUE);
    pcf->getArrayVal("ruleangle", wRuleAngle, RULEANGLE_NUM, 2, FALSE);
    pcf->getArrayVal("rulepos", sRulePos, RULEPOS_NUM * 2, 2, FALSE);
    pcf->getArrayVal("headtail", wHeadTailList, HEADTAIL_NUM, 2, FALSE);
    pcf->getBase64("drawcursor", &memDrawCursor);

    if(!pcf->getArrayVal("brushsize", &memBrushSize, 2))
    {
        //デフォルトセット
        memBrushSize.addWORD(10);
        memBrushSize.addWORD(0x8000 | 255);
    }

    //-------- filter

    if(pcf->setGroup("filter"))
    {
        while(pcf->getNextStr(&i, &strb))
            pFilterConf->setDat(i, strb);
    }

    //-------- recentfile

    pcf->setGroup("recentfile");

    pcf->getStrArray(0, strRecFile, RECFILE_NUM);

    //-------- デバイス

    CDevItem *pdi;

    pcf->setGroup("device");

    devcnt = pcf->getInt("cnt", 0);

    CAPP->setDevice(pDev);

    for(i = 0; i < devcnt; i++)
    {
        strb.setAddInt("name", i);
        pcf->getStr(strb, &str);

        pdi = pDev->addDevice(str);

        //動作

        strb.setAddInt("act", i);
        pcf->getBase64(strb, &mem);

        pdi->setAction(&mem);
    }
}

//! 設定ファイルに書き込み

void CConfig::save(AXConfWrite *pcf)
{
    AXByteString strb;
    int i;

    //-------- window

    pcf->putGroup("window");

    pcf->putRectSize("main", rcsMainWin);
    pcf->putPoint("tool", ptToolWin);
    pcf->putRectSize("brush", rcsBrushWin);
    pcf->putRectSize("layer", rcsLayerWin);
    pcf->putRectSize("option", rcsOptWin);
    pcf->putRectSize("imgview", rcsImgViewWin);
    pcf->putRectSize("prev", rcsPrevWin);
    pcf->putRectSize("text", rcsTextWin);

    pcf->putInt("maximized", bMaximized);

    pcf->putInt("brushH1", nBrushWinH[0]);
    pcf->putInt("brushH2", nBrushWinH[1]);

    //--------- env

    pcf->putGroup("env");

    pcf->putHex("flags", uFlags);
    pcf->putHex("viewflags", uViewFlags);
    pcf->putHex("viewbkflags", uViewBkFlags);
    pcf->putHex("prevflags", uPrevWinFlags);
    pcf->putHex("imgvflags", uImgVFlags);
    pcf->putHex("etcflags", uEtcFlags);
    pcf->putHex("drawshape", uDrawShapeVal);

    pcf->putInt("initw", nInitImgW);
    pcf->putInt("inith", nInitImgH);
    pcf->putInt("undocnt", nUndoCnt);
    pcf->putInt("prevscale", nPrevWinScale);
    pcf->putInt("prevlpscale", nPrevWinLoupeScale);
    pcf->putInt("scalewup", nScaleWidthUp);
    pcf->putInt("scalewdown", nScaleWidthDown);
    pcf->putInt("rotwidth", nRotateWidth);
    pcf->putInt("gridw", nGridW);
    pcf->putInt("gridh", nGridH);
    pcf->putInt("gridspx", nGridSplitX);
    pcf->putInt("gridspy", nGridSplitY);

    pcf->putInt("dragbrushsizeW", sDragBrushSizeW);

    pcf->putInt("optwintab", btOptWinTab);
    pcf->putInt("imgbkcolsel", btImgBkColSel);
    pcf->putInt("brushwintab", btBrushWinTab);
    pcf->putInt("optheadtail", btOptHeadTail);
    pcf->putInt("scaletype", btScaleType);
    pcf->putInt("pnglevel", btPNGLevel);
    pcf->putInt("jpegqua", btJPEGQua);
    pcf->putInt("jpegsamp", wJPEGSamp);
    pcf->putInt("exportFormat", btExportFormat);
    pcf->putInt("toolboxtype", btToolboxType);

    pcf->putHex("canvasbkcol", dwCanvasBkCol);
    pcf->putHex("imgbkcol", dwImgBkCol);
    pcf->putHex("gridcol", dwGridCol);
    pcf->putHex("gridspcol", dwGridSplitCol);

    pcf->putStr("opendir", strOpenDir);
    pcf->putStr("savedir", strSaveDir);
    pcf->putStr("exportdir", strExportDir);
    pcf->putStr("imgviewdir", strImgViewDir);
    pcf->putStr("texturedir", strTextureDir);
    pcf->putStr("brushdir", strBrushDir);
    pcf->putStr("subwinfont", strSubWinFont);

    //-------- dat

    pcf->putGroup("dat");

    pcf->putArrayHex("imgbkcollist", dwImgBkColList, IMGBKCOL_NUM);
    pcf->putArrayHex("gridlist", dwGridList, GRIDLIST_NUM);
    pcf->putArrayHex("newregsize", dwNewRegSize, NEWREG_NUM);
    pcf->putArrayHex("newrecsize", dwNewRecentSize, NEWRECENT_NUM);
    pcf->putArrayVal("newregdpi", wNewRegDPI, NEWREG_NUM, 2);
    pcf->putArrayVal("newrecdpi", wNewRecentDPI, NEWRECENT_NUM, 2);
    pcf->putArrayHex("layerpal", dwLayerPal, LAYERPAL_NUM);
    pcf->putArrayVal("ruleangle", wRuleAngle, RULEANGLE_NUM, 2);
    pcf->putArrayVal("rulepos", sRulePos, RULEPOS_NUM * 2, 2);
    pcf->putArrayVal("headtail", wHeadTailList, HEADTAIL_NUM, 2);
    pcf->putArrayVal("brushsize", memBrushSize, getBrushSizeCnt(), 2);
    pcf->putBase64("drawcursor", memDrawCursor, memDrawCursor.getSize());

    //--------- filter

    pcf->putGroup("filter");

    pFilterConf->writeConf(pcf);

    //--------- recentfile

    pcf->putGroup("recentfile");
    pcf->putStrArray(0, strRecFile, RECFILE_NUM);

    //--------- デバイス

    CDevItem *p;

    pcf->putGroup("device");
    pcf->putInt("cnt", pDev->getCnt());

    for(p = pDev->getTopItem(), i = 0; p; p = p->next(), i++)
    {
        strb.setAddInt("name", i);
        pcf->putStr(strb, p->getName());

        strb.setAddInt("act", i);
        pcf->putBase64(strb, p->getActBuf(), p->getActBufSize());
    }
}
