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

#include <string.h>

#define _AX_X11_ATOM
#define _AX_X11_UTIL
#include "AXX11.h"

#include "AXTopWindow.h"
#include "AXLayout.h"
#include "AXButton.h"
#include "AXApp.h"
#include "AXAppAtom.h"
#include "AXString.h"
#include "AXByteString.h"
#include "AXRect.h"
#include "AXAccelerator.h"
#include "AXMenuBar.h"
#include "AXUtilX.h"
#include "AXKey.h"
#include "AXMessageBox.h"

#ifndef _AX_OPT_NOIM
#include "AXAppIM.h"
#endif


//-------------------------

#define DISP   ((Display *)m_pDisp)

//"_MOTIF_WM_HINTS" のデータフラグ

#define MWM_HINTS_FUNCTIONS     (1<<0)
#define MWM_HINTS_DECORATIONS   (1<<1)
#define MWM_HINTS_INPUT_MODE    (1<<2)

#define MWM_FUNC_ALL            (1<<0)
#define MWM_FUNC_RESIZE         (1<<1)
#define MWM_FUNC_MOVE           (1<<2)
#define MWM_FUNC_MINIMIZE       (1<<3)
#define MWM_FUNC_MAXIMIZE       (1<<4)
#define MWM_FUNC_CLOSE          (1<<5)

#define MWM_INPUT_MODELESS                  0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL 1
#define MWM_INPUT_SYSTEM_MODAL              2
#define MWM_INPUT_FULL_APPLICATION_MODAL    3

#define MWM_DECOR_ALL           (1<<0)
#define MWM_DECOR_BORDER        (1<<1)
#define MWM_DECOR_RESIZEH       (1<<2)
#define MWM_DECOR_TITLE         (1<<3)
#define MWM_DECOR_MENU          (1<<4)
#define MWM_DECOR_MINIMIZE      (1<<5)
#define MWM_DECOR_MAXIMIZE      (1<<6)

//-------------------------

/*!
    @class AXTopWindow
    @brief トップレベルウィンドウクラス

    - トップレベルウィンドウ＝ルートウィンドウの子。
    - attachMenuBar() でメニューバーを関連付けると、Alt+キーで、バーのホットキーが有効になる。

    @ingroup window
*/

//---------------------------


AXTopWindow::~AXTopWindow()
{
    //入力コンテキスト削除

#ifndef _AX_OPT_NOIM
    if(m_pIC) delete m_pIC;
#endif
}

//! コンストラクタ

AXTopWindow::AXTopWindow(AXWindow *pOwner,UINT uStyle)
    : AXWindow(pOwner, uStyle)
{
    m_uType = TYPE_TOPLEVEL;
    m_uFlags |= FLAG_TOPLEVEL;

	//値初期化

    m_pFocus        = NULL;
    m_pDefButton    = NULL;
    m_pIC           = NULL;
    m_pAccel        = NULL;
    m_pMenuBar      = NULL;

    //装飾セット

    _setDecoration();

    //オーナーありの場合、オーナーウィンドウより背面に表示されないようにする
    //(WM_TRANSIENT_FOR)

    if(pOwner && (uStyle & WS_TRANSIENT_FOR))
        ::XSetTransientForHint(DISP, m_id, pOwner->getid());

    //ウィンドウマネージャの機能

    if(!(uStyle & WS_DISABLE_WM))
    {
        //------- 受け取るメッセージ(WM_PROTOCOLS)

        Atom atm[3];
        int cnt = 0;

        atm[cnt++] = axatom(AXAppAtom::WM_DELETE_WINDOW);
        atm[cnt++] = axatom(AXAppAtom::_NET_WM_PING);

        if(!(uStyle & WS_NOT_TAKEFOCUS))
            atm[cnt++] = axatom(AXAppAtom::WM_TAKE_FOCUS);

        ::XSetWMProtocols(DISP, m_id, atm, cnt);

        //-------- IM

    #ifndef _AX_OPT_NOIM
          m_pIC = axapp->createIC(this);
    #endif
    }
}


//===============================
//サブ処理
//===============================


