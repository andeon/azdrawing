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

#include <stdio.h>

#include "CImgViewWin.h"

#include "CArrowMenuBtt.h"
#include "CImage32.h"
#include "CBrushWin.h"
#include "CDraw.h"

#include "AXDir.h"
#include "AXList.h"
#include "AXFileStat.h"
#include "AXImage.h"
#include "AXMenu.h"
#include "AXFileDialog.h"
#include "AXDND.h"
#include "AXApp.h"
#include "AXKey.h"

#include "CConfig.h"
#include "defGlobal.h"
#include "defStrID.h"


//-------------------------------

#define IMGV_BKCOL  0xb0b0b0

class CImgVListItem:public AXListItem
{
public:
    AXString    m_strFileName;

    CImgVListItem(const AXString &filename);
    CImgVListItem *next() const { return (CImgVListItem *)m_pNext; }
    CImgVListItem *prev() const { return (CImgVListItem *)m_pPrev; }
};

CImgVListItem::CImgVListItem(const AXString &filename)
{
    m_strFileName = filename;
}

//-------------------------------


/*!
    @class CImgViewWin
    @brief イメージビューウィンドウ
*/


CImgViewWin *CImgViewWin::m_pSelf = NULL;


CImgViewWin::~CImgViewWin()
{
    delete m_pimg;
    delete m_pimgSrc;
}

CImgViewWin::CImgViewWin(AXWindow *pOwner,UINT addstyle)
    : AXTopWindow(pOwner,
                  WS_HIDE | WS_DROP | WS_TITLE | WS_CLOSE | WS_BORDER | WS_DISABLE_IM | addstyle)
{
    m_pSelf = this;

    //

    m_pimg    = new AXImage;
    m_pimgSrc = new CImage32;

    m_pbtt = new CArrowMenuBtt(this);

    m_bDrag = FALSE;
    m_nScrX = m_nScrY = 0;
    m_dScale = m_dScaleDiv = 1.0;

    //

    _setWinTitle();

    enableDND();

    attachAccel(((AXTopWindow *)m_pOwner)->getAccel());
}

//! 表示/非表示 切り替え

void CImgViewWin::showChange()
{
    if(g_pconf->isViewImgViewWin())
        showRestore();
    else
        hide();
}


//=========================
//コマンド
//=========================


//! 画像クリア

void CImgViewWin::_clearImg()
{
    m_pimgSrc->free();
    m_strFileName.empty();

    _setWinTitle();
    _draw();
}

//! 画像開く

void CImgViewWin::_openFile()
{
    AXString str,filter;

    //ファイル名取得

    filter = "BMP/PNG/GIF/JPEG\t*.bmp;*.png;*.gif;*.jpg;*.jpeg\tAll Files\t*\t";

    if(!AXFileDialog::openFile(this, filter, 0, g_pconf->strImgViewDir, 0, &str))
        return;

    //読み込み

    m_strFileName = str;

    _loadImg(TRUE);
}

//! 画像読み込み
/*!
    @param bSaveDir 読み込んだファイルのディレクトリを記録するか
*/

BOOL CImgViewWin::_loadImg(BOOL bSaveDir)
{
    if(!m_pimgSrc->loadImage(m_strFileName, NULL))
    {
        _clearImg();
        return FALSE;
    }

    //フォルダ記録

    if(bSaveDir)
        g_pconf->strImgViewDir.path_removeFileName(m_strFileName);

    //更新

    m_nScrX = m_nScrY = 0;

    _setWinTitle();

    if(!_adjustWinSize())
    {
        _adjustScale();
        _draw();
    }

    return TRUE;
}

//! 拡大縮小

