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

#include "CMainWin.h"

#include "CCanvasWin.h"
#include "CStatusBar.h"
#include "CToolWin.h"
#include "CBrushWin.h"
#include "COptionWin.h"
#include "CLayerWin.h"
#include "CPrevWin.h"
#include "CImgViewWin.h"

#include "CConfig.h"
#include "CDraw.h"

#include "AXLayout.h"
#include "AXMenu.h"
#include "AXMenuBar.h"
#include "AXAboutDialog.h"
#include "AXAccelerator.h"
#include "AXKey.h"
#include "AXDND.h"
#include "AXMessageBox.h"
#include "AXUtilFile.h"
#include "AXUtilRes.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"

#define _AZDW_MENUDAT_MAINMENU
#include "menudat.h"

#define _APP_ABOUT_NAME      "AzDrawing ver 1.5"
#define _APP_ABOUT_COPYRIGHT "Copyright (C) 2013-2015 Azel"

//----------------------


/*!
    @class CMainWin
    @brief メインウィンドウ
*/


CMainWin *CMainWin::m_pSelf = NULL;


CMainWin::~CMainWin()
{
    if(m_pAccel) delete m_pAccel;
}

CMainWin::CMainWin()
    : AXTopWindow(NULL, WS_TOP_NORMAL | WS_HIDE | WS_DISABLE_IM | WS_DROP)
{
    AXLayout *plTop;
    AXMem mem;

    m_pSelf = this;

    m_uTmpFlags = 0;

    enableDND();

    //メインレイアウト

    setLayout(plTop = new AXLayoutVert);

    //メニュー

    _createMenu();

    _setAccelerator();

    //キャンバスとステータスバー

    plTop->addItem(new CCanvasWin(this));
    plTop->addItem(new CStatusBar(this));

    //アイコン

    if(AXLoadResAppIcon(&mem, "azdrawing.png"))
        setAppIcon(mem, TRUE);
}

//! ウィンドウ初期表示

void CMainWin::showMainWin()
{
    int w,h;

    setWinTitle();

    //サイズ変更

    w = g_pconf->rcsMainWin.w;
    h = g_pconf->rcsMainWin.h;

    if(w == CConfig::WININITVAL) w = 550;
    if(h == CConfig::WININITVAL) h = 450;

    if(g_pconf->bMaximized)
        w = 550, h = 450;

    resize(w, h);

    //

    g_pdraw->initShow();

    STATUSBAR->setInfoLabel();
    STATUSBAR->setLabel_help();

    show();

    //位置

    if(g_pconf->rcsMainWin.x == CConfig::WININITVAL ||
        g_pconf->rcsMainWin.y == CConfig::WININITVAL)
        moveCenter();
    else
        moveInRoot(g_pconf->rcsMainWin.x, g_pconf->rcsMainWin.y);

    //最大化

    if(g_pconf->bMaximized)
    {
        maximize();

        m_uTmpFlags |= TMPFLAG_MAXIMIZE_INIT;
    }
}


//==============================
// ハンドラ
//==============================


//! 閉じる

BOOL CMainWin::onClose()
{
	if(checkImageUpdate())
		axapp->exit();

    return TRUE;
}

//! 表示時

BOOL CMainWin::onMap()
{
    m_uTmpFlags |= TMPFLAG_SHOW;

    return TRUE;
}

//! サイズなど変更時

BOOL CMainWin::onConfigure(AXHD_CONFIGURE *phd)
{
    AXTopWindow::onConfigure(phd);

    /* [初期表示で最大化時]
        初期表示が最大化の場合に、キャンバスサイズからの表示倍率計算が正しく行えるようにする。
        サイズがウィンドウマネージャによって変更された時に実行。 */

    if((m_uTmpFlags & TMPFLAG_MAXIMIZE_INIT) && isMaximized())
    {
        m_uTmpFlags &= ~TMPFLAG_MAXIMIZE_INIT;

        g_pdraw->initShow();
    }

    return TRUE;
}

//! D&D

BOOL CMainWin::onDND_Drop(AXDND *pDND)
{
    AXString str;

    if(pDND->getFirstFile(&str))
    {
        pDND->endDrop();

        //

        if(checkImageUpdate())
            loadImage(str);
    }

    return TRUE;
}