//! フォーカス（子ウィンドウ）変更
/*!
    @param p フォーカスを設定したいウィンドウ（NULLでなし）
    @param detail onFocusIn/Out に渡す引数
*/

void AXTopWindow::_changeFocusWindow(AXWindow *p,int detail)
{
    if(p != m_pFocus)
    {
        if(m_pFocus)
        {
            m_pFocus->_killFocus(detail);
            m_pFocus->m_uFlags &= ~FLAG_DEFAULT_FOCUS;
        }

        m_pFocus = p;

        if(m_pFocus)
            m_pFocus->_setFocus(detail);
    }
}

//! 現在のフォーカスで指定キーを処理するかどうか＆キータイプ取得

BOOL AXTopWindow::_isFocusProcKey(UINT keysym,UINT *pType)
{
    UINT keytype;

    //キータイプ

    if(ISKEY_TAB(keysym))
        keytype = ACCEPTKEYTYPE_TAB;
    else if(ISKEY_ENTER(keysym))
        keytype = ACCEPTKEYTYPE_ENTER;
    else if(keysym == KEY_ESCAPE)
        keytype = ACCEPTKEYTYPE_ESCAPE;
    else
        keytype = (UINT)-1;

    *pType = keytype;

    //フォーカスウィンドウがあるか

    if(!m_pFocus) return FALSE;

    //フォーカスウィンドウでキー処理無効

    if(m_pFocus->m_uFlags & FLAG_FOCUS_DISABLE_KEY) return FALSE;

    //特殊キーをフォーカス側で処理するか

    if(keytype == (UINT)-1)
        return TRUE;
    else
        return m_pFocus->isAcceptKey(keytype);
}

//! ウィンドウタイトルセット（UTF8）
//! @param len 負の値で自動

void AXTopWindow::_setTitle(LPCSTR pBuf,int len)
{
    if(len < 0) len = ::strlen(pBuf);

    ::XChangeProperty(DISP, m_id, axatom(AXAppAtom::_NET_WM_NAME),
        axapp->atom(AXAppAtom::UTF8_STRING), 8, PropModeReplace, (LPBYTE)pBuf, len);

    ::XChangeProperty(DISP, m_id, axatom(AXAppAtom::_NET_WM_ICON_NAME),
        axapp->atom(AXAppAtom::UTF8_STRING), 8, PropModeReplace, (LPBYTE)pBuf, len);
}

//! ウィンドウ装飾セット

void AXTopWindow::_setDecoration()
{
    long val[4];
    //[0]flags,[1]functions,[2]decorations,[3]inputmode

    val[0] = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS | MWM_HINTS_INPUT_MODE;
    val[1] = MWM_FUNC_MOVE;
    val[2] = 0;
    val[3] = MWM_INPUT_MODELESS;

    if(m_uStyle & WS_TITLE)
        val[2] |= MWM_DECOR_TITLE;

    if(m_uStyle & WS_MINIMIZE)
    {
        val[2] |= MWM_DECOR_MINIMIZE;
        val[1] |= MWM_FUNC_MINIMIZE;
    }

    if(m_uStyle & WS_MAXIMIZE)
    {
        val[2] |= MWM_DECOR_MAXIMIZE;
        val[1] |= MWM_FUNC_MAXIMIZE;
    }

    if(m_uStyle & WS_CLOSE)
        val[1] |= MWM_FUNC_CLOSE;

    if(m_uStyle & WS_BORDER)
        val[2] |= MWM_DECOR_BORDER;

    if(m_uStyle & WS_MENUBTT)
        val[2] |= MWM_DECOR_MENU;

    if(!(m_uStyle & WS_DISABLE_RESIZE))
        val[1] |= MWM_FUNC_RESIZE;

    ::XChangeProperty(DISP, m_id, axatom(AXAppAtom::_MOTIF_WM_HINTS),
        axatom(AXAppAtom::_MOTIF_WM_HINTS),
        32, PropModeReplace, (LPBYTE)val, 4);
}

//! IM 対応の LookupString
/*!
    キー入力時の文字列取得。

    @param pKeySym キー識別子が返る
    @return [0]なにもなし [1]KeySymのみ [2]文字列あり
*/

