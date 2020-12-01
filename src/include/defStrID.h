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

#ifndef _AZDRAW_DEFSTRID_H_
#define _AZDRAW_DEFSTRID_H_

/* グループ */

enum STRGROUPID
{
    STRGID_MAINMENU     = 0,
    STRGID_IMGVIEWMENU  = 1,
    STRGID_PREVWINMENU  = 2,
    STRGID_TOOLNAME     = 3,
    STRGID_TOOLWINCMD   = 4,
    STRGID_DRAWTYPE     = 5,
    STRGID_BOXEDIT      = 6,
    STRGID_OPTWINTAB    = 7,
    STRGID_LAYERWINMENU = 8,
    STRGID_LAYERTB_TTIP = 9,
    STRGID_LAYERRMENU   = 10,
    STRGID_PIXELTYPE    = 11,
    STRGID_SELIMGBAR    = 12,
    STRGID_BRUSHSIZELIST = 13,
    STRGID_BRUSHMENU    = 14,
    STRGID_BRUSHVALUE   = 15,
    STRGID_DRAWTEXT     = 16,
    STRGID_SCALEROT     = 17,
    STRGID_FILEDIALOG   = 18,
    STRGID_SCALETYPE    = 19,

    STRGID_OPTWIN_DOT       = 100,
    STRGID_OPTWIN_MOVE      = 101,
    STRGID_OPTWIN_RULE      = 102,
    STRGID_OPTWIN_PAINT     = 103,
    STRGID_OPTWIN_OPTION    = 104,
    STRGID_OPTWIN_GRAD      = 105,
    STRGID_OPTWIN_HEADTAIL  = 106,
    STRGID_OPTWIN_SEL       = 107,

    STRGID_DLG_GRIDOPT      = 1000,
    STRGID_DLG_NEW          = 1001,
    STRGID_DLG_LAYEROPT     = 1002,
    STRGID_DLG_SELIMG       = 1003,
    STRGID_DLG_GRADEDIT     = 1004,
    STRGID_DLG_CHANGEDPI    = 1005,
    STRGID_DLG_RESIZECANVAS = 1006,
    STRGID_DLG_SCALECANVAS  = 1007,
    STRGID_DLG_EXPORT       = 1008,
    STRGID_DLG_KEYOPT       = 1009,
    STRGID_DLG_DEVICEOPT    = 1010,
    STRGID_DLG_OPTION       = 1011,
    STRGID_DLG_DRAWSHAPE    = 1012,

    STRGID_WORD         = 2000,
    STRGID_MESSAGE      = 2001,
    STRGID_HELP         = 2002,
    STRGID_COMMANDLIST  = 2003,
    STRGID_FILTERDLG    = 2004
};

//-------------------

/* 共通単語 */

enum STRID_WORD
{
    STRID_WORD_WIDTH,
    STRID_WORD_HEIGHT,
    STRID_WORD_COLOR,
    STRID_WORD_OPACITY,
    STRID_WORD_NAME,
    STRID_WORD_NOREFLAYER,
    STRID_WORD_USEBRUSHVAL,
    STRID_WORD_ANTIALIAS,
    STRID_WORD_DELETE,
    STRID_WORD_POS,
    STRID_WORD_BRUSH
};

/* メッセージ */

enum STRID_MESSAGE
{
    STRID_MES_ERR_SAVE,
    STRID_MES_DELETE,
    STRID_MES_BRUSH_FOLDER_DEL,
    STRID_MES_ERR_PASTE,
    STRID_MES_ERR_LOAD,
    STRID_MES_ERR_FAILED,
    STRID_MES_NOADW,
    STRID_MES_OVERWRITE,
    STRID_MES_ERR_IMGSIZE,
    STRID_MES_ERR_OPENFILE,
    STRID_MES_ERR_FORMAT,
    STRID_MES_SAVE_CHANGEIMAGE,
    STRID_MES_KEY_SAMEKEY,
    STRID_MES_CLEARALL,
    STRID_MES_DEV_DELETE,
    STRID_MES_DEV_CLEAR,
    STRID_MES_ERR_FILTER_SETTEXTURE
};

