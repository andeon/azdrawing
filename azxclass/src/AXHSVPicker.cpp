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

#include "AXHSVPicker.h"

#include "AXImage.h"
#include "AXApp.h"
#include "AXAppRes.h"
#include "AXUtilColor.h"


#define HSV_SPACE   5
#define HUE_W       13

#define BTTF_H      1
#define BTTF_SV     2


/*!
    @class AXHSVPicker
    @brief HSVカラーマップからの色選択ウィジェット

    @ingroup widget
*/

/*!
    @var AXHSVPicker::HSVPN_CHANGE_H
    @brief Hが変わった（lParam = 現在色）
    @var AXHSVPicker::HSVPN_CHANGE_SV
    @brief SVが変わった（lParam = 現在色）
*/


AXHSVPicker::~AXHSVPicker()
{
    delete m_pimg;
}

AXHSVPicker::AXHSVPicker(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,int h)
    : AXWindow(pParent, uStyle, uLayoutFlags)
{
    _createHSVPicker(h);
}

AXHSVPicker::AXHSVPicker(AXWindow *pParent,UINT uStyle,UINT uLayoutFlags,UINT uItemID,DWORD dwPadding,int h)
    : AXWindow(pParent, uStyle, uLayoutFlags, uItemID, dwPadding)
{
    _createHSVPicker(h);
}

//! 作成

void AXHSVPicker::_createHSVPicker(int h)
{
    m_uType = TYPE_HSVPICKER;
    m_nMinW = h + HSV_SPACE + HUE_W;
    m_nMinH = h;

    m_dwCol = 0xffffff;
    m_fBtt  = 0;
    m_nCurX = 0;
    m_nCurY = 0;
    m_nCurH = 0;

    m_pimg = new AXImage;

    _initImage();
}


//==============================
//
//==============================


//! 現在のHSV値取得
/*!
    @param pHSV [0]H(0-359) [1]S(0-1.0) [2]V(0-1.0)
*/

void AXHSVPicker::getHSVColor(double *pHSV)
{
    int n = m_nMinH - 1;

    pHSV[0] = m_nCurH * 359.0 / n;
    pHSV[1] = (double)m_nCurX / n;
    pHSV[2] = 1.0 - ((double)m_nCurY / n);
}

//! 色相変更

void AXHSVPicker::setHue(int hue)
{
    if(hue < 0) hue = 0; else if(hue > 359) hue %= 360;

    _changeH(hue * (m_nMinH - 1) / 359, FALSE);
}

//! SV位置変更
//! sv = 0.0-1.0

void AXHSVPicker::setSV(double s,double v)
{
    _changeSV(_DTOI(s * (m_nMinH - 1)), _DTOI((1.0 - v) * (m_nMinH - 1)), FALSE);
}

//! RGB値からセット

void AXHSVPicker::setColor(DWORD dwCol)
{
    double hsv[3];

    m_dwCol = dwCol & 0xffffff;

    //カーソル消去

    _drawSVCur();
    _drawHCur();

    //RGB->HSV

    AXRGBtoHSV(m_dwCol, hsv);

    m_nCurH	= _DTOI(hsv[0] * m_nMinH);
    m_nCurX = _DTOI(hsv[1] * (m_nMinH - 1));
    m_nCurY = _DTOI((1.0 - hsv[2]) * (m_nMinH - 1));

    if(m_nCurH >= m_nMinH) m_nCurH = m_nMinH - 1;

    //描画

    _drawSV();
    _drawSVCur();
    _drawHCur();

    redrawUpdate();
}


//==============================
//描画
//==============================


//! 初期イメージ

void AXHSVPicker::_initImage()
{
    int i,n;
    LPBYTE p;

    if(!m_pimg->create(m_nMinW, m_nMinH)) return;

    //SV

    _drawSV();

    //余白

    m_pimg->fillBox(m_nMinH, 0, HSV_SPACE, m_nMinH, axres->colRGB(AXAppRes::FACE));

    //色相

    p = m_pimg->getBufPt(m_nMinH + HSV_SPACE, 0);
    n = m_nMinH - 1;

    for(i = 0; i < m_nMinH; i++, p += m_pimg->getPitch())
        m_pimg->lineHBuf(p, HUE_W, AXHSVtoRGB_fast(i * 359 / n, 255, 255));

    //カーソル

    _drawHCur();
    _drawSVCur();
}

