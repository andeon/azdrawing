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
	AzDrawing - main
*/


#include <unistd.h>

#include "CApp.h"
#include "CConfig.h"
#include "CDraw.h"
#include "CResource.h"
#include "CKeyDat.h"

#include "CMainWin.h"
#include "CCanvasWin.h"
#include "CToolWin.h"
#include "CBrushWin.h"
#include "CLayerWin.h"
#include "COptionWin.h"
#include "CPrevWin.h"
#include "CImgViewWin.h"

#define _DEF_NOEXTERN
#include "defGlobal.h"

#include "AXConfRead.h"
#include "AXConfWrite.h"
#include "AXFreeType.h"
#include "AXFile.h"
#include "AXUtilFile.h"

#include "deflang.h"

//----------------------------

#ifndef _AZDRAW_RESDIR
#define _AZDRAW_RESDIR "/usr/share/azdrawing"
#endif

CConfig     g_config;
AXFreeType  g_freetype;

void init();
void initCreateWindow();
void createConfig();
void loadConfig();
void saveConfig();

//----------------------------


//! メイン

int main(int argc,char **argv)
{
    CApp app;

    //App 初期化

    app.setConfigDirHome(".azdrawing");
    app.setResourceDir(_AZDRAW_RESDIR);

    if(!app.init(argc, argv)) return 1;

    app.loadTranslation(g_deflangdat);
    app.init2();

    createConfig();

    //

    g_pconf = &g_config;

    g_pdraw = new CDraw;

    //設定読み込み

    loadConfig();

    //初期化

    g_pres = new CResource;

    init();

    app.initXI(CANVASAREA);

    //コマンドラインファイル

    if(optind < argc)
    {
        AXString str;

        str.setLocal(argv[optind]);
        MAINWIN->loadImage(str);
    }

    //実行

    app.run();

    //--------- 終了

    //設定保存

    saveConfig();

    //削除

    delete g_pres;
    delete g_pdraw;

    //終了

    app.end2();     //作業用ディレクトリ削除
    app.end();

    return 0;
}

//! 設定ファイルディレクトリなど作成

void createConfig()
{
    AXString str,str2;

    //ディレクトリが存在するか

    axapp->getConfigPath(&str, "");

    if(AXIsExistFile(str, TRUE)) return;

    //----------

    //ディレクトリ作成

    axapp->createConfigDir();

    //brush.datをコピー

    axapp->getResourcePath(&str, "brush_def.dat");
    axapp->getConfigPath(&str2, "brush.dat");

    AXFile::copyFile(str, str2);

    //brush ディレクトリ作成

    axapp->getConfigPath(&str, "brush");
    AXCreateDir(str);

    //texture ディレクトリ作成

    axapp->getConfigPath(&str, "texture");
    AXCreateDir(str);
}

//! 初期化

void init()
{
    AXRectSize rcs;

    //CDraw 初期化

    g_pdraw->initAfterConfig();
    
    //ウィンドウ作成
    
    initCreateWindow();

    //新規イメージ

    g_pdraw->newImage(g_pconf->nInitImgW, g_pconf->nInitImgH, 300, TRUE);

    g_pdraw->updateImage();

    //----------- ウィンドウ表示

    //メインウィンドウ表示

    MAINWIN->showMainWin();

    //ツールウィンドウ

    TOOLWIN->showInit(g_pconf->ptToolWin, -1, -1, CConfig::WININITVAL, g_pconf->isViewToolWin());

    //ブラシウィンドウ

    rcs.set(-1, -1, 220, 380);
    BRUSHWIN->showInit(g_pconf->rcsBrushWin, rcs, CConfig::WININITVAL, g_pconf->isViewBrushWin());
    BRUSHWIN->initAfterShow();

    //オプションウィンドウ

    rcs.set(-1, -1, 220, 150);
    OPTWIN->showInit(g_pconf->rcsOptWin, rcs, CConfig::WININITVAL, g_pconf->isViewOptWin());

    //レイヤウィンドウ

    rcs.set(-1, -1, 210, 300);
    LAYERWIN->showInit(g_pconf->rcsLayerWin, rcs, CConfig::WININITVAL, g_pconf->isViewLayerWin());

    //プレビューウィンドウ

    rcs.set(-1, -1, 220, 220);
    PREVWIN->showInit(g_pconf->rcsPrevWin, rcs, CConfig::WININITVAL, g_pconf->isViewPrevWin());
    PREVWIN->draw();

    //イメージビュー

    rcs.set(-1, -1, 200, 200);
    IMGVIEWWIN->showInit(g_pconf->rcsImgViewWin, rcs, CConfig::WININITVAL, g_pconf->isViewImgViewWin());
}