/* メインメニュー */

enum STRID_MENU
{
    STRID_MENU_TOP_FILE = 1,
    STRID_MENU_TOP_EDIT,
    STRID_MENU_TOP_SEL,
    STRID_MENU_TOP_FILTER,
    STRID_MENU_TOP_VIEW,
    STRID_MENU_TOP_OPTION,
    STRID_MENU_TOP_HELP,

    //ファイル
    STRID_MENU_FILE_NEW = 10000,
    STRID_MENU_FILE_OPEN,
    STRID_MENU_FILE_SAVE,
    STRID_MENU_FILE_SAVERE,
    STRID_MENU_FILE_EXPORT,
    STRID_MENU_FILE_EXPORTFORMAT,
    STRID_MENU_FILE_RECENT,
    STRID_MENU_FILE_EXIT,

    STRID_MENU_FILE_FORMAT_ADW = 10050,
    STRID_MENU_FILE_FORMAT_APD,
    STRID_MENU_FILE_FORMAT_PSD,
    STRID_MENU_FILE_FORMAT_BMP,
    STRID_MENU_FILE_FORMAT_PNG,
    STRID_MENU_FILE_FORMAT_JPEG,
    STRID_MENU_FILE_FORMAT_PNG32,
    STRID_MENU_FILE_FORMAT_PSD1BIT,
    STRID_MENU_FILE_FORMAT_PSD8BIT,

    //編集
    STRID_MENU_EDIT_UNDO = 10100,
    STRID_MENU_EDIT_REDO,
    STRID_MENU_EDIT_RESIZECANVAS,
    STRID_MENU_EDIT_SCALECANVAS,
    STRID_MENU_EDIT_CHANGEDPI,
    STRID_MENU_EDIT_CURHREV,
    STRID_MENU_EDIT_ALLHREV,
    STRID_MENU_EDIT_DRAWSHAPE,

    //選択範囲
    STRID_MENU_SEL_DESELECT = 10200,
    STRID_MENU_SEL_INVERSE,
    STRID_MENU_SEL_ALLSEL,
    STRID_MENU_SEL_COPY,
    STRID_MENU_SEL_CUT,
    STRID_MENU_SEL_PASTENEW,
    STRID_MENU_SEL_FILL,
    STRID_MENU_SEL_ERASE,
    STRID_MENU_SEL_INPORT,
    STRID_MENU_SEL_EXPORT_SEL,
    STRID_MENU_SEL_EXPORT_SELIMG,

    //フィルタ
    STRID_MENU_FILTER_TOP = 10300,
    STRID_MENU_FILTER_END = 10400,

    STRID_MENU_FILTER_COLOR = 10300,
    STRID_MENU_FILTER_BLUR,
    STRID_MENU_FILTER_DRAW,
    STRID_MENU_FILTER_OTHERLAYER,
    STRID_MENU_FILTER_MOZAIC,
    STRID_MENU_FILTER_MOZAIC2,
    STRID_MENU_FILTER_CRYSTAL,
    STRID_MENU_FILTER_NOISE,
    STRID_MENU_FILTER_EDGE,
    STRID_MENU_FILTER_HALFTONE,
    STRID_MENU_FILTER_1DOTLINE,

    STRID_MENU_FILTER_COL_INVERSE,
    STRID_MENU_FILTER_COL_TEXTURE,
    STRID_MENU_FILTER_COL_LEVEL,
    STRID_MENU_FILTER_COL_GAMMA,
    STRID_MENU_FILTER_COL_2COL,
    STRID_MENU_FILTER_COL_DITHER,

    STRID_MENU_FILTER_BLUR_BLUR,
    STRID_MENU_FILTER_BLUR_GAUSS,
    STRID_MENU_FILTER_BLUR_MOTION,

