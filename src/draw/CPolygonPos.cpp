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

#include "CPolygonPos.h"


/*!
    @class CPolygonPos
    @brief 多角形描画用の位置計算クラス
*/


CPolygonPos::~CPolygonPos()
{
    free();
}

CPolygonPos::CPolygonPos()
{
    m_pBuf      = NULL;
    m_pEdge     = NULL;
    m_pValBuf   = NULL;
}

//! 解放

void CPolygonPos::free()
{
    AXFree((void **)&m_pBuf);
    AXFree((void **)&m_pEdge);
    AXFree((void **)&m_pValBuf);
}

//! 初期バッファ確保

BOOL CPolygonPos::alloc(int cnt)
{
    free();

    //確保

    m_pBuf = (POSDAT *)AXMalloc(sizeof(POSDAT) * cnt);
    if(!m_pBuf) return FALSE;

    m_nBufCnt   = cnt;
    m_nCnt      = 0;

    return TRUE;
}

//! 点追加時の再確保

BOOL CPolygonPos::_resize()
{
    int cnt;

    if(m_nCnt + 1 <= m_nBufCnt) return TRUE;

    cnt = m_nBufCnt + 50;

    if(!AXRealloc((void **)&m_pBuf, sizeof(POSDAT) * cnt))
        return FALSE;

    m_nBufCnt = cnt;

    return TRUE;
}

//! 描画点追加

BOOL CPolygonPos::add(int x,int y)
{
    POSDAT *p;

    //前の位置と同じなら追加しない

    if(m_nCnt)
    {
        p = m_pBuf + m_nCnt - 1;

        if(p->x == x && (p->y >> 4) == y) return TRUE;
    }

    //確保

    if(!_resize()) return FALSE;

    //追加

    p = m_pBuf + m_nCnt;

    p->x = x;
    p->y = y << 4;

    m_nCnt++;

    //最小・最大XY

    if(m_nCnt == 1)
    {
        m_nMinX = m_nMaxX = x;
        m_nMinY = m_nMaxY = y;
    }
    else
    {
        if(x < m_nMinX) m_nMinX = x;
        if(x > m_nMaxX) m_nMaxX = x;

        if(y < m_nMinY) m_nMinY = y;
        if(y > m_nMaxY) m_nMaxY = y;
    }

    return TRUE;
}

//! 点追加終了

void CPolygonPos::endPos()
{
    //始点を追加

    if(m_nCnt)
        add(m_pBuf->x, m_pBuf->y >> 4);

    //Xの範囲拡張

    m_nMinX--;
    m_nMaxX++;
}


//================================
// 描画時（CLayerImg から呼ばれる）
//================================


//! 交点バッファ初期確保

BOOL CPolygonPos::allocEdge()
{
    //交点バッファ

    m_pEdge = (EDGEDAT *)AXMalloc(sizeof(EDGEDAT) * 50);
    if(!m_pEdge) return FALSE;

    m_nEdgeBufCnt   = 50;
    m_nEdgeCnt      = 0;

    //濃度バッファ

    m_nValBufCnt = m_nMaxX - m_nMinX + 1;

    m_pValBuf = (LPWORD)AXMalloc(m_nValBufCnt * sizeof(WORD));
    if(!m_pValBuf) return FALSE;

    return TRUE;
}

//! アンチエイリアス時、指定Yとの交点リスト作成

BOOL CPolygonPos::findIntersection(int y)
{
    POSDAT *p,*p1,*p2;
    int i,j,y1,y2,bUpper,val[4];
    double dd,yy;

    y = (y << 4) + 2;

    //交点数クリア

    m_nEdgeCnt = 0;

    //各辺から交点検索 ( x = 8bit固定小数 )

    for(i = 0, p = m_pBuf; i < m_nCnt - 1; i++, p++)
    {
        y1 = p[0].y;
        y2 = p[1].y;

        bUpper = (y1 < y2);

        //横線

        if(y1 == y2) continue;

        //辺の範囲外

        if(!((y <= y1 && y >= y2) || (y >= y1 && y <= y2))) continue;

        //入れ替え

        if(y1 < y2)
            p1 = p, p2 = p + 1;
        else
            p1 = p + 1, p2 = p;

        //交点計算 (1/4分をまとめて)

        dd = (double)(p2->x - p1->x) / (p2->y - p1->y) * 256.0;
        yy = y - p1->y;

        for(j = 0; j < 4; j++, yy += 4.0)
            val[j] = (p1->x << 8) + (int)(yy * dd);

        if(!_addEdge(val, bUpper)) return FALSE;
    }

    //交点が2の倍数でない、または交点なし

    if(m_nEdgeCnt == 0 || (m_nEdgeCnt & 1)) return FALSE;

    return TRUE;
}

//! 非アンチエイリアス時、指定Yとの交点リスト作成

