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

#include "AXClipboard.h"

#include "AXApp.h"
#include "AXAppAtom.h"
#include "AXUtilX.h"
#include "AXMem.h"
#include "AXString.h"
#include "AXByteString.h"


#define TARGET_DEFCNT   2

//-----------------------------

/*!
    @class AXClipboard
    @brief クリップボード管理クラス（AXAppで作成される）

    - AXApp::getClipboard() または axclipb マクロでクラスのポインタを取得できる。
    - クリップボードにセットできるのは、常に一つのデータのみ。
    - クリップボードのデータは、基本的にアプリケーション側が保持している。
    - セットするデータは一種類のみだが、データが要求された時に複数の形式に変換して対応することができる。
    - AXString または UNICODE(16bit) のテキストをセットする場合は、setText() を使えば自動で各処理が行われる。
    - 独自のデータをセットしたい場合は setDat() でセットする。@n
      他のクライアントからデータが要求された時はコールバック関数が呼ばれるので、
      idSendWindow のウィンドウの atomProp のプロパティに atomType のデータタイプで、
      XChangeProperty() を使ってプロパティにデータをセットする。@n
      AXClipboard::sendDat() を使って、データを送っても良い。

    @ingroup core
*/


AXClipboard::AXClipboard()
{
    m_uDatType  = TYPE_NONE;
    m_pDatBuf   = NULL;
    m_pAtomList = NULL;
}

AXClipboard::~AXClipboard()
{
    clear();
}

//! 現在セットされているデータをクリア

void AXClipboard::clear()
{
    AXFree((void **)&m_pDatBuf);
    AXFree((void **)&m_pAtomList);

    m_uDatType = TYPE_NONE;
}

//! 現在のデータが指定タイプのデータか

BOOL AXClipboard::isExist(UINT type) const
{
    return (m_pDatBuf && m_uDatType == type);
}


//==================================
//セット
//==================================


//! 文字列セット

BOOL AXClipboard::setText(ULONG idWindow,const AXString &str)
{
    return setDat(idWindow, TYPE_TEXT, (LPCUSTR)str, str.getLen() * sizeof(UNICHAR), NULL, 0, NULL);
}

//! UNICODE(16bit)文字列セット
/*!
    @param len NULL文字を含まない長さ
*/

BOOL AXClipboard::setText(ULONG idWindow,LPCUSTR pstr,int len)
{
    return setDat(idWindow, TYPE_TEXT, pstr, len * sizeof(UNICHAR), NULL, 0, NULL);
}

//! クリップボードにデータセット
/*!
    指定したデータをこのクラスの内部にコピーし、保存する。@n
    他クライアントからのデータ要求に対応できるようにクリップボードの所有権を持たせる。

    @param idWindow     このデータを処理するウィンドウ
    @param type         クリップボードのデータタイプ
    @param pBuf         セットするデータ
    @param size         pBuf のデータサイズ
    @param pAtomList    このデータに対応するアトムタイプのリスト（NULLでデフォルトのタイプ）
    @param atomcnt      pAtomList のデータ数
    @param callback     データを処理する時に呼ばれる関数（NULLでデフォルトの処理）
*/

BOOL AXClipboard::setDat(ULONG idWindow,UINT type,const void *pBuf,UINT size,
        const ULONG *pAtomList,int atomcnt,BOOL (*callback)(AXClipboard *,CALLBACK *))
{
    clear();

    //

    m_uDatType  = type;
    m_uDatSize  = size;
    m_callback  = callback;

    //データコピー

    m_pDatBuf = (LPBYTE)AXMalloc(size);
    if(!m_pDatBuf) return FALSE;

    ::memcpy(m_pDatBuf, pBuf, size);

    //アトムリストセット

    if(!_setAtomList(pAtomList, atomcnt))
    {
        clear();
        return FALSE;
    }

    //クリップボード所有権セット

    ::XSetSelectionOwner(axdisp(), axatom(AXAppAtom::CLIPBOARD), idWindow, axapp->getLastTime());

    return TRUE;
}

//! アトムリストセット