    STRID_MENU_FILTER_DRAW_AMITONE1,
    STRID_MENU_FILTER_DRAW_AMITONE2,
    STRID_MENU_FILTER_DRAW_LINE,
    STRID_MENU_FILTER_DRAW_CHECK,

    STRID_MENU_FILTER_COMIC,
    STRID_MENU_FILTER_COMIC_CONCLINE,
    STRID_MENU_FILTER_COMIC_FLASH,
    STRID_MENU_FILTER_COMIC_BETAFLASH,
    STRID_MENU_FILTER_COMIC_UNIFLASH,
    STRID_MENU_FILTER_COMIC_UNIFLASH_WAVE,

    //表示
    STRID_MENU_VIEW_TOOLWIN = 10400,
    STRID_MENU_VIEW_LAYERWIN,
    STRID_MENU_VIEW_OPTIONWIN,
    STRID_MENU_VIEW_BRUSHWIN,
    STRID_MENU_VIEW_PREVIEWWIN,
    STRID_MENU_VIEW_IMGVIEWWIN,
    STRID_MENU_VIEW_ALLHIDEVIEW = 10410,
    STRID_MENU_VIEW_GRID,
    STRID_MENU_VIEW_GRIDSPLIT,
    STRID_MENU_VIEW_CANVASHREV,
    STRID_MENU_VIEW_CANVASSCROLLBAR,
    STRID_MENU_VIEW_RESET,

    //設定
    STRID_MENU_OPT_OPTION = 10500,
    STRID_MENU_OPT_DEVICE,
    STRID_MENU_OPT_KEY,
    STRID_MENU_OPT_GRID,

    //ヘルプ
    STRID_MENU_HELP_ABOUT = 10600
};

/* イメージビューウィンドウメニュー */

enum STRID_IMGVIEWMENU
{
    STRID_IMGV_OPEN = 30000,
    STRID_IMGV_NEXT,
    STRID_IMGV_PREV,
    STRID_IMGV_CLEAR,
    STRID_IMGV_SCALEUP,
    STRID_IMGV_SCALEDOWN,
    STRID_IMGV_FITSIZE,
    STRID_IMGV_FULLVIEW,
    STRID_IMGV_SCALE_20,
    STRID_IMGV_SCALE_40,
    STRID_IMGV_SCALE_60,
    STRID_IMGV_SCALE_80,
    STRID_IMGV_SCALE_100,
    STRID_IMGV_SCALE_200,
    STRID_IMGV_SCALE_400,

    STRID_IMGV_SPOIT_LAYER = 10000,
    STRID_IMGV_SPOIT_BRUSH
};

/* プレビューウィンドウメニュー */

enum STRID_PREVWINMENU
{
    STRID_PREVW_LEFTBTT = 0,
    STRID_PREVW_RIGHTBTT,
    STRID_PREVW_FULLVIEW,
    STRID_PREVW_CHANGE_LOUPE,
    STRID_PREVW_CHANGE_NORMAL,

    STRID_PREVW_BTT_PREVSCR = 100,
    STRID_PREVW_BTT_CANVASSCR
};

/* ツールウィンドウのボタン:ツールチップ */

enum STRID_TOOLWINCMD
{
    STRID_TOOLWINCMD_UNDO,
    STRID_TOOLWINCMD_REDO,
    STRID_TOOLWINCMD_DESELECT,
    STRID_TOOLWINCMD_CANVASHREV
};

/* オプションウィンドウタブ名 */

enum STRID_OPTWINTAB
{
    STRID_OPTWINTAB_TOOL,
    STRID_OPTWINTAB_RULE,
    STRID_OPTWINTAB_TEXTURE,
    STRID_OPTWINTAB_INOUT,
    STRID_OPTWINTAB_ENV
};

/* レイヤウィンドウメニュー */

enum STRID_LAYERWINMENU
{
    STRID_LAYERMENU_TOP = 12000,
    STRID_LAYERMENU_END = 12100,

