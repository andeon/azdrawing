/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#define _AX_X11_UTIL
#define _AX_X11_LOCALE
#define _AX_X11_ATOM
#define _AX_X11_XSHM
#define _AX_X11_XI2
#include "AXX11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/select.h>
#include <fcntl.h>

#include "AXApp.h"
#include "AXRootWindow.h"
#include "AXAppWinHash.h"
#include "AXAppRes.h"
#include "AXAppAtom.h"
#include "AXFont.h"
#include "AXList.h"
#include "AXMem.h"
#include "AXTranslation.h"
#include "AXClipboard.h"
#include "AXDND.h"
#include "AXString.h"
#include "AXCursor.h"
#include "AXUtil.h"
#include "AXUtilX.h"
#include "AXUtilFile.h"
#include "AXUtilTime.h"

#ifndef _AX_OPT_NOIM
 #include "AXAppIM.h"
#endif
#ifndef _AX_OPT_NOTHREAD
 #include "AXThread.h"
#endif

//--------------------------

#define DISP        ((Display*)m_pDisp)
#define ERROR(mes)  fprintf(stderr, mes)

//#define _PUTEVENT

//--------------------------

AXApp *AXApp::m_pApp    = NULL;

ULONG AXApp::m_dbcWin   = 0;
ULONG AXApp::m_dbcTime  = 0;
int AXApp::m_dbcX       = 0;
int AXApp::m_dbcY       = 0;
UINT AXApp::m_dbcButton = 0;

WORD g_wEmptyString[2] = {'-',0};    //空文字列用

//--------------------------

int XErrHandler(Display *,XErrorEvent *);

//--------------------------

//実行データ

class AXAppRun
{
public:
    AXAppRun    *pBackRun;  //戻るデータ
    BOOL        bEndFlag;   //終了フラグ
    AXWindow    *pModal,    //モーダルウィンドウ（このウィンドウ以外の処理は行わない）
                *pPopup;    //ポップアップウィンドウ（キー処理を常にこのウィンドウで行う）

    AXAppRun(AXAppRun *pBack)
        :pBackRun(pBack),bEndFlag(0),pModal(NULL),pPopup(NULL) { }
    AXAppRun(AXAppRun *pBack,AXWindow *modal)
        :pBackRun(pBack),bEndFlag(0),pModal(modal),pPopup(NULL) { }
};

//タイマーデータ

class AXAppTimerItem:public AXListItem
{
public:
    AXWindow    *m_pWin;
    UINT        m_uTimerID;
    ULONG       m_lParam;
    ULONGLONG   m_lInterval;    //ナノ秒
    TIMENANO    m_timeEnd;

    AXAppTimerItem *prev() const { return (AXAppTimerItem *)m_pPrev; }
    AXAppTimerItem *next() const { return (AXAppTimerItem *)m_pNext; }
};

//--------------------------

/*!
    @class AXApp
    @brief アプリケーションクラス

    @ingroup core

    - アイコンなどの画像ファイルを置く場所は setResourceDir() で、設定ファイルなどを保存する場所は setConfigDir() で指定する。@n
      init() 内でコマンドライン引数によりパスが変更されるかもしれないので、set~Dir() は init() より前に実行すること。
    - XInput2 を使う場合は、AXApp の派生クラスを作って onInputEvent() を処理する。
    - グラブを AXApp で管理しているのは、
      ダイアログでの ESC/TAB/ENTER 押し・アクセラレータコマンド・メニューのホットキーでの表示 などが
      グラブ中に実行されないように判定するため。@n
      XInput2 側でグラブする場合は、isGrab() をオーバーライド処理すること。


    <h2>[コマンドライン引数]</h2>

    - <b>--font "フォントパターン(Xft)"</b> @n
      GUI のフォントを指定。"フォント名-サイズ(pt)" または "フォント名:size=サイズ"
    - <b>--lang 言語名</b> @n
      翻訳データの言語を指定。（例： "en_US"）
    - <b>--selfdir</b> @n
      リソースファイルや設定ファイル保存先を実行ファイルと同じディレクトリに設定する。@n
      インストールせずにソフトを実行することが出来る。

    <h2>[マクロ]</h2>

    - <b>axapp</b> @n
    AXApp のポインタ。(AXApp::m_pApp)
    - <b>axdisp()</b> @n
    ((Display *)(AXApp::m_pApp)->getDisp())
    - <b>axres</b> @n
    ((AXApp::m_pApp)->getRes())
    - <b>axclipb</b> @n
    ((AXApp::m_pApp)->getClipboard())
    - <b>axatom(no)</b> @n
    AXAppAtom からアトム識別子取得。((AXApp::m_pApp)->atom(no))
    - <b>_trgroup(id)</b> @n
    翻訳のカレントグループをセット
    - <b>_str(id)</b> @n
    翻訳のカレントグループから文字列取得(LPCUSTR)
    - <b>_string(gid,id)</b> @n
    翻訳の指定グループから文字列取得(LPCUSTR)
*/


//--------------------------


AXApp::AXApp()
{
    m_pApp      = this;

    m_pDisp     = NULL;
    m_pwinRoot  = NULL;

    m_pWinTbl   = NULL;
    m_pRes      = NULL;
    m_pAtom     = NULL;

    m_pTimerList    = NULL;
    m_pNotifyList   = NULL;

    m_ptransDef     = NULL;
    m_ptransApp     = NULL;
    m_pClipboard    = NULL;
    m_pDND          = NULL;
    m_pIM           = NULL;

    ::memset(m_pCursor, 0, sizeof(LPVOID) * CURSOR_NUM);

    m_nXIopcode = -1;

    m_pMutex    = NULL;
    m_fdPipe[0] = m_fdPipe[1] = 0;

    m_pNowRun   = NULL;

    m_pDrawTop = m_pDrawBottom = NULL;
    m_pReConfTop = m_pReConfBottom = NULL;

    m_pwinGrab = NULL;

    m_bUpdate       = FALSE;
    m_uSupportFlag  = 0;
    m_timeLast = m_timeUser = 0;

    m_cLang[0] = 0;

    m_strResDir.path_setExePath();
    m_strConfDir = m_strResDir;
}

AXApp::~AXApp()
{
    end();
}

//! 終了

void AXApp::end()
{
    int i;

    if(m_pDisp)
    {
        //ルートウィンドウ削除（残っているウィンドウすべて削除）

        _DELETE(m_pwinRoot);

        //スレッド関連

    #if !defined(_AX_OPT_NOTHREAD)

        if(m_fdPipe[0]) { ::close(m_fdPipe[0]); m_fdPipe[0] = 0; }
        if(m_fdPipe[1]) { ::close(m_fdPipe[1]); m_fdPipe[1] = 0; }

        if(m_pMutex)
        {
            m_pMutex->unlock();
            delete m_pMutex;
            m_pMutex = NULL;
        }

    #endif

        //カーソル

        for(i = 0; i < CURSOR_NUM; i++)
        {
            _DELETE(m_pCursor[i]);
        }

        //

    #if !defined(_AX_OPT_NOIM)
        _DELETE(m_pIM);
    #endif

        _DELETE(m_pDND);
        _DELETE(m_pClipboard);
        _DELETE(m_ptransDef);
        _DELETE(m_ptransApp);
        _DELETE(m_pTimerList);
        _DELETE(m_pNotifyList);
        _DELETE(m_pRes);
        _DELETE(m_pAtom);
        _DELETE(m_pWinTbl);

        //ディスプレイ閉じる

        ::XSync(DISP, FALSE);
        ::XCloseDisplay(DISP);

        m_pDisp = NULL;
    }
}

