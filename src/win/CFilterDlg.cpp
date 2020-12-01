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

#include "CFilterDlg.h"

#include "CFilterBar.h"
#include "CFilterPrev.h"
#include "CFilterWidgetLevel.h"

#include "CDraw.h"
#include "CConfig.h"
#include "CLayerList.h"
#include "CFilterConfigList.h"

#include "AXBuf.h"
#include "AXLayout.h"
#include "AXSpacerItem.h"
#include "AXLabel.h"
#include "AXButton.h"
#include "AXLineEdit.h"
#include "AXCheckButton.h"
#include "AXComboBox.h"
#include "AXByteString.h"
#include "AXApp.h"
#include "AXTrSysID.h"

#include "defGlobal.h"
#include "defStrID.h"


//------------------------

#define WID_BTT_OK      1
#define WID_BTT_CANCEL  2
#define WID_BTT_RESET   3
#define WID_PREV        90
#define WID_CK_PREV     91
#define WID_CK_CLIP     92
#define WID_CB_LAYER    93
#define WID_LEVEL       94

#define WID_BAR_TOP     100
#define WID_BAREDIT_TOP 150
#define WID_COMBO_TOP   200
#define WID_CHECK_TOP   250

//------------------------


/*!
    @class CFilterDlg
    @brief フィルタダイアログ
*/


CFilterDlg::CFilterDlg(AXWindow *pOwner,WORD wTitleID,const BYTE *pDat,BOOL (*func)(DRAWINFO *),DRAWINFO *pInfo)
    : AXDialog(pOwner, WS_DIALOG_NORMAL | WS_BK_FACE)
{
    AXBuf buf;
    int prevw,prevh;

    m_func  = func;
    m_pinfo = pInfo;
    m_uMenuID   = wTitleID;
    m_pConfItem = NULL;

    m_bPreview  = g_pconf->uEtcFlags & CConfig::ETCFLAG_FILTER_PREVIEW;

    m_nBarCnt   = 0;
    m_nEditCnt  = 0;
    m_nComboCnt = 0;
    m_nCheckCnt = 0;

    m_pPrev     = NULL;
    m_pcbLayer  = NULL;
    m_pwidLevel = NULL;

    //タイトル

    setTitle(_string(STRGID_MAINMENU, wTitleID));

    //---------- データ

    buf.init((BYTE *)pDat, 1000, AXBuf::ENDIAN_BIG);

    //フラグ (BYTE)

    m_uFlags = buf.getBYTE();

    m_bCanvasPrev = m_uFlags & DATFLAG_CANVASPREVIEW;

    //プレビュー幅・高さ

    prevw = buf.getWORD();
    prevh = buf.getWORD();

    //---------- 設定

    if(m_uFlags & DATFLAG_SAVECONF)
        m_pConfItem = g_pconf->pFilterConf->findID(m_uMenuID);

    //---------- レイアウト・ウィジェット

    _create_widget(&buf, prevw, prevh);

    //--------- プレビュー用準備

    if(m_uFlags & DATFLAG_PREVIEW)
    {
        //ダイアログ上プレビュー

        g_pdraw->setBeforeDraw_filter(TRUE);

        m_pinfo->prs      = &m_rsPrev;
        m_pinfo->pimgSrc  = &g_pdraw->getCurLayerImg();
        m_pinfo->pimgDst  = &m_imgPrev;
        m_pinfo->pProgDlg = NULL;

        m_pPrev->getDrawRect(&m_rsPrev);

        m_imgPrev.create(g_pdraw->m_nImgW, g_pdraw->m_nImgH);

        if(m_bPreview)
            _prev();
        else
            m_pPrev->drawImg(*m_pinfo->pimgSrc);
    }
    else if(m_uFlags & DATFLAG_CANVASPREVIEW)
    {
        //キャンバスプレビュー

        g_pdraw->beginFilterCanvasPrev(this);

        m_pinfo->prs      = &m_rsPrev;
        m_pinfo->pimgSrc  = &g_pdraw->getCurLayerImg();
        m_pinfo->pimgDst  = g_pdraw->m_pimgTmp;
        m_pinfo->pProgDlg = NULL;

        _prev();
    }

    //-----------

    calcDefSize();
    resize(m_nDefW, m_nDefH);

    show();
}

//! 閉じる

