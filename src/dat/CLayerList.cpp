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

#include "CLayerList.h"

#include "defStruct.h"


//******************************
// CLayerList
//******************************

/*!
    @class CLayerList
    @brief レイヤのリストデータ
*/


//! レイヤ追加
/*!
    @param pInfo ポインタを指定すると、この情報を使って作成する
*/

CLayerItem *CLayerList::addLayer(int w,int h,LAYERINFO *pInfo)
{
    CLayerItem *p;

    if(m_nCnt >= 400) return NULL;

    p = new CLayerItem;

    if(!p->create(w, h, m_nCnt, pInfo))
    {
        delete p;
        return NULL;
    }

    AXList::add(p);

    return p;
}

//! レイヤ複製

CLayerItem *CLayerList::copyLayer(CLayerItem *pSrc)
{
    CLayerItem *p;

    if(m_nCnt >= 400) return NULL;

    p = new CLayerItem;

    if(!p->copy(pSrc))
    {
        delete p;
        return NULL;
    }

    AXList::add(p);

    return p;
}

//! 指定イメージを複製して作成

CLayerItem *CLayerList::addLayerFromImg(const CLayerImg &src)
{
    CLayerItem *p;

    if(m_nCnt >= 400) return NULL;

    p = new CLayerItem;

    if(!p->createFromImg(src, m_nCnt))
    {
        delete p;
        return NULL;
    }

    AXList::add(p);

    return p;
}

//! 全レイヤのマスクフラグをOFF

void CLayerList::maskOffAll()
{
    CLayerItem *p;

    for(p = getTopItem(); p; p = p->next())
        p->m_btFlag &= ~CLayerItem::FLAG_MASK;
}

//! 作業用フラグがONのレイヤ数取得

int CLayerList::getWorkFlagOnCnt()
{
    CLayerItem *p;
    int cnt = 0;

    for(p = getTopItem(); p; p = p->next())
    {
        if(p->m_bWorkFlag) cnt++;
    }

    return cnt;
}

//! 作業用フラグをすべてOFF

void CLayerList::clearWorkFlag()
{
    CLayerItem *p;

    for(p = getTopItem(); p; p = p->next())
        p->m_bWorkFlag = FALSE;
}

//! 複数レイヤ結合時の情報取得
/*!
    結合するレイヤの作業用フラグを ON にする。

    @param type [0]表示レイヤ [1]チェックレイヤ
    @param pCnt 結合するレイヤ数が返る。
    @return 結合先レイヤ（最後のレイヤ）。NULL で結合なし
*/

CLayerItem *CLayerList::getCombineSomeInfo(int type,int *pCnt)
{
    CLayerItem *p,*pDst = NULL;
    int cnt = 0;
    BOOL flag;

    //上層レイヤから順に

    for(p = getBottomItem(); p; p = p->prev())
    {
        if(type == 0)
            flag = p->isView();
        else
            flag = p->isCheck();

        if(flag)
        {
            pDst = p;
            cnt++;
        }

        //作業用フラグ

        p->m_bWorkFlag = (flag)? 1: 0;
    }

    *pCnt = cnt;

    //結合数が1以下の場合はなし

    return (cnt >= 2)? pDst: NULL;
}

//! グループレイヤを表示/非表示

void CLayerList::toggleShowGroup(CLayerItem *pCur)
{
    CLayerItem *p;
    int nUnder;

    //--------- 下レイヤグループかどうか

    nUnder = 0;

    if(pCur->isGroupUnder())
        //現在のレイヤが下レイヤグループ化の場合
        nUnder = 1;
    else
    {
        //下レイヤグループ化の最後の下レイヤの場合（上レイヤが下グループ化）

        if(pCur->m_pNext)
        {
            if((pCur->next())->isGroupUnder())
                nUnder = 2;
        }
    }

    //----------- 表示/非表示

    if(nUnder)
    {
        //下レイヤグループ化でつながっている場合、一番上のレイヤを探した後、順に下へ

        p = (nUnder == 1)? pCur: pCur->next();

        for(; p; p = p->next())
        {
            if(p->m_pNext == NULL) break;

            if(!(p->next())->isGroupUnder()) break;
        }

        //

        for(; p; p = p->prev())
        {
            p->m_btFlag ^= CLayerItem::FLAG_VIEW;

            if(!p->isGroupUnder()) break;
        }
    }
    else
    {
        //通常グループ

        for(p = getTopItem(); p; p = p->next())
        {
            if(p == pCur || p->isGroup())
                p->m_btFlag ^= CLayerItem::FLAG_VIEW;
        }
    }
}

//! グループのオフセット位置加算＆プレビュー更新セット＆作業フラグON（イメージ移動時）