//! 初期化
/*!
    ※コマンドライン引数は getopt_long() で処理している。

    @param bLocale setlocale() を行うか
*/

BOOL AXApp::init(int argc,char **argv,BOOL bLocale)
{
    int n,major,minor;
    LPSTR szFontPattern = NULL;
    struct option argopt[4] =
    {
        {"font"   , 1, NULL, 'f'},
        {"lang"   , 1, NULL, 'l'},
        {"selfdir", 0, NULL, 's'},
        {NULL, 0, 0, 0}
    };

    //コマンドライン処理

    while(1)
    {
        n = ::getopt_long(argc, argv, "", argopt, NULL);
        if(n == -1) break;

        switch(n)
        {
            case 'f':
                szFontPattern = optarg;
                break;
            case 'l':
                if(::strlen(optarg) <= 5)
                    ::strcpy(m_cLang, optarg);
                break;
            case 's':
                m_strResDir.path_setExePath();
                m_strConfDir.path_setExePath();
                break;
        }
    }

    //ロケール

    if(bLocale)
        ::setlocale(LC_ALL, "");

    //エラーハンドラセット

    ::XSetErrorHandler(XErrHandler);

    //ディスプレイ開く

	m_pDisp = (LPVOID)::XOpenDisplay(NULL);
	if(!m_pDisp)
	{
		ERROR("! failed XOpenDisplay\n");
		return FALSE;
	}

    //ロケール＆入力メソッドセット

    if(::XSupportsLocale())
    {
#if !defined(_AX_OPT_NOIM)
        if(::XSetLocaleModifiers(""))
        {
            m_pIM = new AXAppIM;

            //初期化失敗時はNULLにする
            if(!m_pIM->init(m_pDisp))
            {
                delete m_pIM;
                m_pIM = NULL;
            }
        }
#endif
    }

    //フォント初期化(Xft)

    if(!AXFontInit())
    {
        ERROR("! failed Init Xft\n");
        return FALSE;
    }

    //XInput2 初期化

#if defined(_AX_OPT_XI2)

    major = 2, minor = 0;

    if(::XQueryExtension(DISP, "XInputExtension", &m_nXIopcode, &n, &n))
    {
        if(::XIQueryVersion(DISP, &major, &minor) == BadRequest || major < 2)
            ERROR("! not supported XInput2\n");
    }

#endif

    //各情報取得

    m_idRoot      = DefaultRootWindow(DISP);
    m_nScreen     = DefaultScreen(DISP);
    m_pVisual     = (LPVOID)DefaultVisual(DISP, m_nScreen);
    m_idColmap    = DefaultColormap(DISP, m_nScreen);
    m_nConnection = ConnectionNumber(DISP);
    m_nDepth      = DefaultDepth(DISP, m_nScreen);

    //アトム

    m_pAtom = new AXAppAtom;

    //サポートチェック

    _checkSupport();

    //RGB シフト数

    Visual *visual = (Visual *)m_pVisual;

    m_nRedLShift    = AXGetFirstOnBit(visual->red_mask);
    m_nGreenLShift  = AXGetFirstOnBit(visual->green_mask);
    m_nBlueLShift   = AXGetFirstOnBit(visual->blue_mask);

    m_nRedRShift    = 8 - AXGetFirstOffBit(visual->red_mask >> m_nRedLShift);
    m_nGreenRShift  = 8 - AXGetFirstOffBit(visual->green_mask >> m_nGreenLShift);
    m_nBlueRShift   = 8 - AXGetFirstOffBit(visual->blue_mask >> m_nBlueLShift);

    //各作成

    m_pRes          = new AXAppRes(szFontPattern);
    m_pWinTbl       = new AXAppWinHash;
    m_pTimerList    = new AXList;
    m_pNotifyList   = new AXList;
    m_ptransDef     = new AXTranslation;
    m_ptransApp     = new AXTranslation;
    m_pClipboard    = new AXClipboard;
    m_pDND          = new AXDND;

    m_pwinRoot      = new AXRootWindow;

    //カーソル

    m_pCursor[CURSOR_HSPLIT] = new AXCursor(AXCursor::SPLIT_H);
    m_pCursor[CURSOR_VSPLIT] = new AXCursor(AXCursor::SPLIT_V);

    //スレッド関連

#if defined(_AX_OPT_NOTHREAD)

    m_nMaxFD = m_nConnection + 1;

#else

    if(::pipe(m_fdPipe) == 0)
    {
        ::fcntl(m_fdPipe[0], F_SETFL, O_NONBLOCK);
        ::fcntl(m_fdPipe[1], F_SETFL, O_NONBLOCK);
    }

    m_nMaxFD = 1 + ((m_nConnection < m_fdPipe[0])? m_fdPipe[0]: m_nConnection);

    m_pMutex = new AXMutex;
    m_pMutex->init();
    m_pMutex->lock();

#endif

    return TRUE;
}

//! 各機能がサポートされているかチェック

void AXApp::_checkSupport()
{
    AXMem mem;
    int i,cnt;
    Atom *pAtom;

    //XSHM（共有メモリ拡張機能）が使えるか

    if(::XQueryExtension(DISP, "MIT-SHM", &i, &i, &i))
    {
        if(::XShmQueryVersion(DISP, &i, &i, &i))
            m_uSupportFlag |= SUPPORT_XSHM;
    }

    //ウィンドウマネージャの有効プロパティ

    if(AXGetProperty32(m_idRoot, ::XInternAtom(DISP, "_NET_SUPPORTED", FALSE), XA_ATOM, &mem, &cnt))
    {
        pAtom = (Atom *)mem.getBuf();

        for(i = 0; i < cnt; i++, pAtom++)
        {
            if(*pAtom == atom(AXAppAtom::_NET_WM_USER_TIME))
                m_uSupportFlag |= SUPPORT_USERTIME;
            else if(*pAtom == atom(AXAppAtom::_NET_ACTIVE_WINDOW))
                m_uSupportFlag |= SUPPORT_NETACTIVEWINDOW;
        }
    }
}

//! _NET_WM_USER_TIME 更新

void AXApp::_updateUserTime(AXWindow *pwin,ULONG time)
{
    m_timeUser = time;

    (pwin->getTopLevel())->_updateUserTime(time);
}


//=================================
//
//=================================


//! グラブされているか

BOOL AXApp::isGrab()
{
    return (m_pwinGrab != NULL);
}

//! モーダル中か

BOOL AXApp::isModal()
{
    if(m_pNowRun)
        return (m_pNowRun->pModal != NULL);
    else
        return FALSE;
}

