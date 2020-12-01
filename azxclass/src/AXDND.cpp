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
#include "AXX11.h"

#include <string.h>

#include "AXDND.h"

#include "AXApp.h"
#include "AXAppAtom.h"
#include "AXWindow.h"
#include "AXMem.h"
#include "AXString.h"
#include "AXByteString.h"
#include "AXUtilX.h"
#include "AXUtilStr.h"


/*!
    @class AXDND
    @brief ドラッグ＆ドロップ処理（AXAppで管理される）

    - D&D を有効にするには、トップレベルウィンドウで enableDND() を実行すること。
    - ドロップの対象ウィンドウには WS_DROP スタイルを付加すること。@n
      トップレベルに付加すれば、トップレベル下にカーソルが来た時にドロップできる。
    - ドロップされた時、ウィンドウのハンドラ onDND_Drop() が実行される。
    - onDND_Drop() はトップレベル・子ウィンドウどちら側でも処理可能。@n
      トップレベル側で処理する場合は、TRUE を返す。@n
      トップレベル側で FALSE が返された場合（デフォルト）は、ドロップ対象の子ウィンドウで onDND_Drop() が来る。

    @ingroup core
*/

/*
    [MIMEタイプ例]
    text/plain
    text/plain;charset=utf-8
    text/uri-list
*/


AXDND::AXDND()
{
    m_patomType = NULL;
    m_nTypeCnt  = 0;
    m_idSrcWin  = 0;
    m_idDstWin  = 0;
    m_pwinDrop  = NULL;
}

AXDND::~AXDND()
{
    _end();
}

//! 終了

void AXDND::_end()
{
    AXFree((void **)&m_patomType);

    m_nTypeCnt  = 0;
    m_idSrcWin  = 0;
    m_idDstWin  = 0;
    m_pwinDrop  = NULL;
}


//================================
//
//================================


//! onDND_Drop() 内で、ドロップ処理を終了させる
/*!
    呼ばなくても自動的に終了処理は行われるが、ドロップ後の処理を行う前に終了させておきたい場合に使う。
*/

void AXDND::endDrop()
{
    XEvent ev;

    if(m_idDstWin)
    {
        //終了を送信

        AXSetEvClientMessage(&ev, m_idSrcWin, axatom(AXAppAtom::XdndFinished));

        ev.xclient.data.l[0] = m_idDstWin;
        ev.xclient.data.l[1] = 1;
        ev.xclient.data.l[2] = axatom(AXAppAtom::XdndActionCopy);

        ::XSendEvent(axdisp(), m_idSrcWin, TRUE, NoEventMask, &ev);

        //終了

        _end();
    }
}

//! タイプリスト内に指定したタイプがあるか

BOOL AXDND::isExistType(ULONG atomType) const
{
    int i;
    ULONG *p;

    if(m_patomType)
    {
        for(i = 0, p = m_patomType; i < m_nTypeCnt; i++)
        {
            if(*(p++) == atomType) return TRUE;
        }
    }

    return FALSE;
}

//! ローカルファイル複数取得（\\tで区切り。\\t\\tで終了）

BOOL AXDND::getFiles(AXString *pstr)
{
    AXMem mem;
    AXByteString str;

    if(!m_pwinDrop) return FALSE;

    if(!isExistType(axatom(AXAppAtom::text_uri_list))) return FALSE;

    if(!AXGetSelectionData(m_pwinDrop->getid(), axatom(AXAppAtom::XdndSelection),
                            axatom(AXAppAtom::text_uri_list), &mem))
        return FALSE;

    str.set((LPSTR)mem, mem.getSize());

    //タブで区切ったリストに

    AXURIListToString(pstr, str, TRUE);

    return pstr->isNoEmpty();
}

//! 最初のローカルファイル名取得

BOOL AXDND::getFirstFile(AXString *pstr)
{
    AXMem mem;
    AXString str;
    AXByteString strSrc;

    if(!m_pwinDrop) return FALSE;

    if(!isExistType(axatom(AXAppAtom::text_uri_list))) return FALSE;

    if(!AXGetSelectionData(m_pwinDrop->getid(), axatom(AXAppAtom::XdndSelection),
                            axatom(AXAppAtom::text_uri_list), &mem))
        return FALSE;

    strSrc.set((LPSTR)mem, mem.getSize());

    AXURIListToString(&str, strSrc, TRUE);
    str.replace('\t', 0);

    *pstr = (LPUSTR)str;

    return pstr->isNoEmpty();
}


//================================
//各メッセージ処理
//================================


//! ClientMessage イベント処理

