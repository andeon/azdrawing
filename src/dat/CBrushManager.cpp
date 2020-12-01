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

#include "CBrushManager.h"

#include "CBrushImgList.h"
#include "CBrushTreeItem.h"

#include "AXFileWriteBuf.h"
#include "AXMem.h"
#include "AXBuf.h"
#include "AXTreeView.h"


/*!
    @class CBrushManager
    @brief ブラシデータ管理クラス
*/

/*
    - m_pEditItem が常に編集用の現在データ。
    - 連動保存時は選択ブラシのデータも一緒に変更する。
    - ブラシ/テクスチャ画像は、ブラシが選択された時に読み込む（登録ブラシは最初から読み込む）
    - ブラシデータの読み込み・保存は CBrushWin で行われる。
*/


CBrushManager *CBrushManager::m_pSelf = NULL;


CBrushManager::~CBrushManager()
{
    delete m_pImgList;
    delete m_pEditItem;
}

CBrushManager::CBrushManager()
{
    int i;

    m_pSelf = this;

    //

    m_pImgList  = new CBrushImgList;

    m_pEditItem = new CBrushTreeItem;
    m_pEditItem->setNew();

    //

    m_pSelItem = NULL;

    for(i = 0; i < REGITEM_NUM; i++)
        m_pRegItem[i] = NULL;
}

//! 新規ブラシ作成

CBrushTreeItem *CBrushManager::newBrush()
{
    CBrushTreeItem *p = new CBrushTreeItem;

    p->setNew();

    return p;
}

//! ブラシ複製

CBrushTreeItem *CBrushManager::cloneBrush(const CBrushTreeItem *pSrc)
{
    CBrushTreeItem *p = new CBrushTreeItem;

    //コピー

    p->m_strText = pSrc->m_strText;
    p->copyFrom(pSrc);

    //画像

    _loadImage(p, TRUE);

    return p;
}

//! テキスト形式のデータから新規作成（貼り付け時用）
/*!
    @return NULL でエラー
*/

CBrushTreeItem *CBrushManager::newBrushFromText(const AXString &str)
{
    CBrushTreeItem *p;

    //ヘッダ判定

    if(str.compare("AZDWLBR;", 8) && str.compare("AZDW2BR;", 8))
        return NULL;

    //

    p = newBrush();

    if(!p->setFromTextFormat(str))
    {
        delete p;
        return NULL;
    }

    //画像

    _loadImage(p, TRUE);

    return p;
}

//! ブラシ削除処理

void CBrushManager::delBrush(CBrushTreeItem *p)
{
    int i;

    //画像解放

    _freeImage(p);

    //登録ブラシの場合解除

    for(i = 0; i < REGITEM_NUM; i++)
    {
        if(p == m_pRegItem[i])
            m_pRegItem[i] = NULL;
    }
}

//! ブラシ選択変更時
/*!
    @param p NULL でブラシ選択なし
    @return 選択が変更されたか
*/

BOOL CBrushManager::changeSelBrush(CBrushTreeItem *p)
{
    if(p == m_pSelItem) return FALSE;

    //編集用の画像データ解放

    _freeImage(m_pEditItem);

    //

    m_pSelItem = p;

    //

    if(m_pSelItem)
    {
        //編集用にデータコピー

        m_pEditItem->copyFrom(m_pSelItem);

        //編集用:画像読み込み

        _loadImage(m_pEditItem, TRUE);

        //選択データ:画像読み込み

        _loadImage(m_pSelItem, FALSE);
    }

    return TRUE;
}

//! ブラシ手動保存（編集用ブラシ -> 選択ブラシへコピー）

void CBrushManager::saveBrushManual()
{
    _freeImage(m_pSelItem);

    m_pSelItem->copyFrom(m_pEditItem);

    _loadImage(m_pSelItem, TRUE);
}

//! 指定ブラシの登録番号取得

int CBrushManager::getRegBrushNo(CBrushTreeItem *p)
{
    int i;

    for(i = 0; i < REGITEM_NUM; i++)
    {
        if(p == m_pRegItem[i]) return i;
    }

    return -1;
}

