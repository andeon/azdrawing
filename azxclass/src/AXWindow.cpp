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

#include <unistd.h>

#define _AX_X11_ATOM
#include "AXX11.h"

#include "AXWindow.h"
#include "AXTopWindow.h"
#include "AXLayout.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXAppAtom.h"
#include "AXLayout.h"
#include "AXUtilX.h"
#include "AXCursor.h"

//-------------------------

#define DISP                ((Display *)m_pDisp)
#define EVENTMASK_ENABLED   (ButtonPressMask|ButtonReleaseMask|PointerMotionMask)

//-------------------------

/*!
    @class AXWindow
    @brief ウィンドウクラス

    - 削除（delete）時には、子ウィンドウやレイアウト、レイアウトの子アイテムは全て自動で削除される。
    - レイアウトにセットされていて、親のデストラクタではなく直接削除する時は、removeFromLayout() で先にレイアウトから取り除いておくこと。
    - フォント(m_pFont)はデフォルトで親ウィンドウと同じものがセットされる。
    - 通知ウィンドウ（m_pNotify）はデフォルトで親ウィンドウにセットされる。@n
      ウィジェットによっては、getNotify() で m_pNotify が返るとは限らない（例： AXFileListView）

    @ingroup window
*/


//-------------------------


AXWindow::~AXWindow()
{
    _deleteSub();

    if(m_id)
    {
        m_uFlags &= ~FLAG_EXIST;

        //カーソル削除

        if(m_pCursor && (m_uFlags & FLAG_CURSOR_NEW))
            delete m_pCursor;

        //フォーカス取り除く

        _removeFocusMe();

        //AXAppのリストから削除

        axapp->delTimerWin(this);
        axapp->removeDraw(this);
        axapp->removeNotify(this);
        axapp->removeReconfig(this);

        //ハッシュテーブルから取り除く

        axapp->removeWindow(this);

		//前後をつなげる

        if(m_pPrev)
            m_pPrev->m_pNext = m_pNext;
        else if(m_pParent)
            m_pParent->m_pFirst = m_pNext;

        if(m_pNext)
            m_pNext->m_pPrev = m_pPrev;
        else if(m_pParent)
            m_pParent->m_pLast = m_pPrev;

        //実際のウィンドウ削除

        ::XDestroyWindow(DISP, m_id);
    }
}

//! 子ウィンドウなど削除

void AXWindow::_deleteSub()
{
    //レイアウト＆子アイテム

    if(m_pLayout)
    {
        delete m_pLayout;
        m_pLayout = NULL;
    }

    //残りの子ウィンドウ削除

    while(m_pFirst)
        delete m_pFirst;
}


//========================
//コンストラクタ
//========================


//! ルートウィンドウ用

AXWindow::AXWindow()
{
    long val[4];

    _init();

    m_id = axapp->getRootID();

    m_nW = ::XDisplayWidth(DISP, axapp->getDefScreen());
    m_nH = ::XDisplayHeight(DISP, axapp->getDefScreen());

    m_uType  = TYPE_ROOT;
    m_uFlags = FLAG_EXIST | FLAG_ENABLED | FLAG_VISIBLE;

    m_pFont   = axres->font(AXAppRes::FONT_NORMAL);
    m_pCursor = NULL;

    //m_pad* に作業領域セット

    if(AXGetProperty32Array(m_id, AXGetAtom("_NET_WORKAREA"), XA_CARDINAL, val, 4))
    {
        m_padLeft   = val[0];
        m_padTop    = val[1];
        m_padRight  = val[0] + val[2];
        m_padBottom = val[1] + val[3];
    }
}

//! トップレベルウィンドウ作成

AXWindow::AXWindow(AXWindow *pOwner,UINT uStyle)
{
    _createwin(NULL, pOwner, uStyle);
}

//! 子ウィンドウ作成
/*!
    @param uLayoutFlags レイアウトフラグ。参照： @link axlayoutitem AXLayoutItem @endlink
*/

AXWindow::AXWindow(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags)
{
    _createwin(pParent, NULL, uStyle);

    m_uLFlags = uLayoutFlags;
}

//! 子ウィンドウ作成（余白指定）
/*!
    @param dwPadding 各余白。0〜255。MAKE_DW4(left,top,right,bottom)
*/

AXWindow::AXWindow(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding)
{
    _createwin(pParent, NULL, uStyle);

    m_uLFlags   = uLayoutFlags;
    m_uItemID   = uItemID;

    m_padLeft   = dwPadding >> 24;
    m_padTop    = (dwPadding >> 16) & 0xff;
    m_padRight  = (dwPadding >> 8) & 0xff;
    m_padBottom = dwPadding & 0xff;
}


//========================
//初期化
//========================


//! 初期化

void AXWindow::_init()
{
    m_pParent = m_pOwner = m_pTopLevel = m_pNotify = NULL;
    m_pFirst = m_pLast = m_pPrev = m_pNext = NULL;
    m_pHashPrev = m_pHashNext = NULL;
    m_pDrawPrev = m_pDrawNext = NULL;
    m_pReConfPrev = m_pReConfNext = NULL;
    m_pLayout = NULL;

    m_uItemID       = 0;
    m_uStyle        = 0;
    m_uFlags        = 0;
    m_lEventMask    = 0;
    m_idUserTimeWin = 0;

    m_rcDraw.set(0);

    m_pFont   = NULL;
    m_pCursor = NULL;
}