BOOL CFilterDlg::onClose()
{
    _end(FALSE);
    return TRUE;
}

//! 通知

BOOL CFilterDlg::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    UINT id = pwin->getItemID();
    int n;

    if(id >= WID_BAR_TOP && id < WID_BAR_TOP + MAXNUM_BAR)
    {
        //バー

        if(uNotify == CFilterBar::NOTIFY_CHANGE)
        {
            id -= WID_BAR_TOP;
            n = m_pbar[id]->getPos();
            m_pinfo->nValBar[id] = n;
            m_peditBar[id]->setVal(n);
        }
        else
            _prev();
    }
    else if(id >= WID_BAREDIT_TOP && id < WID_BAREDIT_TOP + MAXNUM_BAR)
    {
        //バーエディット

        if(uNotify == AXLineEdit::EN_CHANGE)
        {
            id -= WID_BAREDIT_TOP;
            n = m_peditBar[id]->getVal();
            m_pinfo->nValBar[id] = n;
            m_pbar[id]->setPos(n);

            addTimer(0, 500);
        }
    }
    else if(id >= WID_COMBO_TOP && id < WID_COMBO_TOP + MAXNUM_COMBO)
    {
        //コンボボックス

        if(uNotify == AXComboBox::CBN_SELCHANGE)
        {
            id -= WID_COMBO_TOP;
            m_pinfo->nValCombo[id] = m_pcombo[id]->getCurSel();

            _prev();
        }
    }
    else if(id >= WID_CHECK_TOP && id < WID_CHECK_TOP + MAXNUM_CHECK)
    {
        //チェックボタン

        m_pinfo->nValCheck[id - WID_CHECK_TOP] ^= 1;
        _prev();
    }
    else
    {
        //------ ほか

        switch(id)
        {
            //プレビュー位置変更
            case WID_PREV:
                m_pPrev->getDrawRect(&m_rsPrev);

                if(m_bPreview)
                    _prev();
                else
                    m_pPrev->drawImg(*(m_pinfo->pimgSrc));
                break;
            //プレビューON/OFF
            case WID_CK_PREV:
                m_bPreview ^= 1;

                g_pconf->uEtcFlags ^= CConfig::ETCFLAG_FILTER_PREVIEW;

                _prev();
                break;

            //クリッピング
            case WID_CK_CLIP:
                m_pinfo->bClipping ^= 1;
                _prev();
                break;
            //レイヤリスト
            case WID_CB_LAYER:
                if(uNotify == AXComboBox::CBN_SELCHANGE)
                {
                    m_pinfo->pimgRef = &(g_pdraw->getLayer(m_pcbLayer->getCurSel()))->m_img;
                    _prev();
                }
                break;
            //レベル補正
            case WID_LEVEL:
                m_pwidLevel->getVal(m_pinfo->nValBar);
                _prev();
                break;

            //リセット
            case WID_BTT_RESET:
                _reset();
                break;
            //OK
            case WID_BTT_OK:
                _end(TRUE);
                break;
            //キャンセル
            case WID_BTT_CANCEL:
                _end(FALSE);
                break;
        }
    }

    return TRUE;
}

//! タイマー

BOOL CFilterDlg::onTimer(UINT uTimerID,ULONG lParam)
{
    _prev();

    delTimer(uTimerID);

    return TRUE;
}

//! コマンド

BOOL CFilterDlg::onCommand(UINT uID,ULONG lParam,int from)
{
    //キャンバスが左クリックされた時、プレビュー更新(CDraw から呼ばれる)

    if(m_bCanvasPrev)
        _prev();

    return TRUE;
}


//==============================
// サブ
//==============================


//! 終了

void CFilterDlg::_end(BOOL bOK)
{
    delTimerAll();

    if(m_bCanvasPrev)
        g_pdraw->endFilterCanvasPrev();

    if(m_uFlags & DATFLAG_SAVECONF)
        _saveConfig();

    endDialog(bOK);
}

//! 設定保存