    STRID_LAYERMENU_NEW = STRID_LAYERMENU_TOP,
    STRID_LAYERMENU_LOADIMG,
    STRID_LAYERMENU_COPY,
    STRID_LAYERMENU_DEL,
    STRID_LAYERMENU_CLEAR,
    STRID_LAYERMENU_COMBINECLEAR,
    STRID_LAYERMENU_COMBINEDEL,
    STRID_LAYERMENU_ALLCOMBINE,
    STRID_LAYERMENU_ALLCOMBINE_NEW,
    STRID_LAYERMENU_MOVEUP,
    STRID_LAYERMENU_MOVEDOWN,
    STRID_LAYERMENU_OPTION,
    STRID_LAYERMENU_ALLSHOW,
    STRID_LAYERMENU_ALLHIDE,
    STRID_LAYERMENU_CURSHOW,
    STRID_LAYERMENU_GROUPSHOWTOGGLE,
    STRID_LAYERMENU_CHECKSHOWTOGGLE,
    STRID_LAYERMENU_SAVEPNG,
    STRID_LAYERMENU_COMBINE_CHECK,
    STRID_LAYERMENU_COMBINE_SUB
};

/* レイヤウィンドウ、右クリックメニュー */

enum STRID_LAYERRMENU
{
    STRID_LAYERRMENU_PALETTE1,
    STRID_LAYERRMENU_PALETTE2,
    STRID_LAYERRMENU_PAINT,
    STRID_LAYERRMENU_HILIGHT,
    STRID_LAYERRMENU_MASK,
    STRID_LAYERRMENU_MASKUNDER,
    STRID_LAYERRMENU_GROUP,
    STRID_LAYERRMENU_GROUPUNDER
};

/* ピクセル描画タイプ */

enum STRID_PIXELTYPE
{
    STRID_PIXTYPE_BLEND,
    STRID_PIXTYPE_COMPARE,
    STRID_PIXTYPE_OVERWRITE,
    STRID_PIXTYPE_OVERWRITE2,
    STRID_PIXTYPE_ERASE,
    STRID_PIXTYPE_BLUR
};

/* 画像選択バー名 */

enum STRID_SELIMGBAR
{
    STRID_SELIMGBAR_NONE,
    STRID_SELIMGBAR_USEOPT,
    STRID_SELIMGBAR_FORCE_NONE,
    STRID_SELIMGBAR_DEFAULT
};

/* ブラシサイズ・濃度リストメニュー/ダイアログ */

enum STRID_BRUSHSIZELIST
{
    STRID_BSMENU_ADD_SIZE,
    STRID_BSMENU_ADD_VAL,
    STRID_BSMENU_DEL,
    STRID_BSDLG_MESSAGE = 100
};

/* ブラシメニュー */

enum STRID_BRUSHMENU
{
    STRID_BRMENU_ADD_BRUSH,
    STRID_BRMENU_ADD_FOLDER,
    STRID_BRMENU_DELETE,
    STRID_BRMENU_RENAME,
    STRID_BRMENU_COPY,
    STRID_BRMENU_PASTE,
    STRID_BRMENU_COPYTEXT,
    STRID_BRMENU_REGCLEAR = 100,
    STRID_BRMENU_REG1,
    STRID_BRMENU_REG2,
    STRID_BRMENU_REG3,
    STRID_BRMENU_REG4,
    STRID_BRMENU_REG5
};

/* ブラシ値 */