//! ウィンドウ作成

void AXWindow::_createwin(AXWindow *pParent,AXWindow *pOwner,UINT uStyle)
{
    XSetWindowAttributes attr;
    ULONG mask;
    BOOL bTopLevel;

    //初期化

    _init();

    m_nX = m_nY = 0;
    m_nW = m_nH = 1;

    //pParent が NULL の場合、ルートウィンドウが親

    if(!pParent) pParent = axapp->getRootWindow();

    bTopLevel = (pParent == axapp->getRootWindow());

    //--------- ウィンドウ属性
    // do_not_propagate_mask : 親に伝搬させないイベント

    mask = CWBitGravity | CWDontPropagate | CWOverrideRedirect | CWEventMask;

    attr.bit_gravity            = ForgetGravity;
    attr.override_redirect      = (uStyle & WS_DISABLE_WM)? TRUE: FALSE;
    attr.event_mask             = ExposureMask | EnterWindowMask | LeaveWindowMask;
    attr.do_not_propagate_mask  = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ButtonMotionMask;

    //背景色

    if(uStyle & WS_BK_FACE)
    {
        mask |= CWBackPixel;
        attr.background_pixel = axres->colPix(AXAppRes::FACE);
    }

    //トップレベルの場合

    if(bTopLevel)
        attr.event_mask |= FocusChangeMask | StructureNotifyMask | KeyPressMask | KeyReleaseMask;

    //有効状態の場合

    if(!(uStyle & WS_DISABLE))
        attr.event_mask |= EVENTMASK_ENABLED;

    //----------- Xウィンドウ作成

    m_id = ::XCreateWindow(DISP, pParent->getid(),
        0, 0, 1, 1, 0,
        CopyFromParent, CopyFromParent, CopyFromParent,
        mask, &attr);

    if(!m_id) return;

    //ハッシュテーブルに登録

    axapp->addWindow(this);

    //プロセスID

    long pid = ::getpid();

    AXSetPropertyCARDINAL(m_id, axatom(AXAppAtom::_NET_WM_PID), &pid, 1);

    //トップレベル時

    if(bTopLevel && !(uStyle & WS_DISABLE_WM))
    {
        //_NET_WM_USER_TIME

        _setUserTimeWindow();
        _updateUserTime((uStyle & WS_HIDE)? 0: axapp->getUserTime());
    }

    //マップ（表示時）

    if(!(uStyle & WS_HIDE))
        ::XMapWindow(DISP, m_id);

    //------------ 情報セット

    m_pParent   = pParent;
    m_pOwner    = pOwner;
    m_pTopLevel = _getTopLevel();

    //

    m_pPrev = m_pParent->m_pLast;       //前のウィンドウ = 親の最後の子ウィンドウ

    m_pParent->m_pLast = this;          //親の最後の子ウィンドウ = this

    if(m_pPrev)
        m_pPrev->m_pNext = this;        //前のウィンドウの次 = this
    else
        m_pParent->m_pFirst = this;     //親の最初の子ウィンドウ

    //

    m_uStyle     = uStyle;
    m_lEventMask = attr.event_mask;

    m_uFlags |= FLAG_EXIST;
    if(!(m_uStyle & WS_HIDE))    m_uFlags |= FLAG_VISIBLE;
    if(!(m_uStyle & WS_DISABLE)) m_uFlags |= FLAG_ENABLED;

    if(m_pTopLevel == this)
        m_pNotify = this;    //トップレベルの場合、自身へ通知
    else
        m_pNotify = (m_pParent->m_uFlags & FLAG_CHILD_NOTIFY_PARENT)? m_pParent->m_pNotify: pParent;

    m_pFont = m_pParent->m_pFont;       //フォントは親から引き継ぐ
}

//! _NET_WM_USER_TIME 用ウィンドウセット

void AXWindow::_setUserTimeWindow()
{
    if(axapp->isSupport_UserTime())
    {
        //_NET_WM_USER_TIME 用の子ウィンドウ

        m_idUserTimeWin = ::XCreateSimpleWindow(DISP, m_id, -1, -1, 1, 1, 0, 0, 0);

        //_NET_WM_USER_TIME_WINDOW セット

        ::XChangeProperty(DISP, m_id, axatom(AXAppAtom::_NET_WM_USER_TIME_WINDOW), XA_WINDOW, 32,
            PropModeReplace, (unsigned char *)&m_idUserTimeWin, 1);
    }
}


//==================================
//内部処理
//==================================


//! トップレベルウィンドウ取得（ルートは除く）

AXWindow *AXWindow::_getTopLevel()
{
	AXWindow *p = this,*parent;

	while(1)
	{
		parent = p->getParent();
		if(!parent) break;

		if(!parent->getParent()) break;

		p = parent;
	}

	return p;
}

//! 内部フォーカスセット
/*!
    トップレベルで onFocusIn() が来た時や、タブキーなどによるフォーカス移動時に呼ばれる。
*/

void AXWindow::_setFocus(int detail)
{
    if(!(m_uFlags & FLAG_FOCUSED) && (m_uFlags & FLAG_TAKE_FOCUS))
    {
        m_uFlags |= FLAG_DEFAULT_FOCUS | FLAG_FOCUSED;
        onFocusIn(detail);
    }
}

//! 内部フォーカス消去

