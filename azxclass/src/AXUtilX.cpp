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

#define _AX_X11_ATOM
#define _AX_X11_UTIL
#include "AXX11.h"

#include <string.h>

#include "AXDef.h"
#include "AXApp.h"
#include "AXAppAtom.h"
#include "AXMem.h"
#include "AXString.h"
#include "AXUtilX.h"
#include "AXUtilTime.h"


/*!
    @defgroup axutilx AXUtilX
    @brief X関連ユーティリティ

    @{

    @ingroup util
*/



//! アトム名からATOM取得

ULONG AXGetAtom(LPCSTR szName)
{
    return ::XInternAtom(axdisp(), szName, FALSE);
}

//! アトム識別子からアトム名取得

void AXGetAtomName(AXString *pstr,ULONG atom)
{
    LPSTR pc = ::XGetAtomName(axdisp(), atom);

    if(!pc)
        pstr->empty();
    else
    {
        pstr->setLocal(pc);

        ::XFree(pc);
    }
}


//==============================
//プロパティ - セット
//==============================


//! プロパティに Atom 値セット

void AXSetPropertyAtom(ULONG idWin,ULONG atomProp,ULONG *pAtom,int num)
{
    ::XChangeProperty(axdisp(), idWin, atomProp, XA_ATOM, 32,
        PropModeReplace, (unsigned char *)pAtom, num);
}

//! プロパティに CARDINAL 値(long)セット

void AXSetPropertyCARDINAL(ULONG idWin,ULONG atomProp,long *pVal,int num)
{
    ::XChangeProperty(axdisp(), idWin, atomProp, XA_CARDINAL, 32,
        PropModeReplace, (unsigned char *)pVal, num);
}

//! プロパティに "COMPOUND_TEXT" 文字列セット

void AXSetPropertyCompoundText(ULONG idWin,ULONG atomProp,const AXString &str)
{
    XTextProperty tp;
    LPWSTR pw;

    pw = new WCHAR[str.getLen() + 1];

    str.toWide(pw);

    if(::XwcTextListToTextProperty(axdisp(), &pw, 1, XCompoundTextStyle, &tp) == Success)
        ::XSetTextProperty(axdisp(), idWin, &tp, atomProp);

    delete []pw;

    if(tp.value) ::XFree(tp.value);
}

//! プロパティにデータセット(format=8)

void AXSetProperty8(ULONG idWin,ULONG atomProp,ULONG atomType,const void *pBuf,UINT uSize,BOOL bAdd)
{
    int mode;
    LPBYTE p = (LPBYTE)pBuf;
    UINT maxsize,size;

    maxsize = ::XMaxRequestSize(axdisp()) * 4;
    mode    = (bAdd)? PropModeAppend: PropModeReplace;

    while(uSize)
    {
        size = (uSize > maxsize)? maxsize: uSize;

        ::XChangeProperty(axdisp(), idWin, atomProp, atomType, 8, mode, p, size);

        mode    = PropModeAppend;
        p       += size;
        uSize   -= size;
    }
}


//==============================
//プロパティ - 読み込み
//==============================


//! プロパティからformat=8データ取得

BOOL AXGetProperty8(ULONG idWin,ULONG atomProp,AXMem *pmem)
{
    Atom type;
    int ret,format;
    ULONG nitems,after;
    long offset;
    LPBYTE pdat,pDst;

    if(atomProp == None) return FALSE;

    //データサイズ取得

    ret = ::XGetWindowProperty(axdisp(), idWin, atomProp, 0, 0,
                FALSE, AnyPropertyType, &type, &format, &nitems, &after, &pdat);

    if(pdat) ::XFree(pdat);

    if(ret != Success || type == None || format != 8) return FALSE;

    //読み込み

    if(!pmem->alloc(after)) return FALSE;

    pDst = *pmem;

    for(offset = 0; after > 0; offset += nitems)
    {
        ret = ::XGetWindowProperty(axdisp(), idWin, atomProp, offset / 4, after / 4 + 1,
                    FALSE, AnyPropertyType, &type, &format, &nitems, &after, &pdat);

        if(ret != Success) { pmem->free(); return FALSE; }

        ::memcpy(pDst + offset, pdat, nitems);
        ::XFree(pdat);
    }

    return TRUE;
}