enum STRID_BRUSHVALUE
{
    STRID_BRVAL_DATTYPE,
    STRID_BRVAL_SAVE,
    STRID_BRVAL_SIZE,
    STRID_BRVAL_VAL,
    STRID_BRVAL_HOSEI,
    STRID_BRVAL_PIXTYPE,
    STRID_BRVAL_SIZEMIN,
    STRID_BRVAL_VALMIN,
    STRID_BRVAL_INTERVAL,
    STRID_BRVAL_RANDOM_SIZE,
    STRID_BRVAL_RANDOM_POS,
    STRID_BRVAL_RANDOM_CIRCLE,
    STRID_BRVAL_RANDOM_BOX,
    STRID_BRVAL_PRESS_SIZE,
    STRID_BRVAL_PRESS_VAL,
    STRID_BRVAL_BRUSHIMG,
    STRID_BRVAL_BRUSHIMG_ROTTYPE,
    STRID_BRVAL_BRUSHIMG_ANGLE,
    STRID_BRVAL_BRUSHIMG_RANDOM,
    STRID_BRVAL_TEXTUREIMG,
    STRID_BRVAL_ANTIALIAS,
    STRID_BRVAL_CURVE,

    STRID_BRVAL_DATTYPE_LINK = 100,
    STRID_BRVAL_DATTYPE_MANUAL,

    STRID_BRVAL_HOSEI_NONE = 200,
    STRID_BRVAL_HOSEI_STR1,
    STRID_BRVAL_HOSEI_STR2,
    STRID_BRVAL_HOSEI_STR3,

    STRID_BRVAL_IMGROT_FIX = 300,
    STRID_BRVAL_IMGROT_DIRMOV
};

/* テキスト描画 */

enum STRID_DRAWTEXT
{
    STRID_DTEXT_TITLE,
    STRID_DTEXT_PREVIEW,
    STRID_DTEXT_STYLE,
    STRID_DTEXT_SIZE,
    STRID_DTEXT_CHARSPACE,
    STRID_DTEXT_LINESPACE,
    STRID_DTEXT_VERT,
    STRID_DTEXT_2COL,
    STRID_DTEXT_HINTING,
    STRID_DTEXT_HINT,

    STRID_DTEXT_HT_NONE = 100,
    STRID_DTEXT_HT_NORMAL,
    STRID_DTEXT_HT_LIGHT,
    STRID_DTEXT_HT_MAX
};

/* 拡大縮小・回転 */

enum STRID_SCALEROT
{
    STRID_SCROT_TITLE,
    STRID_SCROT_SCALE,
    STRID_SCROT_ANGLE,
    STRID_SCROT_PREVIEW,
    STRID_SCROT_HQ_PREVIEW
};

/* ファイルダイアログ */

enum STRID_FILEDIALOG
{
    STRID_FILEDLG_PNGLEVEL,
    STRID_FILEDLG_JPEGQUA,
    STRID_FILEDLG_JPEGSAMP
};

//--------------- オプションウィンドウ

/* 定規 */

enum STRID_OPTWIN_RULE
{
    STRID_OPTWINRULE_ANGLE,
    STRID_OPTWINRULE_CTPOS
};

/* 設定 */

enum STRID_OPTWIN_OPTION
{
    STRID_OPTWINOPT_IMGBKCOL
};

/* ドットペン */

enum STRID_OPTWIN_DOT
{
    STRID_OPTWINDOT_THIN
};

/* 移動 */

enum STRID_OPTWIN_MOVE
{
    STRID_OPTWINMOVE_TYPE1,
    STRID_OPTWINMOVE_TYPE2,
    STRID_OPTWINMOVE_TYPE3
};

/* 塗りつぶし */

enum STRID_OPTWIN_PAINT
{
    STRID_OPTWINPAINT_TYPE1,
    STRID_OPTWINPAINT_TYPE2
};

/* グラデーション */

enum STRID_OPTWIN_GRAD
{
    STRID_OPTWINGRAD_REVERSE,
    STRID_OPTWINGRAD_LOOP
};

/* 入り抜き */

enum STRID_OPTWIN_HEADTAIL
{
    STRID_OPTWINHT_LINE,
    STRID_OPTWINHT_BEZIER,
    STRID_OPTWINHT_HEAD,
    STRID_OPTWINHT_TAIL,
    STRID_OPTWINHT_SET
};

/* 選択範囲 */

enum STRID_OPTWIN_SEL
{
    STRID_OPTWINSEL_COL,
    STRID_OPTWINSEL_OPACITY
};

//--------------- ダイアログ

