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
    CDraw [file] - 画像読み込み/保存関連
*/

#include <string.h>

#include "CDraw.h"

#include "CImage32.h"
#include "CTileImg.h"
#include "CLayerList.h"
#include "CProgressDlg.h"

#include "AXByteString.h"
#include "AXFile.h"
#include "AXBMPSave.h"
#include "AXPNGSave.h"
#include "AXJPEG.h"
#include "AXPSDSave.h"
#include "AXPSDLoad.h"
#include "AXZlib.h"
#include "AXUtil.h"
#include "AXUtilFile.h"
#include "AXApp.h"



//=================================
// ADW (AzDrawing)
//=================================


//! ADW(ver2)保存

BOOL CDraw::saveADW(const AXString &filename,CProgressDlg *pProgDlg)
{
    AXFile file;
    AXZlib zlib;
    CLayerItem *p;
    AXRect rc;
    DWORD dwTileCnt,dwSize;
    WORD name[25];
    int i,topx,topy,ix,iy,nTileW,nTileH,xpos,ypos;
    LPUSTR pc;
    CTileImg **ppTile;

    if(!zlib.allocBuf(4096)) return FALSE;
    if(!zlib.initEncBuf(6)) return FALSE;

    //ファイル開く

    if(!file.openWrite(filename)) return FALSE;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    //------- ヘッダ

    file.writeStr("AZDWDAT");
    //バージョン
    file.writeBYTE(1);

    //------- プレビュー（なし）

    file.writeDWORD((DWORD)0);
    file.writeDWORD((DWORD)0);

    //------- 全体情報

    //データサイズ
    file.writeWORD(12);
    //幅
    file.writeWORD(m_nImgW);
    //高さ
    file.writeWORD(m_nImgH);
    //DPI
    file.writeWORD(m_nImgDPI);
    //レイヤ数
    file.writeWORD(m_player->getCnt());
    //カレントレイヤNo
    file.writeWORD(m_nCurLayerNo);
    //レイヤ情報サイズ
    file.writeWORD(79);

    //------- レイヤ

    pProgDlg->setProgMax(m_player->getCnt() * 4);

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        //---------- イメージ情報

        p->m_img.getExistTileRectPx(&rc);

        dwTileCnt = p->m_img.getAllocTileCnt();

        //px範囲とタイル数

        file.writeDWORD(&rc.left);
        file.writeDWORD(&rc.top);
        file.writeDWORD(&rc.right);
        file.writeDWORD(&rc.bottom);
        file.writeDWORD(&dwTileCnt);

        //--------- レイヤ情報

        //名前

        for(i = 0, pc = p->m_strName; *pc && i < 24; i++, pc++)
            AXSetWORDLE(name + i, *pc);

        name[i] = 0;

        //

        file.write(name, 50);
        file.write(&p->m_btAlpha, 1);
        file.writeDWORD(&p->m_dwCol);
        file.write(&p->m_btFlag, 1);
        file.write(&p->m_btAType, 1);
        file.writeWORD(&p->m_wAnimCnt);

        //---------- 各タイル

        p->m_img.calcTileToPixel(&topx, &topy, 0, 0);

        ppTile  = p->m_img.getTileBuf();
        nTileW  = p->m_img.getTileXCnt();
        nTileH  = p->m_img.getTileYCnt();
        ypos    = (topy - rc.top) / 64;

        pProgDlg->beginProgSub(4, dwTileCnt, FALSE);

        for(iy = nTileH; iy > 0; iy--, ypos++)
        {
            xpos = (topx - rc.left) / 64;

            for(ix = nTileW; ix > 0; ix--, ppTile++, xpos++)
            {
                if(!(*ppTile)) continue;

                //圧縮

                zlib.reset();

                if(zlib.putEncBuf(zlib.getBuf(), 4096, (*ppTile)->getBuf(), 4096))
                    dwSize = zlib.getEncSize();
                else
                    dwSize = 0;

                //書き込み

                file.writeWORD(xpos);
                file.writeWORD(ypos);

                if(dwSize == 0)
                {
                    //圧縮失敗または出力サイズが4096以上の場合は無圧縮

                    file.writeWORD(4096);
                    file.write((*ppTile)->getBuf(), 4096);
                }
                else
                {
                    file.writeWORD((WORD)dwSize);
                    file.write(zlib.getBuf(), dwSize);
                }

                //

                pProgDlg->incProgSub();
            }
        }
    }

    file.close();

    return TRUE;
}

//! ADW読み込み

int CDraw::loadADW(const AXString &filename,CProgressDlg *pProgDlg)
{
    AXFile file;
    BYTE ver;
    int ret;

    //ファイル開く

    if(!file.openRead(filename)) return LOADERR_OPENFILE;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    //ヘッダ

    if(!file.readCompare("AZDWDAT")) return LOADERR_FORMAT;

    //バージョン

    file.read(&ver, 1);

    //各バージョン処理

    if(ver == 0)
        ret = _loadADW_ver1(&file, pProgDlg);
    else if(ver == 1)
        ret = _loadADW_ver2(&file, pProgDlg);
    else
        ret = LOADERR_FORMAT;

    //

    file.close();

    return ret;
}