BOOL AXClipboard::_setAtomList(const ULONG *pAtomList,int atomcnt)
{
    ULONG *p;

    //数（pAtomList==NULL で各タイプのデフォルト）

    if(!pAtomList)
    {
        if(m_uDatType == TYPE_TEXT) atomcnt = 2;
    }

    //確保

    m_pAtomList = (ULONG *)AXMalloc(sizeof(ULONG) * (atomcnt + TARGET_DEFCNT));
    if(!m_pAtomList) return FALSE;

    //データタイプセット

    p = m_pAtomList;

    if(pAtomList)
        ::memcpy(m_pAtomList, pAtomList, sizeof(ULONG) * atomcnt);
    else
    {
        switch(m_uDatType)
        {
            case TYPE_TEXT:
                *(p++) = axatom(AXAppAtom::UTF8_STRING);
                *(p++) = axatom(AXAppAtom::COMPOUND_TEXT);
                break;
        }
    }

    //TARGET時のアトムセット

    *(p++) = axatom(AXAppAtom::TARGETS);
    *(p++) = axatom(AXAppAtom::MULTIPLE);

    //

    m_nAtomCnt = atomcnt;

    return TRUE;
}

//! 現在のデータをクリップボードマネージャに保存
/*!
    データはアプリケーション自身が保持しているので、アプリケーションが終了した場合は
    クリップボードデータは消えることになる。@n
    アプリケーションが終了した後もクリップボードにデータを置いておきたい場合は、save() を実行して
    クリップボードマネージャの方にデータを保存させる。@n
    ※各対応データ形式ごとにデータが保存されるので、コールバック関数が複数回来る。@n
    ※save() 時のコールバック呼び出し時は、CALLBACK::bMultiple が TRUE。

    @param idWindow データを処理するウィンドウ
    @param type     0 で現在のデータを保存。それ以外で、現在のデータが指定形式だった場合のみ保存
*/

BOOL AXClipboard::save(ULONG idWindow,UINT type)
{
    if(!m_pDatBuf) return FALSE;

    //タイプ指定

    if(type && m_uDatType != type) return FALSE;

    //マネージャの所有者がいない

    if(::XGetSelectionOwner(axdisp(), axatom(AXAppAtom::CLIPBOARD_MANAGER)) == None)
        return FALSE;

    //保存要求
    //※先にプロパティを消しておかないと"MULTIPLE"でNULLが来る

    ::XDeleteProperty(axdisp(), idWindow, axatom(AXAppAtom::AXSELECTION));

    ::XSetSelectionOwner(axdisp(), axatom(AXAppAtom::CLIPBOARD), idWindow, axapp->getLastTime());

    ::XConvertSelection(axdisp(), axatom(AXAppAtom::CLIPBOARD_MANAGER), axatom(AXAppAtom::SAVE_TARGETS),
            axatom(AXAppAtom::AXSELECTION), idWindow, axapp->getLastTime());

    return TRUE;
}


//================================
//補助処理
//================================


//! コールバック時、データを送る

void AXClipboard::sendDat(CALLBACK *pDat,const void *pBuf,UINT uSize)
{
    AXSetProperty8(pDat->idSendWindow, pDat->atomProp, pDat->atomType, pBuf, uSize);
}


//================================
//データ取得
//================================


//! クリップボードから指定タイプのデータ取得

BOOL AXClipboard::getDat(ULONG idWindow,ULONG atomType,AXMem *pmem)
{
    return AXGetSelectionData(idWindow, axatom(AXAppAtom::CLIPBOARD), atomType, pmem);
}

//! 指定したアトムリストの中から取得可能なデータを取得
/*!
    @param pAtomList データタイプのリスト。配列の上にあるものほど優先度が高い
    @return 取得したデータタイプ。0でなし。
*/

ULONG AXClipboard::getDat(ULONG idWindow,ULONG *pAtomList,int atomcnt,AXMem *pmem)
{
    ULONG type;

    //タイプ取得

    type = AXGetSelectionTargetType(idWindow, axatom(AXAppAtom::CLIPBOARD), pAtomList, atomcnt);
    if(type == 0) return 0;

    //データ取得

    if(AXGetSelectionData(idWindow, axatom(AXAppAtom::CLIPBOARD), type, pmem))
        return type;
    else
        return 0;
}

//! クリップボードからテキスト取得
/*!
    @param idWindow データ取得に使うウィンドウ
*/