//! AXAppAtom からアトム識別子取得

ULONG AXApp::atom(int no) const
{
    return m_pAtom->get(no);
}

//! アトム名からアトム識別子取得

ULONG AXApp::getAtom(LPCSTR szAtomName) const
{
    return ::XInternAtom(DISP, szAtomName, FALSE);
}

//! IDからウィンドウのクラスポインタ取得

AXWindow *AXApp::getWindowFromID(ULONG idWin) const
{
    return m_pWinTbl->search(idWin);
}

//! 現在のモーダルウィンドウ取得
/*!
    @return NULL でモーダル中でない
*/

AXWindow *AXApp::getModalWin()
{
    if(m_pNowRun)
        return m_pNowRun->pModal;
    else
        return NULL;
}

//! 現在のマウス位置をルート座標で取得

void AXApp::getCursorPos(AXPoint *ppt) const
{
    Window win;
    int x,y;
    UINT btt;

    ::XQueryPointer(DISP, m_idRoot, &win, &win, &ppt->x, &ppt->y, &x, &y, &btt);
}


//----------------------


//! 設定ファイル保存先のパスを "ホームディレクトリ＋szPath" で設定

void AXApp::setConfigDirHome(LPCSTR szPath)
{
    m_strConfDir.path_setHomePath();
    m_strConfDir.path_add(szPath);
}

//! 設定ファイル保存先のディレクトリを作成

BOOL AXApp::createConfigDir() const
{
    if(!AXIsExistFile(m_strConfDir, TRUE))
    {
        if(!AXCreateDir(m_strConfDir))
            return FALSE;
    }

    return TRUE;
}

//! リソースファイルのパス取得

void AXApp::getResourcePath(AXString *pstr,LPCSTR szAdd) const
{
    *pstr = m_strResDir;
    pstr->path_add(szAdd);
}

//! 設定ファイル保存先のパス取得

void AXApp::getConfigPath(AXString *pstr,LPCSTR szAdd) const
{
    *pstr = m_strConfDir;
    pstr->path_add(szAdd);
}


//=================================
//色関連
//=================================


//! RGB -> ピクセル値

DWORD AXApp::rgbToPix(int r,int g,int b) const
{
    return ((r >> m_nRedRShift) << m_nRedLShift) |
            ((g >> m_nGreenRShift) << m_nGreenLShift) |
            ((b >> m_nBlueRShift) << m_nBlueLShift);
}

//! RGB -> ピクセル値
/*!
    @param col RGB値。-1 の場合は特殊扱いとし、-1 のまま
*/

DWORD AXApp::rgbToPix(DWORD col) const
{
    if(col == (DWORD)-1)
        return col;
    else
    {
        return ((_GETR(col) >> m_nRedRShift) << m_nRedLShift) |
                ((_GETG(col) >> m_nGreenRShift) << m_nGreenLShift) |
                ((_GETB(col) >> m_nBlueRShift) << m_nBlueLShift);
    }
}


//=================================
//翻訳データ関連
//=================================


//! 翻訳データ読み込み
/*!
    翻訳ファイル(*.axt)は、setResourceDir() で設定されたディレクトリから検索される。@n
    翻訳ファイルのデータ内に文字列がなければ、埋め込みのデフォルトデータを使うようになっている。@n
    埋め込み用のヘッダファイルは axtrans ツールの -h オプションで出力できる。

    @param pDefLangDat  デフォルトの言語データ（ヘッダファイルによる埋め込みデータ）
    @param szLangName   言語名（NULLでシステムの言語またはコマンドラインで指定された言語）。"ja_JP" "en_US" など。
*/

void AXApp::loadTranslation(LPBYTE pDefLangDat,LPCSTR szLangName)
{
    LPCSTR pLang;

    m_ptransDef->setBuf(pDefLangDat);

    //

    if(szLangName)
        pLang = szLangName;
    else if(m_cLang[0])
        pLang = m_cLang;
    else
        pLang = NULL;

    m_ptransApp->loadFile(pLang, m_strResDir);
}

//! 翻訳のカレントグループセット

void AXApp::setTrGroup(WORD wGroupID)
{
    m_ptransDef->setGroup(wGroupID);
    m_ptransApp->setGroup(wGroupID);
}

//! 翻訳のカレントグループから文字列取得

LPCUSTR AXApp::getTrStr(WORD wStrID) const
{
    LPCUSTR p;

    p = m_ptransApp->getString(wStrID);
    if(!p)
    {
        p = m_ptransDef->getString(wStrID);
        if(!p) p = g_wEmptyString;
    }

    return p;
}

//! 翻訳のカレントグループから文字列取得
//! @return 見つからなかった場合は NULL

LPCUSTR AXApp::getTrStrRaw(WORD wStrID) const
{
    LPCUSTR p;

    p = m_ptransApp->getString(wStrID);
    if(!p)
        p = m_ptransDef->getString(wStrID);

    return p;
}

//! 翻訳のカレントグループから埋め込みのデフォルト文字列を取得

LPCUSTR AXApp::getTrStrDef(WORD wStrID) const
{
    LPCUSTR p = m_ptransDef->getString(wStrID);

    return (p)? p: g_wEmptyString;
}

//! 翻訳の指定グループから文字列取得（カレントグループは変更なし）

LPCUSTR AXApp::getTrString(WORD wGroupID,WORD wStrID) const
{
    LPCUSTR p;

    p = m_ptransApp->getString(wGroupID, wStrID);
    if(!p)
    {
        p = m_ptransDef->getString(wGroupID, wStrID);
        if(!p) p = g_wEmptyString;
    }

    return p;
}

//! 翻訳の指定グループから文字列取得
/*!
    @return 見つからなかった場合は NULL
*/

LPCUSTR AXApp::getTrStringRaw(WORD wGroupID,WORD wStrID) const
{
    LPCUSTR p;

    p = m_ptransApp->getString(wGroupID, wStrID);

    if(!p)
        p = m_ptransDef->getString(wGroupID, wStrID);

    return p;
}


//=================================
//ウィンドウ関連
//=================================


//! テーブルにウィンドウ追加

void AXApp::addWindow(AXWindow *pwin)
{
    m_pWinTbl->add(pwin);
}

//! テーブルからウィンドウ削除

void AXApp::removeWindow(AXWindow *pwin)
{
    m_pWinTbl->remove(pwin);
}

//! グラブ開始

BOOL AXApp::grabWindow(AXWindow *pwin)
{
    if(m_pwinGrab)
        return FALSE;
    else
    {
        m_pwinGrab = pwin;
        return TRUE;
    }
}

//! グラブ終了

void AXApp::ungrabWindow()
{
    m_pwinGrab = NULL;
}

//! 入力コンテキスト作成（トップレベル）

AXAppIC *AXApp::createIC(AXWindow *pwin)
{
#if !defined(_AX_OPT_NOIM)

    AXAppIC *pIC;

    if(!m_pIM) return NULL;

    pIC = new AXAppIC;

    if(!pIC->init(m_pIM, pwin))
    {
        delete pIC;
        return NULL;
    }

    return pIC;

#endif
}


