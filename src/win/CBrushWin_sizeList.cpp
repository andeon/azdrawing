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

#include "CBrushWin_sizeList.h"

#include "CConfig.h"
#include "CBrushWin.h"

#include "AXLayout.h"
#include "AXScrollBar.h"
#include "AXImage.h"
#include "AXMenu.h"
#include "AXStrDialog.h"
#include "AXApp.h"
#include "AXUtilStr.h"

#include "defGlobal.h"
#include "defStrID.h"


//----------------------------

class CBrushWin_sizeListArea:public AXWindow
{
protected:
    enum
    {
        ONESIZE    = 27,

        DOWNF_NONE = 0,
        DOWNF_NORMAL,
        DOWNF_DEL
    };

protected:
    AXScrollBar *m_pScr;

    AXImage     m_img;

    int     m_nXCnt,
            m_nDownNo,
            m_fDown;

protected:
    int _getListNo(int x,int y);
    void _addDat(BOOL bSize);
    void _deleteDat(int no);
    void _showMenu(int x,int y,int no);
    void _drawSel(BOOL bErase);
    void _drawBrushPrev(int x,int y,int w,int h,int bsize);

public:
    CBrushWin_sizeListArea(AXWindow *pParent);

    void setScroll(AXScrollBar *p) { m_pScr = p; }
    void setScrollInfo();
    void updateAll();

    void draw();

    virtual BOOL onPaint(AXHD_PAINT *phd);
    virtual BOOL onSize();
    virtual BOOL onButtonDown(AXHD_MOUSE *phd);
    virtual BOOL onButtonUp(AXHD_MOUSE *phd);
};

//----------------------------


//************************************
// CBrushWin_sizeList
//************************************


/*!
    @class CBrushWin_sizeList
    @brief [ブラシウィンドウ]サイズと濃度のリストウィジェット
*/


CBrushWin_sizeList::CBrushWin_sizeList(AXWindow *pParent)
    : AXWindow(pParent, 0, LF_EXPAND_W | LF_FIX_H)
{
    AXLayout *pl;

    m_nH = g_pconf->nBrushWinH[0];

    //

    setLayout(pl = new AXLayoutHorz);

    pl->addItem(m_pArea = new CBrushWin_sizeListArea(this));
    pl->addItem(m_pScrV = new AXScrollBar(this, AXScrollBar::SBS_VERT, LF_EXPAND_H));

    m_pArea->setScroll(m_pScrV);
}

//! 通知

BOOL CBrushWin_sizeList::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    if(pwin == m_pScrV)
        m_pArea->draw();

    return TRUE;
}



//************************************
// CBrushWin_sizeListArea
//************************************



CBrushWin_sizeListArea::CBrushWin_sizeListArea(AXWindow *pParent)
    : AXWindow(pParent, 0, LF_EXPAND_WH)
{
    m_fDown = DOWNF_NONE;
}

//! スクロール情報セット

void CBrushWin_sizeListArea::setScrollInfo()
{
    int max,page;

    max = g_pconf->getBrushSizeCnt();
    max = (max + m_nXCnt - 1) / m_nXCnt;

    page = m_nH / ONESIZE;
    if(page <= 0) page = 1;

    m_pScr->setStatus(0, max, page);
}

//! すべて更新

void CBrushWin_sizeListArea::updateAll()
{
    setScrollInfo();
    draw();
}


//========================
//ハンドラ
//========================


//! 描画

