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

#include "AXX11.h"

#include "AXAppIM.h"

#include "AXTopWindow.h"
#include "AXAppIMWin.h"


//-------------------

void AXIM_CallbackDestroy(XIM *,XPointer,XPointer);

int AXIM_CallbackPEStart(XIC *,XPointer,XPointer);
void AXIM_CallbackPEDone(XIC *,XPointer,XPointer);
void AXIM_CallbackPEDraw(XIC *,XPointer,XIMPreeditDrawCallbackStruct *);
void AXIM_CallbackPECaret(XIC *,XPointer,XIMPreeditCaretCallbackStruct *);

#define IMSTYLE_CALLBACK    (XIMPreeditCallbacks | XIMStatusNothing)
#define IMSTYLE_ROOT        (XIMPreeditNothing | XIMStatusNothing)

//-------------------



//*************************************
// AXAppIM
//*************************************


/*!
    @class AXAppIM
    @brief 入力メソッドクラス（AXApp で作成される）

    - IMが何らかの理由で停止した場合、m_pIM は NULL にセットされる。

    @ingroup core
*/


AXAppIM::AXAppIM()
{
    m_pIM = NULL;
}

AXAppIM::~AXAppIM()
{
    if(m_pIM)
        ::XCloseIM((XIM)m_pIM);
}

//! 初期化

BOOL AXAppIM::init(LPVOID pDisp)
{
    XIM xim;
    XIMStyles *pStyle;
    XIMCallback cbDestroy;
    int i,flag;

    //IM 開く

    xim = ::XOpenIM((Display *)pDisp, NULL, NULL, NULL);
    if(!xim) return FALSE;

    //サポートされているスタイル

    if(::XGetIMValues(xim, XNQueryInputStyle, &pStyle, NULLP) != NULL)
    {
        ::XCloseIM(xim);
        return FALSE;
    }

    for(i = 0, flag = 0; i < pStyle->count_styles; i++)
    {
        if(pStyle->supported_styles[i] == IMSTYLE_ROOT)
            flag |= 1;
        else if(pStyle->supported_styles[i] == IMSTYLE_CALLBACK)
            flag |= 2;
    }

    ::XFree(pStyle);

    if(flag == 0)
    {
        ::XCloseIM(xim);
        return FALSE;
    }

    m_uStyle = (flag & 2)? IMSTYLE_CALLBACK: IMSTYLE_ROOT;

    //コールバック

    cbDestroy.callback      = (XIMProc)AXIM_CallbackDestroy;
    cbDestroy.client_data   = (XPointer)this;

    ::XSetIMValues(xim, XNDestroyCallback, &cbDestroy, NULLP);

    //

    m_pIM = (LPVOID)xim;

    return TRUE;
}


//************************************
// AXAppIC
//************************************

/*!
    @class AXAppIC
    @brief 入力コンテキスト

    - トップレベルウィンドウそれぞれが持つ。

    @ingroup core
*/


AXAppIC::AXAppIC()
{
    m_pIC    = NULL;
    m_pAXIM  = NULL;
    m_pwinPE = NULL;
}

AXAppIC::~AXAppIC()
{
    if(m_pwinPE)
        delete m_pwinPE;

    //IMが停止している場合は破棄処理は行なってはいけない

    if(m_pIC && m_pAXIM->isExist())
        ::XDestroyIC((XIC)m_pIC);
}

//! 作成

BOOL AXAppIC::init(AXAppIM *pAXIM,AXWindow *pwinClient)
{
    long mask;
    int i;
    XIC xic;
    XIMCallback cb[4];
    XVaNestedList listp;

    if(pAXIM->isEmpty()) return FALSE;

    //コンテキスト作成

    if(pAXIM->getStyle() == IMSTYLE_ROOT)
    {
        //-------- root

        xic = ::XCreateIC((XIM)pAXIM->getIM(),
                    XNInputStyle,   IMSTYLE_ROOT,
                    XNClientWindow, pwinClient->getid(),
                    XNFocusWindow,  pwinClient->getid(),
                    NULLP);
    }
    else
    {
        //-------- コールバック

        //前編集属性

        cb[0].callback = (XIMProc)AXIM_CallbackPEStart;
        cb[1].callback = (XIMProc)AXIM_CallbackPEDone;
        cb[2].callback = (XIMProc)AXIM_CallbackPEDraw;
        cb[3].callback = (XIMProc)AXIM_CallbackPECaret;

        for(i = 0; i < 4; i++)
            cb[i].client_data = (XPointer)this;

        listp = ::XVaCreateNestedList(0,
                    XNPreeditStartCallback, cb,
                    XNPreeditDoneCallback,  cb + 1,
                    XNPreeditDrawCallback,  cb + 2,
                    XNPreeditCaretCallback, cb + 3,
                    NULLP);

        //作成

        xic = ::XCreateIC((XIM)pAXIM->getIM(),
                    XNInputStyle,       IMSTYLE_CALLBACK,
                    XNClientWindow,     pwinClient->getid(),
                    XNFocusWindow,      pwinClient->getid(),
                    XNPreeditAttributes,    listp,
                    NULLP);

        ::XFree(listp);
    }

    if(!xic) return FALSE;

    //リセット

    LPSTR pcret = ::XmbResetIC(xic);
    if(pcret) ::XFree(pcret);

    //必要なイベントマスクセット

    ::XGetICValues(xic, XNFilterEvents, &mask, NULLP);

    pwinClient->setEventMask(mask, TRUE);

    //

    m_pIC        = (LPVOID)xic;
    m_pAXIM      = pAXIM;
    m_pwinClient = (AXTopWindow *)pwinClient;

    return TRUE;
}