void AXWindow::_killFocus(int detail)
{
    if(m_uFlags & FLAG_FOCUSED)
    {
        m_uFlags &= ~FLAG_FOCUSED;
        if(m_uFlags & FLAG_EXIST) onFocusOut(detail);
    }
}

//! 自身がフォーカスを持っていた場合、取り消す
/*!
	ウィンドウ削除時や非表示などでフォーカスが無効になる場合に呼ぶ。
*/

void AXWindow::_removeFocusMe()
{
    if(m_uFlags & FLAG_FOCUSED)
        ((AXTopWindow *)m_pTopLevel)->_changeFocusWindow(NULL, FOCUSDETAIL_MANUAL);
}

//! _NET_WM_USER_TIME 更新（トップレベル）
/*!
    ユーザーが操作をした最終時間。@n
    ボタン押し・キー押し・表示前などにセットする。
*/

void AXWindow::_updateUserTime(ULONG time)
{
    if(m_idUserTimeWin)
    {
        ::XChangeProperty(DISP, m_idUserTimeWin, axatom(AXAppAtom::_NET_WM_USER_TIME),
                XA_CARDINAL, 32, PropModeReplace, (unsigned char *)&time, 1);
    }
}

//! 指定特殊キーを自身で処理するか
/*!
    トップレベルの onKeyDown/Up 時に呼ばれる。@n
    タブキーやエンターキーをトップレベルではなく自身で処理したい場合、TRUEを返す。

    @param keytype AXWindow::ACCEPTKEYTYPE
*/

BOOL AXWindow::isAcceptKey(UINT keytype)
{
    return FALSE;
}

//! (IM) 前編集開始時に表示位置取得 ※ルート座標

void AXWindow::getICPos(AXPoint *pPos)
{
    pPos->set(0, 0);
    translateTo(NULL, &pPos->x, &pPos->y);
}


//================================
//情報取得
//================================


//! 表示されているか

BOOL AXWindow::isVisible()
{
    return ((m_uFlags & FLAG_VISIBLE) != 0);
}

//! 親の状態も含め表示されているか

BOOL AXWindow::isVisibleReal()
{
    AXWindow *p;

    if(!(m_uFlags & FLAG_VISIBLE)) return FALSE;

    for(p = m_pParent; p; p = p->m_pParent)
    {
        if(!(p->m_uFlags & FLAG_VISIBLE)) return FALSE;
    }

    return TRUE;
}

//! 有効か

BOOL AXWindow::isEnabled()
{
    return ((m_uFlags & FLAG_ENABLED) != 0);
}

//! フォーカスを持っているか

BOOL AXWindow::isFocused()
{
    return ((m_uFlags & FLAG_FOCUSED) != 0);
}

//! 位置がウィンドウ範囲内に含まれるか（自身の座標）

BOOL AXWindow::isContain(int x,int y)
{
    return (x >= 0 && x < m_nW && y >= 0 && y < m_nH);
}

//! 通知ウィンドウ取得

AXWindow *AXWindow::getNotify()
{
    return m_pNotify;
}

//-----------------

//! ウィンドウの位置（ルート座標）とサイズ取得
/*!
    ※トップレベルの場合、ウィンドウ枠は含まない。
*/

void AXWindow::getWindowRect(AXRectSize *prcs)
{
    int x = 0,y = 0;

    translateTo(NULL, &x, &y);
    prcs->set(x, y, m_nW, m_nH);
}

//! 指定条件に一致する子ウィンドウ取得（全階層）
/*!
    検索対象は、このウィンドウの子ウィンドウのみ。

    @param type 検索タイプ
    @param data 検索データ
*/

AXWindow *AXWindow::searchChild(SEARCHTYPE type,ULONG data)
{
    AXWindow *p = m_pFirst;

    while(p)
    {
        switch(type)
        {
            case SEARCHTYPE_FLAGS:
                if((p->m_uFlags & (UINT)data) == data) return p;
                break;
            case SEARCHTYPE_ITEMID:
                if(p->m_uItemID == (UINT)data) return p;
                break;
        }

        //次へ

        if(p->m_pFirst)
            p = p->m_pFirst;
        else
        {
            while(1)
            {
                if(p->m_pNext)
                {
                    p = p->m_pNext;
                    break;
                }
                else
                {
                    p = p->m_pParent;
                    if(!p) break;

                    if(p == this) return NULL;
                }
            }
        }
    }

    return NULL;
}

//! ウィンドウツリーから条件に合うウィンドウ検索
/*!
    子ウィンドウだけでなく、次の親方向も検索される。@n
    主にフォーカスウィンドウの取得などに使う。

    @param type 検索タイプ
    @param data 検索データ
    @param pTop 検索先頭位置（実際にはこの次のウィンドウから検索）。NULL で子ウィンドウの先頭から。
*/

AXWindow *AXWindow::searchTree(SEARCHTYPE type,ULONG data,AXWindow *pTop)
{
    AXWindow *p;
    BOOL flag;

    if(pTop)
    {
        p = pTop;
        flag = FALSE;
    }
    else
    {
        p = m_pFirst;
        flag = TRUE;
    }

    while(p)
    {
        if(!flag)
            flag = TRUE;
        else
        {
            switch(type)
            {
                case SEARCHTYPE_FLAGS:
                    if((p->m_uFlags & (UINT)data) == data) return p;
                    break;
                case SEARCHTYPE_ITEMID:
                    if(p->m_uItemID == (UINT)data) return p;
                    break;
            }
        }

        //次へ

        if(p->m_pFirst)
            p = p->m_pFirst;
        else
        {
            while(1)
            {
                if(p->m_pNext)
                {
                    p = p->m_pNext;
                    break;
                }
                else
                {
                    p = p->m_pParent;
                    if(!p) break;
                }
            }
        }
    }

    return NULL;
}