void CFilterDlg::_saveConfig()
{
    AXByteString str;
    int i;

    //バー

    for(i = 0; i < m_nBarCnt; i++)
    {
        if(m_cSigBar[i])
        {
            str += m_cSigBar[i];
            str += m_pinfo->nValBar[i];
        }
    }

    //チェック

    for(i = 0; i < m_nCheckCnt; i++)
    {
        if(m_cSigCheck[i])
        {
            str += m_cSigCheck[i];
            str += m_pinfo->nValCheck[i];
        }
    }

    //コンボボックス

    for(i = 0; i < m_nComboCnt; i++)
    {
        if(m_cSigCombo[i])
        {
            str += m_cSigCombo[i];
            str += m_pinfo->nValCombo[i];
        }
    }

    g_pconf->pFilterConf->setDat(m_uMenuID, str);
}

//! リセット

void CFilterDlg::_reset()
{
    int i;

    //バー

    for(i = 0; i < m_nBarCnt; i++)
    {
        m_pbar[i]->setPos(m_sDefBar[i]);
        m_peditBar[i]->setVal(m_sDefBar[i]);

        m_pinfo->nValBar[i] = m_sDefBar[i];
    }

    _prev();
}

//! プレビュー

void CFilterDlg::_prev()
{
    if(m_bCanvasPrev)
    {
        //-------- キャンバスプレビュー

        FLAGRECT rcf = g_pdraw->m_rcfDrawUp;

        AXWindow::setCursor(&g_pdraw->m_cursorWait);
        axapp->flush();

        //プレビューイメージ・範囲クリア

        if(g_pdraw->m_rcfDrawUp.flag)
        {
            g_pdraw->m_pimgTmp->freeAllTile();
            g_pdraw->m_rcfDrawUp.flag = FALSE;
        }

        //描画

        g_pdraw->m_datDraw.rcfDraw.flag = FALSE;

        m_func(m_pinfo);

        //更新（前回の範囲と合わせた範囲）

        g_pdraw->calcUnionFlagRect(&rcf, g_pdraw->m_datDraw.rcfDraw);

        g_pdraw->updateCanvasAndPrevWin(rcf, FALSE);
        axapp->update();

        //m_rcfDrawUp = 今回の範囲

        g_pdraw->m_rcfDrawUp = g_pdraw->m_datDraw.rcfDraw;

        AXWindow::unsetCursor();
    }
    else if(m_pPrev && m_bPreview)
    {
        //----------- ダイアログ上プレビュー

        AXWindow::setCursor(&g_pdraw->m_cursorWait);
        axapp->flush();

        m_imgPrev.freeAllTile();

        //m_imgPrev に描画

        m_func(m_pinfo);

        //プレビューイメージに描画

        m_pPrev->drawImg(m_imgPrev);

        AXWindow::unsetCursor();
    }
}


//==============================
// 初期化
//==============================


//! レイアウト・ウィジェット作成

void CFilterDlg::_create_widget(AXBuf *pbuf,int prevw,int prevh)
{
    AXLayout *plTop,*pl2,*plm,*plh;
    AXWindow *pwin;

    //トップレイアウト

    setLayout(plTop = new AXLayoutVert);
    plTop->setSpacing(10);

    //----------- プレビュー+ウィジェット

    if(m_uFlags & DATFLAG_VERT)
        plTop->addItem(pl2 = new AXLayoutVert(LF_EXPAND_W, 10));
    else
        plTop->addItem(pl2 = new AXLayoutHorz(LF_EXPAND_W, 10));

    //プレビュー

    if(m_uFlags & DATFLAG_PREVIEW)
        pl2->addItem(m_pPrev = new CFilterPrev(this, 0, WID_PREV, 0, prevw, prevh, &g_pdraw->getCurLayerImg()));

    //ウィジェット

    pl2->addItem(plm = new AXLayoutMatrix(2, LF_EXPAND_W));

    _create_subwidget(pbuf, plm);

    //---------- ボタン

    plTop->addItem(plh = new AXLayoutHorz(LF_EXPAND_W, 3));
    plh->setPaddingTop(15);

    //プレビューチェック

    if(m_uFlags & DATFLAG_PREVIEW)
        plh->addItem(new AXCheckButton(this, 0, 0, WID_CK_PREV, 0, STRID_FILTERDLG_PREVIEW, m_bPreview));

    //OK

    plh->addItem(pwin = new AXButton(this, 0, LF_EXPAND_X|LF_RIGHT, WID_BTT_OK, 0, TRSYS_GROUPID, TRSYS_OK));

    setDefaultButton(pwin);

    //キャンセル

    plh->addItem(new AXButton(this, 0, 0, WID_BTT_CANCEL, 0, TRSYS_GROUPID, TRSYS_CANCEL));

    //リセット

    if(m_uFlags & DATFLAG_RESET)
        plh->addItem(new AXButton(this, 0, 0, WID_BTT_RESET, 0, STRID_FILTERDLG_RESET));
}

