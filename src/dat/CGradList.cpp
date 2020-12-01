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

#include "CGradList.h"

#include "AXImage.h"
#include "AXConfWrite.h"
#include "AXConfRead.h"
#include "AXByteString.h"


//--------------------

#define GRAD_GETPOS(v)  ((v) & CGradListItem::POS_MASK)
#define GRAD_GETVAL(v)  ((v) >> CGradListItem::VAL_SHIFT)

//--------------------


//*************************************
// CGradList
//*************************************


/*!
    @class CGradList
    @brief グラデーションリストクラス
*/

/*
    データが変更された場合 m_bUpdate = TRUE とする。
    m_bUpdate が FALSE の場合は終了時にデータファイルを保存しない。

    選択情報はデータとは別に通常の設定ファイルに保存される。

    [ポイントデータ] int
    下位16bit: 位置(0-1024)
    16-23bit: 濃度(0-255)
*/


CGradList::CGradList()
{
    CGradListItem *p;

    m_bUpdate = FALSE;

    //デフォルトのデータは常に先頭

    AXList::add(p = new CGradListItem);
    p->setDefault();

    m_pSelItem = p;
}

//! グラデーションデータ追加
/*!
    新規追加して選択する
*/

BOOL CGradList::addGrad()
{
    CGradListItem *p;

    if(m_nCnt >= MAXCNT) return FALSE;

    AXList::add(p = new CGradListItem);
    p->setNewDat();

    m_pSelItem = p;

    m_bUpdate = TRUE;

    return TRUE;
}

//! 現在のグラデーションデータを削除

BOOL CGradList::delGrad()
{
    CGradListItem *p;

    //デフォルトデータは削除しない

    if(!m_pSelItem->prev()) return FALSE;

    //削除

    p = m_pSelItem->next();
    if(!p) p = m_pSelItem->prev();

    AXList::deleteItem(m_pSelItem);

    m_pSelItem = p;

    m_bUpdate = TRUE;

    return TRUE;
}

//! 描画時の濃度テーブルをセット（選択されているデータで）

void CGradList::setGradTable(LPBYTE pBuf,int cnt,int val,BOOL bRev)
{
    LPBYTE pd,pd2;
    LPINT pg;
    int i,c,pos,gpos;

    pd = pBuf;
    pg = m_pSelItem->m_memDat;

    for(i = 0; i < cnt; i++, pd++)
    {
        pos = (i << CGradListItem::POS_BIT) / (cnt - 1);

        while(pos > GRAD_GETPOS(pg[1])) pg++;

        gpos = GRAD_GETPOS(*pg);

        c = GRAD_GETVAL(pg[1]) - GRAD_GETVAL(*pg);
        c = c * (pos - gpos) / (GRAD_GETPOS(pg[1]) - gpos) + GRAD_GETVAL(*pg);

        *pd = c * val / 255;
    }

    //左右反転

    if(bRev)
    {
        pd  = pBuf;
        pd2 = pBuf + cnt - 1;

        for(i = cnt / 2; i > 0; i--, pd++, pd2--)
        {
            c    = *pd;
            *pd  = *pd2;
            *pd2 = c;
        }
    }
}

//! データ番号から選択

void CGradList::setSelItemFromNo(int no)
{
    m_pSelItem = (CGradListItem *)AXList::getItem(no);
    if(!m_pSelItem) m_pSelItem = getTopItem();
}

//! ファイルに保存

void CGradList::saveFile(const AXString &filename)
{
    AXConfWrite cf;
    CGradListItem *p;
    AXByteString strb;
    int i;

    //データが更新されていない場合は保存しない

    if(!m_bUpdate) return;

    //※デフォルトのデータは保存しない

    if(!cf.open(filename)) return;

    cf.putGroup("gradient");

    cf.putInt("ver", 1);
    cf.putInt("cnt", m_nCnt - 1);

    for(p = getTopItem()->next(), i = 1; p; p = p->next(), i++)
    {
        strb.setAddInt("name", i);
        cf.putStr(strb, p->m_strName);

        strb.setAddInt("dat", i);
        cf.putArrayHex(strb, p->m_memDat, p->m_memDat.getSize() / sizeof(int));
    }

    cf.close();
}

//! ファイルから読み込み