//==================================
//X処理
//==================================


//! Xイベントマスク変更
/*!
	@param bAdd TRUEで追加、FALSEで削除
*/

void AXWindow::setEventMask(ULONG mask,BOOL bAdd)
{
	if(bAdd)
		m_lEventMask |= mask;
	else
		m_lEventMask &= ~mask;

	::XSelectInput(DISP, m_id, m_lEventMask);
}

//! Xイベントマスクセット

void AXWindow::selectInput(ULONG mask)
{
    m_lEventMask = mask;
    ::XSelectInput(DISP, m_id, mask);
}

//! Xイベントマスクを ExposureMask のみにセット

void AXWindow::selectInputExposure()
{
    selectInput(ExposureMask);
}

//! Xウィンドウタイプ（_NET_WM_WINDOW_TYPE）セット

void AXWindow::setWindowType(ULONG atom)
{
    AXSetPropertyAtom(m_id, axapp->getAtom("_NET_WM_WINDOW_TYPE"), &atom, 1);
}

//! 背景色を FACELIGHT にセット

void AXWindow::setBackgroundFaceLight()
{
    ::XSetWindowBackground(DISP, m_id, axres->colPix(AXAppRes::FACELIGHT));
}


//==================================
//再描画など
//==================================


//! 再描画要求

void AXWindow::redraw()
{
    m_uFlags |= FLAG_REDRAW;
    axapp->addDraw(this, 0, 0, m_nW, m_nH);
}

void AXWindow::redraw(const AXRectSize &rcs)
{
    m_uFlags |= FLAG_REDRAW;
    axapp->addDraw(this, rcs.x, rcs.y, rcs.w, rcs.h);
}

void AXWindow::redraw(int x,int y,int w,int h)
{
    m_uFlags |= FLAG_REDRAW;
    axapp->addDraw(this, x, y, w, h);
}

//! 再描画＆更新要求

void AXWindow::redrawUpdate()
{
    m_uFlags |= FLAG_REDRAW;
    axapp->addDraw(this, 0, 0, m_nW, m_nH);
    axapp->update();
}

//! 再構成セット

void AXWindow::setReconfig()
{
    axapp->addReconfig(this);
}


//===============================
//レイアウト関連
//===============================


//! レイアウトセット
/*!
    セットされたレイアウトは、デストラクタ時に自動で delete される。
*/

void AXWindow::setLayout(AXLayout *p)
{
    m_pLayout = p;

    //レイアウト自身の親セット

    if(p)
        p->m_pLParent = this;
}

//! レイアウト

void AXWindow::layout()
{
    if(m_pLayout)
        m_pLayout->layout();

    m_uFlags |= FLAG_LAYOUTED;
}

//! 標準サイズ計算

void AXWindow::calcDefSize()
{
    if(m_pLayout)
    {
        m_pLayout->calcDefSize();

        m_nDefW = m_pLayout->getDefW();
        m_nDefH = m_pLayout->getDefH();

        m_uFlags |= FLAG_CALCULATED;
    }
    else
        m_nDefW = m_nDefH = 1;
}

//! 親のレイアウトから取り外す

void AXWindow::removeFromLayout()
{
    if(m_pLParent)
        ((AXLayout *)m_pLParent)->removeItem(this);
}

//! 再レイアウト＆標準サイズが現在のサイズより大きければリサイズ

void AXWindow::relayoutAndResize()
{
    calcDefSize();

    if(m_nDefW > m_nW || m_nDefH > m_nH)
        resize((m_nDefW > m_nW)? m_nDefW: m_nW, (m_nDefH > m_nH)? m_nDefH: m_nH);
    else
        layout();
}


//===============================
//状態変更
//===============================


//! 表示/非表示

void AXWindow::show(BOOL bShow)
{
    if(bShow) show(); else hide();
}

//! 有効/無効

void AXWindow::enable(BOOL bEnable)
{
    if(bEnable) enable(); else disable();
}

//! 表示

void AXWindow::show()
{
    if(!(m_uFlags & FLAG_VISIBLE))
    {
        m_uFlags |= FLAG_VISIBLE;

        _updateUserTime(axapp->getUserTime());

        ::XMapWindow(DISP, m_id);
    }
}

//! 非表示

void AXWindow::hide()
{
    if(m_uFlags & FLAG_VISIBLE)
    {
        _removeFocusMe();

        m_uFlags &= ~FLAG_VISIBLE;
        ::XUnmapWindow(DISP, m_id);
    }
}

//! 有効

void AXWindow::enable()
{
    AXWindow *p;

    if(!(m_uFlags & FLAG_ENABLED))
    {
        m_uFlags |= FLAG_ENABLED;

        setEventMask(EVENTMASK_ENABLED, TRUE);
        redraw();
    }

    //子ウィンドウ

    for(p = m_pFirst; p; p = p->m_pNext)
        p->enable();
}

//! 無効