//! コマンド
/*!
    キー設定でのショートカットキーやデバイス設定でのコマンドとしても実行される。
*/

BOOL CMainWin::onCommand(UINT uID,ULONG lParam,int from)
{
    //キャンバス操作中は実行しない

    if(!g_pdraw->isNowCtlNone()) return TRUE;

    //ショートカットキーなどのコマンド

    if(uID >= 1000 && uID < 10000)
    {
        _command_ex(uID);
        return TRUE;
    }

    //レイヤコマンド

    if(uID >= STRID_LAYERMENU_TOP && uID < STRID_LAYERMENU_END)
        return LAYERWIN->onCommand(uID, lParam, from);

    //ファイル履歴

    if(uID >= CMDID_MENU_RECENTFILE && uID < CMDID_MENU_RECENTFILE + CConfig::RECFILE_NUM)
    {
        if(AXIsExistFile(g_pconf->strRecFile[uID - CMDID_MENU_RECENTFILE]))
        {
            if(checkImageUpdate())
                loadImage(g_pconf->strRecFile[uID - CMDID_MENU_RECENTFILE]);
        }

        return TRUE;
    }

    //フィルタ

    if(uID >= STRID_MENU_FILTER_TOP && uID < STRID_MENU_FILTER_END)
    {
        _command_filter(uID);
        return TRUE;
    }

    //指定形式でエクスポート

    if(uID >= STRID_MENU_FILE_FORMAT_ADW && uID <= STRID_MENU_FILE_FORMAT_PSD8BIT)
    {
        exportFile(_convFormatType(uID - STRID_MENU_FILE_FORMAT_ADW));
        return TRUE;
    }

    //-------------

    switch(uID)
    {
        //新規作成
        case STRID_MENU_FILE_NEW:
            newImage();
            break;
        //開く
        case STRID_MENU_FILE_OPEN:
            openFile();
            break;
        //上書き保存
        case STRID_MENU_FILE_SAVE:
            saveFile(FALSE);
            break;
        //別名保存
        case STRID_MENU_FILE_SAVERE:
            saveFile(TRUE);
            break;
        //エクスポート
        case STRID_MENU_FILE_EXPORT:
            exportFile();
            break;
        //終了
        case STRID_MENU_FILE_EXIT:
            onClose();
            break;

        //元に戻す
        case STRID_MENU_EDIT_UNDO:
            undoRedo(FALSE);
            break;
        //やり直す
        case STRID_MENU_EDIT_REDO:
            undoRedo(TRUE);
            break;
        //キャンバスサイズ変更
        case STRID_MENU_EDIT_RESIZECANVAS:
            resizeCanvas();
            break;
        //キャンバス拡大縮小
        case STRID_MENU_EDIT_SCALECANVAS:
            scaleCanvas();
            break;
        //DPI値変更
        case STRID_MENU_EDIT_CHANGEDPI:
            changeDPI();
            break;
        //レイヤ左右反転
        case STRID_MENU_EDIT_CURHREV:
            g_pdraw->layer_curHRev();
            break;
        //全レイヤ左右反転
        case STRID_MENU_EDIT_ALLHREV:
            g_pdraw->layer_allHRev();
            break;
        //座標を指定して図形描画
        case STRID_MENU_EDIT_DRAWSHAPE:
            drawShape();
            break;

        //選択解除
        case STRID_MENU_SEL_DESELECT:
            g_pdraw->sel_release(TRUE);
            break;
        //すべて選択
        case STRID_MENU_SEL_ALLSEL:
            g_pdraw->sel_all();
            break;
        //選択反転
        case STRID_MENU_SEL_INVERSE:
            g_pdraw->sel_inverse();
            break;
        //選択塗りつぶし
        case STRID_MENU_SEL_FILL:
            g_pdraw->sel_fillOrErase(FALSE);
            break;
        //選択範囲消去
        case STRID_MENU_SEL_ERASE:
            g_pdraw->sel_fillOrErase(TRUE);
            break;
        //コピー
        case STRID_MENU_SEL_COPY:
            g_pdraw->sel_copy(FALSE);
            break;
        //切り取り
        case STRID_MENU_SEL_CUT:
            g_pdraw->sel_copy(TRUE);
            break;
        //貼り付け
        case STRID_MENU_SEL_PASTENEW:
            g_pdraw->sel_paste();
            break;
        //インポート
        case STRID_MENU_SEL_INPORT:
            sel_inport();
            break;
        //エクスポート（選択）
        case STRID_MENU_SEL_EXPORT_SEL:
            sel_export(FALSE);
            break;
        //エクスポート（選択＋イメージ）
        case STRID_MENU_SEL_EXPORT_SELIMG:
            sel_export(TRUE);
            break;

        //ツールウィンドウ
        case STRID_MENU_VIEW_TOOLWIN:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_TOOLWIN;
            TOOLWIN->showChange();
            break;
        //ブラシウィンドウ
        case STRID_MENU_VIEW_BRUSHWIN:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_BRUSHWIN;
            BRUSHWIN->showChange();
            break;
        //オプションウィンドウ
        case STRID_MENU_VIEW_OPTIONWIN:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_OPTIONWIN;
            OPTWIN->showChange();
            break;
        //レイヤウィンドウ
        case STRID_MENU_VIEW_LAYERWIN:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_LAYERWIN;
            LAYERWIN->showChange();
            break;
        //プレビューウィンドウ
        case STRID_MENU_VIEW_PREVIEWWIN:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_PREVWIN;
            PREVWIN->showChange();
            break;
        //イメージビューウィンドウ
        case STRID_MENU_VIEW_IMGVIEWWIN:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_IMGVIEWWIN;
            IMGVIEWWIN->showChange();
            break;
        //全ウィンドウ非表示/復元
        case STRID_MENU_VIEW_ALLHIDEVIEW:
            allWindowHideShow();
            break;
        //表示倍率・回転リセット
        case STRID_MENU_VIEW_RESET:
            g_pdraw->resetCanvas();
            break;
        //グリッド
        case STRID_MENU_VIEW_GRID:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_GRID;
            g_pdraw->updateCanvas();
            break;
        //分割線
        case STRID_MENU_VIEW_GRIDSPLIT:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_GRIDSPLIT;
            g_pdraw->updateCanvas();
            break;
        //キャンバス左右反転表示
        case STRID_MENU_VIEW_CANVASHREV:
            g_pdraw->canvasHRev();

            TOOLWIN->checkTB_cmd(CToolWin::CMDID_CANVASHREV, g_pdraw->m_bCanvasHRev);
            break;
        //キャンバススクロールバー表示
        case STRID_MENU_VIEW_CANVASSCROLLBAR:
            g_pconf->uViewFlags ^= CConfig::VIEWFLAG_CANVASSCROLL;
            CANVASWIN->reconfig();
            break;

        //環境設定
        case STRID_MENU_OPT_OPTION:
            envOption();
            break;
        //グリッド設定
        case STRID_MENU_OPT_GRID:
            gridOption();
            break;
        //キー設定
        case STRID_MENU_OPT_KEY:
            keyOption();
            break;
        //デバイス設定
        case STRID_MENU_OPT_DEVICE:
            deviceOption();
            break;

        //情報
        case STRID_MENU_HELP_ABOUT:
            AXAboutDialog::about(this, _APP_ABOUT_NAME, _APP_ABOUT_COPYRIGHT);
            break;
    }

    return TRUE;
}