int AXTopWindow::getKeyString(LPVOID pEvent,AXString *pStr,UINT *pKeySym)
{
    int len;
    Status ret;
    KeySym keysym;
    char m[12];

#ifdef _AX_OPT_NOIM

    len = ::XLookupString((XKeyEvent *)pEvent, m, 12, &keysym, NULL);
    ret = (len == 0)? 1: 2;

	m[len] = 0;
    *pStr = m;

#else

    if(!m_pIC)
    {
        //IM非対応

        len = ::XLookupString((XKeyEvent *)pEvent, m, 12, &keysym, NULL);
        ret = (len == 0)? 1: 2;

		m[len] = 0;
        *pStr = m;
    }
    else
    {
        //IM対応
        //※XwcLookupString ではNULL文字はセットされない

        LPWSTR pwc = new WCHAR[32];

        len = ::XwcLookupString((XIC)m_pIC->getIC(), (XKeyPressedEvent *)pEvent, pwc, 32, &keysym, &ret);

        if(ret == XBufferOverflow)
        {
            delete []pwc;
            pwc = new WCHAR[len + 1];

            len = ::XwcLookupString((XIC)m_pIC->getIC(), (XKeyPressedEvent *)pEvent, pwc, len + 1, &keysym, &ret);
        }

        pStr->setWide(pwc, len);

        delete []pwc;

        if(ret == XLookupNone)
            ret = 0;
        else if(ret == XLookupKeySym)
            ret = 1;
        else
            ret = 2;
    }

#endif

    if(ret != 2) pStr->empty();

    *pKeySym = keysym;

    return ret;
}

//! (IM) 前編集開始時、位置取得

void AXTopWindow::getICPos(AXPoint *pPos)
{
    if(m_pFocus)
        m_pFocus->getICPos(pPos);
    else
        AXWindow::getICPos(pPos);
}


//===============================
//
//===============================


//! ウィンドウタイトル名セット(AXString)

void AXTopWindow::setTitle(const AXString &str)
{
    AXByteString utf8;

    str.toUTF8(&utf8);
    _setTitle(utf8, utf8.getLen());
}

//! ウィンドウタイトル名セット(UNICODE-16bit)

void AXTopWindow::setTitle(LPCUSTR pText)
{
    AXString str(pText);
    AXByteString utf8;

    str.toUTF8(&utf8);
    _setTitle(utf8, utf8.getLen());
}

//! ウィンドウタイトル名セット(ASCII文字列)

void AXTopWindow::setTitle(LPCSTR pText)
{
    _setTitle(pText, -1);
}

//! ウィンドウタイトル名セット(翻訳文字列)

void AXTopWindow::setTitle(WORD wStrID)
{
    setTitle(_str(wStrID));
}

//! アプリケーションアイコンをセット
/*!
    複数のサイズのイメージをセット可能。

    @param pBuf long 配列。[0]幅 [1]高さ [2-]イメージ(ARGB)
    @param bClear TRUEで現在のイメージをクリアしてセット。FALSEで追加。
*/

void AXTopWindow::setAppIcon(const void *pBuf,BOOL bClear)
{
	const long *pl = (const long *)pBuf;

    if(pBuf)
    {
        ::XChangeProperty(DISP, m_id, axatom(AXAppAtom::_NET_WM_ICON),
                    XA_CARDINAL, 32, (bClear)? PropModeReplace: PropModeAppend,
                    (LPBYTE)pBuf, 2 + pl[0] * pl[1]);
    }
}

//! D&Dを有効にする

void AXTopWindow::enableDND()
{
    //"XdndAware" プロパティにDNDバージョンセット
    //※子ウィンドウではなくトップレベルにセットする

    Atom ver = 4;

    ::XChangeProperty(DISP, m_id, axatom(AXAppAtom::XdndAware), XA_ATOM, 32,
            PropModeReplace, (unsigned char *)&ver, 1);
}

//! ウィンドウ装飾変更

