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
/*
    CDrawPaint [run] - 各塗りつぶし処理
*/


#include "CDrawPaint.h"



//==========================
//指定濃度未満の範囲
//==========================


//! 実行

void CDrawPaint::_runLess()
{
    int lx,rx,ly,oy,_lx,_rx;
    PAINTBUF *pst,*ped;

    pst = m_pBuf;
    ped = m_pBuf + 1;

    pst->lx = pst->rx = m_ptStart.x;
    pst->y  = pst->oy = m_ptStart.y;

    //

    do
    {
        lx = pst->lx;
        rx = pst->rx;
        ly = pst->y;
        oy = pst->oy;

        _lx = lx - 1;
        _rx = rx + 1;

        if(++pst == m_pBuf + BUFSIZE) pst = m_pBuf;

        //処理済か

        _setCurrent(lx, ly);

        if(_getCurPixel() >= m_nCompVal) continue;

        //右方向の境界探す

        _setCurrent(rx + 1, ly);

        for(; rx < m_rcArea.right; rx++)
        {
            if(_getCurPixel() >= m_nCompVal) break;

            _movCurRight();
        }

        //左方向の境界探す

        _setCurrent(lx - 1, ly);

        for(; lx > m_rcArea.left; lx--)
        {
            if(_getCurPixel() >= m_nCompVal) break;

            _movCurLeft();
        }

        //lx-rx の水平線描画
        //※判定元に描画できなければエラー

        if(!_drawRefLineH(lx, rx, ly, 255)) return;

        _drawDrawLineH(lx, rx, ly, 255);

        //真上の走査

        if(ly - 1 >= m_rcArea.top)
        {
            if(ly - 1 == oy)
            {
                _less_scan(lx, _lx, ly - 1, ly, &ped);
                _less_scan(_rx, rx, ly - 1, ly, &ped);
            }
            else
                _less_scan(lx, rx, ly - 1, ly, &ped);
        }

        //真下の走査

        if(ly + 1 <= m_rcArea.bottom)
        {
            if(ly + 1 == oy)
            {
                _less_scan(lx, _lx, ly + 1, ly, &ped);
                _less_scan(_rx, rx, ly + 1, ly, &ped);
            }
            else
                _less_scan(lx, rx, ly + 1, ly, &ped);
        }

    }while(pst != ped);
}

//! スキャン

void CDrawPaint::_less_scan(int lx,int rx,int y,int oy,PAINTBUF **pped)
{
    _setCurrent(lx, y);

    while(lx <= rx)
    {
        //開始点

        for(; lx < rx; lx++)
        {
            if(_getCurPixel() < m_nCompVal) break;
            _movCurRight();
        }

        if(_getCurPixel() >= m_nCompVal) break;

        (*pped)->lx = lx;

        //終了点

        for(; lx <= rx; lx++)
        {
            if(_getCurPixel() >= m_nCompVal) break;
            _movCurRight();
        }

        (*pped)->rx = lx - 1;
        (*pped)->y  = y;
        (*pped)->oy = oy;

        if(++(*pped) == m_pBuf + BUFSIZE)
            (*pped) = m_pBuf;
    }
}


//==========================
//不透明範囲消し
//==========================
/*
    ※判定元画像に直接描画する
*/


//! 実行

void CDrawPaint::_runErase()
{
    int lx,rx,ly,oy,_lx,_rx,i;
    PAINTBUF *pst,*ped;

    pst = m_pBuf;
    ped = m_pBuf + 1;

    pst->lx = pst->rx = m_ptStart.x;
    pst->y  = pst->oy = m_ptStart.y;

    //

    do
    {
        lx = pst->lx;
        rx = pst->rx;
        ly = pst->y;
        oy = pst->oy;

        _lx = lx - 1;
        _rx = rx + 1;

        if(++pst == m_pBuf + BUFSIZE) pst = m_pBuf;

        //透明か

        _setCurrent(lx, ly);

        if(_getCurPixel() == 0) continue;

        //右方向の境界探す

        _setCurrent(rx + 1, ly);

        for(; rx < m_rcArea.right; rx++)
        {
            if(_getCurPixel() == 0) break;
            _movCurRight();
        }

        //左方向の境界探す

        _setCurrent(lx - 1, ly);

        for(; lx > m_rcArea.left; lx--)
        {
            if(_getCurPixel() == 0) break;
            _movCurLeft();
        }

        //lx-rx の水平線描画
        //※判定元に描画できなければエラー

        if(!_drawRefLineH(lx, rx, ly, 0)) return;

        for(i = lx; i <= rx; i++)
            m_pimgDst->setPixelDraw(i, ly, 255);

        //真上の走査

        if(ly - 1 >= m_rcArea.top)
        {
            if(ly - 1 == oy)
            {
                _erase_scan(lx, _lx, ly - 1, ly, &ped);
                _erase_scan(_rx, rx, ly - 1, ly, &ped);
            }
            else
                _erase_scan(lx, rx, ly - 1, ly, &ped);
        }

        //真下の走査

        if(ly + 1 <= m_rcArea.bottom)
        {
            if(ly + 1 == oy)
            {
                _erase_scan(lx, _lx, ly + 1, ly, &ped);
                _erase_scan(_rx, rx, ly + 1, ly, &ped);
            }
            else
                _erase_scan(lx, rx, ly + 1, ly, &ped);
        }

    }while(pst != ped);
}