//! 登録ブラシ指定
/*!
    @param no -1 で指定解除
*/

void CBrushManager::setRegBrush(CBrushTreeItem *p,int no)
{
    int i;

    if(no >= 0 && no < REGITEM_NUM)
    {
        //登録

        m_pRegItem[no] = p;

        p->m_nIconNo = CBrushTreeItem::ICONNO_REGBRUSH_TOP + no;
    }
    else
    {
        //解除

        for(i = 0; i < REGITEM_NUM; i++)
        {
            if(p == m_pRegItem[i])
                m_pRegItem[i] = NULL;
        }

        p->m_nIconNo = CBrushTreeItem::ICONNO_BRUSH;
    }
}

//! 選択ブラシのデータタイプが連動保存か（選択なしの場合 FALSE）

BOOL CBrushManager::isDatLink()
{
    if(m_pSelItem)
        return (m_pSelItem->m_btDatType == CBrushTreeItem::DATTYPE_LINK);
    else
        return FALSE;
}

//! 現在のデータが消しゴムタイプか

BOOL CBrushManager::isEditBrushErase()
{
    return (m_pEditItem->m_btPixType == CBrushTreeItem::PIXTYPE_ERASE);
}


//============================
//データ変更
//============================


//! データタイプ

void CBrushManager::changeDatType(int type)
{
    m_pEditItem->m_btDatType = type;

    if(m_pSelItem) m_pSelItem->m_btDatType = type;
}

//! ブラシサイズ
/*!
    @param bDirect ウィジェットからではなく直接値をセットする場合 TRUE
*/

void CBrushManager::changeBrushSize(int size,BOOL bDirect)
{
    WORD max[8] = {100, 500, 1000, 2000, 3000, 4000, 5000, 6000};
    int i;

    //直接変更される場合

    if(bDirect)
    {
        //指定値が最大値を超える場合

        if(size > m_pEditItem->m_wSizeCtlMax)
        {
            for(i = 0; i < 8; i++)
            {
                if(size <= max[i])
                {
                    m_pEditItem->m_wSizeCtlMax = max[i];

                    if(isDatLink()) m_pSelItem->m_wSizeCtlMax = max[i];
                    break;
                }
            }
        }
    }

    //

    m_pEditItem->m_wSize = size;

    if(isDatLink()) m_pSelItem->m_wSize = size;
}

//! ブラシサイズ最大値

void CBrushManager::changeBrushSizeMax(int size,int max)
{
    m_pEditItem->m_wSize = size;
    m_pEditItem->m_wSizeCtlMax = max;

    if(isDatLink())
    {
        m_pSelItem->m_wSize = size;
        m_pSelItem->m_wSizeCtlMax = max;
    }
}

//! 濃度

void CBrushManager::changeVal(int val)
{
    m_pEditItem->m_btVal = val;

    if(isDatLink()) m_pSelItem->m_btVal = val;
}

//! 補正タイプ

void CBrushManager::changeHoseiType(int type)
{
    m_pEditItem->m_btHoseiType = type;

    if(isDatLink()) m_pSelItem->m_btHoseiType = type;
}

//! 補正強さ

void CBrushManager::changeHoseiStr(int str)
{
    m_pEditItem->m_btHoseiStr = str;

    if(isDatLink()) m_pSelItem->m_btHoseiStr = str;
}

//! 塗りタイプ変更

void CBrushManager::changePixType(int type)
{
    m_pEditItem->m_btPixType = type;

    if(isDatLink()) m_pSelItem->m_btPixType = type;
}

//! サイズ最小

void CBrushManager::changeSizeMin(int pers)
{
    m_pEditItem->m_wMinSize = pers;

    if(isDatLink()) m_pSelItem->m_wMinSize = pers;
}

//! 濃度最小

void CBrushManager::changeValMin(int pers)
{
    m_pEditItem->m_wMinVal = pers;

    if(isDatLink()) m_pSelItem->m_wMinVal = pers;
}

//! 間隔