//! ウィンドウ作成

void initCreateWindow()
{
    //各ウィンドウ作成

    UINT addstyle =
		(g_pconf->btToolboxType == CConfig::TOOLBOXTYPE_NORMAL)?
			AXWindow::WS_MENUBTT: AXWindow::WS_TRANSIENT_FOR; 

    new CMainWin;
    new CToolWin(MAINWIN, addstyle);
    new CBrushWin(MAINWIN, addstyle);
    new COptionWin(MAINWIN, addstyle);
    new CLayerWin(MAINWIN, addstyle);
    new CPrevWin(MAINWIN, addstyle);
    new CImgViewWin(MAINWIN, addstyle);

    //ウィンドウタイプセット
    
    long atom = axapp->getAtom(
		(g_pconf->btToolboxType == CConfig::TOOLBOXTYPE_NORMAL)?
			"_NET_WM_WINDOW_TYPE_NORMAL": "_NET_WM_WINDOW_TYPE_UTILITY");
	
	TOOLWIN->setWindowType(atom);
	BRUSHWIN->setWindowType(atom);
	OPTWIN->setWindowType(atom);
	LAYERWIN->setWindowType(atom);
	PREVWIN->setWindowType(atom);
	IMGVIEWWIN->setWindowType(atom);
}


//============================
// conf
//============================


//! 設定ファイル読み込み

void loadConfig()
{
    AXConfRead cf;
    AXString str;

    //---------

    axapp->getConfigPath(&str, "config.conf");

    cf.loadFile(str);

    //バージョン

    cf.setGroup("main");

    if(cf.getInt("ver", 0) != 1)
        cf.free();

    //CConfig

    g_pconf->load(&cf);

    //CDraw

    g_pdraw->loadConfig(&cf);

    cf.free();

    //---------

    //キーデータ

    axapp->getConfigPath(&str, "key.dat");

    g_pconf->pKey->loadFile(str);
}

//! 設定ファイル書き込み

void saveConfig()
{
    AXConfWrite cf;
    AXString str;

    //=============== config.conf

    axapp->getConfigPath(&str, "config.conf");

    if(!cf.open(str)) return;

    //バージョン

    cf.putGroup("main");
    cf.putInt("ver", 1);

    //------- CConfig

    //ウィンドウ位置・サイズ

    MAINWIN->getTopWindowRect(&g_pconf->rcsMainWin);
    g_pconf->bMaximized = MAINWIN->isMaximized();

    TOOLWIN->getTopWindowPos(&g_pconf->ptToolWin);
    BRUSHWIN->getTopWindowRect(&g_pconf->rcsBrushWin);
    OPTWIN->getTopWindowRect(&g_pconf->rcsOptWin);
    LAYERWIN->getTopWindowRect(&g_pconf->rcsLayerWin);
    PREVWIN->getTopWindowRect(&g_pconf->rcsPrevWin);
    IMGVIEWWIN->getTopWindowRect(&g_pconf->rcsImgViewWin);

    BRUSHWIN->getWinH();

    //保存

    g_pconf->save(&cf);

    //-------- CDraw

    g_pdraw->saveConfig(&cf);

    cf.close();

    //===============

    //キーデータ

    axapp->getConfigPath(&str, "key.dat");

    g_pconf->pKey->saveFile(str);
}