//! スキャン

void CDrawPaint::_erase_scan(int lx,int rx,int y,int oy,PAINTBUF **pped)
{
    _setCurrent(lx, y);

    while(lx <= rx)
    {
        //開始点

        for(; lx < rx; lx++)
        {
            if(_getCurPixel()) break;
            _movCurRight();
        }

        if(_getCurPixel() == 0) break;

        (*pped)->lx = lx;

        //終了点

        for(; lx <= rx; lx++)
        {
            if(_getCurPixel() == 0) break;
            _movCurRight();
        }

        (*pped)->rx = lx - 1;
        (*pped)->y  = y;
        (*pped)->oy = oy;

        if(++(*pped) == m_pBuf + BUFSIZE)
            (*pped) = m_pBuf;
    }
}


//===============================
//アンチエイリアス自動判定
//===============================
/*
    境界を検索する時に最大濃度を記憶しておき、濃度が下がった点を境界とする。
    これを水平方向、垂直方向の両方で実行する。
*/


//! 実行

void CDrawPaint::_runAuto()
{
    //水平走査

    _runAuto_horz();

    //垂直走査

    m_imgRef.freeAllTile();

    _runAuto_vert();
}

//! 水平走査

void CDrawPaint::_runAuto_horz()
{
    int lx,rx,ly,oy,_lx,_rx,c,max,lx2,rx2,flag;
    PAINTBUF *pst,*ped;

    pst = m_pBuf;
    ped = m_pBuf + 1;

    pst->lx = pst->rx = m_ptStart.x;
    pst->y  = pst->oy = m_ptStart.y;

    //

    do
    {
        lx = pst->lx;
        rx = pst->rx;
        ly = pst->y;
        oy = pst->oy;

        _lx = lx - 1;
        _rx = rx + 1;

        if(++pst == m_pBuf + BUFSIZE) pst = m_pBuf;

        //処理済か

        _setCurrent(lx, ly);

        if(_getCurPixel() != m_nStartVal) continue;

        //右方向の境界探す

        rx2 = rx;
        _setCurrent(rx + 1, ly);

        for(max = m_nStartVal, flag = 1; rx < m_rcArea.right; rx++)
        {
            c = _getCurPixel();

            if(c != m_nStartVal) flag = 0;
            if(c >= m_nVal || c < max) break;

            _movCurRight();
            max = c;
            if(flag) rx2++;
        }

        //左方向の境界探す

        lx2 = lx;
        _setCurrent(lx - 1, ly);

        for(max = m_nStartVal, flag = 1; lx > m_rcArea.left; lx--)
        {
            c = _getCurPixel();

            if(c != m_nStartVal) flag = 0;
            if(c >= m_nVal || c < max) break;

            _movCurLeft();
            max = c;
            if(flag) lx2--;
        }

        //lx-rx の水平線描画
        //※判定元に描画できなければエラー

        if(!_drawRefLineH(lx, rx, ly, 255)) return;

        _drawDrawLineH(lx, rx, ly, 255);

        //真上の走査

        if(ly - 1 >= m_rcArea.top)
        {
            if(ly - 1 == oy)
            {
                _auto_scan_h(lx2, _lx, ly - 1, ly, &ped);
                _auto_scan_h(_rx, rx2, ly - 1, ly, &ped);
            }
            else
                _auto_scan_h(lx2, rx2, ly - 1, ly, &ped);
        }

        //真下の走査

        if(ly + 1 <= m_rcArea.bottom)
        {
            if(ly + 1 == oy)
            {
                _auto_scan_h(lx2, _lx, ly + 1, ly, &ped);
                _auto_scan_h(_rx, rx2, ly + 1, ly, &ped);
            }
            else
                _auto_scan_h(lx2, rx2, ly + 1, ly, &ped);
        }

    }while(pst != ped);
}