void CBrushManager::changeInterval(int interval)
{
    m_pEditItem->m_wInterval = interval;

    if(isDatLink()) m_pSelItem->m_wInterval = interval;
}

//! ランダムサイズ

void CBrushManager::changeRandomSize(int pers)
{
    m_pEditItem->m_wRan_sizeMin = pers;

    if(isDatLink()) m_pSelItem->m_wRan_sizeMin = pers;
}

//! ランダム位置

void CBrushManager::changeRandomPos(int len)
{
    m_pEditItem->m_wRan_posLen = len;

    if(isDatLink()) m_pSelItem->m_wRan_posLen = len;
}

//! ランダム位置タイプ

void CBrushManager::changeRandomPosType(int type)
{
    m_pEditItem->m_btFlag &= ~CBrushTreeItem::BFLAG_RANDPOS_BOX;
    if(type) m_pEditItem->m_btFlag |= CBrushTreeItem::BFLAG_RANDPOS_BOX;

    if(isDatLink()) m_pSelItem->m_btFlag = m_pEditItem->m_btFlag;
}

//! ブラシ画像
/*!
    @param str 空でデフォルト
*/

void CBrushManager::changeBrushImg(const AXString &str)
{
    //編集データ

    m_pImgList->freeImage(m_pEditItem->m_pimgBrush);

    m_pEditItem->m_strBrushImg = str;
    m_pEditItem->m_pimgBrush = m_pImgList->loadImage(CBrushImgItem::TYPE_BRUSH, str);

    //選択データ

    if(isDatLink())
    {
        m_pImgList->freeImage(m_pSelItem->m_pimgBrush);

        m_pSelItem->m_strBrushImg = str;
        m_pSelItem->m_pimgBrush = m_pImgList->loadImage(CBrushImgItem::TYPE_BRUSH, str);
    }
}

//! ブラシ画像回転タイプ

void CBrushManager::changeBrushImgRotType(int type)
{
    m_pEditItem->m_btRotType = type;

    if(isDatLink()) m_pSelItem->m_btRotType = type;
}

//! ブラシ画像回転角度

void CBrushManager::changeBrushImgAngle(int angle)
{
    m_pEditItem->m_wRotAngle = angle;

    if(isDatLink()) m_pSelItem->m_wRotAngle = angle;
}

//! ブラシ画像回転ランダム

void CBrushManager::changeBrushImgRandom(int w)
{
    m_pEditItem->m_wRotRandom = w;

    if(isDatLink()) m_pSelItem->m_wRotRandom = w;
}

//! テクスチャ画像

void CBrushManager::changeTextureImg(const AXString &str)
{
    //編集データ

    m_pImgList->freeImage(m_pEditItem->m_pimgTex);

    m_pEditItem->m_strTexImg = str;
    m_pEditItem->m_pimgTex = m_pImgList->loadImage(CBrushImgItem::TYPE_TEXTURE, str);

    //選択データ

    if(isDatLink())
    {
        m_pImgList->freeImage(m_pSelItem->m_pimgTex);

        m_pSelItem->m_strTexImg = str;
        m_pSelItem->m_pimgTex = m_pImgList->loadImage(CBrushImgItem::TYPE_TEXTURE, str);
    }
}

//! アンチエイリアス

void CBrushManager::changeAntiAlias(BOOL bOn)
{
    m_pEditItem->m_btFlag &= ~CBrushTreeItem::BFLAG_ANTIALIAS;
    if(bOn) m_pEditItem->m_btFlag |= CBrushTreeItem::BFLAG_ANTIALIAS;

    if(isDatLink()) m_pSelItem->m_btFlag = m_pEditItem->m_btFlag;
}

//! 曲線

void CBrushManager::changeCurve(BOOL bOn)
{
    m_pEditItem->m_btFlag &= ~CBrushTreeItem::BFLAG_CURVE;
    if(bOn) m_pEditItem->m_btFlag |= CBrushTreeItem::BFLAG_CURVE;

    if(isDatLink()) m_pSelItem->m_btFlag = m_pEditItem->m_btFlag;
}

//! 筆圧サイズ