//=================================
//
//=================================


//! 現在のループを抜ける

void AXApp::exit()
{
    if(m_pNowRun)
        m_pNowRun->bEndFlag = TRUE;
}

//! 通常メインループ

void AXApp::run()
{
    AXAppRun dat(m_pNowRun);

    m_pNowRun = &dat;

    while(_main(FALSE));

    m_pNowRun = dat.pBackRun;
}

//! キュー内のイベントを処理し、イベントがなくなったら戻る

void AXApp::runQueue()
{
    AXAppRun dat(m_pNowRun);

    m_pNowRun = &dat;

    while(_main(TRUE) == 1);

    m_pNowRun = dat.pBackRun;
}

//! アイドル処理付きメインループ
/*!
    アイドル時は AXApp::onIdle() が呼ばれる
*/

void AXApp::runIdle()
{
    AXAppRun dat(m_pNowRun);

    m_pNowRun = &dat;

    while(_main(TRUE))
        onIdle();

    m_pNowRun = dat.pBackRun;
}

//! ダイアログ用ループ

void AXApp::runDlg(AXWindow *pDlg)
{
    AXAppRun dat(m_pNowRun, pDlg);

    m_pNowRun = &dat;

    while(_main(FALSE));

    m_pNowRun = dat.pBackRun;
}

//! ポップアップウィンドウ用ループ

void AXApp::runPopup(AXWindow *pPopup)
{
    AXAppRun dat(m_pNowRun, pPopup);

    m_pNowRun = &dat;

    dat.pPopup = pPopup;

    while(_main(FALSE));

    m_pNowRun = dat.pBackRun;
}

//! キュー内のXイベントが処理されるまで待つ

void AXApp::sync()
{
    ::XSync(DISP, FALSE);
}

//! キュー内のイベントを送る

void AXApp::flush()
{
    ::XFlush(DISP);
}

//! mutex ロック

void AXApp::mutexLock()
{
#if !defined(_AX_OPT_NOTHREAD)
    m_pMutex->lock();
#endif
}

//! mutex ロック解除

void AXApp::mutexUnlock()
{
#if !defined(_AX_OPT_NOTHREAD)
    m_pMutex->unlock();
#endif
}

//! select() によるイベント待ちを抜ける（スレッド用）

void AXApp::selectOut()
{
#if !defined(_AX_OPT_NOTHREAD)
    BYTE dat = 0;
    ::write(m_fdPipe[1], &dat, 1);
#endif
}


//=================================
//メインループ
//=================================


//! メイン処理
/*!
    @param bIdle TRUE で、イベントがない場合すぐ戻る
    @return TRUE(or 2) でループを続ける。FALSE でループ終了。2 でイベントがない。
*/

BOOL AXApp::_main(BOOL bIdle)
{
    //再構成

    _reconfig();

    //タイマー処理

    _timer();

    //描画更新

    if(m_bUpdate)
    {
        _draw();
        m_bUpdate = FALSE;
    }

    //通知処理（一つずつ）
    //終了フラグがONの場合は、終了してから実行

    if(!m_pNowRun->bEndFlag)
    {
        if(_notify()) return TRUE;
    }

    //イベントがない場合

    if(::XPending(DISP) == 0)
    {
        //描画処理

        _draw();

        //終了時、ループを抜ける

        if(m_pNowRun->bEndFlag) return FALSE;

        //アイドル処理を行うなら戻る

        if(bIdle) return 2;

        //非アイドル時、何かが起こるまで待つ

        if(!_wait()) return TRUE;
    }

    //イベント処理

    return _event();
}

//! 非アイドル時、何かが起こるまで待つ
/*!
    @return TRUEでイベントが起きた。FALSEでそれ以外
*/

BOOL AXApp::_wait()
{
    AXAppTimerItem *ptimer;
    TIMENANO now,diff;
    fd_set fd;
    int ret;
    struct timeval tv;

    ptimer = (AXAppTimerItem *)m_pTimerList->getTop();

    //------- 待つ

    FD_ZERO(&fd);
    FD_SET(m_nConnection, &fd);
#if !defined(_AX_OPT_NOTHREAD)
    FD_SET(m_fdPipe[0], &fd);
#endif

    if(ptimer)
    {
        //タイマーの最小時間まで待つ

        AXGetTime(&now);

        if(!AXDiffTime(&diff, &ptimer->m_timeEnd, &now)) return FALSE;

        tv.tv_sec   = diff.sec;
        tv.tv_usec  = diff.nsec / 1000; //マイクロ秒

    #if defined(_AX_OPT_NOTHREAD)

        ret = ::select(m_nMaxFD, &fd, NULL, NULL, &tv);

    #else

        m_pMutex->unlock();
        ret = ::select(m_nMaxFD, &fd, NULL, NULL, &tv);
        m_pMutex->lock();

    #endif
    }
    else
    {
        //時間無制限で待つ

    #if defined(_AX_OPT_NOTHREAD)

        ret = ::select(m_nMaxFD, &fd, NULL, NULL, NULL);

    #else

        m_pMutex->unlock();
        ret = ::select(m_nMaxFD, &fd, NULL, NULL, NULL);
        m_pMutex->lock();

    #endif
    }

    //--------

    if(ret <= 0)
        return FALSE;
    else if(FD_ISSET(m_nConnection, &fd))
        return TRUE;
#if !defined(_AX_OPT_NOTHREAD)
    else if(FD_ISSET(m_fdPipe[0], &fd))
    {
        BYTE dat;

        while(1)
        {
            ret = ::read(m_fdPipe[0], &dat, 1);
            if(ret <= 0) break;
        }
        return FALSE;
    }
#endif

    return FALSE;
}

//! イベント処理
/*!
    @return FALSEでループ終了
*/