void CGradList::readFile(const AXString &filename)
{
    AXConfRead cf;
    int i,cnt,n;
    CGradListItem *p;
    AXByteString strb;
    AXMem mem;

    if(!mem.alloc(sizeof(int) * CGradListItem::POINT_MAX)) return;

    //

    if(!cf.loadFile(filename)) return;

    cf.setGroup("gradient");

    if(cf.getInt("ver", 0) != 1) return;

    cnt = cf.getInt("cnt", 0);

    for(i = 0; i < cnt; i++)
    {
        AXList::add(p = new CGradListItem);

        strb.setAddInt("name", i + 1);
        cf.getStr(strb, &p->m_strName);

        strb.setAddInt("dat", i + 1);
        n = cf.getArrayVal(strb, mem, CGradListItem::POINT_MAX, 4, TRUE);

        p->setDat(mem, n);
    }
}


//*************************************
// CGradListItem
//*************************************


//! デフォルトデータセット（255 -> 0）

void CGradListItem::setDefault()
{
    LPINT p;

    m_strName = "default";

    m_memDat.alloc(sizeof(int) * 2);

    p = m_memDat;
    p[0] = 255 << VAL_SHIFT;
    p[1] = POS_MAX;
}

//! 新規時のデータセット

void CGradListItem::setNewDat()
{
    LPINT p;

    m_strName = "new";

    m_memDat.alloc(sizeof(int) * 2);

    p = m_memDat;
    p[0] = 255 << VAL_SHIFT;
    p[1] = POS_MAX;
}

//! データセット

void CGradListItem::setDat(LPINT pDat,int ptcnt)
{
    if(ptcnt < 2)
    {
        //最低２点がない場合デフォルト

        LPINT p;

        m_memDat.alloc(sizeof(int) * 2);

        p = m_memDat;
        p[0] = 255 << VAL_SHIFT;
        p[1] = POS_MAX;
    }
    else
    {
        if(m_memDat.alloc(sizeof(int) * ptcnt))
            m_memDat.copyFrom(pDat, sizeof(int) * ptcnt);
    }
}

//! プレビューイメージ描画
/*!
    @param w,h  0 でイメージ全体
*/

void CGradListItem::drawPrev(AXImage *pimg,int x,int y,int w,int h)
{
    drawPrevDat(pimg, x, y, w, h, m_memDat);
}

//! （関数）指定データからグラデーション描画

void CGradListItem::drawPrevDat(AXImage *pimg,int x,int y,int w,int h,LPINT pDat)
{
    LPINT pg;
    LPBYTE pDst,pDst2;
    int ix,iy,pos,gpos,dw,dh,c,pitch,bpp,xn,r,g,b,n;
    BYTE col[2][3];

    pg = pDat;

    pDst = pimg->getBufPt(x, y);
    dw   = pimg->getWidth();
    dh   = pimg->getHeight();
    bpp   = pimg->getBytes();
    pitch = pimg->getPitch();

    if(w == 0) w = dw;
    if(h == 0) h = dh;

    //チェック柄RGB

    col[0][0] = 0xff, col[0][1] = 0xe8, col[0][2] = 0xeb;
    col[1][0] = 0xff, col[1][1] = 0xc8, col[1][2] = 0xce;

    //グラデーション描画
    //背景4x4グリッドのチェック柄と合成する（色は黒）

    for(ix = 0; ix < w; ix++, pDst += bpp)
    {
        pos = (ix << POS_BIT) / (w - 1);

        while(pos > GRAD_GETPOS(pg[1])) pg++;

        gpos = GRAD_GETPOS(*pg);

        c = GRAD_GETVAL(pg[1]) - GRAD_GETVAL(*pg);
        c = c * (pos - gpos) / (GRAD_GETPOS(pg[1]) - gpos) + GRAD_GETVAL(*pg);

        //縦に描画

        xn = (ix & 7) >> 2;

        for(iy = 0, pDst2 = pDst; iy < h; iy++, pDst2 += pitch)
        {
            n = xn ^ ((iy & 7) >> 2);

            r = col[n][0];
            g = col[n][1];
            b = col[n][2];

            r -= r * c >> 8;
            g -= g * c >> 8;
            b -= b * c >> 8;

            pimg->setPixelBuf(pDst2, r, g, b);
        }
    }
}