//! プロパティから format=32 の配列データを指定数読み込み

BOOL AXGetProperty32Array(ULONG idWin,ULONG atomProp,ULONG atomType,LPVOID pBuf,int cnt)
{
    Atom type;
    int format;
    ULONG num,after;
    LPBYTE pdat;
    BOOL ret = FALSE;

    if(::XGetWindowProperty(axdisp(), idWin, atomProp, 0, 1024, FALSE, atomType,
                &type, &format, &num, &after, &pdat) == Success)
    {
        if(type == atomType && format == 32 && num >= (ULONG)cnt)
        {
            ::memcpy(pBuf, pdat, sizeof(long) * cnt);
            ret = TRUE;
        }

        ::XFree(pdat);
    }

    return ret;
}

//! プロパティから format=32 データ読み込み

BOOL AXGetProperty32(ULONG idWin,ULONG atomProp,ULONG atomType,AXMem *pmem,LPINT pCnt)
{
    Atom type;
    int ret,format;
    ULONG nitems,after;
    LPBYTE pdat,pDst;
    long offset;

    if(atomProp == None) return FALSE;

    //データサイズ取得 (after)

    ret = ::XGetWindowProperty(axdisp(), idWin, atomProp, 0, 0,
                FALSE, atomType, &type, &format, &nitems, &after, &pdat);

    if(pdat) ::XFree(pdat);

    if(ret != Success || format != 32 || type != atomType) return FALSE;

    //確保
    /* [64bit OS 時の注意]
       after は long型 x データ数 のサイズではなく、4Byte x データ数。
       しかし、実際に取得されるデータは long型 x データ数 なので注意。
       内部的には 4Byte データだが、取得される際には 4Byte -> 8Byte 変換される。 */

    if(!pmem->alloc(after / 4 * sizeof(long))) return FALSE;

    *pCnt = after / 4;

    //読み込み
    /* プロパティのデータ位置とサイズは 4Byte データとして扱う。 */

    pDst = *pmem;

    for(offset = 0; after > 0; offset += nitems * 4)
    {
        ret = ::XGetWindowProperty(axdisp(), idWin, atomProp, offset / 4, after / 4,
                    FALSE, atomType, &type, &format, &nitems, &after, &pdat);

        if(ret != Success) return FALSE;

        ::memcpy(pDst, pdat, nitems * sizeof(long));
        pDst += nitems * sizeof(long);

        ::XFree(pdat);
    }

    return TRUE;
}


//=================================
//セレクション関連
//=================================


//! セレクションから利用可能なデータタイプ取得
/*!
    @param pAtom    対応可能なアトムのリスト（先頭にあるものほど優先順位が高い）
    @param cnt      pAtomの数
    @return 実際に利用できるタイプ（0で失敗）
*/

ULONG AXGetSelectionTargetType(ULONG idWin,ULONG atomSelection,ULONG *pAtom,int cnt)
{
    XEvent ev;
    Atom *pTargetAtom;
    int i,j,targetcnt;
    AXMem mem;

    //変換要求

    ::XConvertSelection(axdisp(), atomSelection, axatom(AXAppAtom::TARGETS),
                    axatom(AXAppAtom::AXSELECTION), idWin, CurrentTime);

    //SelectionNotifyイベント取得

    if(!AXRecvEventTimeout(SelectionNotify, 1000, &ev)) return 0;

    //アトムリスト取得

    if(!AXGetProperty32(ev.xselection.requestor, ev.xselection.property, XA_ATOM, &mem, &targetcnt))
        return 0;

    //

    pTargetAtom = (Atom *)mem.getBuf();

/*
    for(i = 0; i < targetcnt; i++)
    {
        LPSTR p = ::XGetAtomName(axdisp(), pTargetAtom[i]);
        fprintf(stderr, "targets: %s\n", p);
        XFree(p);
    }
*/

    for(i = 0; i < cnt; i++)
    {
        for(j = 0; j < targetcnt; j++)
        {
            if(pAtom[i] == pTargetAtom[j])
                return pAtom[i];
        }
    }

    return 0;
}