//! ver2 読み込み

int CDraw::_loadADW_ver2(AXFile *pfile,CProgressDlg *pProgDlg)
{
    AXZlib zlib;
    CLayerItem *p;
    CTileImg **ppTile;
    DWORD dw;
    WORD wSize,wWidth,wHeight,wLayerCnt,wLayerSel,wDPI,wLayerInfoSize,tx,ty;
    WORD wName[25];
    DWORD ti,dwTileCnt;
    AXRect rc;
    LAYERINFO info;
    int i,j;

    if(!zlib.allocBuf(4096)) return LOADERR_ETC;
    if(!zlib.initDecBuf()) return LOADERR_ETC;

    //---------- プレビュー飛ばす

    pfile->seekCur(4);
    pfile->read(&dw, 4);
    pfile->seekCur(dw);

    //---------- 全体情報

    //データサイズ
    pfile->readWORD(&wSize);
    //キャンバス幅
    pfile->readWORD(&wWidth);
    //キャンバス高さ
    pfile->readWORD(&wHeight);
    //DPI
    pfile->readWORD(&wDPI);
    //レイヤ数
    pfile->readWORD(&wLayerCnt);
    //カレントレイヤNo
    pfile->readWORD(&wLayerSel);
    //レイヤ情報サイズ
    pfile->readWORD(&wLayerInfoSize);

    pfile->seekCur(wSize - 12);

    //---------- 新規イメージ

    if(!newImage(wWidth, wHeight, wDPI, FALSE)) return LOADERR_ETC;

    //----------- レイヤ

    pProgDlg->setProgMax(wLayerCnt);

    for(i = wLayerCnt; i > 0; i--)
    {
        //全体の範囲とタイル数

        if(!pfile->readDWORD(&rc.left)) return LOADERR_ETC;
        pfile->readDWORD(&rc.top);
        pfile->readDWORD(&rc.right);
        pfile->readDWORD(&rc.bottom);
        pfile->readDWORD(&dwTileCnt);

        //レイヤ追加

        _imgrectToLAYERINFO(&info, rc);

        p = m_player->addLayer(wWidth, wHeight, &info);
        if(!p) return LOADERR_ETC;

        //レイヤ情報

        pfile->read(wName, 50);
        pfile->read(&p->m_btAlpha, 1);
        pfile->readDWORD(&p->m_dwCol);
        pfile->read(&p->m_btFlag, 1);
        pfile->read(&p->m_btAType, 1);
        pfile->readWORD(&p->m_wAnimCnt);

        //名前

        for(j = 0; j < 25; j++)
            AXGetWORDLE((LPBYTE)(wName + j), wName + j);

        wName[24] = 0;
        p->m_strName = wName;

        //情報余分をシーク

        pfile->seekCur(wLayerInfoSize - 79);

        //タイル

        for(ti = dwTileCnt; ti > 0; ti--)
        {
            if(!pfile->readWORD(&tx)) return LOADERR_ETC;
            pfile->readWORD(&ty);
            pfile->readWORD(&wSize);

            //タイル位置が範囲外 or 圧縮サイズが範囲外

            if(tx >= info.nTileXCnt || ty >= info.nTileYCnt || wSize == 0 || wSize > 4096)
                return LOADERR_ETC;

            //作成

            ppTile = p->m_img.getTilePt(tx, ty);

            if(!(*ppTile))
            {
                *ppTile = p->m_img.allocTile();
                if(!(*ppTile)) return LOADERR_ETC;
            }

            //読み込み

            if(wSize == 4096)
            {
                //無圧縮

                pfile->read((*ppTile)->getBuf(), 4096);
            }
            else
            {
                //ZIP圧縮

                pfile->read(zlib.getBuf(), wSize);

                if(!zlib.reset()) return LOADERR_ETC;

                if(!zlib.getDecBuf((*ppTile)->getBuf(), 4096, zlib.getBuf(), wSize))
                    return LOADERR_ETC;
            }
        }

        //

        pProgDlg->incProg();
    }

    //カレントレイヤ

    changeCurLayer(wLayerSel);

    return LOADERR_SUCCESS;
}

//! ADW ver1 読み込み