void AXWindow::disable()
{
    AXWindow *p;

    if(m_uFlags & FLAG_ENABLED)
    {
        m_uFlags &= ~FLAG_ENABLED;

        setEventMask(EVENTMASK_ENABLED, FALSE);
        redraw();
    }

    //子ウィンドウ

    for(p = m_pFirst; p; p = p->m_pNext)
        p->enable();
}

//! 前面へ

void AXWindow::raise()
{
    ::XRaiseWindow(DISP, m_id);
}

//! 背面へ

void AXWindow::lower()
{
    ::XLowerWindow(DISP, m_id);
}

//! 常に最前面
/*!
    @attention マップされている状態で使わないと有効にならない
*/

void AXWindow::above(BOOL bSet)
{
    AXSendNetWMState(m_id, (bSet)? 1: 0, axapp->getAtom("_NET_WM_STATE_ABOVE"));
}

//! アクティブウィンドウに指定（トップレベル）

void AXWindow::setActive()
{
    _updateUserTime(axapp->getUserTime());

    ::XSetInputFocus(DISP, m_id, RevertToParent, axapp->getLastTime());

    onFocusIn(FOCUSDETAIL_ACTIVE);
}

//! タスクバーに表示されないようにする
/*!
    ウィンドウ表示後に実行しないと有効にならない
*/

void AXWindow::setSkipTaskbar()
{
    AXSendNetWMState(m_id, 1, axapp->getAtom("_NET_WM_STATE_SKIP_TASKBAR"));
}

//--------------------

//! 位置移動

void AXWindow::move(int x,int y)
{
    m_nX = x, m_nY = y;
    ::XMoveWindow(DISP, m_id, x, y);

    onMove();
}

//! サイズ変更
/*!
    親の onSize() の方が子よりも先に実行される。
*/

BOOL AXWindow::resize(int w,int h)
{
    if(w < 1) w = 1;
    if(h < 1) h = 1;

    if(w != m_nW || h != m_nH)
    {
        m_nW = w, m_nH = h;
        ::XResizeWindow(DISP, m_id, w, h);

        /*
            ※onSize() は layout() より前に実行する。
            子アイテムのレイアウト前にレイアウト状態を変更したい場合に必要。
        */

        onSize();
        layout();

        return TRUE;
    }

    return FALSE;
}

//! 位置＆サイズ変更

BOOL AXWindow::moveresize(int x,int y,int w,int h)
{
    BOOL ret = FALSE;

    if(w < 1) w = 1;
    if(h < 1) h = 1;

    if(x != m_nX || y != m_nY || w != m_nW || h != m_nH)
    {
        m_nX = x, m_nY = y;
        m_nW = w, m_nH = h;

        ::XMoveResizeWindow(DISP, m_id, x, y, w, h);

        onMove();
        onSize();

        layout();

        ret = TRUE;
    }

    return ret;
}

//! デフォルトサイズを計算し、そのサイズにリサイズ

void AXWindow::resizeDefSize()
{
    calcDefSize();
    resize(m_nDefW, m_nDefH);
}

//! 初期リサイズ用、保存サイズとデフォルトサイズのどちらかでリサイズ
/*!
    w,h が 0 以下の場合、defw,defh を使う。
*/

void AXWindow::resizeSwitch(int w,int h,int defw,int defh)
{
    resize((w <= 0)? defw: w, (h <= 0)? defh: h);
}


//==================================
//ほか
//==================================


//! スタイルフラグ変更

void AXWindow::changeStyle(UINT style,BOOL bAdd)
{
    if(bAdd)
        m_uStyle |= style;
    else
        m_uStyle &= ~style;
}

//! 内部フォーカス変更（自身にフォーカスをセットする）

void AXWindow::setFocus()
{
    if(m_uFlags & FLAG_TAKE_FOCUS)
        ((AXTopWindow *)m_pTopLevel)->_changeFocusWindow(this, FOCUSDETAIL_MANUAL);
}

//! Xフォーカスセット

void AXWindow::setXFocus()
{
    ::XSetInputFocus(DISP, m_id, RevertToPointerRoot, CurrentTime);
}

//! Xフォーカス消去

void AXWindow::killXFocus()
{
    Window win;
    int ret;

    ::XGetInputFocus(DISP, &win, &ret);

    if(win == m_id)
        ::XSetInputFocus(DISP, (m_pOwner)? m_pOwner->getid(): PointerRoot, RevertToPointerRoot, CurrentTime);
}

//! カーソル変更
/*!
    現在のポインタと同じなら変更なし。

    @param pCursor NULLで解除
*/

void AXWindow::setCursor(AXCursor *pCursor)
{
    if(pCursor != m_pCursor)
    {
        unsetCursor();

        m_pCursor = pCursor;
        m_uFlags  &= ~FLAG_CURSOR_NEW;

        if(pCursor)
            ::XDefineCursor(DISP, m_id, pCursor->getid());
    }
}

//! カーソル変更（指定タイプ）
/*!
    内部でカーソルを作成する。@n
    unsetCursor() やウィンドウ削除時に自動的に削除される。

    @param type  AXCursor::CURSORTYPE
*/

void AXWindow::setCursor(int type)
{
    unsetCursor();

    m_pCursor = new AXCursor(type);
    m_uFlags  |= FLAG_CURSOR_NEW;

    ::XDefineCursor(DISP, m_id, m_pCursor->getid());
}

//! カーソル変更（データから）
/*!
    内部でカーソルを作成する。@n
    unsetCursor() やウィンドウ削除時に自動的に削除される。
*/