//! Hカーソル描画

void AXHSVPicker::_drawHCur()
{
    int x = m_nMinH + HSV_SPACE;

    m_pimg->line(x    , m_nCurH    , x + 4, m_nCurH - 4, AXImage::COL_XOR);
    m_pimg->line(x + 1, m_nCurH + 1, x + 4, m_nCurH + 4, AXImage::COL_XOR);
    m_pimg->line(x + 5, m_nCurH - 5, x + 5, m_nCurH + 5, AXImage::COL_XOR);
}

//! SVカーソル描画

void AXHSVPicker::_drawSVCur()
{
    m_pimg->box(m_nCurX - 3, m_nCurY - 3, 7, 7, AXImage::COL_XOR);
}

//! SV部分描画

void AXHSVPicker::_drawSV()
{
    int x,y,h,a;

    a = m_nMinH - 1;
    h = m_nCurH * 359 / a;

    for(y = 0; y < m_nMinH; y++)
    {
        for(x = 0; x < m_nMinH; x++)
            m_pimg->setPixel(x, y, AXHSVtoRGB_fast(h, x * 255 / a, 255 - y * 255 / a));
    }
}


//==============================
//サブ
//==============================


//! 現在カーソル位置から色計算

void AXHSVPicker::_calcCol()
{
    int n = m_nMinH - 1;

    m_dwCol = AXHSVtoRGB(m_nCurH * 359 / n,
                    (double)m_nCurX / n, 1.0 - ((double)m_nCurY / n));
}

//! H変更時

void AXHSVPicker::_changeH(int cury,BOOL bSend)
{
    if(cury < 0) cury = 0; else if(cury >= m_nMinH) cury = m_nMinH - 1;

    if(m_nCurH == cury) return;

    //カーソル・SV

    _drawHCur();
    _drawSVCur();

    m_nCurH = cury;

    _drawSV();

    _drawHCur();
    _drawSVCur();

    redrawUpdate();

    //通知

    _calcCol();

    if(bSend) getNotify()->onNotify(this, HSVPN_CHANGE_H, m_dwCol);
}

//! SV位置変更時

void AXHSVPicker::_changeSV(int x,int y,BOOL bSend)
{
    if(x < 0) x = 0; else if(x >= m_nMinH) x = m_nMinH - 1;
    if(y < 0) y = 0; else if(y >= m_nMinH) y = m_nMinH - 1;

    if(m_nCurX == x && m_nCurY == y) return;

    //カーソル

    _drawSVCur();

    m_nCurX = x;
    m_nCurY = y;

    _drawSVCur();

    redrawUpdate();

    //通知

    _calcCol();

    if(bSend) getNotify()->onNotify(this, HSVPN_CHANGE_SV, m_dwCol);
}


//==============================
//ハンドラ
//==============================


//! ボタン押し時

BOOL AXHSVPicker::onButtonDown(AXHD_MOUSE *phd)
{
    if(phd->button == BUTTON_LEFT && m_fBtt == 0)
    {
        if(phd->x < m_nMinH)
        {
            //SV
            _changeSV(phd->x, phd->y, TRUE);
            m_fBtt = BTTF_SV;
        }
        else if(phd->x >= m_nMinH + HSV_SPACE)
        {
            //H
            _changeH(phd->y, TRUE);
            m_fBtt = BTTF_H;
        }

        if(m_fBtt)
            grabPointer();
    }

    return TRUE;
}

//! ボタン離し時

BOOL AXHSVPicker::onButtonUp(AXHD_MOUSE *phd)
{
    if(m_fBtt && phd->button == BUTTON_LEFT)
    {
        m_fBtt = 0;
        ungrabPointer();
    }

    return TRUE;
}

//! マウス移動時

BOOL AXHSVPicker::onMouseMove(AXHD_MOUSE *phd)
{
    if(m_fBtt == BTTF_H)
        _changeH(phd->y, TRUE);
    else if(m_fBtt == BTTF_SV)
        _changeSV(phd->x, phd->y, TRUE);

    return TRUE;
}

//! 描画

BOOL AXHSVPicker::onPaint(AXHD_PAINT *phd)
{
    m_pimg->put(m_id, phd->x, phd->y, phd->x, phd->y, phd->w, phd->h);
    return TRUE;
}
