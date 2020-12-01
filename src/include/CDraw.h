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

#ifndef _AZDRAW_DRAW_H_
#define _AZDRAW_DRAW_H_

#include "AXDef.h"
#include "AXRect.h"
#include "AXCursor.h"
#include "AXString.h"
#include "defStruct.h"

class CLayerList;
class CLayerItem;
class CImage32;
class CImage8;
class CImage1;
class CLayerImg;
class CUndo;
class CPolygonPos;
class CSplinePos;
class CPosBuf;
class CGradList;
class CFont;
class AXImage;
class AXString;
class AXConfRead;
class AXConfWrite;
class AXFile;
class AXMem;
class CProgressDlg;
class AXPSDLoad;

//-----------

class CDraw
{
public:
    enum
    {
        IMGSIZE_MAX = 9999,
        SCALE_MIN   = 5,
        SCALE_MAX   = 800,

        VIEWPARAM_SCALE     = 0,
        VIEWPARAM_SCALEDIV  = 1,
        VIEWPARAM_COSREV    = 2,
        VIEWPARAM_SINREV    = 3,
        VIEWPARAM_COS       = 4,
        VIEWPARAM_SIN       = 5,
    };

    enum LOADERR
    {
        LOADERR_SUCCESS,
        LOADERR_ETC,
        LOADERR_IMGSIZE,    //!< 画像サイズエラー
        LOADERR_OPENFILE,   //!< ファイルが開けない
        LOADERR_FORMAT      //!< フォーマットエラー
    };

    enum CURSORTYPE
    {
        CURSOR_NONE,
        CURSOR_DRAW,
        CURSOR_MOVE,
        CURSOR_ROTATE,
        CURSOR_RULE,
        CURSOR_SEL,
        CURSOR_BOXSEL,
        CURSOR_SPOIT,
        CURSOR_WAIT,
        CURSOR_SELDRAG,
        CURSOR_LAYERMOVE,
        CURSOR_TEXT
    };

    enum TOOLNO
    {
        TOOL_BRUSH,
        TOOL_DOT,
        TOOL_POLYPAINT,
        TOOL_POLYERASE,
        TOOL_PAINT,
        TOOL_PAINTERASE,
        TOOL_PAINTCLOSE,
        TOOL_GRAD,
        TOOL_TEXT,
        TOOL_RULE,
        TOOL_MOVE,
        TOOL_ROTATE,
        TOOL_BOXEDIT,
        TOOL_SEL,
        TOOL_SPOIT,

        TOOL_NUM
    };

    enum DRAWTYPE
    {
        DRAWTYPE_FREE,
        DRAWTYPE_LINE,
        DRAWTYPE_BOX,
        DRAWTYPE_CIRCLE,
        DRAWTYPE_SUCCLINE,
        DRAWTYPE_CONCLINE,
        DRAWTYPE_BEZIER,
        DRAWTYPE_SPLINE,

        DRAWTYPE_NUM
    };

    enum BOXEDITTYPE
    {
        BOXEDIT_REVHORZ,
        BOXEDIT_REVVERT,
        BOXEDIT_SCALEROT,
        BOXEDIT_TRIM,

        BOXEDIT_NUM
    };

    enum NOWCTLNO
    {
        NOWCTL_NONE,
        NOWCTL_TEMP,
        NOWCTL_CANVASMOVE,
        NOWCTL_CANVASROTATE,
        NOWCTL_SCALEDRAG,
        NOWCTL_IMGMOVE,
        NOWCTL_DOT_FREE,
        NOWCTL_BRUSH_FREE,
        NOWCTL_XORLINE,
        NOWCTL_XORLINE2,
        NOWCTL_XORBOXWIN,
        NOWCTL_XORBOXIMG,
        NOWCTL_XORCIRCLE,
        NOWCTL_XORLASSO,
        NOWCTL_RULEPOS,
        NOWCTL_FUNCUP,
        NOWCTL_BEZIER,
        NOWCTL_SPLINE,
        NOWCTL_DRAGBRUSHSIZE,
        NOWCTL_PAINTCLOSE_BRUSH,
        NOWCTL_SEL_BRUSH,
        NOWCTL_SEL_DRAGMOVE,
        NOWCTL_TEXT,
        NOWCTL_BOXEDIT_SCALEROT,
        NOWCTL_FILTER_CANVASPREV
    };

