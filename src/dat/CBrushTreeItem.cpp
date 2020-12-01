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

#include "CBrushTreeItem.h"

#include "AXUtilStr.h"


/*!
    @class CBrushTreeItem
    @brief ブラシツリーの、ブラシアイテム

    ブラシ名は AXTreeViewItem::m_strText を使う。
*/


CBrushTreeItem::CBrushTreeItem()
    : AXTreeViewItem(NULL, ICONNO_BRUSH, 0, PARAM_BRUSH)
{

}

//! 新規データセット

void CBrushTreeItem::setNew()
{
    m_strText       = "new";

    m_wSize         = 80;
    m_wSizeCtlMax   = 1000;
    m_wMinSize      = 0;
    m_wMinVal       = 1000;
    m_wInterval     = 10;
    m_wRotAngle     = 0;
    m_wRotRandom    = 0;
    m_wRan_sizeMin  = 1000;
    m_wRan_posLen   = 0;
    m_wPressSize    = 100;
    m_wPressVal     = 100;

    m_btDatType     = DATTYPE_LINK;
    m_btVal         = 255;
    m_btPixType     = 0;
    m_btHoseiType   = 3;
    m_btHoseiStr    = 0;
    m_btRotType     = 0;
    m_btFlag        = BFLAG_ANTIALIAS;

    m_strTexImg     = '?';

    m_bLoadImg      = FALSE;
    m_pimgBrush     = NULL;
    m_pimgTex       = NULL;
}

//! コピー

void CBrushTreeItem::copyFrom(const CBrushTreeItem *p)
{
    m_wSize         = p->m_wSize;
    m_wSizeCtlMax   = p->m_wSizeCtlMax;
    m_wMinSize      = p->m_wMinSize;
    m_wMinVal       = p->m_wMinVal;
    m_wInterval     = p->m_wInterval;
    m_wRotAngle     = p->m_wRotAngle;
    m_wRotRandom    = p->m_wRotRandom;
    m_wRan_sizeMin  = p->m_wRan_sizeMin;
    m_wRan_posLen   = p->m_wRan_posLen;
    m_wPressSize    = p->m_wPressSize;
    m_wPressVal     = p->m_wPressVal;

    m_btDatType     = p->m_btDatType;
    m_btVal         = p->m_btVal;
    m_btPixType     = p->m_btPixType;
    m_btHoseiType   = p->m_btHoseiType;
    m_btHoseiStr    = p->m_btHoseiStr;
    m_btRotType     = p->m_btRotType;
    m_btFlag        = p->m_btFlag;

    m_strBrushImg   = p->m_strBrushImg;
    m_strTexImg     = p->m_strTexImg;
}

//! WORD、BYTE データを配列で取得
/*!
    @param pWord 11個
    @param pByte 7個
*/

void CBrushTreeItem::getDatArray(LPWORD pWord,LPBYTE pByte)
{
    //WORD

    pWord[0] = m_wSize;
    pWord[1] = m_wSizeCtlMax;
    pWord[2] = m_wMinSize;
    pWord[3] = m_wMinVal;
    pWord[4] = m_wInterval;
    pWord[5] = m_wRotAngle;
    pWord[6] = m_wRotRandom;
    pWord[7] = m_wRan_sizeMin;
    pWord[8] = m_wRan_posLen;
    pWord[9] = m_wPressSize;
    pWord[10] = m_wPressVal;

    //BYTE

    pByte[0] = m_btDatType;
    pByte[1] = m_btVal;
    pByte[2] = m_btPixType;
    pByte[3] = m_btHoseiType;
    pByte[4] = m_btHoseiStr;
    pByte[5] = m_btRotType;
    pByte[6] = m_btFlag;
}

//! 配列データからセット

void CBrushTreeItem::setDatArray(LPWORD pWord,LPBYTE pByte)
{
    m_wSize         = pWord[0];
    m_wSizeCtlMax   = pWord[1];
    m_wMinSize      = pWord[2];
    m_wMinVal       = pWord[3];
    m_wInterval     = pWord[4];
    m_wRotAngle     = pWord[5];
    m_wRotRandom    = pWord[6];
    m_wRan_sizeMin  = pWord[7];
    m_wRan_posLen   = pWord[8];
    m_wPressSize    = pWord[9];
    m_wPressVal     = pWord[10];

    m_btDatType     = pByte[0];
    m_btVal         = pByte[1];
    m_btPixType     = pByte[2];
    m_btHoseiType   = pByte[3];
    m_btHoseiStr    = pByte[4];
    m_btRotType     = pByte[5];
    m_btFlag        = pByte[6];
}


//============================
// コピー/貼り付け用
//============================


//! テキスト形式での文字列取得（コピー用）