//! ポップアップメニュー表示時

BOOL CMainWin::onMenuPopup(AXMenuBar *pMenuBar,AXMenu *pMenu,UINT uID)
{
    if(pMenuBar == m_pMenuBar)
    {
        if(uID == STRID_MENU_TOP_VIEW)
        {
            pMenu->check(STRID_MENU_VIEW_TOOLWIN, g_pconf->isViewToolWin());
            pMenu->check(STRID_MENU_VIEW_BRUSHWIN, g_pconf->isViewBrushWin());
            pMenu->check(STRID_MENU_VIEW_OPTIONWIN, g_pconf->isViewOptWin());
            pMenu->check(STRID_MENU_VIEW_LAYERWIN, g_pconf->isViewLayerWin());
            pMenu->check(STRID_MENU_VIEW_PREVIEWWIN, g_pconf->isViewPrevWin());
            pMenu->check(STRID_MENU_VIEW_IMGVIEWWIN, g_pconf->isViewImgViewWin());

            pMenu->check(STRID_MENU_VIEW_GRID, g_pconf->isGrid());
            pMenu->check(STRID_MENU_VIEW_GRIDSPLIT, g_pconf->isGridSplit());

            pMenu->check(STRID_MENU_VIEW_CANVASHREV, g_pdraw->m_bCanvasHRev);
            pMenu->check(STRID_MENU_VIEW_CANVASSCROLLBAR, g_pconf->isCanvasScroll());
        }
    }

    return TRUE;
}


