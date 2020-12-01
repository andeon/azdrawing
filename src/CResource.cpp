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

#include "CResource.h"

#include "CConfig.h"
#include "defGlobal.h"

#include "AXImageList.h"
#include "AXImage32.h"
#include "AXFont.h"
#include "AXTarBall.h"
#include "AXUtilRes.h"
#include "AXApp.h"



/*!
    @class CResource
    @brief リソースクラス
*/


CResource::~CResource()
{
    delete m_pfontSmall;
    delete m_pfontSubWin;

    delete m_pilBrushTreeTB;
    delete m_pilBrushTreeIcon;
    delete m_pilBrushValTab;
    delete m_pilLayerTB;
    delete m_pilToolTB;
    delete m_pilToolSubTB;
    delete m_pilToolUnder;

    delete m_pimg32Rule;
}

// 設定読み込みより後、ウィンドウ作成より前に実行する

CResource::CResource()
{
    AXTarBall tar;
    AXString str;

    //------- 作成

    m_pfontSmall  = new AXFont;
    m_pfontSubWin = new AXFont;

    m_pilBrushTreeTB    = new AXImageList;
    m_pilBrushTreeIcon  = new AXImageList;
    m_pilBrushValTab    = new AXImageList;
    m_pilLayerTB        = new AXImageList;
    m_pilToolTB         = new AXImageList;
    m_pilToolSubTB      = new AXImageList;
    m_pilToolUnder      = new AXImageList;

    m_pimg32Rule = new AXImage32;

    //------- フォント

    m_pfontSmall->createAppFontPxSize(11);

    if(g_pconf->strSubWinFont.isEmpty())
        m_pfontSubWin->createAppFontPxSize(11);
    else
        m_pfontSubWin->createPattern(g_pconf->strSubWinFont);

    //------- tar内画像

    axapp->getResourcePath(&str, "image.tar");

    tar.loadFile(str);

    //

    AXLoadTarResImageList(&tar, m_pilBrushTreeTB, "tb_brushtree.png", 12, 0x00ff00, TRUE);
    AXLoadTarResImageList(&tar, m_pilBrushTreeIcon, "il_brushtree.png", 14, 0x00ff00, FALSE);
    AXLoadTarResImageList(&tar, m_pilBrushValTab, "il_brushtab.png", 15, -1, FALSE);
    AXLoadTarResImageList(&tar, m_pilLayerTB, "tb_layer.png", 13, 0x00ff00, FALSE);
    AXLoadTarResImageList(&tar, m_pilToolTB, "tb_tool.png", 15, 0x00ff00, FALSE);
    AXLoadTarResImageList(&tar, m_pilToolSubTB, "tb_toolsub.png", 15, 0x00ff00, FALSE);
    AXLoadTarResImageList(&tar, m_pilToolUnder, "il_toolwin.png", 8, 0x00ff00, FALSE);

    AXLoadTarResImage32(&tar, m_pimg32Rule, "img_rule.png");
}

//! 定規イメージ選択描画

void CResource::drawRuleImgSel(int no)
{
    int h;

    h = m_pimg32Rule->getHeight();

    //黒以外を白にする

    m_pimg32Rule->replaceExtract(0, 0, m_pimg32Rule->getWidth(), h, 0xffffff, 0);

    //選択部分の黒以外を選択色へ

    m_pimg32Rule->replaceExtract(no * (h - 1) + 1, 1, h - 2, h - 2, 0xffa0a0, 0);
}