int CDraw::_loadADW_ver1(AXFile *pfile,CProgressDlg *pProgDlg)
{
    CLayerItem *p;
    AXZlib zlib;
    AXMem memLine;
    DWORD dwSize,dwLineSize;
    WORD wWidth,wHeight,wLayerCnt,wLayerSel;
    BYTE btComp,btFlag;
    char name[32];
    int i,x,y;
    LPBYTE pSrc;

    if(!zlib.allocBuf(16 * 1024)) return LOADERR_ETC;

    //------- ヘッダデータ

    //幅・高さ
    pfile->readWORD(&wWidth);
    pfile->readWORD(&wHeight);
    //レイヤ数
    pfile->readWORD(&wLayerCnt);
    //レイヤ通算カウント
    pfile->seekCur(2);
    //カレントレイヤNo
    pfile->readWORD(&wLayerSel);
    //圧縮
    pfile->read(&btComp, 1);

    if(btComp != 0) return LOADERR_FORMAT;  //ゼロ圧縮は対象外

    //-------- プレビューイメージ

    pfile->readDWORD(&dwSize);
    pfile->seekCur(dwSize);

    //--------- Y1列分バッファ

    dwLineSize = (wWidth + 3) & (~3);

    if(!memLine.alloc(dwLineSize)) return LOADERR_ETC;

    //---------- 新規イメージ

    if(!newImage(wWidth, wHeight, -1, FALSE)) return LOADERR_ETC;

    //---------- レイヤ

    pProgDlg->setProgMax(wLayerCnt);

    for(i = wLayerCnt; i > 0; i--)
    {
        //レイヤ追加

        p = m_player->addLayer(wWidth, wHeight);
        if(!p) return LOADERR_ETC;

        //情報

        pfile->read(name, 32);
        pfile->read(&p->m_btAlpha, 1);
        pfile->read(&btFlag, 1);
        pfile->readDWORD(&p->m_dwCol);

        p->m_strName.setUTF8(name, 31);

        p->m_btFlag = 0;
        if(btFlag & 1) p->m_btFlag |= CLayerItem::FLAG_VIEW;

        //イメージ読み込み

        pfile->readDWORD(&dwSize);

        if(!zlib.initDecFile(pfile, dwSize)) return LOADERR_ETC;

        for(y = 0; y < m_nImgH; y++)
        {
            if(!zlib.getDecFile(memLine, dwLineSize)) return LOADERR_ETC;

            pSrc = memLine;

            for(x = 0; x < m_nImgW; x++, pSrc++)
            {
                if(*pSrc)
                    p->m_img.setPixel_create(x, y, *pSrc);
            }
        }

        zlib.end();

        //

        pProgDlg->incProg();
    }

    //カレントレイヤ

    changeCurLayer(wLayerSel);

    return LOADERR_SUCCESS;
}


//=================================
// APD (AzPainter)
//=================================


//! APD保存

BOOL CDraw::saveAPD(const AXString &filename,CProgressDlg *pProgDlg)
{
    AXFile file;
    AXZlib zlib;
    AXByteString bstr;
    AXMem memLine;
    CLayerItem *p;
    DWORD pos,linesize;
    AXRectSize rcs;
    int x,y;
    LPBYTE pDst;
    BYTE r,g,b;

    linesize = m_nImgW * 4;

    rcs.set(0, 0, m_nImgW, m_nImgH);

    if(!zlib.allocBuf(16 * 1024)) return FALSE;

    if(!memLine.alloc(linesize)) return FALSE;

    //==================== 書き込み

    //ファイル開く

    if(!file.openWrite(filename)) return FALSE;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    //------- ヘッダ

    file.writeStr("AZPDATA");
    //バージョン
    file.writeBYTE(0);

    //------- データヘッダ

    //データサイズ
    file.writeDWORD(10);
    //幅
    file.writeWORD(m_nImgW);
    //高さ
    file.writeWORD(m_nImgH);
    //レイヤ数
    file.writeWORD(getLayerCnt());
    //レイヤ通算カウント
    file.writeWORD(getLayerCnt());
    //カレントレイヤNo
    file.writeWORD(m_nCurLayerNo);

    //----- プレビューイメージ（なし）

    file.writeDWORD((DWORD)0);
    file.writeDWORD((DWORD)0);

    //----- レイヤ

    //レイヤ情報サイズ
    file.writeDWORD(35);

    bstr.resize(31);

    pProgDlg->setProgMax(getLayerCnt() * 5);

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        //情報

        p->m_strName.toLocal(&bstr, 31);

        file.write(bstr, 32);
        file.writeBYTE(0);
        file.writeBYTE(p->m_btAlpha);
        file.writeBYTE((p->isView())? 1: 0);

        //------- イメージ

        pos = file.getPosition();
        file.writeDWORD((DWORD)0);

        //ボトムアップでリトルエンディアン -> zlib圧縮

        if(!zlib.initEncFile(&file, 6)) return FALSE;

        r = _GETR(p->m_dwCol);
        g = _GETG(p->m_dwCol);
        b = _GETB(p->m_dwCol);

        pProgDlg->beginProgSub(5, m_nImgH, FALSE);

        for(y = m_nImgH - 1; y >= 0; y--)
        {
            pDst = memLine;

            for(x = 0; x < m_nImgW; x++, pDst += 4)
            {
                pDst[0] = b;
                pDst[1] = g;
                pDst[2] = r;
                pDst[3] = p->m_img.getPixel(x, y);
            }

            if(!zlib.putEncFile(memLine, linesize)) return FALSE;

            //

            pProgDlg->incProgSub();
        }

        if(!zlib.end()) return FALSE;

        //圧縮サイズ

        file.seekTop(pos);
        file.writeDWORD(zlib.getEncSize());
        file.seekEnd(0);
    }

    file.close();

    return TRUE;
}