//! セレクションからデータ取得(format=8)

BOOL AXGetSelectionData(ULONG idWin,ULONG atomSelection,ULONG atomType,AXMem *pmem)
{
    XEvent ev;

    ::XConvertSelection(axdisp(), atomSelection, atomType, axatom(AXAppAtom::AXSELECTION), idWin, CurrentTime);

    if(!AXRecvEventTimeout(SelectionNotify, 1000, &ev)) return FALSE;
    if(!AXGetProperty8(ev.xselection.requestor, ev.xselection.property, pmem)) return FALSE;

    ::XDeleteProperty(axdisp(), idWin, axatom(AXAppAtom::AXSELECTION));

    return TRUE;
}

//! セレクションから"COMPOUND_TEXT"文字列を取得しAXStringにセット

BOOL AXGetSelectionCompoundText(ULONG idWin,ULONG atomSelection,AXString *pstr)
{
    XEvent ev;
    XTextProperty tp;
    WCHAR **ppwc;
    int cnt;
    BOOL ret = FALSE;

    //プロパティにデータ受信

    ::XConvertSelection(axdisp(), atomSelection, axatom(AXAppAtom::COMPOUND_TEXT),
                axatom(AXAppAtom::AXSELECTION), idWin, CurrentTime);

    if(!AXRecvEventTimeout(SelectionNotify, 1000, &ev)) return FALSE;

    //文字列リストデータ取得

    ::XGetTextProperty(axdisp(), idWin, &tp, axatom(AXAppAtom::AXSELECTION));

    //取得してセット

    if(tp.format != 0)
    {
        if(::XwcTextPropertyToTextList(axdisp(), &tp, &ppwc, &cnt) == Success)
        {
            *pstr = *ppwc;

            ::XwcFreeStringList(ppwc);

            ret = TRUE;
        }

        ::XFree(tp.value);
    }

    ::XDeleteProperty(axdisp(), idWin, axatom(AXAppAtom::AXSELECTION));

    return ret;
}


//==============================
//イベント関連
//==============================


//! ClientMessage 送信用に XEvent 構造体初期化

void AXSetEvClientMessage(LPVOID pEvent,ULONG idWin,ULONG atomMes)
{
    XEvent *pev = (XEvent *)pEvent;

    ::memset(pev, 0, sizeof(XEvent));

    pev->xclient.type           = ClientMessage;
    pev->xclient.display        = axdisp();
    pev->xclient.message_type   = atomMes;
    pev->xclient.format         = 32;
    pev->xclient.window         = idWin;
}

//! 指定イベントを受け取る（タイムアウト付き）
/*!
    @param evtype    受け取るイベント
    @param timeoutms タイムアウト時間（ミリセカンド）
    @param pEvent (XEvent *)
*/

BOOL AXRecvEventTimeout(int evtype,int timeoutms,LPVOID pEvent)
{
    fd_set fd;
    int xfd;
    struct timeval tv;
    TIMENANO now,end,diff;

    xfd = axapp->getConnection();

    AXGetTime(&end);
    AXAddTime(&end, (ULONGLONG)timeoutms * 1000 * 1000);

    while(1)
    {
        //キュー内にあるか

        if(::XCheckTypedEvent(axdisp(), evtype, (XEvent *)pEvent)) return TRUE;

        //イベントを受け取るまで待つ

        AXGetTime(&now);
        if(!AXDiffTime(&diff, &end, &now)) break;

        FD_ZERO(&fd);
        FD_SET(xfd, &fd);

        tv.tv_sec   = diff.sec;
        tv.tv_usec  = diff.nsec / 1000;

        if(::select(xfd + 1, &fd, NULL, NULL, &tv) != 1) break;
    }

    return FALSE;
}


//==============================
//ウィンドウマネージャ関連
//==============================


