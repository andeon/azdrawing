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

#include "CStatusBar.h"

#include "CDraw.h"
#include "CResource.h"
#include "defGlobal.h"
#include "defStrID.h"

#include "AXLayout.h"
#include "AXLabel.h"
#include "AXApp.h"
#include "AXUtilStr.h"


/*!
    @class CStatusBar
    @brief メインウィンドウのステータスバー
*/


CStatusBar *CStatusBar::m_pSelf = NULL;


CStatusBar::CStatusBar(AXWindow *pParent)
    : AXWindow(pParent, WS_BK_FACE, LF_EXPAND_W)
{
    AXLayout *pl;

    m_pSelf = this;

    setFont(g_pres->m_pfontSmall);

    //

    setLayout(pl = new AXLayoutHorz(0, 5));

    pl->setSpacing(1);

    pl->addItem(m_pInfo = new AXLabel(this, AXLabel::LS_SUNKEN, 0));
    pl->addItem(m_pHelp = new AXLabel(this, 0, LF_EXPAND_W | LF_CENTER_Y));
}

//! 情報テキストセット

void CStatusBar::setInfoLabel()
{
    AXString str;

    str.setInt(g_pdraw->m_nImgW);
    str += "x";
    str += g_pdraw->m_nImgH;
    str += '_';
    str += g_pdraw->m_nImgDPI;
    str += "DPI_";
    str += g_pdraw->m_nCanvasScale;
    str += '%';

    m_pInfo->setText(str);
    layout();
}

//! ヘルプ情報セット

void CStatusBar::setLabel_help()
{
    int subno,id = -1;
    int id_poly[4] = {STRID_HELP_DRAW_BOX, STRID_HELP_DRAW_CIRCLE, STRID_HELP_DRAW_POLYGON, -1},
        id_ptclose[4] = {-1, STRID_HELP_DRAW_BOX, STRID_HELP_DRAW_POLYGON, -1};

    subno = g_pdraw->getToolSubType();

    switch(g_pdraw->m_toolno)
    {
        //ブラシ描画・ドット
        case CDraw::TOOL_BRUSH:
        case CDraw::TOOL_DOT:
            if(subno == 0)
                id = STRID_HELP_DRAW_BRUSH_FREE + g_pdraw->m_toolno;
            else
                id = STRID_HELP_DRAW_LINE + subno - 1;
            break;
        //図形塗り・消し
        case CDraw::TOOL_POLYPAINT:
        case CDraw::TOOL_POLYERASE:
            id = id_poly[subno];
            break;
        //閉領域塗り
        case CDraw::TOOL_PAINTCLOSE:
            id = id_ptclose[subno];
            break;

        case CDraw::TOOL_PAINT:
        case CDraw::TOOL_PAINTERASE:
            id = STRID_HELP_TOOL_PAINT;
            break;
        case CDraw::TOOL_GRAD:
            id = STRID_HELP_TOOL_GRAD;
            break;
        case CDraw::TOOL_TEXT:
            id = STRID_HELP_TOOL_TEXT;
            break;
        case CDraw::TOOL_MOVE:
            id = STRID_HELP_TOOL_MOVE;
            break;
        case CDraw::TOOL_ROTATE:
            id = STRID_HELP_TOOL_ROTATE;
            break;
        case CDraw::TOOL_SEL:
            id = STRID_HELP_TOOL_SEL;
            break;
        case CDraw::TOOL_SPOIT:
            id = STRID_HELP_TOOL_SPOIT;
            break;
    }

    if(id == -1)
        m_pHelp->setText((LPCUSTR)NULL);
    else
        m_pHelp->setText(_string(STRGID_HELP, id));
}

//! ものさし時

void CStatusBar::setLabel_rule(double dLen,int angle)
{
    AXString str;
    char m[32];
    int cm;

    cm = (int)(dLen * 2.54 / g_pdraw->m_nImgDPI * 100.0);

    str.setInt((int)dLen);
    str += " px ( ";

    AXIntToFloatStr(m, cm, 2);
    str += m;
    str += " cm ) : Angle ";

    AXIntToFloatStr(m, angle, 2);
    str += m;

    m_pHelp->setText(str);
}