    enum AFTERCTLTYPE
    {
        AFTERLINE_DRAWLINE = 0,
        AFTERLINE_BEZIER,
        AFTERLINE_GRAD,
        AFTERLINE_CALCRULE,
        AFTERLINE_RULE_LINE,
        AFTERLINE_RULE_GRID,

        AFTERLINE2_SUCC = 0,
        AFTERLINE2_CONC,
        AFTERLINE2_FILLPOLY,

        AFTERBOXWIN_DRAWFILL = 0,
        AFTERBOXWIN_DRAWBOX,
        AFTERBOXWIN_PAINTCLOSE,
        AFTERBOXWIN_DRAWSEL,

        AFTERBOXIMG_HREV = 0,
        AFTERBOXIMG_VREV,
        AFTERBOXIMG_SCALEROT,
        AFTERBOXIMG_TRIM,

        AFTERCIR_DRAWFILL = 0,
        AFTERCIR_DRAWCIRCLE,
        AFTERCIR_RULE,

        AFTERLASSO_FILL = 0,
        AFTERLASSO_PAINTCLOSE,
        AFTERLASSO_SEL,

        AFTERUP_PAINT = 0,
        AFTERUP_PAINTERASE
    };

    enum AFTERCTLTYPE2
    {
        AFTER2FILLPOLY_FILL = 0,
        AFTER2FILLPOLY_PAINTCLOSE,
        AFTER2FILLPOLY_SEL
    };

    enum
    {
        TOOLDOT_MASK_SIZE   = 255,
        TOOLDOT_FLAG_THIN   = 1<<8,

        TOOLPOLY_VAL_MASK      = 255,
        TOOLPOLY_PIX_SHIFT     = 8,
        TOOLPOLY_PIX_MASK      = 15,
        TOOLPOLY_F_BRUSHVAL    = 1<<12,
        TOOLPOLY_F_ANTI        = 1<<13,

        TOOLPAINT_VAL_MASK      = 255,
        TOOLPAINT_VALCMP_SHIFT  = 8,
        TOOLPAINT_TYPE_SHIFT    = 16,
        TOOLPAINT_TYPE_MASK     = 3,
        TOOLPAINT_PIX_SHIFT     = 18,
        TOOLPAINT_PIX_MASK      = 15,
        TOOLPAINT_F_USEBRUSH    = 1<<22,
        TOOLPAINT_F_NOREF       = 1<<23,

        TOOLPAINTE_NOREF    = 1,

        TOOLPAINTC_VAL_MASK     = 255,
        TOOLPAINTC_PIX_SHIFT    = 8,
        TOOLPAINTC_PIX_MASK     = 15,
        TOOLPAINTC_F_USEBRUSH   = 1<<12,
        TOOLPAINTC_F_NOREF      = 1<<13,

        TOOLGRAD_VAL_MASK       = 255,
        TOOLGRAD_PIX_SHIFT      = 8,
        TOOLGRAD_PIX_MASK       = 15,
        TOOLGRAD_F_REV          = 1<<12,
        TOOLGRAD_F_LOOP         = 1<<13,

        BESCALEROT_COL_MASK     = 0xffffff,
        BESCALEROT_F_PREV       = 1<<24,
        BESCALEROT_F_HQPREV     = 1<<25
    };

    enum RULETYPE
    {
        RULETYPE_NONE,
        RULETYPE_LINE,
        RULETYPE_GRIDLINE,
        RULETYPE_CONCLINE,
        RULETYPE_CIRCLE,
        RULETYPE_ELLIPSE
    };

    enum TEXTFLAG
    {
        TEXTF_PREVIEW   = 1,
        TEXTF_VERT      = 2,
        TEXTF_2COL      = 4
    };

public:
    typedef struct
    {
        double x,y,press;
    }DRAWPOINT;

