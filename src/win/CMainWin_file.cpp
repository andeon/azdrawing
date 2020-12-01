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
/*
    CMainWin [file] - ファイル関連
*/

#include "CMainWin.h"

#include "CConfig.h"
#include "CDraw.h"
#include "CImage32.h"
#include "CResizeImage.h"

#include "CProgressDlg.h"
#include "CNewDlg.h"
#include "CGetFileNameDlg.h"
#include "CExportDlg.h"

#include "AXFile.h"
#include "AXMenu.h"
#include "AXMessageBox.h"
#include "AXUtilStr.h"
#include "AXUtilImg.h"
#include "AXApp.h"

#include "defGlobal.h"
#include "defStrID.h"


//----------------------

void *thread_loadImage(void *);
void *thread_saveFile(void *);

struct FILESAVE_PROGINFO
{
    AXString    *pstrFileName;
    int         nFormat,
                nScaleWidth,
                nScaleHeight,
                nScaleType;
    BOOL        bExportScale;   //エクスポートで拡大縮小処理を行うか
};

//----------------------


//! 新規作成

void CMainWin::newImage()
{
    CNewDlg *pdlg;
    int w,h;
    AXString str;

    pdlg = new CNewDlg(this);

    if(!pdlg->runDialog()) return;

    w = g_pconf->dwNewRecentSize[0] >> 16;
    h = g_pconf->dwNewRecentSize[0] & 0xffff;

    //確認

    if(!checkImageUpdate()) return;

    //新規

    if(!g_pdraw->newImage(w, h, g_pconf->wNewRecentDPI[0], TRUE))
    {
        g_pdraw->newImage(300, 300, g_pconf->wNewRecentDPI[0], TRUE);
    }

    //更新

    updateNewCanvas(&str);
}


//=============================
//画像開く
//=============================


//! 開く

void CMainWin::openFile()
{
    AXString str,filter;

    //ファイル名取得

    filter = "Image File (ADW/APD/PSD/BMP/PNG/JPEG/GIF)\t*.adw;*.apd;*.psd;*.bmp;*.png;*.jpg;*.jpeg;*.gif\tAzDrawing file (*.adw)\t*.adw\tAll files\t*";

    if(!AXFileDialog::openFile(this, filter, 0, g_pconf->strOpenDir, 0, &str))
        return;

    //保存確認

    if(!checkImageUpdate()) return;

    //読み込み

    loadImage(str);
}

//! 画像ファイル読み込み処理

BOOL CMainWin::loadImage(const AXString &filename)
{
    AXString str;
    CProgressDlg *pdlg;
    int ret,format;

    //画像フォーマット

    format = _getFileFormat(filename);
    if(format == -1) return FALSE;

    //読み込み

    str = filename;

    pdlg = new CProgressDlg(this, thread_loadImage, (LPVOID)&str, (LPVOID)format);

    ret = pdlg->run();

    //

    if(ret == CDraw::LOADERR_SUCCESS)
    {
        //--------- 成功

        m_nFileFormat = format;

        //フォルダ記録

        g_pconf->strOpenDir.path_removeFileName(str);

        //最近使ったファイル

        _addRecentFile(str);

        //更新

        updateNewCanvas(&str);

        return TRUE;
    }
    else
    {
        //--------- エラー

        WORD errid;

        //エラーメッセージ

        switch(ret)
        {
            case CDraw::LOADERR_IMGSIZE: errid = STRID_MES_ERR_IMGSIZE; break;
            case CDraw::LOADERR_OPENFILE: errid = STRID_MES_ERR_OPENFILE; break;
            case CDraw::LOADERR_FORMAT: errid = STRID_MES_ERR_FORMAT; break;
            default: errid = STRID_MES_ERR_LOAD; break;
        }

        errMes(STRGID_MESSAGE, errid);

        /* レイヤが一つもなければ新規作成。
           カレントレイヤが指定されていなければ途中まで読み込まれた */

        if(g_pdraw->getLayerCnt() == 0 || g_pdraw->m_pCurLayer == NULL)
        {
            //新規イメージ

            if(g_pdraw->getLayerCnt() == 0)
                g_pdraw->newImage(400, 400, -1, TRUE);

            //カレントレイヤ

            if(!g_pdraw->m_pCurLayer)
                g_pdraw->changeCurLayer(0);

            //更新

            updateNewCanvas(&str);
        }

        return FALSE;
    }
}