//! フォーカスセット

void AXAppIC::setFocus()
{
    if(m_pAXIM->isExist())
    {
        ::XSetICFocus((XIC)m_pIC);

        if(m_pwinPE)
        {
            //クライアントウィンドウが移動した場合
            moveWinPos();

            m_pwinPE->show();
        }
    }
}

//! フォーカス消失

void AXAppIC::killFocus()
{
    if(m_pAXIM->isExist())
    {
        ::XUnsetICFocus((XIC)m_pIC);

        if(m_pwinPE) m_pwinPE->hide();
    }
}

//! ウィンドウ位置移動

void AXAppIC::moveWinPos()
{
    AXPoint pt;

    m_pwinClient->getICPos(&pt);
    m_pwinPE->move(pt.x, pt.y);
}

//! 前編集開始

void AXAppIC::PE_start()
{
    AXPoint pt;

    if(m_pwinPE) PE_done();

    m_pwinClient->getICPos(&pt);

    m_pwinPE = new AXAppIMWin(pt.x, pt.y);
}

//! 前編集終了

void AXAppIC::PE_done()
{
    if(m_pwinPE)
    {
        delete m_pwinPE;
        m_pwinPE = NULL;
    }
}

//! 描画

void AXAppIC::PE_draw(LPVOID pParam)
{
    XIMPreeditDrawCallbackStruct *pds = (XIMPreeditDrawCallbackStruct *)pParam;

    if(!pds->text) return;
    if(!pds->text->string.multi_byte) return;

    //ウィンドウ位置
    /*
        ※入力中、Enter を押さずに次の文字を入力した場合、done が来ずに draw が続くので
         常にカーソルの位置に来るようにする
    */

    moveWinPos();

    //

    m_pwinPE->PE_draw(pParam);
}

/*

[XIMPreeditDrawCallbackStruct]
    int caret;      //文字列内でのカーソル位置（文字数）
    int chg_first;  //変換対象部分の開始位置
    int chg_length; //変換対象部分の文字数
    XIMText *text;  //文字（NULLで削除）

[XIMText]
    unsigned short length;  //テキスト文字数
    XIMFeedback * feedback; //各文字における表示タイプの配列
    Bool encoding_is_wchar; //ワイド文字か
    union {
        char * multi_byte;
        wchar_t * wide_char;
    } string;

*/


//************************************
// コールバック
//************************************


//! IMが何らかの理由で停止した時
/*
    ※これ以降、XCloseIM() や XDestroyIC() を呼んではならない
*/

void AXIM_CallbackDestroy(XIM *xim,XPointer client,XPointer empty)
{
	((AXAppIM *)client)->empty();
}

//! 前編集開始
/*
    戻り値：前編集文字列の最大長（-1で無限）
*/

int AXIM_CallbackPEStart(XIC *xic,XPointer client,XPointer empty)
{
	((AXAppIC *)client)->PE_start();

	return -1;
}

//! 前編集終了

void AXIM_CallbackPEDone(XIC *xic,XPointer client,XPointer empty)
{
	((AXAppIC *)client)->PE_done();
}

//! 描画

void AXIM_CallbackPEDraw(XIC *xic,XPointer client,XIMPreeditDrawCallbackStruct *calldat)
{
	((AXAppIC *)client)->PE_draw(calldat);
}

//! テキスト挿入位置移動

void AXIM_CallbackPECaret(XIC *xic,XPointer client,XIMPreeditCaretCallbackStruct *calldat)
{

}