    struct RULEDAT
    {
        DPOINT  ptDown;         //!< 押し時のイメージ位置
        AXPoint ptCtConc,       //!< 中心位置(集中線)
                ptCtCir,        //!< 中心位置(正円)
                ptCtEll;        //!< 中心位置(楕円)
        double  dTmp,           //!< 共通作業用([集中線]角度、[円]半径、[格子線]確定角度)
                dAngleLine,     //!< [平行線]角度
                dAngleGrid,     //!< [格子線]基本の角度
                dEllHV[2],      //!< [楕円]扁平率(0:x/y,1:y/x)
                dEllTmp[4];     //!< [楕円]回転
        int     bEllRevH,       //!< [楕円]左右反転か
                nTmp;           //!< 作業用（[格子線]角度確定フラグ）
    };

public:
    AXImage     *m_pimgCanvas;
    CImage32    *m_pimgBlend;

    CLayerList  *m_player;
    CUndo       *m_pundo;

    AXCursor    m_cursorTool,
                m_cursorDrag,
                m_cursorWait;

    //情報

    CLayerItem  *m_pCurLayer;

    AXSize  m_szCanvas;
    AXPoint m_ptBaseImg,        //!< イメージ座標での基準位置（回転の中心位置）
            m_ptScr;            //!< スクロール位置

    int     m_nImgW,m_nImgH,
            m_nImgDPI,
            m_nCurLayerNo,
            m_nCanvasScale,     //!< 現在のキャンバス表示倍率(%)
            m_nCanvasRot;       //!< 現在のキャンバス回転角度 (-18000〜+18000。0.01単位 ※100=1.0)
    BOOL    m_bCanvasHRev;      //!< キャンバス左右反転表示

    double  m_dViewParam[6];    //! 回転・拡大計算用

    //描画関連

    BYTE    m_toolno,
            m_ruleType,
            m_toolSubNo[TOOL_NUM];

    UINT    m_optToolDot,           //!< ドットペン [下位8bit:px、8bit:細線]
            m_optToolPolyPaint,     //!< 図形塗り [下位8bit:濃度、8-11bit(3bit):塗りタイプ、12bit:ブラシの濃度を使う、13bit:アンチエイリアス]
            m_optToolPolyErase,
            m_optToolPaint,         //!< 塗りつぶし [0-7bit:濃度、8-15bit:比較濃度、16-17bit:タイプ、18-21bit:塗りタイプ、22bit:ブラシ濃度使う、23bit:判定元無効]
            m_optToolPaintClose,    //!< 閉領域 [0-7bit:濃度、8-11bit:塗りタイプ、12bit:ブラシの濃度を使う、13bit:判定元無効]
            m_optToolGrad,          //!< グラデ [0-7bit:濃度、8-11bit:塗りタイプ、12bit:左右反転、13bit:繰り返し]
            m_optToolSel,           //!< 選択範囲色(0-23bit)と濃度(24bit-、0〜128)
            m_optBEScaleRot;        //!< 矩形編集・拡大縮小&回転 [0-23bit:色、24bit:プレビュー、25bit:高品質プレビュー]
    BYTE    m_optToolMove,          //!< 移動 [対象レイヤ]
            m_optToolPaintErase;    //!< 不透明消し [0bit:判定元無効]

    WORD    m_wHeadTailLine,        //!< 入り抜き（直線タイプ）
            m_wHeadTailBezir;       //!< 入り抜き（ベジェ曲線）

    AXString    m_strOptTex;        //!< オプション設定のテクスチャ名

    CGradList   *m_pGrad;           //!< グラデーションデータ

    //テキスト描画

    AXString    m_strDrawText,
                m_strFontFace,
                m_strFontStyle;
    int         m_nTextSize,
                m_nTextCharSP,
                m_nTextLineSP,
                m_nTextHinting;
    UINT        m_uTextFlags;

    //描画作業用

    RULEDAT         m_rule;
    DRAWLAYERINFO   m_datDraw;