//! [スレッド] 画像ファイル読み込み

void *thread_loadImage(void *pParam)
{
    CProgressDlg *pdlg = (CProgressDlg *)pParam;
    AXString *pstr;
    int format,ret = 0;

    pstr   = (AXString *)pdlg->m_pParam1;
    format = (int)((long)pdlg->m_pParam2);

    switch(format)
    {
        //ADW
        case CMainWin::FILEFORMAT_ADW:
            ret = g_pdraw->loadADW(*pstr, pdlg);
            break;
        //APD
        case CMainWin::FILEFORMAT_APD:
            ret = g_pdraw->loadAPD(*pstr, pdlg);
            break;
        //PSD
        case CMainWin::FILEFORMAT_PSD:
            ret = g_pdraw->loadPSD(*pstr, pdlg);
            ret = g_pdraw->errPSDtoLOAD(ret);
            break;

        //画像ファイル
        default:
            pdlg->setProgMax(1);
            ret = g_pdraw->loadImage(*pstr);
            pdlg->incProg();
            break;
    }

    pdlg->endThread(ret);

    return NULL;
}

//! ファイルからヘッダを読み取り画像フォーマット判定
/*!
    @return -1 でエラー
*/

int CMainWin::_getFileFormat(const AXString &filename)
{
    BYTE d[8];
    int n;

    //先頭8バイト読み込み

    if(!AXFile::readFile(filename, d, 8)) return -1;

    //ヘッダから判別

    if(d[0] == 'A' && d[1] == 'Z' && d[2] == 'D' && d[3] == 'W' &&
       d[4] == 'D' && d[5] == 'A' && d[6] == 'T')
        //ADW
        return FILEFORMAT_ADW;
    else if(d[0] == 'A' && d[1] == 'Z' && d[2] == 'P' &&
            d[3] == 'D' && d[4] == 'A' && d[5] == 'T' && d[6] == 'A')
        //APD
        return FILEFORMAT_APD;
    else if(d[0] == '8' && d[1] == 'B' && d[2] == 'P' && d[3] == 'S')
        //PSD
        return FILEFORMAT_PSD;
    else
    {
        //その他

        n = AXGetImageFileType(d);

        switch(n)
        {
            case 'B': return FILEFORMAT_BMP;
            case 'P': return FILEFORMAT_PNG;
            case 'J': return FILEFORMAT_JPEG;
            case 'G': return FILEFORMAT_GIF;
            default: return -1;
        }
    }
}


//=============================
//画像保存
//=============================


//! ファイルに保存
/*!
    @param  bRename 別名で保存
    @return FALSE でキャンセルされた
*/