void CBrushManager::changePressSize(int val)
{
    m_pEditItem->m_wPressSize = val;

    if(isDatLink()) m_pSelItem->m_wPressSize = val;
}

//! 筆圧濃度

void CBrushManager::changePressVal(int val)
{
    m_pEditItem->m_wPressVal = val;

    if(isDatLink()) m_pSelItem->m_wPressVal = val;
}


//============================
//サブ
//============================


//! ブラシ・テクスチャ画像解放

void CBrushManager::_freeImage(CBrushTreeItem *p)
{
    m_pImgList->freeImage(p->m_pimgBrush);
    m_pImgList->freeImage(p->m_pimgTex);

    p->m_pimgBrush = NULL;
    p->m_pimgTex   = NULL;
    p->m_bLoadImg  = FALSE;
}

//! ブラシ・テクスチャ画像読み込み
/*!
    m_bLoadImg が OFF の場合に読み込み。

    @param bForce 強制読み込み
*/

void CBrushManager::_loadImage(CBrushTreeItem *p,BOOL bForce)
{
    if(!p->m_bLoadImg || bForce)
    {
        p->m_pimgBrush = m_pImgList->loadImage(CBrushImgItem::TYPE_BRUSH, p->m_strBrushImg);
        p->m_pimgTex   = m_pImgList->loadImage(CBrushImgItem::TYPE_TEXTURE, p->m_strTexImg);

        p->m_bLoadImg = TRUE;
    }
}


//=============================
// ファイル
//=============================


//! 保存

void CBrushManager::saveFile(const AXString &filename,AXTreeViewItem *pTop,AXTreeViewItem *pSel)
{
    AXFileWriteBuf file;
    AXTreeViewItem *p;
    CBrushTreeItem *pb;
    int type,n;

    if(!file.open(filename)) return;

    //ヘッダ

    file.putStr("AZDWLBR");
    file.putBYTE(0);
    file.putBYTE(0);

    //編集ブラシデータ

    _saveFileBrush(&file, m_pEditItem);

    //ツリーデータ

    for(p = pTop->first(); p; )
    {
        if(p->m_lParam == CBrushTreeItem::PARAM_FOLDER)
            type = 1;
        else
            type = 0;

        //タイプ（[0]ブラシ [1]フォルダ [2]フォルダ終了 [255]データ終了）

        file.putBYTE(type);

        //名前

        file.putStrLenAndUTF8(p->m_strText);

        //フラグ（[0bit]ブラシ選択 [1bit]フォルダ展開）

        n = 0;
        if(type == 0 && p == pSel) n |= 1;
        if(type == 1 && p->isExpand()) n |= 2;

        file.putBYTE(n);

        //ブラシデータ

        if(type == 0)
        {
            pb = (CBrushTreeItem *)p;

            //登録ブラシ番号

            n = getRegBrushNo(pb) + 1;

            file.putBYTE(n);

            //データ

            _saveFileBrush(&file, pb);
        }

        //------- 次のアイテム

        if(p->first())
            p = p->first();
        else
        {
            while(1)
            {
                if(p->next())
                {
                    p = p->next();
                    break;
                }
                else
                {
                    //フォルダ終了

                    file.putBYTE(2);

                    p = p->parent();
                    if(!p || p == pTop) goto END;
                }
            }
        }
    }

END:
    //終了
    file.putBYTE(255);

    file.close();
}

//! ブラシを保存

void CBrushManager::_saveFileBrush(AXFileWriteBuf *pfile,CBrushTreeItem *p)
{
    WORD wdDat[CBrushTreeItem::WORDBASE_NUM];
    BYTE btDat[CBrushTreeItem::BYTEBASE_NUM];
    int i;

    //データ取得

    p->getDatArray(wdDat, btDat);

    //WORDデータ

    for(i = 0; i < CBrushTreeItem::WORDBASE_NUM; i++)
        pfile->putWORDLE(wdDat[i]);

    //BYTEデータ

    pfile->put(btDat, CBrushTreeItem::BYTEBASE_NUM);

    //ブラシ画像名

    pfile->putStrLenAndUTF8(p->m_strBrushImg);

    //テクスチャ画像名

    pfile->putStrLenAndUTF8(p->m_strTexImg);
}