//! APD読み込み

int CDraw::loadAPD(const AXString &filename,CProgressDlg *pProgDlg)
{
    AXFile file;
    AXZlib zlib;
    AXMem memLine;
    DWORD size,dwLayerInfoSize,dwLineSize;
    WORD i,wWidth,wHeight,wLayerCnt,wLayerSel;
    BYTE ver,flag;
    CLayerItem *p;
    char name[32];
    int x,y;
    LPBYTE pSrc;

    if(!zlib.allocBuf(16 * 1024)) return LOADERR_ETC;

    //ファイル開く

    if(!file.openRead(filename)) return LOADERR_OPENFILE;

    file.setEndian(AXFile::ENDIAN_LITTLE);

    //------- ヘッダ

    if(!file.readCompare("AZPDATA")) return LOADERR_FORMAT;

    //バージョン

    file.read(&ver, 1);
    if(ver != 0) return LOADERR_FORMAT;

    //------- ヘッダデータ

    //ヘッダサイズ
    file.readDWORD(&size);

    //幅・高さ
    file.readWORD(&wWidth);
    file.readWORD(&wHeight);
    //レイヤ数
    file.readWORD(&wLayerCnt);
    //レイヤ通算カウント
    file.seekCur(2);
    //カレントレイヤNo
    file.readWORD(&wLayerSel);

    file.seekCur(size - 10);

    //-------- プレビューイメージ

    file.seekCur(4);        //幅・高さ
    file.readDWORD(&size);  //圧縮サイズ
    file.seekCur(size);

    //-------- レイヤ

    //レイヤ情報データサイズ

    file.readDWORD(&dwLayerInfoSize);

    //新規イメージ

    if(!newImage(wWidth, wHeight, -1, FALSE)) return LOADERR_ETC;

    //Y1列分バッファ

    dwLineSize = m_nImgW * 4;
    if(!memLine.alloc(dwLineSize)) return LOADERR_ETC;

    //各レイヤ

    pProgDlg->setProgMax(wLayerCnt * 5);

    for(i = wLayerCnt; i > 0; i--)
    {
        //レイヤ追加

        p = m_player->addLayer(wWidth, wHeight);
        if(!p) return LOADERR_ETC;

        //情報

        file.read(name, 32);
        file.seekCur(1);
        file.read(&p->m_btAlpha, 1);
        file.read(&flag, 1);

        file.seekCur(dwLayerInfoSize - 35);

        //

        p->m_strName.setUTF8(name, 31);

        p->m_btFlag = 0;
        if(flag & 1) p->m_btFlag |= CLayerItem::FLAG_VIEW;

        //-------- イメージ

        //圧縮サイズ

        file.readDWORD(&size);

        //イメージ読み込み（色は黒、アルファ値をセット）

        if(!zlib.initDecFile(&file, size)) return LOADERR_ETC;

        pProgDlg->beginProgSub(5, m_nImgH, FALSE);

        for(y = m_nImgH - 1; y > 0; y--)
        {
            if(!zlib.getDecFile(memLine, dwLineSize)) return LOADERR_ETC;

            pSrc = (LPBYTE)memLine + 3;

            for(x = 0; x < m_nImgW; x++, pSrc += 4)
            {
                if(*pSrc)
                    p->m_img.setPixel_create(x, y, *pSrc);
            }

            //

            pProgDlg->incProgSub();
        }

        zlib.end();
    }

    file.close();

    //カレントレイヤ

    changeCurLayer(wLayerSel);

    return LOADERR_SUCCESS;
}


//=================================
// PSD 保存（レイヤあり）
//=================================


//! PSD保存