void AXWindow::setCursor(const unsigned char *pDat)
{
    unsetCursor();

    m_pCursor = new AXCursor;
    m_pCursor->create(pDat);

    m_uFlags  |= FLAG_CURSOR_NEW;

    ::XDefineCursor(DISP, m_id, m_pCursor->getid());
}

//! カーソルのセットを解除
/*!
    カーソルが作成されている場合は削除し、親ウィンドウと同じカーソルにする
*/

void AXWindow::unsetCursor()
{
    if(m_pCursor)
    {
        if(m_uFlags & FLAG_CURSOR_NEW)
            delete m_pCursor;

        m_pCursor = NULL;

        ::XDefineCursor(DISP, m_id, None);
    }
}

//! マウスカーソルの位置を、このウィンドウの座標で取得

void AXWindow::getCursorPos(AXPoint *ppt)
{
    Window win;
    int x,y,rx,ry;
    UINT btt;

    ::XQueryPointer(DISP, m_id, &win, &win, &rx, &ry, &x, &y, &btt);

    ppt->x = x;
    ppt->y = y;
}

//! マウスカーソルが現在ウィンドウ内にあるか

BOOL AXWindow::isCursorIn()
{
    Window win;
    int x,y,rx,ry;
    UINT btt;

    if(!::XQueryPointer(DISP, m_id, &win, &win, &rx, &ry, &x, &y, &btt))
        return FALSE;
    else
        //x,yは自身の座標
        return isContain(x, y);
}

//! 自身の座標を指定ウィンドウの座標に変換
/*!
    @param pwinTo NULLでルートウィンドウ
*/

void AXWindow::translateTo(AXWindow *pwinTo,LPINT px,LPINT py)
{
    Window win;

    if(!pwinTo) pwinTo = axapp->getRootWindow();

    ::XTranslateCoordinates(DISP, m_id, pwinTo->getid(), *px, *py, px, py, &win);
}

//! 指定ウィンドウの座標を自身の座標へ変換

void AXWindow::translateFrom(AXWindow *pwinFrom,LPINT px,LPINT py)
{
    Window win;

    if(!pwinFrom) pwinFrom = axapp->getRootWindow();

    ::XTranslateCoordinates(DISP, pwinFrom->getid(), m_id, *px, *py, px, py, &win);
}

//! AXRect の座標を指定ウィンドウの座標に変換

void AXWindow::translateRectTo(AXWindow *pwinTo,AXRect *prc)
{
    translateTo(pwinTo, &prc->left, &prc->top);
    translateTo(pwinTo, &prc->right, &prc->bottom);
}


//==========================
//グラブ
//==========================


//! マウスポインタをグラブ
/*!
    グラブ中もキー押しイベントが来るので注意。@n
    TABやENTERなど、トップレベル側で処理するキーは対処しているので、グラブ中は処理されない。@n
    axapp->isGrab() でグラブ中か判定できる。
*/

BOOL AXWindow::grabPointer()
{
    if(axapp->AXApp::isGrab())
        return FALSE;
    else
    {
        if(::XGrabPointer(DISP, m_id, FALSE,
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync,
                    None, None, CurrentTime) == GrabSuccess)
        {
            axapp->grabWindow(this);
            return TRUE;
        }
        else
            return FALSE;
    }
}

//! グラブ中のカーソルを指定してグラブ

BOOL AXWindow::grabPointer(AXCursor *pCursor)
{
    if(axapp->AXApp::isGrab())
        return FALSE;
    else
    {
    #ifdef _AX_OPT_NO_GRAB
        axapp->grabWindow(this);
        return TRUE;
    #else
        if(::XGrabPointer(DISP, m_id, FALSE,
                    ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                    GrabModeAsync, GrabModeAsync,
                    None, pCursor->getid(), CurrentTime) == GrabSuccess)
        {
            axapp->grabWindow(this);
            return TRUE;
        }
        else
            return FALSE;
    #endif
    }
}

//! ポインタのグラブ解除

void AXWindow::ungrabPointer()
{
    if(axapp->AXApp::isGrab())
    {
        axapp->ungrabWindow();

        ::XUngrabPointer(DISP, CurrentTime);
        ::XFlush(DISP);
    }
}

//! グラブ中のカーソル変更
/*!
    @param bEvent マウスイベントを受け取るか
*/

void AXWindow::changeGrabCursor(AXCursor *pCursor,BOOL bEvent)
{
    ::XChangeActivePointerGrab(DISP,
                bEvent? ButtonPressMask | ButtonReleaseMask | PointerMotionMask: 0,
                pCursor->getid(), axapp->getLastTime());
}


//==========================
//タイマー
//==========================


//! タイマー追加
/*!
    @param uTimerID  任意のタイマーID
    @param uInterval 間隔。ミリ秒単位
*/

void AXWindow::addTimer(UINT uTimerID,UINT uInterval,ULONG lParam)
{
    axapp->addTimer(this, uTimerID, uInterval, lParam);
}

//! タイマー削除
/*!
    ・onTimer() 内で実行しても良い（ただし、onTimerで呼ばれたIDのみ）
*/

void AXWindow::delTimer(UINT uTimerID)
{
    axapp->delTimer(this, uTimerID);
}

//! タイマー全て削除

void AXWindow::delTimerAll()
{
    axapp->delTimerWin(this);
}

//! 指定タイマーが存在するか