void CImgViewWin::_scaleUpDown(BOOL bDown)
{
    int n;

    if(!(g_pconf->uImgVFlags & CConfig::IMGVF_FULLVIEW))
    {
        n = (int)(m_dScale * 100 + 0.5);

        if(n >= 100)
            n = n / 100 * 100;
        else
            n = (n + 9) / 10 * 10;

        if(bDown)
        {
            n -= (n <= 100)? 10: 100;
            if(n < 10) n = 10;
        }
        else
        {
            n += (n < 100)? 10: 100;
            if(n > 400) n = 400;
        }

        m_dScale    = n * 0.01;
        m_dScaleDiv = 1.0 / m_dScale;

        //

        _setWinTitle();

        if(!_adjustWinSize())
            _draw();
    }
}

//! 表示倍率セット

void CImgViewWin::_setScale(int no)
{
    if(!(g_pconf->uImgVFlags & CConfig::IMGVF_FULLVIEW))
    {
        if(no >= 5)
            //200%〜
            m_dScale = (no - 4) * 2.0;
        else
            //〜100%
            m_dScale = (no + 1) * 0.2;

        m_dScaleDiv = 1.0 / m_dScale;

        //

        _setWinTitle();

        if(!_adjustWinSize())
            _draw();
    }
}

//! ソート関数

int CImgView_sortfunc(AXListItem *pitem1,AXListItem *pitem2,ULONG lParam)
{
    CImgVListItem *p1 = (CImgVListItem *)pitem1;
    CImgVListItem *p2 = (CImgVListItem *)pitem2;

    return p1->m_strFileName.compare(p2->m_strFileName);
}

//! 次/前のファイル

void CImgViewWin::_nextPrevFile(BOOL bNext)
{
    AXList list;
    AXDir dir;
    AXString str,exts;
    AXFILESTAT stat;
    CImgVListItem *pItem,*pCurItem = NULL;

    if(m_strFileName.isEmpty()) return;

    //------- ファイル列挙

    str.path_removeFileName(m_strFileName);

    if(!dir.open(str)) return;

    exts = "bmp;png;jpg;jpeg;gif;";
    exts.replace(';', 0);

    str.path_filename(m_strFileName);

    while(dir.read())
    {
        dir.getFileStat(&stat);

        if(stat.isDirectory()) continue;

        if(dir.getCurName().path_compareExt(exts))
        {
            list.add(pItem = new CImgVListItem(dir.getCurName()));

            //現在のファイルか

            if(!pCurItem && dir.getCurName() == str)
                pCurItem = pItem;
        }
    }

    dir.close();

    if(!pCurItem) return;

    //------- ソート

    list.sort(CImgView_sortfunc, 0);

    //------- 読み込み

    if(bNext)
        pItem = pCurItem->next();
    else
        pItem = pCurItem->prev();

    if(!pItem) return;

    //

    m_strFileName.path_removeFileName();
    m_strFileName.path_add(pItem->m_strFileName);

    list.deleteAll();

    _loadImg(FALSE);
}


//===============================
//サブ処理
//===============================


//! ウィンドウタイトルセット

void CImgViewWin::_setWinTitle()
{
    AXString str,str2;
    char m[24];
    int n;

    if(!m_pimgSrc->isExist())
        setTitle("ImgView");
    else
    {
        n = (int)(m_dScale * 10000 + 0.5);

        ::sprintf(m, "[%d.%02d%%] ", n / 100, n % 100);

        str2.path_filename(m_strFileName);

        str = m;
        str += str2;

        setTitle(str);
    }
}

//! 描画

void CImgViewWin::_draw()
{
    CImage32::CANVASDRAW info;

    if(!m_pimgSrc->isExist())
        m_pimg->clear(IMGV_BKCOL);
    else
    {
        info.rcsDst.set(0, 0, m_nW, m_nH);

        info.nBaseX     = m_pimgSrc->getWidth() / 2;
        info.nBaseY     = m_pimgSrc->getHeight() / 2;
        info.nScrollX   = m_nW / 2 - m_nScrX;
        info.nScrollY   = m_nH / 2 - m_nScrY;
        info.dScaleDiv  = m_dScaleDiv;
        info.bHRev      = FALSE;
        info.dwExCol    = IMGV_BKCOL;

        if(m_dScale >= 1.0)
            m_pimgSrc->drawCanvasNormal(m_pimg, &info);
        else
            m_pimgSrc->drawCanvasScaleDown(m_pimg, &info);
    }

    redraw();
}