BOOL CDraw::savePSD(const AXString &filename,CProgressDlg *pProgDlg)
{
    AXPSDSave psd;
    AXPSDSave::INFO info;
    AXPSDSave::LAYERINFO linfo;
    int nLayerCnt,ch,x,y,col;
    CLayerItem *p;
    AXRect rc;
    AXRectSize rcs;
    AXByteString bstr;
    LPBYTE pDst,pSrc;

    nLayerCnt = getLayerCnt();

    pProgDlg->setProgMax(nLayerCnt * 4 + 3);

    //

    info.nWidth     = m_nImgW;
    info.nHeight    = m_nImgH;
    info.nImgCh     = 3;
    info.nBits      = 8;
    info.nColMode   = axpsd::COLMODE_RGB;

    if(!psd.openFile(filename, &info)) return FALSE;

    //画像リソース

    psd.beginRes();
    psd.writeRes_resolution(m_nImgDPI, m_nImgDPI);
    psd.writeRes_curlayer(nLayerCnt - 1 - m_nCurLayerNo);
    psd.endRes();

    //------- レイヤ

    if(!psd.beginLayer(nLayerCnt)) goto ERR;

    //各レイヤ情報

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        p->m_img.getExistTileRectPx(&rc);

        p->m_strName.toLocal(&bstr, 255);

        linfo.nLeft     = rc.left;
        linfo.nTop      = rc.top;
        linfo.nRight    = rc.right;
        linfo.nBottom   = rc.bottom;
        linfo.dwBlend   = axpsd::BLEND_NORMAL;
        linfo.btOpacity = (int)(p->m_btAlpha * 255.0 / 128.0 + 0.5);
        linfo.bHide     = !p->isView();
        linfo.szName    = bstr;

        psd.writeLayerInfo(&linfo);
    }

    //イメージ

    if(!psd.beginLayerImageTop()) goto ERR;

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        if(!psd.beginLayerImageEach(&rcs))
            //空の場合
            pProgDlg->addProgPos(4);
        else
        {
            for(ch = 0; ch < 4; ch++)
            {
                psd.beginLayerImageCh();

                if(ch < 3)
                {
                    //RGB

                    if(ch == 0)
                        col = _GETR(p->m_dwCol);
                    else if(ch == 1)
                        col = _GETG(p->m_dwCol);
                    else
                        col = _GETB(p->m_dwCol);

                    ::memset(psd.getLineBuf(), col, rcs.w);

                    for(y = 0; y < rcs.h; y++)
                        psd.writeLayerImageChLine();
                }
                else
                {
                    //A

                    for(y = 0; y < rcs.h; y++)
                    {
                        pDst = psd.getLineBuf();

                        for(x = 0; x < rcs.w; x++)
                            *(pDst++) = p->m_img.getPixel(rcs.x + x, rcs.y + y);

                        psd.writeLayerImageChLine();
                    }
                }

                psd.endLayerImageCh();

                pProgDlg->incProg();
            }
        }
    }

    psd.endLayer();

    //------ 一枚絵

    if(!psd.beginImage()) goto ERR;

    for(ch = 0; ch < 3; ch++)
    {
        psd.beginImageCh();

        pSrc = (LPBYTE)m_pimgBlend->getBuf();

    #ifdef __BIG_ENDIAN__
        pSrc += 1 + ch;
    #else
        pSrc += 2 - ch;
    #endif

        //

        for(y = m_nImgH; y > 0; y--)
        {
            pDst = psd.getLineBuf();

            for(x = m_nImgW; x > 0; x--, pSrc += 4)
                *(pDst++) = *pSrc;

            psd.writeImageChLine();
        }

        psd.endImageCh();

        //

        pProgDlg->incProg();
    }

    //

    psd.close();

    return TRUE;

ERR:
    psd.close();
    AXDeleteFile(filename);
    return FALSE;
}


//=================================
// PSD 保存（レイヤなし）
//=================================


//! PSD保存（レイヤなし）

BOOL CDraw::savePSD_nolayer(const AXString &filename,int bits,CProgressDlg *pProgDlg)
{
    AXPSDSave psd;
    AXPSDSave::INFO info;
    int y,x,i,j;
    BYTE val,f;
    LPBYTE pDst;
    AXImage32::PIXEL *pSrc;

    info.nWidth     = m_nImgW;
    info.nHeight    = m_nImgH;
    info.nImgCh     = 1;
    info.nBits      = bits;
    info.nColMode   = (bits == 1)? axpsd::COLMODE_BITMAP: axpsd::COLMODE_GRAYSCALE;

    if(!psd.openFile(filename, &info)) return FALSE;

    //画像リソース

    psd.beginRes();
    psd.writeRes_resolution(m_nImgDPI, m_nImgDPI);
    psd.endRes();

    //レイヤ(0)

    psd.beginLayer(0);

    //イメージ

    if(!psd.beginImage()) return FALSE;

    pProgDlg->beginProgSub(30, m_nImgH, TRUE);

    psd.beginImageCh();

    pSrc = (AXImage32::PIXEL *)m_pimgBlend->getBuf();

    if(bits == 1)
    {
        //1bit

        for(y = m_nImgH; y > 0; y--)
        {
            pDst = psd.getLineBuf();
            x    = m_nImgW;

            for(i = (m_nImgW + 7) / 8; i > 0; i--)
            {
                val = 0;
                f   = 0x80;

                for(j = 8; j && x; j--, x--, pSrc++, f >>= 1)
                {
                    if((pSrc->c & 0xffffff) != 0xffffff)
                        val |= f;
                }

                *(pDst++) = val;
            }

            psd.writeImageChLine();

            pProgDlg->incProgSub();
        }
    }
    else
    {
        //8bit

        for(y = m_nImgH; y > 0; y--)
        {
            pDst = psd.getLineBuf();

            for(x = m_nImgW; x > 0; x--, pSrc++)
                *(pDst++) = (pSrc->r + pSrc->g + pSrc->b) / 3;

            psd.writeImageChLine();

            pProgDlg->incProgSub();
        }
    }

    psd.endImageCh();

    psd.close();

    return TRUE;
}


