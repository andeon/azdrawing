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

//----------------- メインメニューデータ

#ifdef _AZDW_MENUDAT_MAINMENU

WORD g_mainmenudat[] = {
    //ファイル
    STRID_MENU_TOP_FILE,
    STRID_MENU_FILE_NEW, STRID_MENU_FILE_OPEN, 0xffff,
    STRID_MENU_FILE_SAVE, STRID_MENU_FILE_SAVERE, 0xffff,
    STRID_MENU_FILE_EXPORT, STRID_MENU_FILE_EXPORTFORMAT,
    0xfffe,
        STRID_MENU_FILE_FORMAT_ADW, STRID_MENU_FILE_FORMAT_APD, STRID_MENU_FILE_FORMAT_PSD,
        STRID_MENU_FILE_FORMAT_BMP, STRID_MENU_FILE_FORMAT_PNG, STRID_MENU_FILE_FORMAT_JPEG,
        STRID_MENU_FILE_FORMAT_PNG32, STRID_MENU_FILE_FORMAT_PSD1BIT, STRID_MENU_FILE_FORMAT_PSD8BIT,
    0xfffe,
    0xffff, STRID_MENU_FILE_RECENT, 0xffff, STRID_MENU_FILE_EXIT,

    //編集
    STRID_MENU_TOP_EDIT,
    STRID_MENU_EDIT_UNDO, STRID_MENU_EDIT_REDO, 0xffff,
    STRID_MENU_EDIT_RESIZECANVAS, STRID_MENU_EDIT_SCALECANVAS, STRID_MENU_EDIT_CHANGEDPI, 0xffff,
    STRID_MENU_EDIT_CURHREV, STRID_MENU_EDIT_ALLHREV, STRID_MENU_EDIT_DRAWSHAPE,

    //選択範囲
    STRID_MENU_TOP_SEL,
    STRID_MENU_SEL_DESELECT, STRID_MENU_SEL_INVERSE, STRID_MENU_SEL_ALLSEL, 0xffff,
    STRID_MENU_SEL_COPY, STRID_MENU_SEL_CUT, STRID_MENU_SEL_PASTENEW, 0xffff,
    STRID_MENU_SEL_FILL, STRID_MENU_SEL_ERASE, 0xffff,
    STRID_MENU_SEL_INPORT, STRID_MENU_SEL_EXPORT_SEL, STRID_MENU_SEL_EXPORT_SELIMG,

    //フィルタ
    STRID_MENU_TOP_FILTER,
    STRID_MENU_FILTER_COLOR,
    0xfffe,
        STRID_MENU_FILTER_COL_INVERSE, STRID_MENU_FILTER_COL_TEXTURE,
        STRID_MENU_FILTER_COL_LEVEL, STRID_MENU_FILTER_COL_GAMMA,
        STRID_MENU_FILTER_COL_2COL, STRID_MENU_FILTER_COL_DITHER,
    0xfffe,
    STRID_MENU_FILTER_BLUR,
    0xfffe,
        STRID_MENU_FILTER_BLUR_BLUR, STRID_MENU_FILTER_BLUR_GAUSS,
        STRID_MENU_FILTER_BLUR_MOTION,
    0xfffe,
    STRID_MENU_FILTER_DRAW,
    0xfffe,
        STRID_MENU_FILTER_DRAW_AMITONE1, STRID_MENU_FILTER_DRAW_AMITONE2,
        STRID_MENU_FILTER_DRAW_LINE, STRID_MENU_FILTER_DRAW_CHECK,
    0xfffe,
    STRID_MENU_FILTER_COMIC,
    0xfffe,
        STRID_MENU_FILTER_COMIC_CONCLINE, STRID_MENU_FILTER_COMIC_FLASH,
        STRID_MENU_FILTER_COMIC_BETAFLASH, STRID_MENU_FILTER_COMIC_UNIFLASH,
        STRID_MENU_FILTER_COMIC_UNIFLASH_WAVE,
    0xfffe,
    0xffff,
    STRID_MENU_FILTER_OTHERLAYER, STRID_MENU_FILTER_MOZAIC, STRID_MENU_FILTER_MOZAIC2,
    STRID_MENU_FILTER_CRYSTAL, STRID_MENU_FILTER_NOISE, STRID_MENU_FILTER_EDGE,
    STRID_MENU_FILTER_HALFTONE, STRID_MENU_FILTER_1DOTLINE,

    //表示
    STRID_MENU_TOP_VIEW,
    STRID_MENU_VIEW_TOOLWIN|0x8000, STRID_MENU_VIEW_LAYERWIN|0x8000, STRID_MENU_VIEW_OPTIONWIN|0x8000,
    STRID_MENU_VIEW_BRUSHWIN|0x8000, STRID_MENU_VIEW_PREVIEWWIN|0x8000, STRID_MENU_VIEW_IMGVIEWWIN|0x8000,
    0xffff,
    STRID_MENU_VIEW_ALLHIDEVIEW, STRID_MENU_VIEW_RESET,
    0xffff,
    STRID_MENU_VIEW_GRID|0x8000, STRID_MENU_VIEW_GRIDSPLIT|0x8000, STRID_MENU_VIEW_CANVASHREV|0x8000,
    STRID_MENU_VIEW_CANVASSCROLLBAR|0x8000,

    //設定
    STRID_MENU_TOP_OPTION,
    STRID_MENU_OPT_OPTION, STRID_MENU_OPT_DEVICE, STRID_MENU_OPT_KEY, STRID_MENU_OPT_GRID,

    //ヘルプ
    STRID_MENU_TOP_HELP,
    STRID_MENU_HELP_ABOUT
};

#endif

//----------------- レイヤメニューデータ

#ifdef _AZDW_MENUDAT_LAYERMENU

WORD g_layermenudat[24] = {
    STRID_LAYERMENU_NEW, STRID_LAYERMENU_LOADIMG, STRID_LAYERMENU_COPY, STRID_LAYERMENU_DEL,
    STRID_LAYERMENU_CLEAR, 0xffff, STRID_LAYERMENU_COMBINECLEAR, STRID_LAYERMENU_COMBINEDEL,
    STRID_LAYERMENU_COMBINE_SUB,
    STRID_LAYERMENU_ALLCOMBINE, STRID_LAYERMENU_ALLCOMBINE_NEW, STRID_LAYERMENU_COMBINE_CHECK, 0xffff,
    STRID_LAYERMENU_MOVEUP, STRID_LAYERMENU_MOVEDOWN, STRID_LAYERMENU_OPTION, 0xffff,
    STRID_LAYERMENU_ALLSHOW, STRID_LAYERMENU_ALLHIDE, STRID_LAYERMENU_CURSHOW,
    STRID_LAYERMENU_GROUPSHOWTOGGLE, STRID_LAYERMENU_CHECKSHOWTOGGLE, 0xffff,
    STRID_LAYERMENU_SAVEPNG
};

#else

extern WORD g_layermenudat[24];

#endif