/* グリッド設定ダイアログ */

enum STRID_DLG_GRIDOPT
{
    STRID_GRIDOPT_TITLE,
    STRID_GRIDOPT_GRID,
    STRID_GRIDOPT_GRIDSPLIT,
    STRID_GRIDOPT_COLSPLIT,
    STRID_GRIDOPT_ROWSPLIT
};

/* 新規作成ダイアログ */

enum STRID_DLG_NEW
{
    STRID_NEWDLG_TITLE,
    STRID_NEWDLG_REGIST,
    STRID_NEWDLG_RECENT,
    STRID_NEWDLG_DEFINED
};

/* レイヤ設定ダイアログ */

enum STRID_DLG_LAYEROPT
{
    STRID_LAYEROPT_TITLE,
    STRID_LAYEROPT_PALINFO
};

/* 画像選択ダイアログ */

enum STRID_DLG_SELIMG
{
    STRID_SELIMGDLG_TITLE,
    STRID_SELIMGDLG_NONE,
    STRID_SELIMGDLG_USEOPT,
    STRID_SELIMGDLG_FORCE_NONE,
    STRID_SELIMGDLG_DEFAULT
};

/* グラデーション編集 */

enum STRID_DLG_GRADEDIT
{
    STRID_GRADEDIT_TITLE,
    STRID_GRADEDIT_INFO
};

/* DPI値変更 */

enum STRID_DLG_CHANGEDPI
{
    STRID_CHANGEDPI_TITLE,
    STRID_CHANGEDPI_MESSAGE
};

/* キャンバスサイズ変更 */

enum STRID_DLG_RESIZECANVAS
{
    STRID_RCDLG_TITLE,
    STRID_RCDLG_ALIGN
};

/* キャンバス拡大縮小 */

enum STRID_DLG_SCALECANVAS
{
    STRID_SCDLG_TITLE,
    STRID_SCDLG_SIZE,
    STRID_SCDLG_PERS,
    STRID_SCDLG_KEEPASPECT,
    STRID_SCDLG_CHANGEDPI,
    STRID_SCDLG_TYPE
};

/* エクスポート */

enum STRID_DLG_EXPORT
{
    STRID_EXPORT_TITLE,
    STRID_EXPORT_FORMAT,
    STRID_EXPORT_SCALE,
    STRID_EXPORT_PERS,
    STRID_EXPORT_SCALETYPE,
    STRID_EXPORT_KEEPASPECT
};

/* 座標を指定して図形描画 */

enum STRID_DLG_DRAWSHAPE
{
    STRID_DSHAPE_TITLE,
    STRID_DSHAPE_SHAPE,
    STRID_DSHAPE_DRAWVAL,
    STRID_DSHAPE_UNIT,
    STRID_DSHAPE_START_X,
    STRID_DSHAPE_START_Y,
    STRID_DSHAPE_LENGTH,
    STRID_DSHAPE_ANGLE,
    STRID_DSHAPE_WIDTH,
    STRID_DSHAPE_HEIGHT,
    STRID_DSHAPE_CENTER_X,
    STRID_DSHAPE_CENTER_Y,
    STRID_DSHAPE_RADIUS,

    STRID_DSHAPE_LINE = 100,
    STRID_DSHAPE_BOX,
    STRID_DSHAPE_CIRCLE,
    STRID_DSHAPE_FILLBOX,
    STRID_DSHAPE_FILLCIRCLE,
    STRID_DSHAPE_CURBRUSH = 200,
    STRID_DSHAPE_DOT1PX,
    STRID_DSHAPE_DOT2PX,
    STRID_DSHAPE_DOT3PX
};

/* キー設定 */

enum STRID_DLG_KEYOPT
{
    STRID_KEYOPT_TITLE,
    STRID_KEYOPT_ALLCLEAR,
    STRID_KEYOPT_TIPS
};

/* デバイス設定 */