//=================================
// PSD 読み込み
//=================================


//! PSD読み込み
/*!
    @return AXPSDLoad::ERRCODE
*/

int CDraw::loadPSD(const AXString &filename,CProgressDlg *pProgDlg)
{
    AXPSDLoad psd;
    AXPSDLoad::LAYERINFO linfo;
    LAYERINFO layerinfo;
    AXRect rc;
    AXRectSize rcs;
    CLayerItem *p;
    int i,ret,dpih,dpiv,curlayer = 0,x,y,val,bGrayScale;
    LPBYTE pSrc,pDst;

    ret = psd.openFile(filename);
    if(ret) return ret;

    //グレイスケールか

    bGrayScale = (psd.getColMode() == axpsd::COLMODE_GRAYSCALE);

    //画像サイズ確認

    if(psd.getImgWidht() > IMGSIZE_MAX || psd.getImgHeight() > IMGSIZE_MAX)
        return 1000;

    //新規イメージ

    if(!newImage(psd.getImgWidht(), psd.getImgHeight(), -1, FALSE))
        return AXPSDLoad::ERR_MEMORY;

    //-------- 画像リソースデータ

    ret = psd.beginRes();
    if(ret) return ret;

    //解像度

    if(psd.readRes_resolution(&dpih, &dpiv))
        m_nImgDPI = dpih;

    //カレントレイヤ

    curlayer = psd.readRes_curlayer();

    psd.endRes();

    //-------- レイヤ

    ret = psd.beginLayer();
    if(ret) return ret;

    //-------- レイヤなしの場合は一枚絵から読み込み

    if(psd.getLayerCnt() == 0 || psd.getBits() == 1)
    {
        psd.endLayer();
        return _loadPSD_img(&psd, pProgDlg);
    }

    //-------- レイヤデータ

    //レイヤ情報

    for(i = psd.getLayerCnt(); i > 0; i--)
    {
        ret = psd.readLayerInfo(&linfo);
        if(ret) return ret;

        //レイヤイメージ情報

        rc.left   = linfo.nLeft;
        rc.top    = linfo.nTop;
        rc.right  = linfo.nRight;
        rc.bottom = linfo.nBottom;

        _imgrectToLAYERINFO(&layerinfo, rc);

        //レイヤ作成

        p = m_player->addLayer(m_nImgW, m_nImgH, &layerinfo);
        if(!p) return AXPSDLoad::ERR_MEMORY;

        //名前

        p->m_strName.setLocal(linfo.szName);

        //不透明度

        p->m_btAlpha = (int)(linfo.btOpacity * 128.0 / 255.0 + 0.5);

        //非表示

        if(linfo.bHide) p->m_btFlag &= ~CLayerItem::FLAG_VIEW;
    }

    //レイヤイメージ

    ret = psd.beginLayerImageTop();
    if(ret) return ret;

    pProgDlg->setProgMax(psd.getLayerCnt() * 10);

    for(p = m_player->getTopItem(); p; p = p->next())
    {
        if(!psd.beginLayerImageEach(&rcs))
            //空の場合
            pProgDlg->addProgPos(10);
        else if(bGrayScale)
        {
            //------ グレイスケール

            //色チャンネル
            /* 0:黒、255:白 なので、反転して濃度に */

            ret = psd.beginLayerImageCh(0);
            if(ret) return ret;

            pProgDlg->beginProgSub(5, rcs.h);

            for(y = 0; y < rcs.h; y++)
            {
                ret = psd.readLayerImageChLine();
                if(ret) return ret;

                pSrc = psd.getLineBuf();

                for(x = 0; x < rcs.w; x++)
                {
                    val = 255 - *(pSrc++);

                    if(val)
                        p->m_img.setPixel_create(rcs.x + x, rcs.y + y, val);
                }

                pProgDlg->incProgSub();
            }

            //アルファチャンネル
            /* ない場合もある。現在の濃度にアルファ値を適用 */

            ret = psd.beginLayerImageCh(AXPSDLoad::CHANNELID_ALPHA);

            if(ret == AXPSDLoad::ERR_NO_CHANNEL)
            {
                pProgDlg->addProgPos(5);
                continue;
            }
            else if(ret)
                return ret;

            //

            pProgDlg->beginProgSub(5, rcs.h);

            for(y = 0; y < rcs.h; y++)
            {
                ret = psd.readLayerImageChLine();
                if(ret) return ret;

                pSrc = psd.getLineBuf();

                for(x = 0; x < rcs.w; x++, pSrc++)
                {
                    pDst = p->m_img.getPixelPoint(rcs.x + x, rcs.y + y);

                    if(pDst)
                        *pDst = (*pDst) * (*pSrc) / 255;
                }

                pProgDlg->incProgSub();
            }

            //空タイル削除

            p->m_img.freeTileTransparent();
        }
        else
        {
            //------ RGBA : アルファチャンネルだけ読み込み

            ret = psd.beginLayerImageCh(AXPSDLoad::CHANNELID_ALPHA);

            if(ret == AXPSDLoad::ERR_NO_CHANNEL)
            {
                pProgDlg->addProgPos(10);
                continue;
            }
            else if(ret)
                return ret;

            //

            pProgDlg->beginProgSub(10, rcs.h, FALSE);

            for(y = 0; y < rcs.h; y++)
            {
                ret = psd.readLayerImageChLine();
                if(ret) return ret;

                pSrc = psd.getLineBuf();

                for(x = 0; x < rcs.w; x++, pSrc++)
                {
                    if(*pSrc)
                        p->m_img.setPixel_create(rcs.x + x, rcs.y + y, *pSrc);
                }

                pProgDlg->incProgSub();
            }
        }
    }

    psd.endLayer();

    psd.close();

    //----------

    //カレントレイヤ

    if(curlayer < 0 || curlayer >= getLayerCnt())
        curlayer = 0;

    changeCurLayer(getLayerCnt() - 1 - curlayer);

    return AXPSDLoad::ERR_SUCCESS;
}