void CLayerList::addOffset_group(CLayerItem *pCur,int addx,int addy,int type)
{
    CLayerItem *p;

    switch(type)
    {
        //カレント＋グループ
        case 0:
            if(pCur->isGroupUnder())
            {
                //下レイヤとグループ化

                for(p = pCur; p; p = p->prev())
                {
                    p->m_img.setOffset(addx, addy, TRUE);
                    p->m_bPrevUpdate = TRUE;
                    p->m_bWorkFlag   = TRUE;

                    if(!p->isGroupUnder()) break;
                }
            }
            else
            {
                //カレント＆グループがON のレイヤ

                for(p = getTopItem(); p; p = p->next())
                {
                    if(p == pCur || p->isGroup())
                    {
                        p->m_img.setOffset(addx, addy, TRUE);
                        p->m_bPrevUpdate = TRUE;
                        p->m_bWorkFlag   = TRUE;
                    }
                }
            }
            break;

        //カレントのみ
        case 1:
            pCur->m_img.setOffset(addx, addy, TRUE);
            pCur->m_bPrevUpdate = TRUE;
            pCur->m_bWorkFlag   = TRUE;
            break;

        //すべてのレイヤ
        case 2:
            for(p = getTopItem(); p; p = p->next())
            {
                p->m_img.setOffset(addx, addy, TRUE);
                p->m_bPrevUpdate = TRUE;
                p->m_bWorkFlag   = TRUE;
            }
            break;
    }
}

//! マスクレイヤのイメージ取得
/*!
    @return カレントレイヤがマスクレイヤの場合は NULL
*/

CLayerImg *CLayerList::getMaskLayerImg(CLayerItem *pCur)
{
    CLayerItem *p;

    if(pCur->isMaskUnder())
    {
        //下レイヤをマスクに
        //（「下レイヤをマスクに」OFFのレイヤが出るまで下検索）

        for(p = pCur->prev(); p; p = p->prev())
        {
            if(!p->isMaskUnder()) break;
        }
    }
    else
    {
        //マスクレイヤ検索

        for(p = getTopItem(); p; p = p->next())
        {
            if(p->isMask()) break;
        }
    }

    //マスクがカレントレイヤならなしに

    if(!p || p == pCur) return NULL;

    return &p->m_img;
}

//! 塗りつぶし判定元のリンクセット
/*!
    @param bDisable 判定元無効
    @return 最初のイメージ
*/

CLayerImg *CLayerList::setPaintRefLink(CLayerImg *pCurrent,BOOL bDisable)
{
    CLayerItem *p;
    CLayerImg *pStart = NULL,*pBk;

    //判定元有効時

    if(!bDisable)
    {
        for(p = getTopItem(); p; p = p->next())
        {
            if(!p->isPaint()) continue;

            if(!pStart)
                pStart = &p->m_img;
            else
                pBk->setLink(&p->m_img);

            p->m_img.setLink(NULL);
            pBk = &p->m_img;
        }
    }

    //一つも判定元がなければカレント

    if(!pStart)
    {
        pStart = pCurrent;
        pStart->setLink(NULL);
    }

    return pStart;
}

//! キャンバスサイズ変更
/*!
    オフセット位置を調整するのみ
*/

void CLayerList::resizeCanvas(int addx,int addy)
{
    CLayerItem *p;

    for(p = getTopItem(); p; p = p->next())
        p->m_img.setOffset(addx, addy, TRUE);
}



//******************************
// CLayerItem
//******************************

/*!
    @class CLayerItem
    @brief レイヤ一つのリストデータ
*/


//! 作成

BOOL CLayerItem::create(int w,int h,int no,LAYERINFO *pInfo)
{
    BOOL ret;

    //イメージ作成

    if(pInfo)
        ret = m_img.createFromInfo(*pInfo);
    else
        ret = m_img.create(w, h);

    if(!ret) return FALSE;

    //プレビュー用イメージ

    if(!m_imgPrev.create(PREVIMG_W, PREVIMG_H))
        return FALSE;

    //各情報

    init(no);

    return TRUE;
}

//! 指定イメージを複製して作成

BOOL CLayerItem::createFromImg(const CLayerImg &src,int no)
{
    //イメージ

    if(!m_img.copy(src))
        return FALSE;

    //プレビュー用イメージ

    if(!m_imgPrev.create(PREVIMG_W, PREVIMG_H))
        return FALSE;

    //各情報

    init(no);

    return TRUE;
}

//! 複製

BOOL CLayerItem::copy(CLayerItem *pSrc)
{
    //イメージ

    if(!m_img.copy(pSrc->m_img))
        return FALSE;

    //プレビュー用イメージ

    if(!m_imgPrev.create(PREVIMG_W, PREVIMG_H))
        return FALSE;

    //情報

    m_strName   = pSrc->m_strName;
    m_dwCol     = pSrc->m_dwCol;
    m_btAlpha   = pSrc->m_btAlpha;
    m_btFlag    = pSrc->m_btFlag & (~FLAG_MASK);
    m_wAnimCnt  = pSrc->m_wAnimCnt;
    m_btAType   = pSrc->m_btAType;

    m_bPrevUpdate = FALSE;

    return TRUE;
}

//! 情報初期化

void CLayerItem::init(int no)
{
    m_strName = "Layer";
    m_strName += no;

    m_dwCol     = 0;
    m_btAlpha   = 128;
    m_btFlag    = FLAG_VIEW;
    m_wAnimCnt  = 1;
    m_btAType   = 0;

    m_bPrevUpdate = FALSE;
    m_bWorkFlag   = FALSE;
}