enum STRID_DLG_DEVICEOPT
{
    STRID_DEVOPT_TITLE,
    STRID_DEVOPT_DEFAULT,
    STRID_DEVOPT_DEVDEL,
    STRID_DEVOPT_DEVCLEAR,
    STRID_DEVOPT_BTTMOD,
    STRID_DEVOPT_ACTION,
    STRID_DEVOPT_HELP,
    STRID_DEVOPT_BUTTONCHECK
};

/* 環境設定 */

enum STRID_DLG_OPTION
{
    STRID_OPTDLG_TITLE,
    STRID_OPTDLG_INITW  = 1000,
    STRID_OPTDLG_INITH,
    STRID_OPTDLG_CANVASCOL,
    STRID_OPTDLG_UNDOCNT,
    STRID_OPTDLG_SUBWINFONT,
    STRID_OPTDLG_RESTART_HINT,
    STRID_OPTDLG_SUBWINTYPE,
    
    STRID_OPTDLG_SUBWINTYPE_TOP = 1500,

    STRID_OPTDLG_FLAGS_TOP  = 2000,

    STRID_OPTDLG_STEP_SCALED = 3000,
    STRID_OPTDLG_STEP_SCALEU,
    STRID_OPTDLG_STEP_ROTATE,
    STRID_OPTDLG_STEP_BRUSHSIZE,

    STRID_OPTDLG_TEXTUREDIR = 4000,
    STRID_OPTDLG_BRUSHDIR,

    STRID_OPTDLG_HOTSPOT = 5000,
    STRID_OPTDLG_CURSOR_LOADIMG,
    STRID_OPTDLG_CURSOR_DEFAULT,
    STRID_OPTDLG_CURSOR_HELP
};

/* フィルタダイアログ */

enum STRID_FILTERDLG
{
    STRID_FILTERDLG_PREVIEW = 1,
    STRID_FILTERDLG_CLIPPING,
    STRID_FILTERDLG_RESET
};

//-----------

/* ヘルプ */

enum STRID_HELP
{
    STRID_HELP_TOOL_PAINT,
    STRID_HELP_TOOL_GRAD,
    STRID_HELP_TOOL_TEXT,
    STRID_HELP_TOOL_MOVE,
    STRID_HELP_TOOL_ROTATE,
    STRID_HELP_TOOL_SEL,
    STRID_HELP_TOOL_SPOIT,
    STRID_HELP_DRAW_BRUSH_FREE = 100,
    STRID_HELP_DRAW_DOT_FREE,
    STRID_HELP_DRAW_POLYGON,
    STRID_HELP_DRAW_LINE = 201,
    STRID_HELP_DRAW_BOX,
    STRID_HELP_DRAW_CIRCLE,
    STRID_HELP_DRAW_SUCC,
    STRID_HELP_DRAW_CONC,
    STRID_HELP_DRAW_BEZIER,
    STRID_HELP_DRAW_SPLINE
};

/* キー設定などのコマンドリスト */

enum STRID_COMMANDLIST
{
    STRID_CMDLIST_MAINMENU  = 100,
    STRID_CMDLIST_LAYERMENU,
    STRID_CMDLIST_LAYEREXCMD,
    STRID_CMDLIST_TOOL,
    STRID_CMDLIST_DRAWTYPE,
    STRID_CMDLIST_ETCCMD,
    STRID_CMDLIST_REGBRUSH,
    STRID_CMDLIST_KEY_TOOL,
    STRID_CMDLIST_DEV_TOOL,
    STRID_CMDLIST_KEY_DRAWTYPE,
    STRID_CMDLIST_DEV_DRAWTYPE,
    STRID_CMDLIST_KEY_REGBRUSH,
    STRID_CMDLIST_DEV_REGBRUSH,
    STRID_CMDLIST_KEY_REGBRUSH_PRESSMAX,
    STRID_CMDLIST_DEV_REGBRUSH_PRESSMAX,
    STRID_CMDLIST_KEY_ETC,
    STRID_CMDLIST_DEV_ETC
};

#endif