BOOL CPolygonPos::findIntersectionNoAnti(int y)
{
    POSDAT *p,*p1,*p2;
    int i,y1,y2,bUpper,val[4] = {0,0,0,0};

    y = (y << 4) + 2;

    //交点数クリア

    m_nEdgeCnt = 0;

    //各辺から交点検索

    for(i = 0, p = m_pBuf; i < m_nCnt - 1; i++, p++)
    {
        y1 = p[0].y;
        y2 = p[1].y;

        bUpper = (y1 < y2);

        //横線

        if(y1 == y2) continue;

        //辺の範囲外

        if(!((y <= y1 && y >= y2) || (y >= y1 && y <= y2))) continue;

        //入れ替え

        if(y1 < y2)
            p1 = p, p2 = p + 1;
        else
            p1 = p + 1, p2 = p;

        //交点追加

        val[0] = p1->x + (int)((double)(y - p1->y) * (p2->x - p1->x) / (p2->y - p1->y));

        if(!_addEdge(val, bUpper)) return FALSE;
    }

    //交点が2の倍数でない、または交点なし

    if(m_nEdgeCnt == 0 || (m_nEdgeCnt & 1)) return FALSE;

    //ソート

    _sortEdge(0);

    return TRUE;
}

//! 交点リストから濃度バッファにセット

void CPolygonPos::setValBuf()
{
    LPWORD pVal;
    EDGEDAT *pe,*pe1;
    int i,j,pos,cnt,x1,x2;
    int x1n,x2n,x1f,x2f;

    //濃度バッファクリア

    AXMemZero(m_pValBuf, m_nValBufCnt * sizeof(WORD));

    //--------- バッファセット

    for(pos = 0; pos < 4; pos++)
    {
        //交点ソート

        _sortEdge(pos);

        //交点間の値セット

        for(i = 0, pe = m_pEdge; i < m_nEdgeCnt - 1; i++, pe++)
        {
            //カウント

            for(j = 0, cnt = 0, pe1 = m_pEdge; j <= i; j++, pe1++)
            {
                if(pe1->bUpper) cnt++; else cnt--;
            }

            if(cnt == 0) continue;

            //セット

            x1 = pe[0].x[pos];
            x2 = pe[1].x[pos];

            if(x1 == x2) continue;

            x1 -= (m_nMinX << 8);
            x2 -= (m_nMinX << 8);

            x1n = x1 >> 8  , x2n = x2 >> 8;
            x1f = x1 & 0xff, x2f = x2 & 0xff;

            if(x1n == x2n)
                //1px内
                m_pValBuf[x1n] += x2f - x1f;
            else
            {
                //2px以上

                m_pValBuf[x1n] += 255 - x1f;
                m_pValBuf[x2n] += x2f;

                cnt = x2n - x1n - 1;

                if(cnt > 0)
                {
                    pVal = m_pValBuf + x1n + 1;

                    for(j = cnt; j > 0; j--, pVal++)
                        *pVal += 255;
                }
            }
        }
    }

    //--------- バッファ 平均

    pVal = m_pValBuf;

    for(i = m_nValBufCnt; i > 0; i--, pVal++)
        *pVal = *pVal >> 2;
}

//! 交点追加

BOOL CPolygonPos::_addEdge(LPINT pVal,BOOL bUpper)
{
    //拡張

    if(m_nEdgeCnt + 1 > m_nEdgeBufCnt)
    {
        if(!AXRealloc((void **)&m_pEdge, sizeof(EDGEDAT) * (m_nEdgeBufCnt + 50)))
            return FALSE;

        m_nEdgeBufCnt += 50;
    }

    //追加

    m_pEdge[m_nEdgeCnt].x[0]    = pVal[0];
    m_pEdge[m_nEdgeCnt].x[1]    = pVal[1];
    m_pEdge[m_nEdgeCnt].x[2]    = pVal[2];
    m_pEdge[m_nEdgeCnt].x[3]    = pVal[3];
    m_pEdge[m_nEdgeCnt].bUpper  = bUpper;

    m_nEdgeCnt++;

    return TRUE;
}

//! 交点を小さい順に並び替え

void CPolygonPos::_sortEdge(int pos)
{
    int i,j,min;
    EDGEDAT *p = m_pEdge,*p2,*pRep,edge;

    for(i = 0; i < m_nEdgeCnt - 1; i++, p++)
    {
        min  = p->x[pos];
        pRep = p;

        //一番小さい値取得

        p2 = p + 1;

        for(j = i + 1; j < m_nEdgeCnt; j++, p2++)
        {
            if(p2->x[pos] < min)
                min = p2->x[pos], pRep = p2;
        }

        //入れ替え

        if(p != pRep)
        {
            edge  = *p;
            *p    = *pRep;
            *pRep = edge;
        }
    }
}