BOOL AXApp::_event()
{
    XEvent ev,ev2;
    AXWindow *pwin;
    AXHD hd;
    int type;
    BOOL bSkip = FALSE;

    //イベント取得

    ::XNextEvent(DISP, &ev);

    //時間記録

    _event_time(&ev);

    //IM用イベント処理

#if !defined(_AX_OPT_NOIM)

    if(m_pIM)
    {
        if(::XFilterEvent(&ev, None)) return TRUE;
    }

#endif

    //GenericEvent イベント処理 (XInput2)

#if defined(_AX_OPT_XI2)

    if(ev.xany.type == GenericEvent)
    {
        if(ev.xcookie.extension == m_nXIopcode)
        {
            if(::XGetEventData(DISP, &ev.xcookie))
            {
                _event_XI2(&ev);

                ::XFreeEventData(DISP, &ev.xcookie);
            }
        }

        return TRUE;
    }

#endif

    //ウィンドウIDから検索

    pwin = m_pWinTbl->search(ev.xany.window);
    if(!pwin) return TRUE;

    //モーダル（モーダル自身とその子供以外のウィンドウの処理は行わない）

    if(m_pNowRun->pModal)
    {
        if(m_pNowRun->pModal->getid() != pwin->getTopLevel()->getid())
            bSkip = TRUE;
    }

    //---------- イベント処理

    type = ev.xany.type;

    switch(type)
    {
        //再描画
        case Expose:
        case GraphicsExpose:
            addDraw(pwin, ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height);
            break;

        //マウス移動
        case MotionNotify:
            if(bSkip) break;

            if(ev.xmotion.is_hint)
            {
                //圧縮されている場合
                ev.xmotion.same_screen = ::XQueryPointer(DISP, ev.xany.window, &ev.xmotion.root, &ev.xmotion.subwindow,
                        &ev.xmotion.x_root, &ev.xmotion.y_root, &ev.xmotion.x, &ev.xmotion.y, &ev.xmotion.state);
            }

            hd.mouse.x      = ev.xmotion.x;
            hd.mouse.y      = ev.xmotion.y;
            hd.mouse.rootx  = ev.xmotion.x_root;
            hd.mouse.rooty  = ev.xmotion.y_root;
            hd.mouse.state  = _convButtonState(ev.xmotion.state);
            hd.mouse.time   = ev.xmotion.time;
            hd.mouse.type   = AXWindow::EVENT_MOUSEMOVE;

            pwin->onMouseMove((AXHD_MOUSE *)&hd);
            break;

        //ボタン押し・離し
        case ButtonPress:
            _updateUserTime(pwin, ev.xbutton.time);

        case ButtonRelease:
            hd.mouse.x      = ev.xbutton.x;
            hd.mouse.y      = ev.xbutton.y;
            hd.mouse.rootx  = ev.xbutton.x_root;
            hd.mouse.rooty  = ev.xbutton.y_root;
            hd.mouse.button = ev.xbutton.button;
            hd.mouse.state  = _convButtonState(ev.xbutton.state);
            hd.mouse.time   = ev.xbutton.time;
            hd.mouse.type   = (type == ButtonPress)? AXWindow::EVENT_BUTTONDOWN: AXWindow::EVENT_BUTTONUP;

            if(bSkip)
                pwin->onMouseInSkip((AXHD_MOUSE *)&hd);
            else
            {
                if(!(pwin->m_uFlags & AXWindow::FLAG_WHEELEVENT_NORMAL) &&
                   (ev.xbutton.button == Button4 || ev.xbutton.button == Button5))
                {
                    //ホイール（離し時は処理しない）

                    if(type == ButtonPress)
                        pwin->onMouseWheel((AXHD_MOUSE *)&hd, (ev.xbutton.button == Button4));
                }
                else
                {
                    //通常ボタン

                    if(type == ButtonPress)
                        _event_ButtonPress(pwin, &ev, &hd);
                    else
                        pwin->onButtonUp((AXHD_MOUSE *)&hd);
                }
            }
            break;

        //Enter/Leave
        case EnterNotify:
        case LeaveNotify:
            hd.enter.x      = ev.xcrossing.x;
            hd.enter.y      = ev.xcrossing.y;
            hd.enter.rootx  = ev.xcrossing.x_root;
            hd.enter.rooty  = ev.xcrossing.y_root;
            hd.enter.time   = ev.xcrossing.time;
            hd.enter.detail = ev.xcrossing.detail;

            if(type == EnterNotify)
            {
                //------ Enter

                if(!bSkip && ev.xcrossing.mode == NotifyNormal)
                    pwin->onEnter((AXHD_ENTERLEAVE *)&hd);
            }
            else
            {
                //------ Leave

                if(ev.xcrossing.mode == NotifyUngrab)
                {
                    //グラブ解除時

                    ungrabWindow(); //予期せぬ解放時のため

                    pwin->onUngrab((AXHD_ENTERLEAVE *)&hd);
                }
                else if(ev.xcrossing.mode == NotifyNormal ||
                        (ev.xcrossing.mode == NotifyGrab && ev.xcrossing.detail == NotifyNonlinear))
                    pwin->onLeave((AXHD_ENTERLEAVE *)&hd);
            }
            break;

        //キー押し/離し
        case KeyPress:
            _updateUserTime(pwin, ev.xkey.time);

        case KeyRelease:
            /* 離し時、次のイベントが KeyPress の場合はオートリピートなので処理しない
              （次の KeyPress は普通に実行される） */

            if(type == KeyRelease && ::XPending(DISP))
            {
                ::XPeekEvent(DISP, &ev2);
                if(ev2.xkey.type == KeyPress && ev2.xkey.keycode == ev.xkey.keycode) break;
            }

            hd.key.code     = ev.xkey.keycode;
            hd.key.state    = _convButtonState(ev.xkey.state);
            hd.key.time     = ev.xkey.time;
            hd.key.pEvent   = (LPVOID)&ev;

            //※XLookupKeysym() だと正しく取得できない

            char m[24];
            KeySym key;
            ::XLookupString((XKeyEvent *)&ev, m, 24, &key, NULL);

            hd.key.keysym = key;

            //ポップアップ時はポップアップウィンドウで

            if(m_pNowRun->pPopup)
            {
                if(type == KeyPress)
                    (m_pNowRun->pPopup)->onKeyDown((AXHD_KEY *)&hd);
                else
                    (m_pNowRun->pPopup)->onKeyUp((AXHD_KEY *)&hd);
            }
            else
            {
                if(type == KeyPress)
                    pwin->onKeyDown((AXHD_KEY *)&hd);
                else
                    pwin->onKeyUp((AXHD_KEY *)&hd);
            }
            break;

        //ウィンドウの位置やサイズなどの構成が変更
        case ConfigureNotify:
            _compEvent_Configure(&ev);

            hd.configure.x = ev.xconfigure.x;
            hd.configure.y = ev.xconfigure.y;
            hd.configure.w = ev.xconfigure.width;
            hd.configure.h = ev.xconfigure.height;
            hd.configure.bSendEvent = ev.xconfigure.send_event;

            pwin->onConfigure((AXHD_CONFIGURE *)&hd);

        #ifdef _PUTEVENT
            fprintf(stderr, "[Configure:%lX] (%d,%d)-(%dx%d) %d\n",
                        ev.xany.window,
                        hd.configure.x, hd.configure.y, hd.configure.w, hd.configure.h,
                        hd.configure.bSendEvent);
        #endif
            break;

        //セレクション要求
        case SelectionRequest:
            if(ev.xselectionrequest.selection == atom(AXAppAtom::CLIPBOARD))
                m_pClipboard->OnSelectionRequest(&ev);
            break;

        //セレクション所有権消去
        case SelectionClear:
            if(ev.xselectionclear.selection == atom(AXAppAtom::CLIPBOARD))
                m_pClipboard->clear();
            break;

        //フォーカスIN
        case FocusIn:
            if(ev.xfocus.mode == NotifyNormal)
                pwin->onFocusIn(ev.xfocus.detail);
            break;

        //フォーカスOUT
        case FocusOut:
            if(ev.xfocus.mode == NotifyNormal)
                pwin->onFocusOut(ev.xfocus.detail);
            break;

        //マップされた時
        case MapNotify:
            pwin->onMap();

        #ifdef _PUTEVENT
            fprintf(stderr, "[Map:%lX]\n", ev.xany.window);
        #endif
            break;

        //クライアントメッセージ
        case ClientMessage:
            if(ev.xclient.message_type == atom(AXAppAtom::WM_PROTOCOLS))
            {
                //---- WM_PROTOCOLS

                if((ULONG)ev.xclient.data.l[1] > m_timeLast)
                    m_timeLast = ev.xclient.data.l[1];

                if((ULONG)ev.xclient.data.l[0] == atom(AXAppAtom::WM_TAKE_FOCUS))
                {
                    /*
                        指定ウィンドウ（トップレベル）にフォーカスセット。
                        ダイアログ等の場合はそのウィンドウ以外にフォーカスをセットしない

                            ※XSetInputFocus()を行う場合、一つのダイアログ終了後に続けてダイアログを表示しようとすると
                            Xエラーが出るので、XErrHandler() でエラーを無効にしている。
                    */

                    if(m_timeUser == 0) m_timeUser = m_timeLast;

                    if(m_pNowRun->pModal)
                        m_pNowRun->pModal->setActive();
                    else
                        pwin->setActive();
                }
                else if((ULONG)ev.xclient.data.l[0] == atom(AXAppAtom::_NET_WM_PING))
                {
                    /* アプリが応答可能かどうかを判断するためウィンドウマネージャから送られるので、返答 */

                    _sendPing(ev.xclient.data.l);
                }
                else if((ULONG)ev.xclient.data.l[0] == atom(AXAppAtom::WM_DELETE_WINDOW))
                {
                    //閉じるボタンが押された

                    _updateUserTime(pwin, m_timeLast);

                    if(!bSkip) pwin->onClose();
                }
            }
            else
                //----- D&D
                m_pDND->onClientMessage(&ev);
            break;

        //キーボードマッピング
        case MappingNotify:
            if(ev.xmapping.request != MappingPointer)
                ::XRefreshKeyboardMapping((XMappingEvent *)&ev);
            break;
    }

    return TRUE;
}