BOOL CMainWin::saveFile(BOOL bRename)
{
    AXString strName,str;
    int format,ret;

    /*
        strName にファイル名、format に保存形式をセットする
    */

    //-------- 上書き保存時

    if(!bRename && m_strFileName.isNoEmpty())
    {
        ret = _save_overwrite();

        if(ret == 0)
            //キャンセル
            return FALSE;
        else if(ret == 1)
        {
            //上書き保存

            strName = m_strFileName;
            format  = m_nFileFormat;
        }
        else if(ret == 2)
            //別名保存
            bRename = TRUE;
    }

    //--------- ファイル名取得
    //（別名保存か新規イメージの場合）

    if(bRename || m_strFileName.isEmpty())
    {
        //フィルタ

        _getSaveFilterString(&str, FILEFORMAT_ALL);

        //ファイル名初期

        if(!m_strFileName.isEmpty())
            strName.path_filenameNoExt(m_strFileName);

        //ファイル名・フォーマット取得

        if(!CGetFileNameDlg::getSaveFile(this, str, 0, g_pconf->strSaveDir, 0, &strName, &format))
            return FALSE;

        //拡張子セット

        _setFormatExt(&strName, format);
    }

    //------- 保存

    CProgressDlg *pdlg;
    FILESAVE_PROGINFO pinfo;

    //

    pinfo.pstrFileName  = &strName;
    pinfo.nFormat       = format;
    pinfo.bExportScale  = FALSE;

    //進捗ダイアログ

    pdlg = new CProgressDlg(this, thread_saveFile, &pinfo);

    if(!pdlg->run())
        errMes(STRGID_MESSAGE, STRID_MES_ERR_SAVE);
    else
    {
        //--------- 成功

        m_strFileName = strName;
        m_nFileFormat = format;

        //保存ディレクトリ

        g_pconf->strSaveDir.path_removeFileName(strName);

        //タイトル

        setWinTitle();

        //最近使ったファイル

        _addRecentFile(strName);

        //UNDOイメージ変更フラグOFF

        g_pdraw->setUndoChangeOff();
    }

    return TRUE;
}

//! 上書き保存時のメッセージ表示など
/*!
    @return [0]キャンセル [1]上書き保存 [2]別名保存
*/

BOOL CMainWin::_save_overwrite()
{
    UINT ret;

    //GIF 形式での上書き保存は不可 -> 別名保存

    if(m_nFileFormat == FILEFORMAT_GIF)
        return 2;

    //ADW 形式以外の場合、ADW で保存するか選択

    if(m_nFileFormat != FILEFORMAT_ADW && (g_pconf->uFlags & CConfig::FLAG_MES_SAVE_ADW))
    {
        ret = AXMessageBox::message(this, NULL, _string(STRGID_MESSAGE, STRID_MES_NOADW),
                                 AXMessageBox::YES | AXMessageBox::NO | AXMessageBox::NOTSHOW,
                                 AXMessageBox::YES);

        //このメッセージ表示しない

        if(ret & AXMessageBox::NOTSHOW)
            g_pconf->uFlags ^= CConfig::FLAG_MES_SAVE_ADW;

        //

        if(ret & AXMessageBox::YES)
            return 2;
        else
            return 1;
    }

    //上書き保存確認メッセージ

    if(g_pconf->uFlags & CConfig::FLAG_MES_SAVE_OVERWRITE)
    {
        AXString str;

        str.path_filename(m_strFileName);
        str += '\n';
        str += _string(STRGID_MESSAGE, STRID_MES_OVERWRITE);

        ret = AXMessageBox::message(this, NULL, str,
                                 AXMessageBox::SAVE | AXMessageBox::CANCEL | AXMessageBox::NOTSHOW,
                                 AXMessageBox::SAVE);

        //

        if(ret & AXMessageBox::NOTSHOW)
            g_pconf->uFlags ^= CConfig::FLAG_MES_SAVE_OVERWRITE;

        if(!(ret & AXMessageBox::SAVE))
            return 0;
    }

    return 1;
}

//! [スレッド] 画像保存（エクスポート時も共通）