//! PSD読み込み：一枚絵イメージから

int CDraw::_loadPSD_img(AXPSDLoad *psd,CProgressDlg *pProgDlg)
{
    int ret,bits,colmode,i,x,y,cnt,f;
    LPBYTE pDst,pSrc;
    CLayerItem *p;

    bits    = psd->getBits();
    colmode = psd->getColMode();

    //レイヤ作成

    p = m_player->addLayer(m_nImgW, m_nImgH);
    if(!p) return AXPSDLoad::ERR_MEMORY;

    //各チャンネル

    ret = psd->beginImage();
    if(ret) return ret;

    if(bits == 1)
    {
        //-------- 1bit

        pProgDlg->beginProgSub(30, m_nImgH, TRUE);

        ret = psd->beginImageCh();
        if(ret) return ret;

        for(y = 0; y < m_nImgH; y++)
        {
            ret = psd->readImageChLine();
            if(ret) return ret;

            pSrc = psd->getLineBuf();

            for(x = 0, f = 0x80; x < m_nImgW; x++)
            {
                if(*pSrc & f)
                    p->m_img.setPixel_create(x, y, 255);

                f >>= 1;
                if(f == 0) f = 0x80, pSrc++;
            }

            pProgDlg->incProgSub();
        }
    }
    else if(bits == 8 && colmode == axpsd::COLMODE_GRAYSCALE)
    {
        //-------- グレイスケール

        pProgDlg->beginProgSub(30, m_nImgH, TRUE);

        ret = psd->beginImageCh();
        if(ret) return ret;

        for(y = 0; y < m_nImgH; y++)
        {
            ret = psd->readImageChLine();
            if(ret) return ret;

            pSrc = psd->getLineBuf();

            for(x = 0; x < m_nImgW; x++, pSrc++)
            {
                if(*pSrc != 255)
                    p->m_img.setPixel_create(x, y, 255 - *pSrc);
            }

            pProgDlg->incProgSub();
        }
    }
    else if(psd->getImgChCnt() >= 3)
    {
        //-------- RGB or RGBA

        cnt = (psd->getImgChCnt() >= 4)? 4: 3;

        pProgDlg->setProgMax(cnt * 20);

        for(i = 0; i < cnt; i++)
        {
            ret = psd->beginImageCh();
            if(ret) return ret;

            pDst = (LPBYTE)m_pimgBlend->getBuf();

        #ifdef __BIG_ENDIAN__
            pDst += (i == 3)? 0: 1 + i;
        #else
            pDst += (i == 3)? 3: 2 - i;
        #endif

            pProgDlg->beginProgSub(20, m_nImgH);

            for(y = m_nImgH; y > 0; y--)
            {
                ret = psd->readImageChLine();
                if(ret) return ret;

                pSrc = psd->getLineBuf();

                for(x = m_nImgW; x > 0; x--, pDst += 4)
                    *pDst = *(pSrc++);

                pProgDlg->incProgSub();
            }
        }

        //m_pimgBlend(32bit) -> レイヤイメージ

        p->m_img.convFrom32Bit(*m_pimgBlend, (cnt == 4));
    }
    else
        return AXPSDLoad::ERR_FORMAT;

    //

    psd->close();

    //カレントレイヤ

    changeCurLayer(0);

    return AXPSDLoad::ERR_SUCCESS;
}

//! PSD 読み込みエラーコードを LOADERR_* に