void AXTopWindow::changeDecoration(UINT style,BOOL bAdd)
{
    changeStyle(style, bAdd);
    _setDecoration();
}

//! ウィンドウ枠の幅取得

void AXTopWindow::getFrameWidth(AXRect *prc)
{
    long val[4];

    if(AXGetProperty32Array(m_id, axatom(AXAppAtom::_NET_FRAME_EXTENTS), XA_CARDINAL, val, 4))
    {
        prc->left   = val[0];
        prc->right  = val[1];
        prc->top    = val[2];
        prc->bottom = val[3];
    }
    else
        prc->set(0);
}

//! 枠も含めたウィンドウ全体サイズ取得

void AXTopWindow::getWindowSize(AXSize *psize)
{
    AXRect rc;

    getFrameWidth(&rc);

    psize->w = m_nW + rc.left + rc.right;
    psize->h = m_nH + rc.top + rc.bottom;
}

//! トップレベル用、ウィンドウの位置（ルート座標）とサイズ取得
/*!
    ・位置はウィンドウ枠を含む。@n
    ・ウィンドウの非表示時も、前回表示時の位置が取得できる。
*/

void AXTopWindow::getTopWindowRect(AXRectSize *prcs)
{
    Window top,parent;

    top = AXGetTopFrameWindow(m_id);

    ::XTranslateCoordinates(DISP, top, axapp->getRootID(), 0, 0, &prcs->x, &prcs->y, &parent);

    prcs->w = m_nW;
    prcs->h = m_nH;
}

//! ウィンドウの位置を取得

void AXTopWindow::getTopWindowPos(AXPoint *ppt)
{
    AXRectSize rcs;

    getTopWindowRect(&rcs);

    ppt->x = rcs.x;
    ppt->y = rcs.y;
}

//! デフォルトボタンセット
/*!
    @param pbtt NULLでデフォルトボタンなしにする
*/

void AXTopWindow::setDefaultButton(AXWindow *pbtt)
{
    if(m_pDefButton)
    {
        m_pDefButton->m_uFlags &= ~AXButton::FLAG_DEFAULTBUTTON;
        m_pDefButton->redraw();
    }

    m_pDefButton = pbtt;

    if(m_pDefButton)
    {
        m_pDefButton->m_uFlags |= AXButton::FLAG_DEFAULTBUTTON;
        m_pDefButton->redraw();
    }
}


//===============================
//レイアウト関連
//===============================


//! レイアウト実行

void AXTopWindow::layout()
{
    if(!(m_uFlags & FLAG_CALCULATED))
        calcDefSize();

    if(m_pLayout)
        m_pLayout->layout();

    m_uFlags |= FLAG_LAYOUTED;
}


//===============================
//状態
//===============================


//! 前回の位置で表示

void AXTopWindow::showRestore()
{
    AXRectSize rcs;

    getTopWindowRect(&rcs);

    show();
    move(rcs.x, rcs.y);
}

//! 初期表示処理
/*!
    @param rcsDef  デフォルトの位置とサイズ（位置は負の場合画面中央）
    @param initVal rcs の各値がこの値ならデフォルト値を使う
    @param bShow   表示するか
*/

void AXTopWindow::showInit(const AXRectSize &rcs,const AXRectSize &rcsDef,int initVal,BOOL bShow)
{
    //サイズ

    resize((rcs.w == initVal)? rcsDef.w: rcs.w,
           (rcs.h == initVal)? rcsDef.h: rcs.h);

    //表示

    if(bShow) show();

    //移動

    if(rcs.x != initVal && rcs.y != initVal)
        moveInRoot(rcs.x, rcs.y);
    else
    {
        if(rcsDef.x >= 0 && rcsDef.y >= 0)
            moveInRoot(rcsDef.x, rcsDef.y);
        else
            moveCenter();
    }
}

//! 初期表示処理（位置のみ）
/*!
    @param defx,defy 負の値で中央位置
*/

void AXTopWindow::showInit(const AXPoint &pt,int defx,int defy,int initVal,BOOL bShow)
{
    if(bShow) show();

    if(pt.x != initVal && pt.y != initVal)
        moveInRoot(pt.x, pt.y);
    else
    {
        if(defx >= 0 && defy >= 0)
            moveInRoot(defx, defy);
        else
            moveCenter();
    }
}