void *thread_saveFile(void *pParam)
{
    CProgressDlg *pdlg = (CProgressDlg *)pParam;
    FILESAVE_PROGINFO *pinfo;
    CImage32 img32,*pimg32;
    int ret = FALSE,bAlpha;

    pinfo = (FILESAVE_PROGINFO *)pdlg->m_pParam1;

    switch(pinfo->nFormat)
    {
        //ADW
        case CMainWin::FILEFORMAT_ADW:
            ret = g_pdraw->saveADW(*pinfo->pstrFileName, pdlg);
            break;
        //APD
        case CMainWin::FILEFORMAT_APD:
            ret = g_pdraw->saveAPD(*pinfo->pstrFileName, pdlg);
            break;
        //PSD（レイヤあり）
        case CMainWin::FILEFORMAT_PSD:
            g_pdraw->blendAllLayer(FALSE);

            ret = g_pdraw->savePSD(*pinfo->pstrFileName, pdlg);

            g_pdraw->updateImage();
            break;
        //PSD（レイヤなし）
        case CMainWin::FILEFORMAT_PSD1BIT:
        case CMainWin::FILEFORMAT_PSD8BIT:
            g_pdraw->blendAllLayer(FALSE);

            ret = g_pdraw->savePSD_nolayer(*pinfo->pstrFileName,
                                           (pinfo->nFormat == CMainWin::FILEFORMAT_PSD1BIT)? 1: 8,
                                           pdlg);

            g_pdraw->updateImage();
            break;

        //一枚絵画像ファイル
        default:
            bAlpha = (pinfo->nFormat == CMainWin::FILEFORMAT_PNG32);

            //m_pimgBlend に合成（メモリ節約のため）

            g_pdraw->blendAllLayer(bAlpha);

            //元イメージ

            if(!pinfo->bExportScale)
                pimg32 = g_pdraw->m_pimgBlend;
            else
            {
                //拡大縮小

                CResizeImage resize;
                AXSize sizeOld,sizeNew;

                if(!img32.create(pinfo->nScaleWidth, pinfo->nScaleHeight))
                    goto END;

                sizeOld.w = g_pdraw->m_nImgW;
                sizeOld.h = g_pdraw->m_nImgH;
                sizeNew.w = pinfo->nScaleWidth;
                sizeNew.h = pinfo->nScaleHeight;

                if(!resize.resize((LPBYTE)g_pdraw->m_pimgBlend->getBuf(), (LPBYTE)img32.getBuf(),
                                  sizeOld, sizeNew, pinfo->nScaleType,
                                  (bAlpha)? CResizeImage::IMGTYPE_32BIT_RGBA: CResizeImage::IMGTYPE_32BIT_RGB,
                                  pdlg))
                    goto END;

                pimg32 = &img32;
            }

            //保存

            if(pinfo->nFormat == CMainWin::FILEFORMAT_BMP)
                //BMP
                ret = g_pdraw->saveBMP(*pinfo->pstrFileName, pimg32, pdlg);
            else if(pinfo->nFormat == CMainWin::FILEFORMAT_PNG || pinfo->nFormat == CMainWin::FILEFORMAT_PNG32)
                //PNG
                ret = g_pdraw->savePNG(*pinfo->pstrFileName, pimg32, g_pconf->btPNGLevel, bAlpha, pdlg);
            else if(pinfo->nFormat == CMainWin::FILEFORMAT_JPEG)
                //JPEG
                ret = g_pdraw->saveJPEG(*pinfo->pstrFileName, pimg32, g_pconf->btJPEGQua, g_pconf->wJPEGSamp, pdlg);

            //m_pimgBlend 戻す

            g_pdraw->updateImage();

            break;
    }

END:
    pdlg->endThread(ret);

    return NULL;
}

//! 保存ダイアログのフィルタ文字列取得

void CMainWin::_getSaveFilterString(AXString *pstr,int format)
{
    LPCSTR pc[] = {
        "AzDrawing (*.adw)\t*.adw\t",
        "AzPainter (*.apd)\t*.apd\t",
        "PhotoShop (*.psd)\t*.psd\t",
        "BMP (*.bmp)\t*.bmp\t",
        "PNG (*.png)\t*.png\t",
        "JPEG (*.jpg)\t*.jpg\t"
    };

    if(format == FILEFORMAT_ALL)
    {
        pstr->empty();

        for(int i = 0; i < 6; i++)
            *pstr += pc[i];
    }
    else
    {
        switch(format)
        {
            case FILEFORMAT_ADW:
                *pstr = pc[0];
                break;
            case FILEFORMAT_APD:
                *pstr = pc[1];
                break;
            case FILEFORMAT_PSD:
            case FILEFORMAT_PSD1BIT:
            case FILEFORMAT_PSD8BIT:
                *pstr = pc[2];
                break;
            case FILEFORMAT_BMP:
                *pstr = pc[3];
                break;
            case FILEFORMAT_PNG:
            case FILEFORMAT_PNG32:
                *pstr = pc[4];
                break;
            case FILEFORMAT_JPEG:
                *pstr = pc[5];
                break;
        }
    }
}