BOOL AXClipboard::getText(ULONG idWindow,AXString *pstr)
{
    pstr->empty();

    if(m_pDatBuf)
    {
        //-------- 自身がデータを持っている場合 (UNICODE-16bit)

        if(m_uDatType != TYPE_TEXT) return FALSE;

        pstr->set((LPUSTR)m_pDatBuf, m_uDatSize / sizeof(UNICHAR));
    }
    else
    {
        //--------- 他クライアントから

        Atom atom[3],type;

        //利用可能タイプ取得

        atom[0] = axatom(AXAppAtom::UTF8_STRING);
        atom[1] = axatom(AXAppAtom::COMPOUND_TEXT);
        atom[2] = XA_STRING;

        type = AXGetSelectionTargetType(idWindow, axatom(AXAppAtom::CLIPBOARD), atom, 3);
        if(!type) return FALSE;

        //テキスト取得

        if(type == axatom(AXAppAtom::COMPOUND_TEXT))
        {
            //COMPOUND_TEXT

            return AXGetSelectionCompoundText(idWindow, axatom(AXAppAtom::CLIPBOARD), pstr);
        }
        else
        {
            //UTF8/STRING

            AXMem mem;

            if(!AXGetSelectionData(idWindow, axatom(AXAppAtom::CLIPBOARD), type, &mem)) return FALSE;

            pstr->setUTF8(mem, mem.getSize());
        }
    }

    pstr->recalcLen();

    return TRUE;
}


//================================
//ハンドラ処理
//================================


//! SelectionRequestイベント時の処理 (selection="CLIPBOARD")

void AXClipboard::OnSelectionRequest(LPVOID pEvent)
{
    XSelectionRequestEvent *pev = (XSelectionRequestEvent *)pEvent;
    XSelectionEvent ev;

    if(!m_pDatBuf)
        pev->property = 0;
    else if(pev->target == axatom(AXAppAtom::TARGETS))
    {
        //"TARGETS" : データの有効なフォーマットタイプをセット

        AXSetPropertyAtom(pev->requestor, pev->property, m_pAtomList, m_nAtomCnt + TARGET_DEFCNT);
    }
    else if(pev->target == axatom(AXAppAtom::MULTIPLE))
    {
        //"MULTIPLE" : 複数タイプデータセット（クリップボードマネージャ保存時）

        _setMultiple(pev->requestor, pev->property);
    }
    else
    {
        //個々のデータをセット（他クライアントからの要求）

        if(!_sendOneDat(pev->requestor, pev->property, pev->target, FALSE))
            pev->property = 0;
    }

    //返信イベント送信

    ev.type         = SelectionNotify;
    ev.send_event   = 1;
    ev.display      = axdisp();
    ev.requestor    = pev->requestor;
    ev.selection    = pev->selection;
    ev.target       = pev->target;
    ev.property     = pev->property;
    ev.time         = pev->time;

    ::XSendEvent(ev.display, ev.requestor, FALSE, 0, (XEvent *)&ev);
}

//! 複数タイプのデータセット

void AXClipboard::_setMultiple(ULONG window,ULONG prop)
{
    AXMem mem;
    int i,cnt;
    ULONG *p;

    //各プロパティとタイプ取得

    if(!AXGetProperty32(window, prop, axatom(AXAppAtom::ATOM_PAIR), &mem, &cnt)) return;

    //2つのAtomが対になっている

    cnt /= 2;

    for(i = 0, p = (ULONG *)mem.getBuf(); i < cnt; i++, p += 2)
        _sendOneDat(window, p[0], p[1], TRUE);
}

//! 指定タイプのデータ送る

BOOL AXClipboard::_sendOneDat(ULONG window,ULONG prop,ULONG type,BOOL bMultiple)
{
    int i;
    CALLBACK dat;

    //コールバックデータ

    dat.pBuf        = m_pDatBuf;
    dat.uSize       = m_uDatSize;
    dat.idSendWindow = window;
    dat.atomProp    = prop;
    dat.atomType    = type;
    dat.bMultiple   = bMultiple;

    //

    for(i = 0; i < m_nAtomCnt; i++)
    {
        if(type == m_pAtomList[i])
        {
            //コールバック関数で処理

            if(m_callback)
                return (*m_callback)(this, &dat);

            //コールバック関数がない場合、デフォルト処理

            if(m_uDatType == TYPE_TEXT)
                return _sendDefText(window, prop, type);
        }
    }

    //利用可能なデータタイプではない
    return FALSE;
}

//! テキストのデフォルト処理

BOOL AXClipboard::_sendDefText(ULONG window,ULONG prop,ULONG type)
{
    AXString str;

    str.set((LPUSTR)m_pDatBuf, m_uDatSize / sizeof(UNICHAR));

    if(type == axatom(AXAppAtom::COMPOUND_TEXT))
        AXSetPropertyCompoundText(window, prop, str);
    else
    {
        AXByteString utf8;

        str.toUTF8(&utf8);
        AXSetProperty8(window, prop, type, (LPSTR)utf8, utf8.getLen(), FALSE);
    }

    return TRUE;
}
