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

#ifndef _AZDRAW_FILTERDLG_H_
#define _AZDRAW_FILTERDLG_H_

#include "AXDialog.h"
#include "CLayerImg.h"
#include "defStruct.h"

class AXLayout;
class AXLineEdit;
class AXComboBox;
class AXBuf;
class CFilterBar;
class CFilterPrev;
class CFilterWidgetLevel;
class CProgressDlg;
class CFilterConfigItem;

class CFilterDlg:public AXDialog
{
public:
    enum
    {
        MAXNUM_BAR      = 8,
        MAXNUM_COMBO    = 1,
        MAXNUM_CHECK    = 2,

        DATFLAG_PREVIEW = 1,
        DATFLAG_VERT    = 2,
        DATFLAG_CANVASPREVIEW   = 4,
        DATFLAG_SAVECONF    = 8,
        DATFLAG_RESET       = 16
    };

    struct DRAWINFO
    {
        RECTANDSIZE     *prs;
        CLayerImg       *pimgSrc,
                        *pimgDst;
        CProgressDlg    *pProgDlg;

        BOOL        bClipping,
                    bCopySrc;
        CLayerImg   *pimgRef;
        int         nValBar[MAXNUM_BAR],
                    nValCombo[MAXNUM_COMBO],
                    nValCheck[MAXNUM_CHECK];
    };

protected:
    DRAWINFO    *m_pinfo;
    BOOL (*m_func)(DRAWINFO *);

    CLayerImg   m_imgPrev;
    RECTANDSIZE m_rsPrev;

    BOOL    m_bPreview,
            m_bCanvasPrev;
    UINT    m_uFlags,
            m_uMenuID;
    int     m_nBarCnt,
            m_nEditCnt,
            m_nComboCnt,
            m_nCheckCnt;

    short   m_sDefBar[MAXNUM_BAR];
    char    m_cSigBar[MAXNUM_BAR],
            m_cSigCombo[MAXNUM_COMBO],
            m_cSigCheck[MAXNUM_CHECK];

    CFilterConfigItem *m_pConfItem;

    CFilterPrev *m_pPrev;
    CFilterBar  *m_pbar[MAXNUM_BAR];
    AXLineEdit  *m_peditBar[MAXNUM_BAR];
    AXComboBox  *m_pcombo[MAXNUM_COMBO],
                *m_pcbLayer;
    CFilterWidgetLevel  *m_pwidLevel;

protected:
    void _end(BOOL bOK);
    void _prev();
    void _saveConfig();
    void _reset();

    void _create_widget(AXBuf *pbuf,int prevw,int prevh);
    void _create_subwidget(AXBuf *pbuf,AXLayout *plm);

    BOOL _create_bar(AXBuf *pbuf,AXLayout *plm,int size);
    BOOL _create_combo(AXBuf *pbuf,AXLayout *plm,int size);
    BOOL _create_check(AXBuf *pbuf,AXLayout *plm,int size);
    BOOL _create_layerlist(AXBuf *pbuf,AXLayout *plm,int size);

    void _create_def_clipping(AXLayout *plm);
    void _create_def_level(AXLayout *plm);

public:
    CFilterDlg(AXWindow *pOwner,WORD wTitleID,const BYTE *pDat,BOOL (*func)(DRAWINFO *),DRAWINFO *pInfo);

    virtual BOOL onClose();
    virtual BOOL onNotify(AXWindow *pwin,UINT uNotify,ULONG lParam);
    virtual BOOL onTimer(UINT uTimerID,ULONG lParam);
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);
};

#endif