//! 指定ウィンドウの中央位置に移動
/*!
    @param pwin NULLでルートウィンドウ
*/

void AXTopWindow::moveCenter(AXWindow *pwin)
{
    AXRectSize rc;
    AXRect rcWork,rcFrame;
    int w,h;

    if(pwin == NULL) pwin = axapp->getRootWindow();

    pwin->getWindowRect(&rc);
    getFrameWidth(&rcFrame);

    w = m_nW + rcFrame.left + rcFrame.right;
    h = m_nH + rcFrame.top + rcFrame.bottom;

    //指定ウィンドウの中央

    if(w < rc.w)
        rc.x += (rc.w - w) >> 1;

    if(h < rc.h)
        rc.y += (rc.h - h) >> 1;

    //ルートウィンドウの作業領域内に収まるように調整

    (axapp->getRootWindow())->getPadding(&rcWork);

    if(rc.x + w > rcWork.right)  rc.x = rcWork.right - w;
    if(rc.y + h > rcWork.bottom) rc.y = rcWork.bottom - h;

    if(rc.x < rcWork.left) rc.x = rcWork.left;
    if(rc.y < rcWork.top)  rc.y = rcWork.top;

    move(rc.x, rc.y);
}

//! ルート内に収まるように位置移動

void AXTopWindow::moveInRoot(int x,int y)
{
    adjustPosInRoot(&x, &y);
    move(x, y);
}

//! 表示
/*!
    ※表示位置は前回の位置で表示されない。改めてウィンドウマネージャによって位置が決定される。
*/

void AXTopWindow::show()
{
    AXWindow::show();
    raise();
}

//! 非表示

void AXTopWindow::hide()
{
    if(m_uFlags & FLAG_VISIBLE)
    {
        killXFocus();

        m_uFlags &= ~FLAG_VISIBLE;
        ::XWithdrawWindow(DISP, m_id, axapp->getDefScreen());
    }
}

//! 移動

void AXTopWindow::move(int x,int y)
{
    XWindowChanges cw;

    m_nX = x, m_nY = y;

    cw.x = x;
    cw.y = y;

    ::XReconfigureWMWindow(DISP, m_id, axapp->getDefScreen(), CWX | CWY, &cw);

    onMove();
}

//! サイズ変更

BOOL AXTopWindow::resize(int w,int h)
{
    XWindowChanges wc;

    if(w < 1) w = 1;
    if(h < 1) h = 1;

    if(w != m_nW || h != m_nH)
    {
        m_nW = w, m_nH = h;

        //

        wc.x = wc.y = 0;
        wc.width        = w;
        wc.height       = h;
        wc.border_width = 0;
        wc.sibling      = None;
        wc.stack_mode   = Above;

        ::XReconfigureWMWindow(DISP, m_id, axapp->getDefScreen(),
            CWWidth | CWHeight, &wc);

        //

        onSize();
        layout();

        return TRUE;
    }

    return FALSE;
}

//! 移動＆サイズ変更

BOOL AXTopWindow::moveresize(int x,int y,int w,int h)
{
    XWindowChanges wc;

    if(w < 1) w = 1;
    if(h < 1) h = 1;

    if(x != m_nX || y != m_nY || w != m_nW || h != m_nH)
    {
        m_nX = x, m_nY = y;
        m_nW = w, m_nH = h;

        //

        wc.x            = x;
        wc.y            = y;
        wc.width        = w;
        wc.height       = h;
        wc.border_width = 0;
        wc.sibling      = None;
        wc.stack_mode   = Above;

        ::XReconfigureWMWindow(DISP, m_id, axapp->getDefScreen(),
            CWX | CWY | CWWidth | CWHeight, &wc);

        //

        onMove();
        onSize();

        layout();

        return TRUE;
    }

    return FALSE;
}

//! 最小化されているか