//! ウィンドウサイズを画像に合わせる
/*!
    @return resize() が行われたか（TRUE で返った場合、更新処理は必要ない）
*/

BOOL CImgViewWin::_adjustWinSize()
{
    if(m_pimgSrc->isExist() && (g_pconf->uImgVFlags & CConfig::IMGVF_FITSIZE))
    {
        m_nScrX = m_nScrY = 0;

        /* resize() 内で onSize() が呼ばれるので、更新処理は必要ない */

        return resize(_DTOI(m_pimgSrc->getWidth() * m_dScale), _DTOI(m_pimgSrc->getHeight() * m_dScale));
    }
    else
        return FALSE;
}

//! 全体表示時の調整

void CImgViewWin::_adjustScale()
{
    double x,y;

    if(m_pimgSrc->isExist() && (g_pconf->uImgVFlags & CConfig::IMGVF_FULLVIEW))
    {
        //倍率が低い方（等倍以上にはしない）

        x = (double)m_nW / m_pimgSrc->getWidth();
        y = (double)m_nH / m_pimgSrc->getHeight();

        if(x > y) x = y;
        if(x > 1.0) x = 1.0;

        //

        m_dScale    = x;
        m_dScaleDiv = 1.0 / m_dScale;

        m_nScrX = m_nScrY = 0;

        //

        _setWinTitle();
    }
}

//! スクロール位置調整
/*!
    左上・右下はクライアント中心まで
*/

void CImgViewWin::_adjustScrPos()
{
    int x1,y1,x2,y2,cw,ch,w,h;

    if(m_pimgSrc->isExist())
    {
        w   = m_pimgSrc->getWidth();
        h   = m_pimgSrc->getHeight();
        cw  = m_nW / 2;
        ch  = m_nH / 2;

        //

        x1 = (int)(-(w / 2) * m_dScale + cw - m_nScrX);
        y1 = (int)(-(h / 2) * m_dScale + ch - m_nScrY);
        x2 = (int)((w - 1 - w / 2) * m_dScale + cw - m_nScrX);
        y2 = (int)((h - 1 - h / 2) * m_dScale + ch - m_nScrY);

        if(x1 > cw)
            m_nScrX = (int)(-(w / 2) * m_dScale);
        else if(x2 < cw)
            m_nScrX = (int)((w - 1 - w / 2) * m_dScale);

        if(y1 > ch)
            m_nScrY = (int)(-(h / 2) * m_dScale);
        else if(y2 < ch)
            m_nScrY = (int)((h - 1 - h / 2) * m_dScale);
    }
}

//! スポイトメニュー

void CImgViewWin::_spoit(int x,int y,int rootx,int rooty)
{
    AXMenu *pm;
    DWORD col;
    int ret;

    //イメージ座標

    x = (int)((x + m_nScrX - m_nW / 2) * m_dScaleDiv) + m_pimgSrc->getWidth() / 2;
    y = (int)((y + m_nScrY - m_nH / 2) * m_dScaleDiv) + m_pimgSrc->getHeight() / 2;

    //範囲外

    if(x < 0 || y < 0 ||
       x >= m_pimgSrc->getWidth() || y >= m_pimgSrc->getHeight())
       return;

    //色取得

    col = m_pimgSrc->getPixel(x, y);

    //------ メニュー

    _trgroup(STRGID_IMGVIEWMENU);

    pm = new AXMenu;

    pm->addTr(STRID_IMGV_SPOIT_LAYER);
    pm->addTr(STRID_IMGV_SPOIT_BRUSH);

    ret = (int)pm->popup(NULL, rootx, rooty, 0);

    delete pm;

    //------ 処理

    switch(ret)
    {
        //レイヤの線の色
        case STRID_IMGV_SPOIT_LAYER:
            g_pdraw->layer_changeCol(col);
            break;
        //ブラシの濃度
        case STRID_IMGV_SPOIT_BRUSH:
            BRUSHWIN->setBrushVal(255 - ((_GETR(col) * 77 + _GETG(col) * 150 + _GETB(col) * 29) >> 8));
            break;
    }
}