BOOL AXWindow::isTimerExist(UINT uTimerID)
{
    return axapp->isTimerExist(this, uTimerID);
}

//! タイマーが存在するか（タイマーIDは問わない）

BOOL AXWindow::isTimerExistWin()
{
    return axapp->isTimerExistWin(this);
}


//==================================
//
//==================================


//! 子ウィンドウ内から指定アイテムIDのウィンドウ取得

AXWindow *AXWindow::getWidget(UINT uID)
{
    return searchChild(SEARCHTYPE_ITEMID, uID);
}

//! 全子ウィンドウ内の指定アイテムIDのウィンドウを有効/無効

void AXWindow::enableWidget(UINT uID,BOOL bEnable)
{
    AXWindow *p = getWidget(uID);

    if(p)
    {
        if(bEnable) p->enable();
        else p->disable();
    }
}

//! onNotify 通知を送る（キューに追加）

void AXWindow::sendNotify(AXWindow *pwinFrom,UINT uNotify,ULONG lParam)
{
    axapp->addNotify(this, pwinFrom, uNotify, lParam);
}

//! onCommand 通知を送る（キューに追加）

void AXWindow::sendCommand(UINT uID,ULONG lParam,int from)
{
    axapp->addCommand(this, uID, lParam, from);
}


//==================================
//ハンドラ
//==================================


//! 再構成

void AXWindow::reconfig() { }

//! 描画時

BOOL AXWindow::onPaint(AXHD_PAINT *phd) { return FALSE; }

//! オーナードロー描画時
//! @param pdraw 描画先

BOOL AXWindow::onOwnerDraw(AXDrawable *pdraw,AXHD_OWNERDRAW *phd) { return FALSE; }

//! 閉じる時
/*!
    デフォルトでループ終了
*/

BOOL AXWindow::onClose()
{
    axapp->exit();
    return TRUE;
}

//! 位置変更時

BOOL AXWindow::onMove() { return FALSE; }

//! サイズ変更時

BOOL AXWindow::onSize() { return FALSE; }

//! 各ウィジェットからの通知
/*!
    @param pwin 通知元ウィジェット
    @param uNotify 通知番号
    @param lParam パラメータ
*/

BOOL AXWindow::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam) { return FALSE; }

//! メニューなどから項目が決定された時
/*!
    @param uID 項目のID
    @param lParam 項目のパラメータ値
    @param from どこから送られたか。 AXWindow::COMMANDFROM
*/

BOOL AXWindow::onCommand(UINT uID,ULONG lParam,int from) { return FALSE; }

//! タイマー

BOOL AXWindow::onTimer(UINT uTimerID,ULONG lParam) { return FALSE; }

//! メニューがポップアップ表示された時
/*!
    @param pMenuBar メニューバーのポインタ（NULLで独立ポップアップ）
    @param pMenu    ポップアップのメニューデータ
    @param uID      ポップアップ親項目のID
*/

BOOL AXWindow::onMenuPopup(AXMenuBar *pMenuBar,AXMenu *pMenu,UINT uID) { return FALSE; }

//! D&Dでドロップされた時

BOOL AXWindow::onDND_Drop(AXDND *pDND) { return FALSE; }

//----------------

//! ウィンドウがマップされた時

BOOL AXWindow::onMap() { return FALSE; }

//! フォーカスIN

BOOL AXWindow::onFocusIn(int detail)
{
    //ウィジェット用のデフォルト処理（再描画）
    redraw();
    return FALSE;
}

//! フォーカスOUT

BOOL AXWindow::onFocusOut(int detail)
{
    redraw();
    return FALSE;
}

//! ウィンドウ構成変更時

BOOL AXWindow::onConfigure(AXHD_CONFIGURE *phd) { return FALSE; }

//! マウスカーソルがウィンドウ内に入った

BOOL AXWindow::onEnter(AXHD_ENTERLEAVE *phd) { return FALSE; }

//! マウスカーソルがウィンドウ内から出た
/*!
    ※グラブ中は来ない
*/

BOOL AXWindow::onLeave(AXHD_ENTERLEAVE *phd) { return FALSE; }

//! グラブが解除された時

BOOL AXWindow::onUngrab(AXHD_ENTERLEAVE *phd) { return FALSE; }

//! キー押し時
/*!
    ※グラブ中にも来るので注意
*/

BOOL AXWindow::onKeyDown(AXHD_KEY *phd) { return FALSE; }

//! キー離し時
/*!
    ※グラブ中にも来るので注意
*/

BOOL AXWindow::onKeyUp(AXHD_KEY *phd) { return FALSE; }

//! ボタン押し時

BOOL AXWindow::onButtonDown(AXHD_MOUSE *phd) { return FALSE; }

//! ボタン離し時

BOOL AXWindow::onButtonUp(AXHD_MOUSE *phd) { return FALSE; }

//! マウスカーソル移動時

BOOL AXWindow::onMouseMove(AXHD_MOUSE *phd) { return FALSE; }

//! ダブルクリック時

BOOL AXWindow::onDblClk(AXHD_MOUSE *phd) { return FALSE; }

//! マウスホイール操作時（カーソルがウィンドウ上にあって操作された時）

BOOL AXWindow::onMouseWheel(AXHD_MOUSE *phd,BOOL bUp) { return FALSE; }

//! ダイアログ時など、処理がスキップ対象のマウス操作時