BOOL AXTopWindow::isMinimized()
{
    Atom type;
    int format;
    ULONG len,after;
    LPBYTE pDat;
    BOOL ret = FALSE;

    if(::XGetWindowProperty(DISP, m_id,
        axatom(AXAppAtom::WM_STATE), 0, 2, FALSE, AnyPropertyType,
        &type, &format, &len, &after, &pDat) == Success)
    {
        if(format == 32)
            ret = (*((ULONG *)pDat) == IconicState);

        ::XFree((char *)pDat);
    }

    return ret;
}

//! 最大化されているか

BOOL AXTopWindow::isMaximized()
{
    return AXIsExistNetWMState(m_id,
            axatom(AXAppAtom::_NET_WM_STATE_MAXIMIZED_HORZ),
            axatom(AXAppAtom::_NET_WM_STATE_MAXIMIZED_VERT));
}

//! 最小化

BOOL AXTopWindow::minimize()
{
    if(isMinimized()) return FALSE;

    ::XIconifyWindow(DISP, m_id, axapp->getDefScreen());

    return TRUE;
}

//! 最大化

BOOL AXTopWindow::maximize()
{
    if(isMaximized()) return FALSE;

    AXSendNetWMState(m_id, 1,
        axatom(AXAppAtom::_NET_WM_STATE_MAXIMIZED_HORZ),
        axatom(AXAppAtom::_NET_WM_STATE_MAXIMIZED_VERT));

    ::XMapWindow(DISP, m_id);

    return TRUE;
}

//! 最大化/最小化 元に戻す

BOOL AXTopWindow::restore()
{
    if(!isMinimized() && !isMaximized()) return FALSE;

    AXSendNetWMState(m_id, 0,
        axatom(AXAppAtom::_NET_WM_STATE_MAXIMIZED_HORZ),
        axatom(AXAppAtom::_NET_WM_STATE_MAXIMIZED_VERT));

    ::XMapWindow(DISP, m_id);

    return TRUE;
}


//===============================
//フォーカス関連
//===============================


//! デフォルトフォーカス取得

AXWindow *AXTopWindow::getDefaultFocus()
{
    return searchChild(SEARCHTYPE_FLAGS, FLAG_DEFAULT_FOCUS);
}

//! 一番最初のフォーカスセット可能な子ウィンドウ取得

AXWindow *AXTopWindow::getFirstTakeFocus()
{
    return searchChild(SEARCHTYPE_FLAGS, FLAG_TAKE_FOCUS | FLAG_ENABLED | FLAG_VISIBLE);
}

//! 現在のフォーカス位置から次の位置へ（タブ移動用）

BOOL AXTopWindow::moveNextFocus()
{
    AXWindow *p;

    p = searchTree(SEARCHTYPE_FLAGS, FLAG_TAKE_FOCUS | FLAG_VISIBLE | FLAG_ENABLED, m_pFocus);
    if(!p)
    {
        //次の位置がなければ一番最初のフォーカスへ戻る
        p = getFirstTakeFocus();
        if(!p) return FALSE;
    }

    _changeFocusWindow(p, FOCUSDETAIL_TABMOVE);

    return TRUE;
}


//===============================
//ほか
//===============================


//! ルート座標を、ウィンドウがルート内に収まるように位置調整

void AXTopWindow::adjustPosInRoot(LPINT px,LPINT py)
{
    int x,y,rw,rh;
    AXSize size;

    x = *px, y = *py;

    rw = axapp->getRootWindow()->getWidth();
    rh = axapp->getRootWindow()->getHeight();

    getWindowSize(&size);

    if(x + size.w > rw) x = rw - size.w;
    if(y + size.h > rh) y = rh - size.h;

    if(x < 0) x = 0;
    if(y < 0) y = 0;

    *px = x, *py = y;
}

//! エラーメッセージ表示

void AXTopWindow::errMes(LPCUSTR pText)
{
    AXMessageBox::error(this, pText);
}

//! エラーメッセージ表示（翻訳文字列から）

void AXTopWindow::errMes(WORD wStrGroupID,WORD wStrID)
{
    AXMessageBox::error(this, _string(wStrGroupID, wStrID));
}


//===============================
//ハンドラ
//===============================