//=========================
//ハンドラ
//=========================


//! 閉じる（非表示）

BOOL CImgViewWin::onClose()
{
    m_pOwner->onCommand(STRID_MENU_VIEW_IMGVIEWWIN, 0, 0);

    return TRUE;
}

//! 描画

BOOL CImgViewWin::onPaint(AXHD_PAINT *phd)
{
    m_pimg->put(m_id, phd->x, phd->y, phd->x, phd->y, phd->w, phd->h);

    return TRUE;
}

//! サイズ変更時

BOOL CImgViewWin::onSize()
{
    m_pimg->recreate(m_nW, m_nH, 32, 32);

    m_pbtt->moveParent();

    _adjustScale();
    _adjustScrPos();

    _draw();

    return TRUE;
}

//! D&D

BOOL CImgViewWin::onDND_Drop(AXDND *pDND)
{
    AXString str;

    //ファイル読み込み

    if(pDND->getFirstFile(&str))
    {
        m_strFileName = str;
        _loadImg(TRUE);
    }

    return TRUE;
}

//! 通知

BOOL CImgViewWin::onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam)
{
    AXMenu *pm;
    AXRectSize rcs;
    WORD id[] = {
        STRID_IMGV_OPEN, STRID_IMGV_NEXT, STRID_IMGV_PREV, STRID_IMGV_CLEAR, 0xffff,
        STRID_IMGV_SCALEUP, STRID_IMGV_SCALEDOWN, 0xffff,
        STRID_IMGV_FITSIZE|0x8000, 0xffff, STRID_IMGV_FULLVIEW|0x8000,
        STRID_IMGV_SCALE_20, STRID_IMGV_SCALE_40, STRID_IMGV_SCALE_60,
        STRID_IMGV_SCALE_80, STRID_IMGV_SCALE_100, STRID_IMGV_SCALE_200,
        STRID_IMGV_SCALE_400
    };

    //メニュー表示

    if(pwin == m_pbtt)
    {
        _trgroup(STRGID_IMGVIEWMENU);

        pm = new AXMenu;

        pm->addTrArray(id, sizeof(id)/sizeof(WORD));

        pm->check(STRID_IMGV_FITSIZE, g_pconf->uImgVFlags & CConfig::IMGVF_FITSIZE);
        pm->check(STRID_IMGV_FULLVIEW, g_pconf->uImgVFlags & CConfig::IMGVF_FULLVIEW);

        //

        m_pbtt->getWindowRect(&rcs);
        pm->popup(this, rcs.x + rcs.w, rcs.y + rcs.h, AXMenu::POPF_RIGHT);

        delete pm;
    }

    return TRUE;
}

//! コマンド