    //作業用イメージなど

    CLayerImg   *m_pimgUndo,
                *m_pimgSel,
                *m_pimgCopy,
                *m_pimgTmp;

    CImage1     *m_pimg1Work;

    CImage8     *m_pimg8Tex,        //!< オプションテクスチャ画像
                *m_pimg8BrushDef;   //!< ブラシ画像デフォルト

    CPolygonPos *m_pPolyPos;
    CSplinePos  *m_pSplinePos;
    CPosBuf     *m_pPosBuf;
    CFont       *m_pFont;

    //作業用

    int         m_nNowCtlNo,
                m_nAfterCtlType,
                m_nAfterCtlType2,
                m_nDrawToolNo,
                m_nTmp[2];
    UINT        m_uDownBtt,
                m_optToolTmp;
    LPVOID      m_pTmp;
    AXPoint     m_ptTmp[4];
    DPOINT      m_dptTmp[1];
    AXRectSize  m_rcsTmp[1];
    DRAWPOINT   m_ptBkWinPos,
                m_ptNowWinPos,
                m_ptDownWinPos;
    FLAGRECT    m_rcfDrawUp,
                m_rcfSel;
    LAYERINFO   m_infoUndo;

public:
    CDraw();
    ~CDraw();

    BOOL isNowCtlNone() { return m_nNowCtlNo == NOWCTL_NONE; }
    int getToolSubType() { return m_toolSubNo[m_toolno]; }
    BOOL isTextPreview() { return m_uTextFlags & TEXTF_PREVIEW; }

    //

    void initAfterConfig();
    void initShow();

    void setUndoChangeOff();
    void setUndoMaxCnt(int cnt);
    BOOL isChangeImage();

    LPBYTE getCursorDat(int type);
    void createCursor(AXCursor *pcur,int type);
    void setCursorTool();
    void setCursorDrag(int type);
    void setCursorWait();
    void restoreCursor();

    void loadConfig(AXConfRead *pcf);
    void saveConfig(AXConfWrite *pcf);

    //

    BOOL newImage(int w,int h,int dpi,BOOL bAddLayer);
    int loadImage(const AXString &filename);
    BOOL resizeCanvas(int w,int h,int topx,int topy);
    BOOL scaleCanvas(int w,int h,int dpi,int type);

    void changeFile();
    void changeImageSize();
    BOOL changeResizeCanvas(int w,int h);
    void changeTool(int no);
    void changeToolSubType(int no);
    void changeCanvasWinSize(BOOL bReDraw,AXPoint *pct=NULL);
    void changeScale(int scale,BOOL bRedraw=TRUE,BOOL bHiQuality=TRUE,BOOL bScrReset=TRUE);
    void changeRotate(int rotate,BOOL bRedraw=TRUE,BOOL bHiQuality=TRUE,BOOL bScrReset=TRUE);
    void changeScrPos(AXPoint *pct,BOOL bReDraw);
    void changeScaleAndRotate(BOOL bRedraw);

    void canvasHRev();
    void scaleUpDown(BOOL bUp);
    void resetCanvas();
    BOOL undoRedo(BOOL bRedo,LPINT pLayerNo,AXRectSize *prcs);

    void setScroll_default();
    void setScroll_reset();

    BOOL blendAllLayer(BOOL bAlpha);
    BOOL loadTextureImg(CImage8 *pimg,LPCUSTR pName);

    //---------------- draw

    BOOL onDown(const DRAWPOINT &pt,UINT btt);
    BOOL onLDblClk();
    void onMove(const DRAWPOINT &pt,BOOL bCtrl,BOOL bShift);
    BOOL onUp(const DRAWPOINT &pt,int bttno);
    BOOL onCancel();
    void onLDownInDlg(int x,int y);
    BOOL onKey_inCtrl(UINT uKey);