//! 各定義ウィジェット作成

void CFilterDlg::_create_subwidget(AXBuf *pbuf,AXLayout *plm)
{
    int wtype,size,ret;
    WORD wLabelID;

    _trgroup(STRGID_FILTERDLG);

    while(1)
    {
        wtype = pbuf->getBYTE();
        if(wtype == 255) break;

        if(wtype >= 0x80)
        {
            //------ 独自定義ウィジェット

            switch(wtype)
            {
                //キャンバス範囲外クリッピング
                case 0x80:
                    _create_def_clipping(plm);
                    break;
                //レベル補正
                case 0x81:
                    _create_def_level(plm);
                    break;
            }
        }
        else
        {
            //------ 各タイプのウィジェット

            //WORD:ラベル名文字列ID (0でなし)

            pbuf->getWORD(&wLabelID);

            //ラベル

            if(wLabelID)
                plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,6,6), wLabelID));
            else
                plm->addItem(new AXSpacerItem);

            //データサイズ

            size = pbuf->getBYTE();

            //

            switch(wtype)
            {
                //バー
                case 0:
                    ret = _create_bar(pbuf, plm, size);
                    break;
                //チェックボタン
                case 1:
                    ret = _create_check(pbuf, plm, size);
                    break;
                //コンボボックス
                case 2:
                    ret = _create_combo(pbuf, plm, size);
                    break;
                //レイヤリストコンボボックス
                case 3:
                    ret = _create_layerlist(pbuf, plm, size);
                    break;
                //ラベル
                case 4:
                    plm->addItem(new AXLabel(this, 0, LF_CENTER_Y, MAKE_DW4(0,0,0,6), pbuf->getWORD()));
                    ret = TRUE;
                    break;
                default:
                    ret = FALSE;
                    break;
            }

            if(!ret) break;
        }
    }
}


//! キャンバス範囲外クリッピング

void CFilterDlg::_create_def_clipping(AXLayout *plm)
{
    plm->addItem(new AXSpacerItem);
    plm->addItem(new AXCheckButton(this, 0, 0, WID_CK_CLIP, MAKE_DW4(0,0,0,6), STRID_FILTERDLG_CLIPPING, TRUE));

    m_pinfo->bClipping = TRUE;
}

//! レベル補正

void CFilterDlg::_create_def_level(AXLayout *plm)
{
    AXMem mem;

    plm->addItem(m_pwidLevel = new CFilterWidgetLevel(this, WID_LEVEL, 0));

    //

    if(mem.allocClear(256 * sizeof(DWORD)))
    {
        g_pdraw->getCurLayerImg().getHistogram(mem);

        m_pwidLevel->drawAll(mem);
    }

    m_pwidLevel->getVal(m_pinfo->nValBar);
}

//! バー作成（バー＋エディット）

BOOL CFilterDlg::_create_bar(AXBuf *pbuf,AXLayout *plm,int size)
{
    AXLayout *pl;
    short barw,min,max,def,center;
    BYTE dig,defType;
    char sig;
    int n;

    if(m_nBarCnt >= MAXNUM_BAR) return FALSE;

    //値

    pbuf->getWORD(&barw);
    pbuf->getWORD(&min);
    pbuf->getWORD(&max);
    if(size > 6) pbuf->getWORD(&def); else def = min;
    if(size > 8) pbuf->getWORD(&center); else center = min;
    if(size > 10) pbuf->getBYTE(&dig); else dig = 0;
    if(size > 11) pbuf->getBYTE(&sig); else sig = 0;
    if(size > 12) pbuf->getBYTE(&defType); else defType = 0;

    if(defType == 1)
        def = g_pdraw->m_nImgDPI;

    m_cSigBar[m_nBarCnt] = sig;
    m_sDefBar[m_nBarCnt] = def;

    if(m_pConfItem && sig)
    {
        if(m_pConfItem->getVal(sig, &n))
            def = n;
    }

    //

    plm->addItem(pl = new AXLayoutHorz(LF_EXPAND_W, 6));
    pl->setPaddingBottom(6);

    //バー

    pl->addItem(m_pbar[m_nBarCnt] = new CFilterBar(this, LF_CENTER_Y, WID_BAR_TOP + m_nBarCnt, 0,
                                            min, max, def, center));

    m_pbar[m_nBarCnt]->setMinWidth(barw);

    //エディット

    pl->addItem(m_peditBar[m_nBarCnt] = new AXLineEdit(this, AXLineEdit::ES_SPIN, LF_CENTER_Y,
                                                    WID_BAREDIT_TOP + m_nBarCnt, 0));


    m_peditBar[m_nBarCnt]->setInit(6, min, max, dig, def);

    //

    m_pinfo->nValBar[m_nBarCnt] = def;

    m_nBarCnt++;

    return TRUE;
}