BOOL CBrushWin_sizeListArea::onPaint(AXHD_PAINT *phd)
{
    m_img.put(m_id, phd->x, phd->y, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}

//! サイズ変更時

BOOL CBrushWin_sizeListArea::onSize()
{
    m_img.recreate(m_nW, m_nH, ONESIZE, ONESIZE);

    //横の表示数

    m_nXCnt = (m_nW - 1) / ONESIZE;
    if(m_nXCnt < 1) m_nXCnt = 1;

    updateAll();

    return TRUE;
}

//! ボタン押し時

BOOL CBrushWin_sizeListArea::onButtonDown(AXHD_MOUSE *phd)
{
    int no;
    LPWORD p;

    if(m_fDown) return TRUE;

    no = _getListNo(phd->x, phd->y);

    if(phd->button == BUTTON_RIGHT)
        //右クリックメニュー
        _showMenu(phd->rootx, phd->rooty, no);
    else if(phd->button == BUTTON_LEFT)
    {
        //左ボタン（+Shift で削除）

        if(no != -1)
        {
            m_fDown   = (phd->state & STATE_SHIFT)? DOWNF_DEL: DOWNF_NORMAL;
            m_nDownNo = no;

            //サイズ/濃度セット

            if(m_fDown == DOWNF_NORMAL)
            {
                p = (LPWORD)g_pconf->memBrushSize + no;

                if(*p & 0x8000)
                    BRUSHWIN->setBrushVal(*p & 0xff);
                else
                    BRUSHWIN->setBrushSize(*p &0x7fff);
            }

            //

            _drawSel(FALSE);

            grabPointer();
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL CBrushWin_sizeListArea::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fDown && phd->button == BUTTON_LEFT)
    {
        if(m_fDown == DOWNF_DEL)
            _deleteDat(m_nDownNo);
        else
            _drawSel(TRUE);

        //

        m_fDown = DOWNF_NONE;
        ungrabPointer();
    }

    return TRUE;
}


//========================
//サブ
//========================


//! カーソル位置のリスト番号取得

int CBrushWin_sizeListArea::_getListNo(int x,int y)
{
    int no;

    x /= ONESIZE;
    y /= ONESIZE;

    if(x >= m_nXCnt) return -1;

    no = (y + m_pScr->getPos()) * m_nXCnt + x;

    if(no >= g_pconf->getBrushSizeCnt()) return -1;

    return no;
}

//! サイズ・濃度データ追加

void CBrushWin_sizeListArea::_addDat(BOOL bSize)
{
    AXString str;
    LPCUSTR pc;
    LPWORD p;
    int val,i,n,cnt;

    if(!AXStrDialog::getString(m_pTopLevel, NULL, _str(STRID_BSDLG_MESSAGE), &str))
        return;

    //'/'で区切って複数

    str += '/';
    str.replace('/', 0);

    for(pc = str; *pc; pc += AXUStrLen(pc) + 1)
    {
        //値取得

        if(bSize)
        {
            val = (int)(AXUStrToDouble(pc) * 10 + 0.5);
            if(val < 5) val = 5; else if(val > 6000) val = 6000;
        }
        else
        {
            val = AXUStrToInt(pc);
            if(val < 1) val = 1; else if(val > 255) val = 255;
        }

        //挿入位置（サイズ -> 濃度。値の小さい順）

        p   = g_pconf->memBrushSize;
        cnt = g_pconf->getBrushSizeCnt();

        for(i = 0; i < cnt; i++)
        {
            n = *(p++);

            //[サイズ] 濃度が来たらそこへ
            if(bSize && (n & 0x8000)) break;
            //[濃度] サイズは飛ばす
            if(!bSize && !(n & 0x8000)) continue;

            //同じタイプなら値比較（指定値より小さければ続ける）
            //※同じ値がある場合は追加しない

            n &= 0x7fff;

            if(n == val) return;
            if(n > val) break;
        }

        n = i;

        //データ追加（仮）

        g_pconf->memBrushSize.addWORD(0);

        //挿入位置を空けてセット

        p = g_pconf->memBrushSize;

        for(i = cnt; i > n; i--)
            p[i] = p[i - 1];

        if(!bSize) val |= 0x8000;
        p[n] = val;
    }

    //

    updateAll();
}

//! 指定位置のデータ削除

void CBrushWin_sizeListArea::_deleteDat(int no)
{
    LPWORD p = g_pconf->memBrushSize;
    int i,cnt;

    cnt = g_pconf->getBrushSizeCnt() - 1;

    for(i = no; i < cnt; i++)
        p[i] = p[i + 1];

    g_pconf->memBrushSize.setNowSize(cnt * sizeof(WORD));

    updateAll();
}

//! メニュー表示

void CBrushWin_sizeListArea::_showMenu(int x,int y,int no)
{
    AXMenu *pm;
    int ret;

    m_nDownNo = no;

    //選択枠描画

    _drawSel(FALSE);

    //メニュー

    pm = new AXMenu;

    _trgroup(STRGID_BRUSHSIZELIST);

    pm->addTr(STRID_BSMENU_ADD_SIZE);
    pm->addTr(STRID_BSMENU_ADD_VAL);

    if(no != -1)
    {
        pm->addSep();
        pm->addTr(STRID_BSMENU_DEL);
    }

    ret = (int)pm->popup(NULL, x, y, 0);

    delete pm;

    //選択枠戻す

    _drawSel(TRUE);

    //処理

    if(ret == -1) return;

    if(ret == STRID_BSMENU_DEL)
        //削除
        _deleteDat(no);
    else
        //追加
        _addDat((ret == STRID_BSMENU_ADD_SIZE));
}

//! 描画

void CBrushWin_sizeListArea::draw()
{
    LPWORD p;
    int x,y,ix,pos,cnt,val,col;
    char m[16];

    //背景

    m_img.clear(0xbbbbbb);

    //

    cnt = g_pconf->getBrushSizeCnt();
    pos = m_pScr->getPos() * m_nXCnt;
    p   = (LPWORD)g_pconf->memBrushSize + pos;

    //

    for(y = 0; y < m_nH; y += ONESIZE)
    {
        for(ix = 0, x = 0; ix < m_nXCnt; ix++, pos++, p++, x += ONESIZE)
        {
            if(pos >= cnt) goto END;

            val = *p & 0x7fff;

            if(*p & 0x8000)
            {
                //濃度

                col = 255 - val;
                col |= (col << 8) | (col << 16);

                m_img.fillBox(x + 1, y + 1, ONESIZE - 1, ONESIZE - 1, 0xffffff);
                m_img.fillBox(x + 4, y + 4, ONESIZE - 8, 11, col);

                AXIntToStr(m, val);
                m_img.drawNumber(x + 3, y + ONESIZE - 10, m, 0x008000);
            }
            else
            {
                //ブラシサイズ

                m_img.box(x + 1, y + 1, ONESIZE - 1, ONESIZE - 1, 0xffffff);

                _drawBrushPrev(x + 2, y + 2, ONESIZE - 3, ONESIZE - 3, (val > 240)? 240: val);

                AXIntToFloatStr(m, val, 1);
                m_img.drawNumber(x + 2, y + 2, m, 0x0000ff);
            }
        }
    }

END:
    redraw();
}

//! 選択用枠描画

void CBrushWin_sizeListArea::_drawSel(BOOL bErase)
{
    int x,y;

    if(m_nDownNo == -1) return;

    x = (m_nDownNo % m_nXCnt) * ONESIZE;
    y = (m_nDownNo / m_nXCnt - m_pScr->getPos()) * ONESIZE;

    m_img.box(x, y, ONESIZE + 1, ONESIZE + 1, (bErase)? 0xbbbbbb: 0xff0000);

    redraw(x, y, ONESIZE + 1, ONESIZE + 1);
}

//! ブラシサイズプレビュー描画

void CBrushWin_sizeListArea::_drawBrushPrev(int x,int y,int w,int h,int bsize)
{
    int pitch,bpp;
    int ix,iy,jx,jy,cx,cy,f,rr,val;
    int xx[4],yy[4];
    LPBYTE p;

    p     = m_img.getBufPt(x, y);
    bpp   = m_img.getBytes();
    pitch = m_img.getPitch() - bpp * w;

    cx = w * 2;
    cy = h * 2;

    rr = bsize * 4 / 20;
    rr *= rr;

    for(iy = 0; iy < h; iy++)
    {
        f = (iy << 2) - cy;

        for(jy = 0; jy < 4; jy++, f++)
            yy[jy] = f * f;

        //

        for(ix = 0; ix < w; ix++, p += bpp)
        {
            f = (ix << 2) - cx;

            for(jx = 0; jx < 4; jx++, f++)
                xx[jx] = f * f;

            //4x4

            val = 0;

            for(jy = 0; jy < 4; jy++)
            {
                for(jx = 0; jx < 4; jx++)
                {
                    if(xx[jx] + yy[jy] < rr)
                        val += 255;
                }
            }

            //

            val >>= 4;

            if(iy < 8) val >>= 2;   //文字にかかる部分は色を薄くする

            val = 255 - val;

            m_img.setPixelBuf(p, val, val, val);
        }

        p += pitch;
    }
}