    BOOL onDown_keycmd(int cmdid);
    BOOL onDown_tool(int tool,int subno);
    BOOL onDown_draw(int toolno,int drawtype,int brushno,BOOL bPressMax=FALSE);
    BOOL onDown_drawPolygon(int toolno,int type);
    BOOL onDown_drawPaintClose(int type);
    BOOL onDown_drawSel(int type);
    void onDown_rule(BOOL bDot);

    //draw_brushdot

    void onDown_brush_free(int nBrushNo,BOOL bPressMax);
    void onMove_brush_free();
    void onUp_brush_free();

    BOOL onDown_dot_free(BOOL bErase);
    void onMove_dot_free();
    void onUp_dot_free();

    void draw_line();
    void draw_box();
    void draw_circle();

    void onDown2_lineSuccConc(BOOL bSucc);
    BOOL onCancel_lineSuccConc(BOOL bStartTo);

    BOOL onUp_firstBezier();
    void onMove_bezier();
    BOOL onUp_bezier();
    BOOL onCancel_bezier();

    BOOL onDown_spline();
    BOOL onUp_spline();
    BOOL onCancel_spline();
    BOOL onKeyBack_spline();

    //draw1

    void draw_fillPolygon_common();
    void draw_fillBox();
    void draw_fillCircle();
    void draw_paintCommon(int type,int nVal,int nCompareVal,BOOL bDisableRef);
    void draw_grad();

    void draw_paintClose_box();
    void draw_paintClose_common(BOOL bPolygon);

    void draw_boxedit_revHV(BOOL bHorz);
    void draw_boxedit_scaleRot();
    void draw_boxedit_scaleRotPrev(const CImage8 &img8,const AXRect &rc,double scale,double angle);

    BOOL onDown_paintclose_brush();
    void onMove_paintclose_brush();
    void onUp_paintclose_brush();

    BOOL onDown_fillPoly();
    void onDown2_fillPoly();
    BOOL onCancel_fillPoly(BOOL bDraw=TRUE);

    BOOL onDown_funcUp(int aftertype);
    void onUp_funcUp();

    BOOL onDown_imgMove();
    void onMove_imgMove(BOOL bHorz,BOOL bVert);
    void onUp_imgMove();

    BOOL onDown_canvasMove();
    void onMove_canvasMove();

    BOOL onDown_canvasRotate();
    void onMove_canvasRotate(BOOL bCalcOnly,BOOL b45);

    BOOL onDown_scaleDrag();
    void onMove_scaleDrag();

    BOOL onDown_dragBrushSize(BOOL bRegBrush1);
    void onMove_dragBrushSize();

    BOOL onDown_spoit();

    //draw2

    void drawShape(UINT val,double *pParam);

    void beginFilterCanvasPrev(LPVOID pDlg);
    void endFilterCanvasPrev();
    void onDown_filterPrevInDlg(int winx,int winy);

    void onDown_text();
    void onDown_textInDlg(int winx,int winy);
    void draw_textPrev();
    void text_move1px(int dir);
    void text_createFontInit();
    void text_createFont();
    void text_clearCache();

    BOOL allocSelArray();
    void freeSelTransparent();
    BOOL isSelExist();

    void sel_release(BOOL bUpdate);
    void sel_all();
    void sel_inverse();
    void sel_fillOrErase(BOOL bErase);
    void sel_copy(BOOL bCut);
    void sel_paste();
    BOOL sel_saveFile(const AXString &filename,const AXRectSize &rcs,BOOL bImg);
    BOOL sel_loadFile(const AXString &filename);

    void draw_sel_polygonCommon();
    void draw_sel_box();

    BOOL onDown_sel_brush();
    void onMove_sel_brush();
    void onUp_sel_brush();

    BOOL onDown_sel_dragMove();
    void onMove_sel_dragMove();
    void onUp_sel_dragMove();

    //xor

    BOOL onDown_xorLine(int aftertype);
    void onMove_xorLine(BOOL b45);
    BOOL onUp_xorLine();

    BOOL onDown_xorLine2(int aftertype);
    void onDown2_xorLine2();
    BOOL onCancel_xorLine2();