//! 水平スキャン

void CDrawPaint::_auto_scan_h(int lx,int rx,int y,int oy,PAINTBUF **pped)
{
    _setCurrent(lx, y);

    while(lx <= rx)
    {
        //開始点

        for(; lx < rx; lx++)
        {
            if(_getCurPixel() == m_nStartVal) break;
            _movCurRight();
        }

        if(_getCurPixel() != m_nStartVal) break;

        (*pped)->lx = lx;

        //終了点

        for(; lx <= rx; lx++)
        {
            if(_getCurPixel() != m_nStartVal) break;
            _movCurRight();
        }

        (*pped)->rx = lx - 1;
        (*pped)->y  = y;
        (*pped)->oy = oy;

        if(++(*pped) == m_pBuf + BUFSIZE)
            (*pped) = m_pBuf;
    }
}

//! 垂直走査

void CDrawPaint::_runAuto_vert()
{
    int ly,ry,xx,ox,_ly,_ry,c,max,ly2,ry2,flag;
    PAINTBUF *pst,*ped;

    pst = m_pBuf;
    ped = m_pBuf + 1;

    pst->lx = pst->rx = m_ptStart.y;
    pst->y  = pst->oy = m_ptStart.x;

    //

    do
    {
        ly = pst->lx;
        ry = pst->rx;
        xx = pst->y;
        ox = pst->oy;

        _ly = ly - 1;
        _ry = ry + 1;

        if(++pst == m_pBuf + BUFSIZE) pst = m_pBuf;

        //処理済か

        _setCurrent(xx, ly);

        if(_getCurPixel() != m_nStartVal) continue;

        //下方向の境界探す

        ry2 = ry;
        _setCurrent(xx, ry + 1);

        for(max = m_nStartVal, flag = 1; ry < m_rcArea.bottom; ry++)
        {
            c = _getCurPixel();

            if(c != m_nStartVal) flag = 0;
            if(c >= m_nVal || c < max) break;

            _movCurDown();
            max = c;
            if(flag) ry2++;
        }

        //上方向の境界探す

        ly2 = ly;
        _setCurrent(xx, ly - 1);

        for(max = m_nStartVal, flag = 1; ly > m_rcArea.top; ly--)
        {
            c = _getCurPixel();

            if(c != m_nStartVal) flag = 0;
            if(c >= m_nVal || c < max) break;

            _movCurUp();
            max = c;
            if(flag) ly2--;
        }

        //ly-ry の垂直線描画
        //※判定元に描画できなければエラー

        if(!_drawRefLineV(ly, ry, xx, 255)) return;

        _drawDrawLineV(ly, ry, xx, 255);

        //左の走査

        if(xx - 1 >= m_rcArea.left)
        {
            if(xx - 1 == ox)
            {
                _auto_scan_v(ly2, _ly, xx - 1, xx, &ped);
                _auto_scan_v(_ry, ry2, xx - 1, xx, &ped);
            }
            else
                _auto_scan_v(ly2, ry2, xx - 1, xx, &ped);
        }

        //右の走査

        if(xx + 1 <= m_rcArea.right)
        {
            if(xx + 1 == ox)
            {
                _auto_scan_v(ly2, _ly, xx + 1, xx, &ped);
                _auto_scan_v(_ry, ry2, xx + 1, xx, &ped);
            }
            else
                _auto_scan_v(ly2, ry2, xx + 1, xx, &ped);
        }

    }while(pst != ped);
}

//! 垂直スキャン

void CDrawPaint::_auto_scan_v(int ly,int ry,int x,int ox,PAINTBUF **pped)
{
    _setCurrent(x, ly);

    while(ly <= ry)
    {
        //開始点

        for(; ly < ry; ly++)
        {
            if(_getCurPixel() == m_nStartVal) break;
            _movCurDown();
        }

        if(_getCurPixel() != m_nStartVal) break;

        (*pped)->lx = ly;

        //終了点

        for(; ly <= ry; ly++)
        {
            if(_getCurPixel() != m_nStartVal) break;
            _movCurDown();
        }

        (*pped)->rx = ly - 1;
        (*pped)->y  = x;
        (*pped)->oy = ox;

        if(++(*pped) == m_pBuf + BUFSIZE)
            (*pped) = m_pBuf;
    }
}