//! ウィンドウ構成変更時

BOOL AXTopWindow::onConfigure(AXHD_CONFIGURE *phd)
{
    //ルート上の位置（ウィンドウ枠は含まない）

    if(phd->bSendEvent && phd->x != m_nX && phd->y != m_nY)
    {
        m_nX = phd->x;
        m_nY = phd->y;

        onMove();
    }

    //サイズ

    if(phd->w != m_nW || phd->h != m_nH)
    {
        m_nW = phd->w;
        m_nH = phd->h;

        onSize();
        layout();
    }

    return TRUE;
}

//! フォーカスIN

BOOL AXTopWindow::onFocusIn(int detail)
{
    m_uFlags |= FLAG_FOCUSED;

    if(!m_pFocus)
    {
        m_pFocus = getDefaultFocus();
        if(!m_pFocus) m_pFocus = getFirstTakeFocus();

        if(m_pFocus) m_pFocus->_setFocus(detail);
    }

#ifndef _AX_OPT_NOIM
    if(m_pIC) m_pIC->setFocus();
#endif

    return TRUE;
}

//! フォーカスOUT

BOOL AXTopWindow::onFocusOut(int detail)
{
    m_uFlags &= ~FLAG_FOCUSED;

    if(m_pFocus)
    {
        m_pFocus->_killFocus(detail);
        m_pFocus = NULL;
    }

#ifndef _AX_OPT_NOIM
    if(m_pIC) m_pIC->killFocus();
#endif

    return TRUE;
}

//! キー押し
/*!
    ※ポインタグラブ中も来るので注意。

    @return TRUEでキーに対して処理が行われた
*/

BOOL AXTopWindow::onKeyDown(AXHD_KEY *phd)
{
    UINT keytype;

    if(!axapp->isGrab())
    {
        //アクセラレータ

        if(m_pAccel)
        {
            if(m_pAccel->onKey(phd, FALSE))
                return TRUE;
        }

        //メニューバー Alt+ のホットキー

        if(m_pMenuBar)
        {
            if(m_pMenuBar->_showFromHotKey(phd->keysym, phd->state))
                return TRUE;
        }
    }

    //キーをフォーカス側で処理するか

    if(_isFocusProcKey(phd->keysym, &keytype))
    {
        if(m_pFocus->isEnabled())
            return m_pFocus->onKeyDown(phd);
        else
            return FALSE;
    }

    //各キー処理

    if(!axapp->isGrab())
    {
        switch(keytype)
        {
            //TABキー : フォーカス移動
            case ACCEPTKEYTYPE_TAB:
                if(m_uStyle & WS_TABMOVE)
                    moveNextFocus();
                return TRUE;
            //Enterキー : デフォルトボタン
            case ACCEPTKEYTYPE_ENTER:
                if(m_pDefButton && m_pDefButton->isEnabled())
                    m_pDefButton->onKeyDown(phd);
                return TRUE;
        }
    }

    return FALSE;
}

//! キー離し

BOOL AXTopWindow::onKeyUp(AXHD_KEY *phd)
{
    UINT keytype;

    if(!axapp->isGrab())
    {
        //アクセラレータ

        if(m_pAccel)
        {
            if(m_pAccel->onKey(phd, TRUE))
                return TRUE;
        }

        //メニューバー Alt+ のホットキー

        if(m_pMenuBar)
        {
            if(m_pMenuBar->_showFromHotKey(phd->keysym, phd->state))
                return TRUE;
        }
    }

    //キーをフォーカス側で処理するか

    if(_isFocusProcKey(phd->keysym, &keytype))
    {
        if(m_pFocus->isEnabled())
            return m_pFocus->onKeyUp(phd);
        else
            return FALSE;
    }

    //各キー処理

    if(!axapp->isGrab())
    {
        switch(keytype)
        {
            //Enterキー : デフォルトボタン
            case ACCEPTKEYTYPE_ENTER:
                if(m_pDefButton && m_pDefButton->isEnabled())
                    m_pDefButton->onKeyUp(phd);
                return TRUE;
        }
    }

    return FALSE;
}