    BOOL onDown_xorBoxWin(int aftertype);
    void onMove_xorBoxWin(BOOL bSquare);
    void onUp_xorBoxWin();

    BOOL onDown_xorBoxImg(int aftertype);
    void onMove_xorBoxImg(BOOL bSquare);
    void onUp_xorBoxImg();

    BOOL onDown_xorCircle(int aftertype);
    void onMove_xorCircle(BOOL bSquare);
    void onUp_xorCircle();

    BOOL onDown_xorLasso(int aftertype);
    void onMove_xorLasso();
    void onUp_xorLasso();

    BOOL onDown_ruleCenterPos();

    void drawXorLine(const AXPoint &pt1,const AXPoint &pt2);
    void drawXorBoxWin(const AXPoint &pt1,const AXPoint &pt2);
    void drawXorBoxImg();
    void drawXorCircle(const AXPoint &pt1,const AXPoint &pt2);
    void drawXorLasso(BOOL bErase);
    void drawXorBezier(BOOL bErase);
    void drawXorSpline(AXPoint *ppt);
    void drawXorRulePos(const AXPoint &pt);
    void drawXorBrushCircle(BOOL bErase);

    //sub

    BOOL isDownKeyCtrl();
    BOOL isDownKeyShift();
    BOOL isDownKeyAlt();
    BOOL isDownKeyCtrlOnly();
    BOOL isDownKeyShiftOnly();
    BOOL isDownKeyCtrlShiftOnly();
    BOOL isBttActionMain(UINT btt);

    void getNowPoint(AXPoint *ppt);
    int getDrawValBrush(int val,BOOL bUseBrush);
    void getDrawRectSel(AXRect *prc);
    void setMainStatus_line(const AXPoint &pt1,const AXPoint &pt2);

    BOOL create1bitWork(int w,int h);
    void free1bitWork();

    void beginDrawUndo();
    void endDrawUndo(const AXRectSize &rcs);

    void commonFreeDraw(BOOL bTimer=TRUE);
    void commonDrawEnd();
    void commonAfterDraw();

    void setBeforeDraw(int toolno,int brushno=-1,BOOL bDrawFree=FALSE);
    void setBeforeDraw_clearMask();
    void setBeforeDraw_brush(int brushno,BOOL bFree);
    void setBeforeDraw_drawBrushFree();
    void setBeforeDraw_paintCloseBrush();
    void setBeforeDraw_selBrush();
    void setBeforeDraw_selDragMove(BOOL bOverwrite);
    void setBeforeDraw_filter(BOOL bPrev);
    void setBeforeDraw_filterDrawFlash();

    void getDrawPoint(DRAWPOINT *pdst,BOOL bEnableRule,BOOL bRuleInt=FALSE);
    void getDrawPointInt(AXPoint *ppt,BOOL bEnableRule);

    //----- update

    void updateAll();
    void updateAllAndLayer(BOOL bChangeCnt=TRUE);
    void updateRect(const AXRectSize &rcs);
    void updateAfterUndo(int layerno,const AXRectSize &rcs);

    void updateAfterDraw(const AXRectSize &rcs);
    void updateLayerPrev(CLayerItem *p=NULL,BOOL bForce=FALSE);
    void updateLayerPrevAll(BOOL bRedraw);
    void updatePrevWin(const FLAGRECT &rcf);
    void updateCanvasAndPrevWin(const FLAGRECT &rcf,BOOL bPrevWin);

    void updateImage();
    void update_blendImage(const AXRectSize &rcs);

    void updateCanvas(BOOL bReDraw=TRUE,BOOL bHiQuality=TRUE);
    void update_drawCanvas(const AXRectSize &rcsDst,BOOL bHiQuality);

    //----- layer

    int getLayerCnt();
    CLayerItem *getLayer(int no);
    int getLayerPos(CLayerItem *p);
    CLayerImg& getCurLayerImg();