void CBrushTreeItem::getTextFormat(AXString *pstr)
{
    WORD wd[WORDBASE_NUM];
    BYTE bt[BYTEBASE_NUM];
    int i;
    char m[32];

    getDatArray(wd, bt);

    *pstr = "AZDWLBR;";

    //名前

    _textformat_str(pstr, m_strText);

    //WORDデータ

    for(i = 0; i < WORDBASE_NUM; i++)
    {
        AXValToHexStr(m, wd[i]);
        *pstr += m;
        *pstr += ';';
    }

    //BYTEデータ

    for(i = 0; i < BYTEBASE_NUM; i++)
    {
        AXValToHexStr(m, bt[i]);
        *pstr += m;
        *pstr += ';';
    }

    //ブラシ画像名

    _textformat_str(pstr, m_strBrushImg);

    //テクスチャ画像名

    _textformat_str(pstr, m_strTexImg);
}

//! テキスト形式用に文字列追加

void CBrushTreeItem::_textformat_str(AXString *pDst,const AXString &str)
{
    int len;
    LPCUSTR pc;
    char m[16];

    //Unicode(WORD)を16進数で
    /* 値の文字列に ';' の区切り文字が含まれると正しく処理できないため */

    for(pc = str, len = str.getLen(); len > 0; len--, pc++)
    {
        AXValToHexStrDig(m, *pc, 4);
        *pDst += m;
    }

    *pDst += ';';
}


//=======================
//貼り付け
//=======================


//! 貼り付け時用のテキスト形式からデータセット

BOOL CBrushTreeItem::setFromTextFormat(const AXString &strText)
{
    AXString str;
    LPCUSTR pc,pcEnd;
    int no,no2;
    WORD wd[WORDBASE_NUM];
    BYTE bt[BYTEBASE_NUM];

    str = strText;
    str.replace(';', 0);

    //Windows版のフォーマット

    if(str == "AZDW2BR")
        return _pasteText_win(str);

    //データ取得

    pcEnd = (LPUSTR)str + str.getLen();
    no2   = 2 + WORDBASE_NUM + BYTEBASE_NUM;

    for(pc = str, no = 0; pc < pcEnd; pc += AXUStrLen(pc) + 1, no++)
    {
        if(no == 0)
            //ヘッダ
            continue;
        else if(no == 1)
            //名前
            _pasteText_getStr(&m_strText, pc);
        else if(no >= 2 && no < 2 + WORDBASE_NUM)
            //WORDデータ
            wd[no - 2] = AXUStrHexToVal(pc, 4);
        else if(no >= 2 + WORDBASE_NUM && no < no2)
            //BYTEデータ
            bt[no - 2 - WORDBASE_NUM] = AXUStrHexToVal(pc, 2);
        else if(no == no2)
            //ブラシ画像
            _pasteText_getStr(&m_strBrushImg, pc);
        else if(no == no2 + 1)
            //テクスチャ画像
            _pasteText_getStr(&m_strTexImg, pc);
        else
            break;
    }

    //データが足りない

    if(no < 2 + WORDBASE_NUM + BYTEBASE_NUM + 2) return FALSE;

    //データセット

    setDatArray(wd, bt);

    return TRUE;
}

//! Windows版フォーマットからデータセット

BOOL CBrushTreeItem::_pasteText_win(const AXString &str)
{
    LPCUSTR pc,pcEnd;
    int no;
    WORD wd[11];
    BYTE bt[7];
    BYTE pixtype[6] = {
        PIXTYPE_BLEND, PIXTYPE_COMPARE, PIXTYPE_OVERWRITE,
        PIXTYPE_ERASE, PIXTYPE_BLUR, PIXTYPE_OVERWRITE2
    };

    pcEnd = (LPUSTR)str + str.getLen();

    for(pc = str, no = 0; pc < pcEnd; pc += AXUStrLen(pc) + 1, no++)
    {
        if(no == 0)
            //ヘッダ
            continue;
        else if(no == 1)
            //名前
            _pasteText_getStr(&m_strText, pc);
        else if(no == 2)
            //ブラシ画像
            m_strBrushImg = pc;
        else if(no == 3)
            //テクスチャ画像
            m_strTexImg = pc;
        else if(no >= 4 && no < 4 + 11)
            //WORDデータ
            wd[no - 4] = AXUStrHexToVal(pc, 4);
        else if(no >= 4 + 11 && no < 4 + 11 + 7)
            //BYTEデータ
            bt[no - 4 - 11] = AXUStrHexToVal(pc, 2);
        else
            break;
    }

    //データが足りない

    if(no < 4 + 11 + 7) return FALSE;

    //データ調整

    wd[8] *= 10;    //ランダム位置 小数点1桁 -> 2桁

    if(bt[0] == 2) bt[0] = DATTYPE_MANUAL;          //データ固定 -> 手動
    bt[2] = pixtype[bt[2]];                         //塗りタイプ
    if(bt[4] > HOSEISTR_MAX) bt[4] = HOSEISTR_MAX;  //補正強さ

    //データセット

    setDatArray(wd, bt);

    return TRUE;
}

//! テキスト形式から文字列取得(16bit Unicode:16進数)

void CBrushTreeItem::_pasteText_getStr(AXString *pDst,LPCUSTR pc)
{
    LPUSTR p = (LPUSTR)pc;
    UNICHAR c;

    pDst->empty();

    while(*p)
    {
        c = AXUStrHexToVal(p, 4, &p);
        if(c == 0) break;

        *pDst += c;
    }
}