//! 拡張子をセット

void CMainWin::_setFormatExt(AXString *pstr,int format)
{
    LPCSTR pc[] = { "adw", "apd", "psd", "bmp", "png", "jpg" };
    int no = -1;

    switch(format)
    {
        case FILEFORMAT_ADW:
            no = 0;
            break;
        case FILEFORMAT_APD:
            no = 1;
            break;
        case FILEFORMAT_PSD:
        case FILEFORMAT_PSD1BIT:
        case FILEFORMAT_PSD8BIT:
            no = 2;
            break;
        case FILEFORMAT_BMP:
            no = 3;
            break;
        case FILEFORMAT_PNG:
        case FILEFORMAT_PNG32:
            no = 4;
            break;
        case FILEFORMAT_JPEG:
            no = 5;
            break;
    }

    if(no != -1)
        pstr->path_setExt(pc[no]);
}


//===============================
// エクスポート
//===============================


//! 画像エクスポート
/*!
    @param format 保存フォーマット（-1 でダイアログ表示）
*/

void CMainWin::exportFile(int format)
{
    FILESAVE_PROGINFO info;
    AXString strName,filter;

    //--------- 保存パラメータ

    info.pstrFileName = &strName;

    if(format == -1)
    {
        //ダイアログ

        CExportDlg *pdlg;
        CExportDlg::VALUE val;

        val.nFormat    = g_pconf->btExportFormat;
        val.nWidth     = g_pdraw->m_nImgW;
        val.nHeight    = g_pdraw->m_nImgH;
        val.nScaleType = g_pconf->btScaleType;

        //

        pdlg = new CExportDlg(this, &val);
        if(!pdlg->runDialog()) return;

        //設定保存

        g_pconf->btExportFormat = val.nFormat;
        g_pconf->btScaleType    = val.nScaleType;

        //

        info.nFormat      = _convFormatType(val.nFormat);
        info.bExportScale = (val.nWidth != g_pdraw->m_nImgW || val.nHeight != g_pdraw->m_nImgH);
        info.nScaleWidth  = val.nWidth;
        info.nScaleHeight = val.nHeight;
        info.nScaleType   = val.nScaleType;
    }
    else
    {
        //フォーマット指定（等倍）

        info.nFormat      = format;
        info.bExportScale = FALSE;
    }

    //--------- ファイル名取得

    //フィルタ

    _getSaveFilterString(&filter, info.nFormat);

    //ファイル名取得

    if(!CGetFileNameDlg::getSaveFile(this, filter, 0, g_pconf->strExportDir, 0, &strName))
        return;

    //拡張子セット

    _setFormatExt(&strName, info.nFormat);

    //ディレクトリ保存

    g_pconf->strExportDir.path_removeFileName(strName);

    //--------- 保存処理

    CProgressDlg *pProgDlg;

    pProgDlg = new CProgressDlg(this, thread_saveFile, &info);

    if(!pProgDlg->run())
        errMes(STRGID_MESSAGE, STRID_MES_ERR_SAVE);
}

//! フォーマット番号を CMainWin::FILEFORMAT に変換

int CMainWin::_convFormatType(int format)
{
    int tbl[9] = {
        FILEFORMAT_ADW, FILEFORMAT_APD, FILEFORMAT_PSD,
        FILEFORMAT_BMP, FILEFORMAT_PNG, FILEFORMAT_JPEG,
        FILEFORMAT_PNG32, FILEFORMAT_PSD1BIT, FILEFORMAT_PSD8BIT
    };

    return tbl[format];
}


//===============================
// サブ
//===============================


//! 最近使ったファイルに追加

void CMainWin::_addRecentFile(const AXString &filename)
{
    AXAddRecentString(g_pconf->strRecFile, CConfig::RECFILE_NUM, filename);

    m_pmenuRecFile->setStrArray(CMDID_MENU_RECENTFILE, g_pconf->strRecFile, CConfig::RECFILE_NUM);
}