//! XSendEvent で ClientMessage を送る（ウィンドウマネージャー用）

void AXSendWMEvent(ULONG idWin,ULONG atom,long data1,long data2,long data3,long data4)
{
    XEvent ev;

    AXSetEvClientMessage(&ev, idWin, atom);

    ev.xclient.data.l[0] = data1;
    ev.xclient.data.l[1] = data2;
    ev.xclient.data.l[2] = data3;
    ev.xclient.data.l[3] = data4;

    ::XSendEvent(axdisp(), axapp->getRootID(), FALSE,
        SubstructureRedirectMask | SubstructureNotifyMask, &ev);
}

//! _NET_WM_STATE 送る
/*!
    @param action     [0]はずす [1]追加 [2]トグル
    @param atomSecond 0でなし
*/

void AXSendNetWMState(ULONG idWin,int action,ULONG atomFirst,ULONG atomSecond)
{
    AXSendWMEvent(idWin, axatom(AXAppAtom::_NET_WM_STATE),
                  action, atomFirst, atomSecond);
}

//! _NET_WM_STATE に指定した ATOM 値が存在するか

BOOL AXIsExistNetWMState(ULONG idWin,ULONG atomFirst,ULONG atomSecond)
{
    Atom type;
    int format;
    ULONG nitems,after,i;
    Atom *pDat;
    BOOL ret = FALSE;

    if(::XGetWindowProperty(axdisp(), idWin,
        axatom(AXAppAtom::_NET_WM_STATE), 0, 2, FALSE, AnyPropertyType,
        &type, &format, &nitems, &after, (BYTE **)&pDat) == Success)
    {
        if(type == XA_ATOM && format == 32)
        {
            for(i = 0; i < nitems; i++)
            {
                if(pDat[i] == atomFirst) ret = TRUE;
                if(atomSecond && pDat[i] == atomSecond) ret = TRUE;
            }
        }

        ::XFree((char *)pDat);
    }

    return ret;
}


//==============================
//ウィンドウ関連
//==============================


//! トップレベルウィンドウの枠のウィンドウ取得（なければトップレベル）
//! @return ウィンドウXID

ULONG AXGetTopFrameWindow(ULONG idWin)
{
    Window root,parent,*child;
    UINT chcnt;

    while(1)
    {
        ::XQueryTree(axdisp(), idWin, &root, &parent, &child, &chcnt);
        if(child) ::XFree(child);

        if(parent == root) break;

        idWin = parent;
    }

    return idWin;
}

//! 現在のアクティブウィンドウ取得

ULONG AXGetActiveWindow()
{
    Window win = 0;
    int revert;

    if(axapp->isSupport_NetActiveWindow())
    {
        AXGetProperty32Array(axapp->getRootID(), axatom(AXAppAtom::_NET_ACTIVE_WINDOW),
                    XA_WINDOW, &win, 1);
    }
    else
        ::XGetInputFocus(axdisp(), &win, &revert);

    return (ULONG)win;
}

//! 指定ウィンドウ下の指定位置上にある子ウィンドウ取得
/*!
    @param idWin 0 でルートウィンドウ
*/

ULONG AXGetChildWindowPt(ULONG idWin,int rootx,int rooty)
{
    Window child;
    int x,y;

    if(idWin == 0) idWin = axapp->getRootID();

    while(1)
    {
        if(!::XTranslateCoordinates(axdisp(), axapp->getRootID(), idWin, rootx, rooty, &x, &y, &child))
            return 0;

        if(child == None) break;

        idWin = child;
    }

    return idWin;
}

//! キー識別子からキー名取得

BOOL AXGetKeysymString(AXString *pstr,UINT keysym)
{
    LPSTR pc;

    pstr->empty();

    if(keysym == 0) return FALSE;

    pc = ::XKeysymToString(keysym);
    if(!pc) return FALSE;

    pstr->setLocal(pc);

    return TRUE;
}

//! グラブ解放

void AXUngrabPointer()
{
    ::XUngrabPointer(axdisp(), axapp->getLastTime());
}

//@}