BOOL AXWindow::onMouseInSkip(AXHD_MOUSE *phd){ return FALSE; }


//---------------------------

/*!
    @enum AXWindow::MOUSEBUTTON
    @brief マウスボタンの種類。AXHD_BUTTON::button

    @enum AXWindow::BUTTONSTATE
    @brief マウスボタンの装飾フラグ。AXHD_BUTTON::state など。

    ※ウィンドウマネージャによって何らかの機能が付けられているキーは、キー押しイベント自体が来ない。

    @enum AXWindow::EVENTTYPE
    @brief イベントの種類
*/

/*!
    @enum AXWindow::WINDOWSTYLE
    @brief ウィンドウスタイル

    @var AXWindow::WS_HIDE
    @brief （共通）非表示状態で作成
    @var AXWindow::WS_DISABLE
    @brief （共通）無効状態で作成
    @var AXWindow::WS_DROP
    @brief （共通）ドラッグ＆ドロップによる、ドロップを受け取る
    @var AXWindow::WS_BK_FACE
    @brief （共通）ウィンドウの背景色として FACE 色をセット
    @var AXWindow::WS_EXTRA
    @brief ここから、各ウィンドウごとのスタイル

    @var AXWindow::WS_DISABLE_WM
    @brief （トップレベル）ウィンドウマネージャによる制御を無効
    @var AXWindow::WS_TITLE
    @brief （トップレベル）タイトルバー
    @var AXWindow::WS_MINIMIZE
    @brief （トップレベル）最小化ボタン
    @var AXWindow::WS_MAXIMIZE
    @brief （トップレベル）最大化ボタン
    @var AXWindow::WS_CLOSE
    @brief （トップレベル）閉じるボタン
    @var AXWindow::WS_BORDER
    @brief （トップレベル）ウィンドウ枠
    @var AXWindow::WS_MENUBTT
    @brief （トップレベル）メニューボタンアイコン
    @var AXWindow::WS_DISABLE_IM
    @brief （トップレベル）入力メソッドを使わない。@nこのトップレベルウィンドウ上では日本語入力が行えなくなる。
    @var AXWindow::WS_TABMOVE
    @brief （トップレベル）タブキーでウィジェット間を移動できるようにする
    @var AXWindow::WS_TRANSIENT_FOR
    @brief （トップレベル）オーナーウィンドウのサブウィンドウとして指定（ダイアログなど）@n
        オーナーウィンドウより常に全面に表示され、また、タスクバーには表示されない。
    @var AXWindow::WS_DISABLE_RESIZE
    @brief （トップレベル）ユーザーによるウィンドウのリサイズを出来ないようにする
    @var AXWindow::WS_NOT_TAKEFOCUS
    @brief （トップレベル）ウィンドウマネージャからフォーカスを受け取らない
*/

/*!
    @var AXWindow::FLAG_EXIST
    @brief ウィンドウが存在している（removeFocusMe() 時に参照される）
    @var AXWindow::FLAG_VISIBLE
    @brief 表示されている
    @var AXWindow::FLAG_ENABLED
    @brief 有効状態
    @var AXWindow::FLAG_TAKE_FOCUS
    @brief フォーカスを受け取る
    @var AXWindow::FLAG_DEFAULT_FOCUS
    @brief デフォルトフォーカス
    @var AXWindow::FLAG_FOCUSED
    @brief 現在フォーカスがある
    @var AXWindow::FLAG_CALCULATED
    @brief レイアウト標準サイズ計算が行われたか ※レイアウトがセットされている場合のみ
    @var AXWindow::FLAG_LAYOUTED
    @brief レイアウトが実行されたか（各レイアウトアイテム毎）
    @var AXWindow::FLAG_CURSOR_NEW
    @brief m_pCursor が new で作成されているか
    @var AXWindow::FLAG_REDRAW
    @brief redraw〜() 実行で ON になり、onPaint() 後 OFFになる。Pixmap等に描画して転送する場合の作業用フラグに。
    @var AXWindow::FLAG_CHILD_NOTIFY_PARENT
    @brief 親ウィンドウにこのフラグをセットする。子ウィンドウ作成時に通知対象（m_pNotify）を親ウィンドウの通知と同じにする。
    @var AXWindow::FLAG_WHEELEVENT_NORMAL
    @brief ホイールのイベントを、OnMouseWheel() ではなく通常通り OnButtonDown/Up で送る
    @bar AXWindow::FLAG_FOCUS_DISABLE_KEY
    @brief このウィンドウにフォーカスがある時、フォーカス側でキー処理を行わない
*/

/*!
    @fn void AXWindow::setChildNotifyParent()
    @brief このウィンドウを親とする子ウィンドウ作成時、子ウィンドウの通知対象（m_pNotify）を親ウィンドウの通知対象と同じにする。

    AXGroupBox などでは子ウィンドウの通知が親ウィンドウ（AXGroupBoxなど）に行くので、
    AXGroupBox でこれをセットしておけば、子ウィンドウごとに通知を設定しなくてよい。
*/
/*!
    @fn AXWindow *getNotify()
    @brief 通知先ウィンドウを取得（仮想関数）

    AXFileListView など、自身が送る通知を自身で受け取る場合などには getNotify() で this を返すようにし、自身で処理。@n
    通常の通知先は m_pNotify を使う。@n
    （※通常、各ウィジェットは getNotify() で通知を送っている）
*/