int CDraw::errPSDtoLOAD(int err)
{
    int ret;

    switch(err)
    {
        case AXPSDLoad::ERR_SUCCESS:
            ret = LOADERR_SUCCESS;
            break;
        case AXPSDLoad::ERR_OPENFILE:
            ret = LOADERR_OPENFILE;
            break;
        case AXPSDLoad::ERR_FORMAT:
            ret = LOADERR_FORMAT;
            break;
        case 1000:
            ret = LOADERR_IMGSIZE;
            break;
        default:
            ret = LOADERR_ETC;
            break;
    }

    return ret;
}


//=================================
// BMP/PNG/JPEG
//=================================


//! BMP保存

BOOL CDraw::saveBMP(const AXString &filename,const CImage32 *pimgSrc,CProgressDlg *pProgDlg)
{
    AXBMPSave bmp;
    AXBMPSave::INFO info;
    int y;
    LPDWORD ps;

    info.nWidth     = pimgSrc->getWidth();
    info.nHeight    = pimgSrc->getHeight();
    info.nBits      = 24;
    info.nResoH     = AXDPItoDPM(m_nImgDPI);
    info.nResoV     = info.nResoH;

    if(!bmp.openFile(filename, &info)) return FALSE;

    //

    pProgDlg->beginProgSub(30, info.nHeight, TRUE);

    ps = pimgSrc->getBufPt(0, info.nHeight - 1);

    for(y = info.nHeight; y > 0; y--, ps -= info.nWidth)
    {
        bmp.putLineFrom32bit(ps);

        pProgDlg->incProgSub();
    }

    bmp.close();

    return TRUE;
}

//! PNG保存

BOOL CDraw::savePNG(const AXString &filename,const CImage32 *pimgSrc,int nLevel,BOOL b32bit,CProgressDlg *pProgDlg)
{
    AXPNGSave png;
    AXPNGSave::INFO info;
    AXMem mem;
    int y,n,pitch;
    LPDWORD ps;

    info.nWidth     = pimgSrc->getWidth();
    info.nHeight    = pimgSrc->getHeight();
    info.nBits      = (b32bit)? 32: 24;
    info.nPalCnt    = 0;

    if(!png.openFile(filename, &info)) return FALSE;

    //

    pitch = png.getPitch();

    if(!mem.alloc(pitch)) return FALSE;

    //解像度

    n = AXDPItoDPM(m_nImgDPI);
    png.put_pHYs(n, n);

    //イメージ

    if(!png.startImg(nLevel)) return FALSE;

    pProgDlg->beginProgSub(30, info.nHeight, TRUE);

    ps = pimgSrc->getBuf();

    for(y = info.nHeight; y > 0; y--, ps += info.nWidth)
    {
        png.lineFrom32bit(mem, ps, info.nBits);
        png.putImg(mem, pitch);

        pProgDlg->incProgSub();
    }

    png.endImg();

    //

    png.close();

    return TRUE;
}

//! JPEG保存

BOOL CDraw::saveJPEG(const AXString &filename,const CImage32 *pimgSrc,int nQuality,int nSamp,CProgressDlg *pProgDlg)
{
    AXJPEG jpeg;
    AXJPEG::SAVEINFO info;
    LPDWORD ps;
    int y;

    info.nWidth     = pimgSrc->getWidth();
    info.nHeight    = pimgSrc->getHeight();
    info.nQuality   = nQuality;
    info.nDPI_H     = m_nImgDPI;
    info.nDPI_V     = m_nImgDPI;
    info.uFlags     = 0;

    if(nSamp == 444)
        info.nSamplingType = AXJPEG::SAMP_444;
    else if(nSamp == 422)
        info.nSamplingType = AXJPEG::SAMP_422;
    else
        info.nSamplingType = AXJPEG::SAMP_411;

    if(!jpeg.openFileSave(filename, &info)) return FALSE;

    //

    pProgDlg->beginProgSub(30, info.nHeight, TRUE);

    ps = pimgSrc->getBuf();

    for(y = info.nHeight; y > 0; y--, ps += info.nWidth)
    {
        jpeg.putLineFrom32bit(ps);

        pProgDlg->incProgSub();
    }

    jpeg.close();

    return TRUE;
}


//================================
//サブ
//================================


//! ADW/PSD 読み込み時、イメージの範囲から LAYERINFO 構造体セット

void CDraw::_imgrectToLAYERINFO(LAYERINFO *pInfo,const AXRect &rc)
{
    AXMemZero(pInfo, sizeof(LAYERINFO));

    if(rc.left == rc.right || rc.top == rc.bottom)
        //全体が透明の状態
        pInfo->nTileXCnt = pInfo->nTileYCnt = 1;
    else
    {
        pInfo->nOffX     = rc.left;
        pInfo->nOffY     = rc.top;
        pInfo->nTileXCnt = (rc.right - rc.left + 63) / 64;
        pInfo->nTileYCnt = (rc.bottom - rc.top + 63) / 64;
    }
}