//! XInput2 イベント処理

void AXApp::_event_XI2(LPVOID pev)
{
#if defined(_AX_OPT_XI2)

    XGenericEventCookie *p = (XGenericEventCookie *)pev;
    XIDeviceEvent *pDE;
    AXWindow *pwin;
    BOOL bSkip = FALSE;

    pDE = (XIDeviceEvent *)p->data;

    //最後の時間

    m_timeLast = pDE->time;

    //ウィンドウIDから検索

    pwin = m_pWinTbl->search(pDE->event);
    if(!pwin) return;

    //_NET_WM_USER_TIME

    if(pDE->evtype == XI_ButtonPress || pDE->evtype == XI_KeyPress)
        _updateUserTime(pwin, pDE->time);

    //モーダル（モーダル自身とその子供以外のウィンドウの処理は行わない）

    if(m_pNowRun->pModal)
    {
        if(m_pNowRun->pModal->getid() != pwin->getTopLevel()->getid())
            bSkip = TRUE;
    }

    //イベント処理

    onInputEvent(pwin, p->data, bSkip);

#endif
}

//! イベント処理前、時間を記録

void AXApp::_event_time(LPVOID pev)
{
    XEvent *p = (XEvent *)pev;

    switch(p->type)
    {
        case MotionNotify:
            m_timeLast = p->xmotion.time;
            break;
        case ButtonPress:
            m_timeUser = p->xbutton.time;
        case ButtonRelease:
            m_timeLast = p->xbutton.time;
            break;
        case KeyPress:
            m_timeUser = p->xkey.time;
        case KeyRelease:
            m_timeLast = p->xkey.time;
            break;
        case PropertyNotify:
            m_timeLast = p->xproperty.time;
            break;
        case EnterNotify:
        case LeaveNotify:
            m_timeLast = p->xcrossing.time;
            break;
        case SelectionClear:
            m_timeLast = p->xselectionclear.time;
            break;
    }
}

//! 描画処理

void AXApp::_draw()
{
    XEvent ev;
    AXWindow *pwin,*pnext;
    AXHD_PAINT hd;
    AXRect rc;

    //------------ バッファに残っているExposeイベントを処理

    while(::XCheckMaskEvent(DISP, ExposureMask, &ev))
    {
        if(ev.xany.type != NoExpose)
        {
            pwin = m_pWinTbl->search(ev.xany.window);
            if(pwin) addDraw(pwin, ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height);
        }
    }

    //------------- 描画処理

    for(pwin = m_pDrawTop; pwin; pwin = pnext)
    {
        if(pwin->isVisible())
        {
            rc = pwin->m_rcDraw;

            //クリッピング

            if(rc.left < 0) rc.left = 0;
            if(rc.top  < 0) rc.top  = 0;
            if(rc.right >= pwin->getWidth()) rc.right = pwin->getWidth() - 1;
            if(rc.bottom >= pwin->getHeight()) rc.bottom = pwin->getHeight() - 1;

            //

            hd.x = rc.left;
            hd.y = rc.top;
            hd.w = rc.right - rc.left + 1;
            hd.h = rc.bottom - rc.top + 1;

            if(hd.w > 0 && hd.h > 0)
                pwin->onPaint(&hd);

            pwin->m_uFlags &= ~AXWindow::FLAG_REDRAW;
        }

        //次

        pnext = pwin->m_pDrawNext;

        //リストから外す

        pwin->m_pDrawPrev = NULL;
        pwin->m_pDrawNext = NULL;
    }

    m_pDrawTop = m_pDrawBottom = NULL;

    //

    ::XFlush(DISP);
}


//=========================
//サブ処理
//=========================


//! マウスボタン/キーの装飾フラグを変換

UINT AXApp::_convButtonState(UINT state)
{
    UINT ret = 0;

    if(state & ShiftMask)   ret |= AXWindow::STATE_SHIFT;
    if(state & ControlMask) ret |= AXWindow::STATE_CTRL;
    if(state & Mod1Mask)    ret |= AXWindow::STATE_ALT;
    if(state & Button1Mask) ret |= AXWindow::STATE_LBUTTON;
    if(state & Button2Mask) ret |= AXWindow::STATE_MBUTTON;
    if(state & Button3Mask) ret |= AXWindow::STATE_RBUTTON;

    return ret;
}

//! ButtonPress イベント処理

void AXApp::_event_ButtonPress(AXWindow *pwin,LPVOID pev,LPVOID phd)
{
    XButtonEvent *p = (XButtonEvent *)pev;
    BOOL bDbl = FALSE;

    //ダブルクリック判定

    if(p->window == m_dbcWin && p->time < m_dbcTime + 350)
    {
        if(p->button == m_dbcButton && ::abs(p->x_root - m_dbcX) < 3 &&
            ::abs(p->y_root - m_dbcY) < 3)
            bDbl = TRUE;
    }

    //ダブルクリック時は onDblClk
    //（FALSE が返された場合は onButtonDown を呼ぶ）

    if(bDbl)
    {
        m_dbcWin = 0;

        if(!pwin->onDblClk((AXHD_MOUSE *)phd))
            pwin->onButtonDown((AXHD_MOUSE *)phd);
    }
    else
    {
        m_dbcWin    = p->window;
        m_dbcTime   = p->time;
        m_dbcX      = p->x_root;
        m_dbcY      = p->y_root;
        m_dbcButton = p->button;

        pwin->onButtonDown((AXHD_MOUSE *)phd);
    }
}

