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

#ifndef _AZDRAW_MAINWIN_H_
#define _AZDRAW_MAINWIN_H_

#include "AXTopWindow.h"
#include "AXString.h"

class AXMenu;

class CMainWin:public AXTopWindow
{
public:
    static CMainWin *m_pSelf;

public:
    enum FILEFORMAT
    {
        FILEFORMAT_ALL  = -1,

        FILEFORMAT_ADW  = 0,
        FILEFORMAT_APD,
        FILEFORMAT_PSD,
        FILEFORMAT_BMP,
        FILEFORMAT_PNG,
        FILEFORMAT_JPEG,
        FILEFORMAT_GIF,

        FILEFORMAT_PNG32 = 100,
        FILEFORMAT_PSD1BIT,
        FILEFORMAT_PSD8BIT
    };

protected:
    UINT        m_uTmpFlags;
    int         m_nFileFormat;  //!< 編集ファイルの画像フォーマット
    AXString    m_strFileName;  //!< 編集ファイル名

    AXMenu  *m_pmenuRecFile;

    enum
    {
        CMDID_MENU_RECENTFILE = 0x00010000,

        TMPFLAG_SHOW          = 1,
        TMPFLAG_MAXIMIZE_INIT = 2
    };

protected:
    void _createMenu();
    void _setAccelerator();

    void _addRecentFile(const AXString &filename);
    int _save_overwrite();
    int _getFileFormat(const AXString &filename);
    void _getSaveFilterString(AXString *pstr,int format);
    void _setFormatExt(AXString *pstr,int format);
    int _convFormatType(int format);

    void _command_ex(UINT id);

    void _command_filter(UINT id);

public:
    virtual ~CMainWin();
    CMainWin();

    BOOL isShowMain() { return (m_uTmpFlags & TMPFLAG_SHOW) && !(m_uTmpFlags & TMPFLAG_MAXIMIZE_INIT); }
    const AXString& getEditFileName() const { return m_strFileName; }

    //

    void showMainWin();

    void updateNewCanvas(AXString *pstr);
    BOOL checkImageUpdate();
    void setWinTitle();
    void getFileInitDir(AXString *pstr);

    //file

    void newImage();
    void openFile();
    BOOL loadImage(const AXString &filename);
    BOOL saveFile(BOOL bRename);
    void exportFile(int format=-1);

    //command

    void allWindowHideShow();

    void undoRedo(BOOL bRedo);
    void changeDPI();
    void resizeCanvas();
    void scaleCanvas();
    void drawShape();

    void sel_export(BOOL bImg);
    void sel_inport();

    void keyOption();
    void gridOption();
    void deviceOption();
    void envOption();

    //

    virtual BOOL onClose();
    virtual BOOL onMap();
    virtual BOOL onConfigure(AXHD_CONFIGURE *phd);
    virtual BOOL onDND_Drop(AXDND *pDND);
    virtual BOOL onCommand(UINT uID,ULONG lParam,int from);
    virtual BOOL onMenuPopup(AXMenuBar *pMenuBar,AXMenu *pMenu,UINT uID);
};

#define MAINWIN     (CMainWin::m_pSelf)

#endif