//! 読み込み
/*!
    選択ブラシもセットされる。@n
    ブラシ画像・テクスチャ画像は登録ブラシのみ読み込まれる。
*/

void CBrushManager::loadFile(const AXString &filename,AXTreeView *ptree)
{
    AXMem mem;
    AXBuf buf;
    AXTreeViewItem *pParent,*p;
    CBrushTreeItem *pb,*pSelBrush = NULL;
    BYTE ver,subver,type,f,regno;
    AXString str;
    int i;

    if(!AXFile::readFileFull(filename, &mem, FALSE)) return;

    buf.init(mem, mem.getSize(), AXBuf::ENDIAN_LITTLE);

    //ヘッダ

    if(!buf.getStrCompare("AZDWLBR")) return;

    if(!buf.getBYTE(&ver)) return;
    if(!buf.getBYTE(&subver)) return;

    if(ver != 0 || subver != 0) return;

    //編集ブラシデータ

    _loadFileBrush(&buf, m_pEditItem);

    //ツリーデータ

    pParent = ptree->getTopItem();

    while(1)
    {
        if(!buf.getBYTE(&type)) break;

        if(type == 255) break;

        //フォルダ終了

        if(type == 2)
        {
            pParent = pParent->parent();
            if(!pParent) break;

            continue;
        }

        //------ ブラシ/フォルダ

        //名前

        if(!buf.getStrLenAndUTF8(&str)) break;

        //フラグ

        if(!buf.getBYTE(&f)) break;

        //

        if(type == 1)
        {
            //フォルダ

            p = ptree->addItem(pParent, str, 0, 0, CBrushTreeItem::PARAM_FOLDER);

            pParent = p;

            if(f & 2) p->m_uFlags |= AXTreeViewItem::FLAG_EXPAND;
        }
        else
        {
            //---- ブラシ

            //登録ブラシ番号

            if(!buf.getBYTE(&regno)) break;

            //作成

            pb = new CBrushTreeItem;
            pb->setNew();

            pb->m_strText = str;

            ptree->addItem(pParent, pb);

            if(f & 1) pSelBrush = pb;

            //登録ブラシ（0:なし、1〜:登録番号）

            if(regno)
                m_pRegItem[regno - 1] = pb;

            //データ

            if(!_loadFileBrush(&buf, pb)) break;
        }
    }

    //編集用ブラシ・画像読み込み

    _loadImage(m_pEditItem, TRUE);

    //登録ブラシのアイコン番号セット・画像読み込み

    for(i = 0; i < REGITEM_NUM; i++)
    {
        if(m_pRegItem[i])
        {
            m_pRegItem[i]->m_nIconNo = CBrushTreeItem::ICONNO_REGBRUSH_TOP + i;

            _loadImage(m_pRegItem[i], TRUE);
        }
    }

    //選択ブラシ

    if(pSelBrush)
    {
        m_pSelItem = pSelBrush;

        _loadImage(m_pSelItem, FALSE);
    }
}

//! ブラシデータ読み込み

BOOL CBrushManager::_loadFileBrush(AXBuf *pbuf,CBrushTreeItem *p)
{
    WORD wdDat[CBrushTreeItem::WORDBASE_NUM];
    BYTE btDat[CBrushTreeItem::BYTEBASE_NUM];
    int i;

    //WORDデータ

    for(i = 0; i < CBrushTreeItem::WORDBASE_NUM; i++)
    {
        if(!pbuf->getWORD(&wdDat[i])) return FALSE;
    }

    //BYTEデータ

    if(!pbuf->getDat(btDat, CBrushTreeItem::BYTEBASE_NUM)) return FALSE;

    //データセット

    p->setDatArray(wdDat, btDat);

    //ファイル名

    if(!pbuf->getStrLenAndUTF8(&p->m_strBrushImg)) return FALSE;
    if(!pbuf->getStrLenAndUTF8(&p->m_strTexImg)) return FALSE;

    return TRUE;
}