//! ConfigureNotify イベント圧縮処理

void AXApp::_compEvent_Configure(LPVOID pev)
{
    XConfigureEvent *p = (XConfigureEvent *)pev;
    XEvent tmp;

    /*
        同じウィンドウのイベントを削除してまとめる。
        send_event が 0 以外の場合、x,y はルートウィンドウ座標での位置（枠含まない）
        send_event が 0 の場合、x,y はウィンドウ（枠含む）の左上位置からの相対位置
    */

    while(::XCheckTypedWindowEvent(DISP, p->window, ConfigureNotify, &tmp))
    {
        p->width  = tmp.xconfigure.width;
        p->height = tmp.xconfigure.height;

        if(tmp.xconfigure.send_event)
        {
            p->x = tmp.xconfigure.x;
            p->y = tmp.xconfigure.y;
            p->send_event = 1;
        }
    }
}

//! _NET_WM_PINGが送られてきた時のレスポンス

void AXApp::_sendPing(long *pdat)
{
    XEvent ev;

    ev.xclient.type         = ClientMessage;
    ev.xclient.display      = DISP;
    ev.xclient.message_type = atom(AXAppAtom::WM_PROTOCOLS);
    ev.xclient.format       = 32;
    ev.xclient.window       = m_idRoot;
    ev.xclient.data.l[0]    = pdat[0];
    ev.xclient.data.l[1]    = pdat[1];
    ev.xclient.data.l[2]    = pdat[2];
    ev.xclient.data.l[3]    = 0;
    ev.xclient.data.l[4]    = 0;

    ::XSendEvent(DISP, m_idRoot, FALSE, SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}


//=================================
//ハンドラ
//=================================


//! Xエラーハンドラ

int XErrHandler(Display *disp,XErrorEvent *pev)
{
	char buf[256],m[32],reqbuf[256];

	//エラーを無効にする

	if(pev->request_code == 42) return 0;   //WM_TAKE_FOCUS

	if(pev->error_code == BadAtom && pev->request_code == 17) return 0; //XGetAtomName の BadAtom

	//エラー表示

	::XGetErrorText(disp, pev->error_code, buf, 256);

	::sprintf(m, "%d", pev->request_code);
	::XGetErrorDatabaseText(disp, "XRequest", m, "", reqbuf, 256);

	fprintf(stderr, "[X Error] %d %s\n  req:%d(%s) minor:%d\n",
			pev->error_code, buf, pev->request_code, reqbuf, pev->minor_code);

	return 1;
}

//! アイドル処理時

void AXApp::onIdle()
{

}

//! XInput2イベント時
/*!
    @param pwin     イベントの送信先ウィンドウ
    @param pEvent   (XIDeviceEvent*)
    @param bSkip    モーダルダイアログ表示中のためスキップ対象
*/

void AXApp::onInputEvent(AXWindow *pwin,LPVOID pEvent,BOOL bSkip)
{

}


//*************************************
//描画
//*************************************


//! 描画追加

void AXApp::addDraw(AXWindow *pwin,int x,int y,int w,int h)
{
    AXRect rc;

    rc.setFromSize(x, y, w, h);

    if(pwin->m_pDrawPrev || pwin->m_pDrawNext || m_pDrawTop == pwin)
        //描画リストに入っているなら範囲追加
        //※リストがひとつの場合、prev/nextがNULLなので m_pDrawTop == pwin で判定
        pwin->m_rcDraw.combine(rc);
    else
    {
        //新たに描画リストに追加

        pwin->m_rcDraw = rc;

        if(!m_pDrawTop)
            //リストの最初
            m_pDrawTop = m_pDrawBottom = pwin;
        else
        {
            m_pDrawBottom->m_pDrawNext = pwin;
            pwin->m_pDrawPrev = m_pDrawBottom;
            m_pDrawBottom = pwin;
        }
    }
}

//! 指定ウィンドウを描画リストからはずす

void AXApp::removeDraw(AXWindow *pwin)
{
    if(pwin->m_pDrawPrev || pwin->m_pDrawNext || m_pDrawTop == pwin)
    {
        if(!pwin->m_pDrawPrev)
            m_pDrawTop = pwin->m_pDrawNext;
        else
            (pwin->m_pDrawPrev)->m_pDrawNext = pwin->m_pDrawNext;

        if(!pwin->m_pDrawNext)
            m_pDrawBottom = pwin->m_pDrawPrev;
        else
            (pwin->m_pDrawNext)->m_pDrawPrev = pwin->m_pDrawPrev;

        pwin->m_pDrawPrev = pwin->m_pDrawNext = NULL;
    }
}


//*************************************
//通知リスト
//*************************************


class AXAppNotifyItem:public AXListItem
{
public:
    typedef struct
    {
        AXWindow    *pwinFrom;
        UINT        uNotify;
        ULONG       lParam;
    }PARAM_NOTIFY;

    typedef struct
    {
        UINT    uID;
        ULONG   lParam;
        int     from;
    }PARAM_COMMAND;

    typedef union
    {
        PARAM_NOTIFY    notify;
        PARAM_COMMAND   command;
    }PARAM;

    enum
    {
        TYPE_NOTIFY,
        TYPE_COMMAND
    };

public:
    AXWindow    *m_pwinSend;
    int         m_nType;
    PARAM       m_param;

    AXAppNotifyItem(AXWindow *pwin,int type) : m_pwinSend(pwin),m_nType(type) { }

    AXAppNotifyItem *prev() const { return (AXAppNotifyItem *)m_pPrev; }
    AXAppNotifyItem *next() const { return (AXAppNotifyItem *)m_pNext; }
};


//! onNotify 通知追加

void AXApp::addNotify(AXWindow *pwin,AXWindow *pwinFrom,UINT uNotify,ULONG lParam)
{
    AXAppNotifyItem *p;

    p = new AXAppNotifyItem(pwin, AXAppNotifyItem::TYPE_NOTIFY);

    p->m_param.notify.pwinFrom  = pwinFrom;
    p->m_param.notify.uNotify   = uNotify;
    p->m_param.notify.lParam    = lParam;

    m_pNotifyList->add(p);
}

//! onCommand 通知追加

void AXApp::addCommand(AXWindow *pwin,UINT uID,ULONG lParam,int from)
{
    AXAppNotifyItem *p;

    p = new AXAppNotifyItem(pwin, AXAppNotifyItem::TYPE_COMMAND);

    p->m_param.command.uID      = uID;
    p->m_param.command.lParam   = lParam;
    p->m_param.command.from     = from;

    m_pNotifyList->add(p);
}

//! 指定ウィンドウの通知を取り除く

void AXApp::removeNotify(AXWindow *pwin)
{
    AXAppNotifyItem *p,*pnext;

    for(p = (AXAppNotifyItem *)m_pNotifyList->getTop(); p; p = pnext)
    {
        pnext = p->next();

        if(p->m_pwinSend == pwin)
            m_pNotifyList->deleteItem(p);
    }
}

//! 通知処理
/*
    ハンドラ内で AXApp::run() などのループが呼ばれる場合があるので、
    まとめて処理せず一つずつ実行する。
    また、実行する前にデータを削除する。
*/

BOOL AXApp::_notify()
{
    AXAppNotifyItem *p;
    AXWindow *pwin;
    AXAppNotifyItem::PARAM param;
    int type;

    p = (AXAppNotifyItem *)m_pNotifyList->getTop();
    if(!p) return FALSE;

    pwin    = p->m_pwinSend;
    type    = p->m_nType;
    param   = p->m_param;

    m_pNotifyList->deleteItem(p);

    switch(type)
    {
        case AXAppNotifyItem::TYPE_NOTIFY:
            pwin->onNotify(param.notify.pwinFrom, param.notify.uNotify, param.notify.lParam);
            break;
        case AXAppNotifyItem::TYPE_COMMAND:
            pwin->onCommand(param.command.uID, param.command.lParam, param.command.from);
            break;
    }

    return TRUE;
}


//*************************************
//再構成リスト
//*************************************


//! 再構成追加

void AXApp::addReconfig(AXWindow *pwin)
{
    //すでにリストにある

    if(pwin->m_pReConfPrev || pwin->m_pReConfNext || m_pReConfTop == pwin)
        return;

    //新たに描画リストに追加

    if(!m_pReConfTop)
        m_pReConfTop = m_pReConfBottom = pwin;
    else
    {
        m_pReConfBottom->m_pReConfNext = pwin;
        pwin->m_pReConfPrev = m_pReConfBottom;
        m_pReConfBottom = pwin;
    }
}

//! 指定ウィンドウを再構成リストからはずす

void AXApp::removeReconfig(AXWindow *pwin)
{
    if(pwin->m_pReConfPrev || pwin->m_pReConfNext || m_pReConfTop == pwin)
    {
        if(!pwin->m_pReConfPrev)
            m_pReConfTop = pwin->m_pReConfNext;
        else
            (pwin->m_pReConfPrev)->m_pReConfNext = pwin->m_pReConfNext;

        if(!pwin->m_pReConfNext)
            m_pReConfBottom = pwin->m_pReConfPrev;
        else
            (pwin->m_pReConfNext)->m_pReConfPrev = pwin->m_pReConfPrev;

        pwin->m_pReConfPrev = pwin->m_pReConfNext = NULL;
    }
}

//! 再構成処理

void AXApp::_reconfig()
{
    AXWindow *p,*pnext;

    for(p = m_pReConfTop; p; p = pnext)
    {
        pnext = p->m_pReConfNext;

        p->m_pReConfPrev = p->m_pReConfNext = NULL;

        p->reconfig();
    }

    m_pReConfTop = m_pReConfBottom = NULL;
}


//*************************************
// タイマー関連
//*************************************


//! 追加
/*!
    @param uInterval ミリ秒
*/

void AXApp::addTimer(AXWindow *pwin,UINT uTimerID,UINT uInterval,ULONG lParam)
{
    AXAppTimerItem *p,*pnext,*pins = NULL;
    TIMENANO endtime;
    ULONGLONG iv;

    if(uInterval == 0) uInterval = 1;

    iv = (ULONGLONG)uInterval * 1000 * 1000;

    AXGetTime(&endtime);
    AXAddTime(&endtime, iv);

    //同じデータがある場合削除、かつ挿入位置取得
    //※時間が小さい順に並んでいる

    for(p = (AXAppTimerItem *)m_pTimerList->getTop(); p; p = pnext)
    {
        pnext = p->next();

        if(p->m_pWin == pwin && p->m_uTimerID == uTimerID)
            m_pTimerList->deleteItem(p);
        else if(!pins && AXCompareTime(&endtime, &p->m_timeEnd) < 0)
            pins = p;
    }

    //追加

    p = new AXAppTimerItem;
    m_pTimerList->insert(p, pins);

    //セット

    p->m_pWin       = pwin;
    p->m_uTimerID   = uTimerID;
    p->m_lParam     = lParam;
    p->m_lInterval  = iv;
    p->m_timeEnd    = endtime;
}

//! タイマー削除

void AXApp::delTimer(AXWindow *pwin,UINT uTimerID)
{
    AXAppTimerItem *p;

    for(p = (AXAppTimerItem *)m_pTimerList->getTop(); p; p = p->next())
    {
        if(p->m_pWin == pwin && p->m_uTimerID == uTimerID)
        {
            m_pTimerList->deleteItem(p);
            break;
        }
    }
}

//! 指定ウィンドウのタイマーすべて削除

void AXApp::delTimerWin(AXWindow *pwin)
{
    AXAppTimerItem *p,*pnext;

    for(p = (AXAppTimerItem *)m_pTimerList->getTop(); p; p = pnext)
    {
        pnext = p->next();

        if(p->m_pWin == pwin)
            m_pTimerList->deleteItem(p);
    }
}

//! 指定タイマーが存在するか

BOOL AXApp::isTimerExist(AXWindow *pwin,UINT uTimerID)
{
    AXAppTimerItem *p;

    for(p = (AXAppTimerItem *)m_pTimerList->getTop(); p; p = p->next())
    {
        if(p->m_pWin == pwin && p->m_uTimerID == uTimerID) return TRUE;
    }

    return FALSE;
}

//! 指定ウィンドウにタイマーが存在するか

BOOL AXApp::isTimerExistWin(AXWindow *pwin)
{
    AXAppTimerItem *p;

    for(p = (AXAppTimerItem *)m_pTimerList->getTop(); p; p = p->next())
    {
        if(p->m_pWin == pwin) return TRUE;
    }

    return FALSE;
}

//! タイマー処理

void AXApp::_timer()
{
    TIMENANO now;
    AXAppTimerItem *p,*pnext,*pp;

    AXGetTime(&now);

    for(p = (AXAppTimerItem *)m_pTimerList->getTop(); p; p = pnext)
    {
        pnext = p->next();

        //以降はすべて時間が来ていない

        if(AXCompareTime(&now, &p->m_timeEnd) < 0) break;

        //次の時間

        p->m_timeEnd = now;
        AXAddTime(&p->m_timeEnd, p->m_lInterval);

        //リスト順移動（時間の小さい順）

        for(pp = pnext; pp; pp = pp->next())
        {
            if(AXCompareTime(&p->m_timeEnd, &pp->m_timeEnd) < 0) break;
        }

        m_pTimerList->move(p, pp);

        //ハンドラ実行
        //※onTimer 中に delTimer が呼ばれる可能性があるので、
        //　この後にリストアイテムに関して処理を行わないこと。

        (p->m_pWin)->onTimer(p->m_uTimerID, p->m_lParam);
    }
}