//! チェックボタン作成

BOOL CFilterDlg::_create_check(AXBuf *pbuf,AXLayout *plm,int size)
{
    WORD wStrID;
    BYTE bCheck;
    char sig;
    int n;

    if(m_nCheckCnt >= MAXNUM_CHECK) return FALSE;

    //値

    pbuf->getWORD(&wStrID);
    if(size > 2) pbuf->getBYTE(&bCheck); else bCheck = FALSE;
    if(size > 3) pbuf->getBYTE(&sig); else sig = 0;

    m_cSigCheck[m_nCheckCnt] = sig;

    if(m_pConfItem && sig)
    {
        if(m_pConfItem->getVal(sig, &n))
            bCheck = n;
    }

    //作成

    plm->addItem(new AXCheckButton(this, 0, 0, WID_CHECK_TOP + m_nCheckCnt, MAKE_DW4(0,0,0,6), wStrID, bCheck));

    //

    m_pinfo->nValCheck[m_nCheckCnt] = (bCheck)? 1: 0;

    m_nCheckCnt++;

    return TRUE;
}

//! コンボボックス作成

BOOL CFilterDlg::_create_combo(AXBuf *pbuf,AXLayout *plm,int size)
{
    BYTE cnt,def;
    WORD strtop;
    char sig;
    int n;

    if(m_nComboCnt >= MAXNUM_COMBO) return FALSE;

    //値

    pbuf->getBYTE(&cnt);
    pbuf->getWORD(&strtop);
    if(size > 3) pbuf->getBYTE(&def); else def = 0;
    if(size > 4) pbuf->getBYTE(&sig); else sig = 0;

    m_cSigCombo[m_nComboCnt] = sig;

    if(m_pConfItem && sig)
    {
        if(m_pConfItem->getVal(sig, &n))
            def = n;
    }

    //コンボボックス

    plm->addItem(m_pcombo[m_nComboCnt] = new AXComboBox(this, 0, LF_CENTER_Y,
                                                        WID_COMBO_TOP + m_nComboCnt, MAKE_DW4(0,0,0,6)));

    //項目

    m_pcombo[m_nComboCnt]->addItemMulTr(strtop, cnt);
    m_pcombo[m_nComboCnt]->setAutoWidth();
    m_pcombo[m_nComboCnt]->setCurSel(def);

    //

    m_pinfo->nValCombo[m_nComboCnt] = def;

    m_nComboCnt++;

    return TRUE;
}

//! レイヤリストコンボボックス作成

BOOL CFilterDlg::_create_layerlist(AXBuf *pbuf,AXLayout *plm,int size)
{
    CLayerItem *p;
    AXString str;
    int i;

    if(m_pcbLayer) return FALSE;

    plm->addItem(m_pcbLayer = new AXComboBox(this, 0, LF_CENTER_Y, WID_CB_LAYER, MAKE_DW4(0,0,0,6)));

    //リスト項目

    for(p = g_pdraw->m_player->getBottomItem(), i = 1; p; p = p->prev(), i++)
    {
        str.setInt(i);
        str += ": ";
        str += p->m_strName;
        if(p == g_pdraw->m_pCurLayer) str += " *";

        m_pcbLayer->addItem(str);
    }

    m_pcbLayer->setCurSel(g_pdraw->m_nCurLayerNo);
    m_pcbLayer->setAutoWidth();

    m_pinfo->pimgRef = &g_pdraw->getCurLayerImg();

    return TRUE;
}