BOOL CImgViewWin::onCommand(UINT uID,ULONG lParam,int from)
{
    //倍率

    if(uID >= STRID_IMGV_SCALE_20 && uID <= STRID_IMGV_SCALE_400)
    {
        g_pconf->uImgVFlags &= ~CConfig::IMGVF_FULLVIEW;

        _setScale(uID - STRID_IMGV_SCALE_20);
        return TRUE;
    }

    //

    switch(uID)
    {
        //開く
        case STRID_IMGV_OPEN:
            _openFile();
            break;
        //次のファイル
        case STRID_IMGV_NEXT:
            _nextPrevFile(TRUE);
            break;
        //前のファイル
        case STRID_IMGV_PREV:
            _nextPrevFile(FALSE);
            break;
        //クリア
        case STRID_IMGV_CLEAR:
            _clearImg();
            break;
        //拡大
        case STRID_IMGV_SCALEUP:
            _scaleUpDown(FALSE);
            break;
        //縮小
        case STRID_IMGV_SCALEDOWN:
            _scaleUpDown(TRUE);
            break;
        //画像に合わせてサイズ変更
        case STRID_IMGV_FITSIZE:
            /*  全体表示と同時には使えない */

            if(g_pconf->uImgVFlags & CConfig::IMGVF_FULLVIEW)
            {
                g_pconf->uImgVFlags &= ~CConfig::IMGVF_FULLVIEW;
                g_pconf->uImgVFlags |= CConfig::IMGVF_FITSIZE;
            }
            else
                g_pconf->uImgVFlags ^= CConfig::IMGVF_FITSIZE;

            if(g_pconf->uImgVFlags & CConfig::IMGVF_FITSIZE)
                _adjustWinSize();
            break;
        //全体表示
        case STRID_IMGV_FULLVIEW:
            /* 画像に合わせて〜と同時には使えない */

            if(g_pconf->uImgVFlags & CConfig::IMGVF_FITSIZE)
            {
                g_pconf->uImgVFlags &= ~CConfig::IMGVF_FITSIZE;
                g_pconf->uImgVFlags |= CConfig::IMGVF_FULLVIEW;
            }
            else
                g_pconf->uImgVFlags ^= CConfig::IMGVF_FULLVIEW;

            if(g_pconf->uImgVFlags & CConfig::IMGVF_FULLVIEW)
            {
                _adjustScale();
                _draw();
            }
            break;
    }

    return TRUE;
}

//! キー押し時

BOOL CImgViewWin::onKeyDown(AXHD_KEY *phd)
{
    if(m_bDrag) return TRUE;

    switch(phd->keysym)
    {
        case 'O':
        case 'o':
            _openFile();
            break;
        case KEY_RIGHT:
        case KEY_NUM_RIGHT:
            _nextPrevFile(TRUE);
            break;
        case KEY_LEFT:
        case KEY_NUM_LEFT:
            _nextPrevFile(FALSE);
            break;
        case 'C':
        case 'c':
            _clearImg();
            break;
        case KEY_UP:
        case KEY_NUM_UP:
            _scaleUpDown(FALSE);
            break;
        case KEY_DOWN:
        case KEY_NUM_DOWN:
            _scaleUpDown(TRUE);
            break;
        case 'Z':
        case 'z':
            _setScale(4);
            break;
    }

    return TRUE;
}

//! ボタン押し時

BOOL CImgViewWin::onButtonDown(AXHD_MOUSE *phd)
{
    if(!m_bDrag && m_pimgSrc->isExist())
    {
        if(phd->button == BUTTON_LEFT)
        {
            //左ドラッグ - スクロール

            if(!(g_pconf->uImgVFlags & CConfig::IMGVF_FULLVIEW))
            {
                grabPointer();

                m_bDrag = TRUE;
                m_nBkX  = phd->rootx;
                m_nBkY  = phd->rooty;
            }
        }
        else if(phd->button == BUTTON_RIGHT)
        {
            //右ボタン - スポイトメニュー

            _spoit(phd->x, phd->y, phd->rootx, phd->rooty);
        }
    }

    return TRUE;
}

//! ボタン離し時

BOOL CImgViewWin::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_bDrag && phd->button == BUTTON_LEFT)
    {
        ungrabPointer();
        m_bDrag = FALSE;
    }

    return TRUE;
}

//! マウス移動時

BOOL CImgViewWin::onMouseMove(AXHD_MOUSE *phd)
{
    //スクロール処理

    if(m_bDrag)
    {
        m_nScrX += m_nBkX - phd->rootx;
        m_nScrY += m_nBkY - phd->rooty;

        m_nBkX = phd->rootx;
        m_nBkY = phd->rooty;

        _adjustScrPos();

        addTimer(0, 5);
    }

    return TRUE;
}

//! タイマー

BOOL CImgViewWin::onTimer(UINT uTimerID,ULONG lParam)
{
    //スクロール更新

    delTimer(uTimerID);

    _draw();
    redrawUpdate();

    return TRUE;
}