//==============================
// サブ処理
//==============================


//! ファイルダイアログ時の初期ディレクトリ取得

void CMainWin::getFileInitDir(AXString *pstr)
{
    if(m_strFileName.isEmpty())
        *pstr = g_pconf->strSaveDir;
    else
        pstr->path_removeFileName(m_strFileName);
}

//! ウィンドウタイトルのセット

void CMainWin::setWinTitle()
{
    AXString str(255),str2;

    str = "AzDrawing - ";

    //ファイル名

    if(m_strFileName.isEmpty())
        str += "New";
    else
    {
        //ディレクトリ除く
        str2.path_filename(m_strFileName);
        str += str2;
    }

    setTitle(str);
}

//! 保存確認
/*!
    @return FALSE で処理をキャンセル
*/

BOOL CMainWin::checkImageUpdate()
{
    UINT ret;

    if(g_pdraw->isChangeImage())
    {
        ret = AXMessageBox::message(this, NULL,
                    _string(STRGID_MESSAGE, STRID_MES_SAVE_CHANGEIMAGE),
                    AXMessageBox::SAVE | AXMessageBox::SAVENO | AXMessageBox::CANCEL,
                    AXMessageBox::CANCEL);

        //キャンセル

        if(ret == AXMessageBox::CANCEL) return FALSE;

        //保存する

        if(ret == AXMessageBox::SAVE)
        {
            if(!saveFile(FALSE)) return FALSE;
        }
    }

    return TRUE;
}

//! 編集サイズが変更された後の更新
/*!
    @param pstr ファイル名。NULLで変更なし。空で新規作成
*/

void CMainWin::updateNewCanvas(AXString *pstr)
{
    g_pdraw->changeImageSize();
    g_pdraw->setScroll_default();

    //------- ファイル変更時

    if(pstr)
    {
        //編集ファイル名・タイトル

        m_strFileName = *pstr;
        setWinTitle();

        //

        g_pdraw->changeFile();
    }

    //------- 各更新

    STATUSBAR->setInfoLabel();

    g_pdraw->changeCanvasWinSize(FALSE);

    PREVWIN->changeImgSize();

    g_pdraw->updateAll();

    //レイヤ

    g_pdraw->updateLayerPrevAll(FALSE);

    LAYERWIN->updateLayerAll(TRUE);
}


//==============================
// 初期化処理
//==============================


//! メニュー作成

void CMainWin::_createMenu()
{
    AXMenuBar *pbar;

    _trgroup(STRGID_MAINMENU);

    //------ メニューバー

    pbar = new AXMenuBar(this, 0, LF_EXPAND_W);

    getLayout()->addItem(pbar);

    attachMenuBar(pbar);

    //------ 項目

    pbar->addItemTrMultiple(g_mainmenudat, sizeof(g_mainmenudat)/sizeof(WORD), STRID_MENU_FILE_NEW);

    //最近使ったファイル

    m_pmenuRecFile = new AXMenu;
    m_pmenuRecFile->setStrArray(CMDID_MENU_RECENTFILE, g_pconf->strRecFile, CConfig::RECFILE_NUM);

    pbar->setItemSubMenu(STRID_MENU_FILE_RECENT, m_pmenuRecFile);
}

//! アクセラレータセット