    void changeCurLayer(int no,BOOL bPrev=FALSE);
    void layer_revView(int no);
    void layer_changeMask(int no,int type);
    void layer_revPaintFlag(int no);
    void layer_changeGroup(int no,int type);
    void layer_revCheck(int no);
    void layer_revHilight(int no);
    void layer_changeAlpha(int no,int alpha,BOOL bRedraw);
    void layer_moveDND(int pos,BOOL bUpdate);

    void layer_curUpDown(BOOL bDown);
    void layer_curAlphaUpDown(BOOL bDown);
    void layer_changeCol(DWORD col);

    BOOL layer_new(BOOL bUpdate,LAYERINFO *pUndoInfo=NULL);
    BOOL layer_newFromImg(AXString *pStr,CImage32 *pSrcImg=NULL,BOOL bUpdate=TRUE,BOOL bSrcAlpha=FALSE);
    void layer_newPaste();
    void layer_copy();
    void layer_del(BOOL bUpdate);
    void layer_curClear();
    void layer_combineClear();
    void layer_combineDel(int type);
    void layer_combineSome(BOOL bCheck);
    void layer_combineAllViewNew();
    void layer_moveUp(BOOL bUpdate);
    void layer_moveDown(BOOL bUpdate);
    void layer_viewAll(int type);
    void layer_toggleShowGroup();
    void layer_toggleShowCheck();
    void layer_curHRev();
    void layer_allHRev();

    //------ calc

    void calcViewParam();

    void calcWindowToImage(AXPoint *pDst,double x,double y);
    void calcWindowToImage(double *pX,double *pY,double x,double y);
    void calcImageToWindow(AXPoint *pDst,int x,int y);
    BOOL calcImageToWindowRect(AXRectSize *pdst,const AXRectSize &src);

    BOOL calcUnionRectSize(AXRectSize *pdst,const AXRectSize &src);
    void calcUnionFlagRect(FLAGRECT *pd,const FLAGRECT &src);
    BOOL calcImgRectInCanvas(AXRectSize *prcs,const FLAGRECT &rcf);
    void calcAddFlagRect(FLAGRECT *pd,int addx,int addy);
    BOOL calcRectInCanvasWin(AXRectSize *prcs,const AXRect &rcSrc);

    void calcMoveDstPos(AXPoint *pDst,const AXPoint &src);
    void calcDrawBoxPointInt(AXPoint *pDst);
    void calcDrawLineRect(DRECT *prc);
    void calcDrawBoxPoint(double *pDst);
    void calcLine45(AXPoint *pdst,const AXPoint &st);
    void calcSelCopyRect(AXRect *prc);

    void calcCanvasScrollMax(AXSize *psize);
    int calcCanvasScaleFromSize();

    //------- save

    BOOL saveADW(const AXString &filename,CProgressDlg *pProgDlg);
    int loadADW(const AXString &filename,CProgressDlg *pProgDlg);

    BOOL saveAPD(const AXString &filename,CProgressDlg *pProgDlg);
    int loadAPD(const AXString &filename,CProgressDlg *pProgDlg);

    BOOL savePSD(const AXString &filename,CProgressDlg *pProgDlg);
    int loadPSD(const AXString &filename,CProgressDlg *pProgDlg);
    int errPSDtoLOAD(int err);

    BOOL savePSD_nolayer(const AXString &filename,int bits,CProgressDlg *pProgDlg);

    BOOL saveBMP(const AXString &filename,const CImage32 *pimgSrc,CProgressDlg *pProgDlg);
    BOOL savePNG(const AXString &filename,const CImage32 *pimgSrc,int nLevel,BOOL b32bit,CProgressDlg *pProgDlg);
    BOOL saveJPEG(const AXString &filename,const CImage32 *pimgSrc,int nQuality,int nSamp,CProgressDlg *pProgDlg);

protected:
    void _imgrectToLAYERINFO(LAYERINFO *pInfo,const AXRect &rc);
    int _loadADW_ver2(AXFile *pfile,CProgressDlg *pProgDlg);
    int _loadADW_ver1(AXFile *pfile,CProgressDlg *pProgDlg);
    int _loadPSD_img(AXPSDLoad *psd,CProgressDlg *pProgDlg);
};

#endif