BOOL AXDND::onClientMessage(LPVOID pEvent)
{
    XClientMessageEvent *pev = (XClientMessageEvent *)pEvent;

    if(pev->message_type == axatom(AXAppAtom::XdndEnter))
        onEnter(pEvent);
    else if(pev->message_type == axatom(AXAppAtom::XdndPosition))
        onPosition(pEvent);
    else if(pev->message_type == axatom(AXAppAtom::XdndLeave))
    {
        //カーソルがトップレベルウィンドウから離れた時

        if((ULONG)pev->data.l[0] == m_idSrcWin)
            _end();
    }
    else if(pev->message_type == axatom(AXAppAtom::XdndDrop))
        onDrop(pEvent);
    else
        return FALSE;

    return TRUE;
}

//! XdndEnter 時 (D&Dのカーソルがトップレベルウィンドウ内に入った)

void AXDND::onEnter(LPVOID pEvent)
{
    XClientMessageEvent *pev = (XClientMessageEvent *)pEvent;

    _end();

    //バージョン

    if(((pev->data.l[1] >> 24) & 255) > 4) return;

    //ソースウィンドウ

    m_idSrcWin = (ULONG)pev->data.l[0];

    //データタイプ

    if(pev->data.l[1] & 1)
    {
        //4つ以上の場合 - プロパティから読み込み

        AXMem mem;
        int cnt;

        if(AXGetProperty32(m_idSrcWin, axatom(AXAppAtom::XdndTypeList), XA_ATOM, &mem, &cnt))
        {
            m_patomType = (ULONG *)AXMalloc(sizeof(ULONG) * cnt);
            m_nTypeCnt  = cnt;

            ::memcpy(m_patomType, mem, cnt * sizeof(ULONG));
        }
    }
    else
    {
        //3つ以下

        int i;

        m_patomType = (ULONG *)AXMalloc(sizeof(ULONG) * 3);
        m_nTypeCnt  = 0;

        for(i = 2; i <= 4; i++)
        {
            if(pev->data.l[i])
                m_patomType[m_nTypeCnt++] = pev->data.l[i];
        }
    }
}

//! XdndPosition 時 (カーソル位置を元に、対象のターゲットに対してドロップが有効かを返す)

void AXDND::onPosition(LPVOID pEvent)
{
    XClientMessageEvent *pev = (XClientMessageEvent *)pEvent;
    int x,y,bDrop = FALSE;
    ULONG id;
    AXWindow *pwin,*ptop;
    XEvent ev;

    if((ULONG)pev->data.l[0] != m_idSrcWin) return;

    //対象のトップレベル
    //pev->window = トップレベルウィンドウ

    ptop = axapp->getWindowFromID(pev->window);

    //------ ドロップ先＆ドロップ可能か

    if(axapp->isModal() && axapp->getModalWin() != ptop)
        //モーダル中で、ドロップ先がモーダルウィンドウ以外の場合は無効
        bDrop = FALSE;
    else if(ptop)
    {
        if(ptop->getStyle() & AXWindow::WS_DROP)
        {
            //トップレベルがドロップ可の場合 -> ドロップ可

            m_pwinDrop  = ptop;
            bDrop       = TRUE;
        }
        else
        {
            //カーソル位置下の子ウィンドウで判定

            x = ((ULONG)pev->data.l[2] >> 16) & 0xffff;
            y = pev->data.l[2] & 0xffff;

            id = AXGetChildWindowPt(pev->window, x, y);

            pwin = axapp->getWindowFromID(id);
            if(pwin)
            {
                m_pwinDrop  = pwin;
                bDrop       = (pwin->getStyle() & AXWindow::WS_DROP);
            }
        }
    }

	//------- メッセージ送信

    AXSetEvClientMessage(&ev, m_idSrcWin, axatom(AXAppAtom::XdndStatus));

    ev.xclient.data.l[0] = pev->window;
    ev.xclient.data.l[1] = (bDrop)? 1: 0;   //[0bit]Dropを受け付ける [1bit]矩形内移動中も XdndPosition の送信が必要
    ev.xclient.data.l[2] = 0;
    ev.xclient.data.l[3] = 0;
    ev.xclient.data.l[4] = axatom(AXAppAtom::XdndActionCopy);

    ::XSendEvent(axdisp(), m_idSrcWin, TRUE, NoEventMask, &ev);
}

//! XdndDrop 時 （ドロップされた時）

void AXDND::onDrop(LPVOID pEvent)
{
    XClientMessageEvent *pev = (XClientMessageEvent *)pEvent;
    AXWindow *ptop;

    if((ULONG)pev->data.l[0] != m_idSrcWin) return;

    m_idDstWin = pev->window;

    //ハンドラ
    //※先にトップレベルで実行。FALSEが返ったら子ウィンドウで実行

    ptop = axapp->getWindowFromID(pev->window);

    if(ptop && !ptop->onDND_Drop(this))
    {
        if(ptop != m_pwinDrop && m_pwinDrop && (m_pwinDrop->getStyle() & AXWindow::WS_DROP))
            m_pwinDrop->onDND_Drop(this);
    }

    //終了（onDND_Drop() 内で呼んでも可）

    endDrop();
}