void CMainWin::_setAccelerator()
{
    UINT i;
    WORD idMain[] = {
        STRID_MENU_FILE_NEW, STRID_MENU_FILE_OPEN, STRID_MENU_FILE_SAVE, STRID_MENU_FILE_SAVERE,
        STRID_MENU_FILE_EXPORT,
        STRID_MENU_EDIT_UNDO, STRID_MENU_EDIT_REDO,
        STRID_MENU_SEL_DESELECT, STRID_MENU_SEL_INVERSE, STRID_MENU_SEL_ALLSEL,
        STRID_MENU_SEL_COPY, STRID_MENU_SEL_CUT, STRID_MENU_SEL_PASTENEW,
        STRID_MENU_SEL_FILL, STRID_MENU_SEL_ERASE, STRID_MENU_VIEW_ALLHIDEVIEW,
        STRID_MENU_VIEW_GRID, STRID_MENU_VIEW_GRIDSPLIT, STRID_MENU_VIEW_CANVASHREV, STRID_MENU_OPT_KEY,
        STRID_MENU_VIEW_RESET
    };
    WORD idLayer[] = {
        STRID_LAYERMENU_NEW, STRID_LAYERMENU_LOADIMG, STRID_LAYERMENU_COPY, STRID_LAYERMENU_DEL,
        STRID_LAYERMENU_CLEAR, STRID_LAYERMENU_COMBINECLEAR, STRID_LAYERMENU_COMBINEDEL,
        STRID_LAYERMENU_OPTION, STRID_LAYERMENU_ALLSHOW, STRID_LAYERMENU_ALLHIDE,
        STRID_LAYERMENU_CURSHOW, STRID_LAYERMENU_GROUPSHOWTOGGLE, STRID_LAYERMENU_CHECKSHOWTOGGLE,
        STRID_LAYERMENU_COMBINE_SUB
    };
    UINT keyMain[] = {
        ACKEY_CTRL|'N', ACKEY_CTRL|'O', ACKEY_CTRL|'S', ACKEY_CTRL|'W',
        ACKEY_CTRL|'E',
        ACKEY_CTRL|'Z', ACKEY_CTRL|'Y',
        ACKEY_CTRL|'D', ACKEY_CTRL|'I', ACKEY_CTRL|'A',
        ACKEY_CTRL|'C', ACKEY_CTRL|'X', ACKEY_CTRL|'V',
        KEY_INSERT, KEY_DELETE, ACKEY_CTRL|KEY_TAB,
        ACKEY_CTRL|'G', ACKEY_CTRL|'M', ACKEY_CTRL|'Q', ACKEY_CTRL|'K',
        ACKEY_CTRL|'F'
    };
    UINT keyLayer[] = {
        ACKEY_CTRL|ACKEY_SHIFT|'N', ACKEY_CTRL|ACKEY_SHIFT|'I', ACKEY_CTRL|ACKEY_SHIFT|'C', ACKEY_CTRL|ACKEY_SHIFT|'D',
        ACKEY_CTRL|ACKEY_SHIFT|'X', ACKEY_CTRL|ACKEY_SHIFT|'Z', ACKEY_CTRL|ACKEY_SHIFT|'B',
        ACKEY_CTRL|ACKEY_SHIFT|'O', ACKEY_CTRL|ACKEY_SHIFT|'A', ACKEY_CTRL|ACKEY_SHIFT|'H',
        ACKEY_CTRL|ACKEY_SHIFT|'L', ACKEY_CTRL|ACKEY_SHIFT|'G', ACKEY_CTRL|ACKEY_SHIFT|'Q',
        ACKEY_CTRL|ACKEY_SHIFT|'U'
    };

    //

    attachAccel(new AXAccelerator);

    m_pAccel->setDefaultWindow(this);

    //メインウィンドウ

    for(i = 0; i < sizeof(idMain)/sizeof(WORD); i++)
        m_pAccel->add(idMain[i], keyMain[i]);

    //レイヤウィンドウ

    for(i = 0; i < sizeof(idLayer)/sizeof(WORD); i++)
        m_pAccel->add(idLayer[i], keyLayer[i]);
}
