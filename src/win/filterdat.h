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
    フィルタダイアログデータ（ビッグエンディアン）


    BYTE : フラグ
           [0bit] ダイアログ上プレビューあり
           [1bit] プレビューとウィジェットを垂直レイアウトにする
           [2bit] キャンバスプレビュー
           [3bit] 設定を保存する
           [4bit] リセットボタンあり（バー値のみ）
    WORD : プレビュー幅
    WORD : プレビュー高さ

    (以下、各ウィジェットごと)

    BYTE : ウィジェットタイプ（255 で終了。0x80〜は独自定義ウィジェットで、以下のデータはない）
    WORD : ラベル文字列ID（文字列グループは STRGID_FILTERDLG）
    BYTE : データサイズ
    データ...

    ---- <type 0 : バー>

    short : バーの最小幅
    short : 最小値
    short : 最大値
    [short : デフォルト値] (default: 最小値)
    [short : 中央値] (default: 最小値)
    [BYTE  : エディット桁数] (default: 0)
    [char  : 設定保存時の識別子] (default: 0)
    [BYTE  : デフォルト値タイプ] (default: 0) 1 でデフォルト値は現在の DPI

    ---- <type 1 : チェックボタン>

    WORD : テキスト文字列ID
    [BYTE : チェックON/OFF] (default: 0)
    [char : 設定保存時の識別子] (default: 0)

    ---- <type 2 : コンボボックス>

    BYTE : 項目数
    WORD : 項目の文字列ID先頭
    [BYTE : デフォルト選択] (default: 0)
    [char : 設定保存時の識別子] (default: 0)

    ---- <type 3 : レイヤリスト・コンボボックス>

    データ無し

    ---- <type 4 : ラベル>

    WORD : 文字列ID

    ---- <独自定義ウィジェット>

    <type 0x80> : キャンバス範囲外をクリッピング チェックボックス
    <type 0x81> : レベル補正用ウィジェット
*/


//色

const BYTE g_filter_level[] = {
    1, 0,250, 0,250,
    0x81,
    255
};

const BYTE g_filter_gamma[] = {
    3, 0,250, 0,250,
    0, 0,10, 11, 0,250, 0,10, 0x01,0x90, 0,100, 0,100, 2,
    255
};

const BYTE g_filter_2col[] = {
    3, 0,250, 0,250,
    0, 0,11, 10, 0,250, 0,1, 0,255, 0,128, 0,128,
    255
};

const BYTE g_filter_2col_dither[] = {
    1, 0,250, 0,250,
    2, 0,12, 3, 5, 0x03,0xe8,
    255
};

//ぼかし

const BYTE g_filter_blur[] = {
    1, 0,220, 0,220,
    0, 0,22, 6, 0,100, 0,1, 0,16,
    0x80,
    255
};

const BYTE g_filter_gaussblur[] = {
    1, 0,200, 0,200,
    0, 0,22, 6, 0,100, 0,1, 0,20,
    0x80,
    255
};

const BYTE g_filter_motionblur[] = {
    1, 0,200, 0,200,
    0, 0,22, 6, 0,180, 0,1, 0,20,
    0, 0,23, 10, 0,180, 0xff,0x4c, 0,0xb4, 0,0, 0,0,
    0x80,
    255
};

//描画

const BYTE g_filter_drawamitone1[] = {
    1, 0,220, 0,220,
    0, 0,33, 11, 0,180, 0,10, 0x0f,0xa0, 0x02,0x58, 0,0, 1,
    0, 0,34, 8, 0,180, 0,1, 0,100, 0,10,
    0, 0,23, 10, 0,180, 0xff,0x4c, 0,0xb4, 0,45, 0,0,
    0, 0,35, 13, 0,180, 0,1, 0x09,0x60, 0,0, 0,0, 0, 0, 1,  //DPI
    1, 0,0, 3, 0,36, 1,
    255
};

const BYTE g_filter_drawamitone2[] = {
    1, 0,220, 0,220,
    0, 0,13, 11, 0,180, 0,10, 0x0f,0xa0, 0,50, 0,0, 1,
    0, 0,34, 8, 0,180, 0,1, 0,100, 0,10,
    0, 0,23, 10, 0,180, 0xff,0x4c, 0,0xb4, 0,45, 0,0,
    1, 0,0, 3, 0,36, 1,
    255
};

const BYTE g_filter_drawline[] = {
    1, 0,230, 0,230,
    0, 0,27, 6, 0,180, 0,1, 0,100,
    0, 0,28, 6, 0,180, 0,1, 0,100,
    0, 0,29, 8, 0,180, 0,1, 0x01,0x90, 0,2,
    0, 0,30, 8, 0,180, 0,1, 0x01,0x90, 0,2,
    1, 0,0, 3, 0,31, 1,
    1, 0,0, 2, 0,32,
    255
};

const BYTE g_filter_drawcheck[] = {
    1, 0,220, 0,220,
    0, 0,24, 8, 0,180, 0,2, 0x01,0x90, 0,5,
    0, 0,25, 8, 0,180, 0,2, 0x01,0x90, 0,5,
    1, 0,0, 3, 0,26, 1,
    255
};

//漫画用

const BYTE g_filter_comicConcLine[] = {
    4|8|16, 0,0, 0,0,
    0, 0,37, 12, 0,200, 0,10, 0x13,0x88, 0,200, 0,0, 0, 'a',    //半径(10-5000)
    0, 0,38, 12, 0,200, 0xfc,0x18, 0x03,0xe8, 0,0, 0,0, 3, 'b',  //縦横比
    0, 0,39, 12, 0,200, 0,2, 0x01,0x2c, 0,15, 0,0, 0, 'c',    //密度(2-300)
    0, 0,40, 12, 0,200, 0,0, 0,99, 0,100, 0,0, 0, 'd',        //間隔ランダム(0-99)
    0, 0,41, 12, 0,200, 0,5, 0x07,0xd0, 0,50, 0,0, 1, 'e',    //線の太さ(0.5-200.0)
    0, 0,42, 12, 0,200, 0,0, 0x03,0xe8, 0,0, 0,0, 1, 'f',
    0, 0,43, 12, 0,200, 0,1, 0x03,0xe8, 0x01,0xf4, 0,0, 1, 'g', //線の長さ(0.1-100.0)
    0, 0,44, 12, 0,200, 0,0, 0x03,0xe8, 0,200, 0,0, 1, 'h',     //線の長さランダム(0.0-100.0)
    2, 0,45,  5, 3, 0x04,0xb0, 0, 'i',  //描画タイプ
    1, 0,0, 3, 0,46, 1,     //簡易プレビュー
    4, 0,0, 2, 0,47,        //ヘルプ
    255
};

const BYTE g_filter_comicFlash[] = {
    4|8|16, 0,0, 0,0,
    0, 0,37, 12, 0,200, 0,10, 0x0f,0xa0, 0,200, 0,0, 0, 'a',  //半径(10-4000)
    0, 0,38, 12, 0,200, 0xfc,0x18, 0x03,0xe8, 0,0, 0,0, 3, 'b', //縦横比
    0, 0,39, 12, 0,200, 0,2, 0x01,0x2c, 0,100, 0,0, 0, 'c',   //密度(2-300)
    0, 0,40, 12, 0,200, 0,0, 0,99, 0,20, 0,0, 0, 'd',       //間隔ランダム(0-99)
    0, 0,41, 12, 0,200, 0,5, 0x07,0xd0, 0,80, 0,0, 1, 'e',  //線の太さ(0.5-200.0)
    0, 0,42, 12, 0,200, 0,0, 0x03,0xe8, 0,30, 0,0, 1, 'f',
    0, 0,43, 12, 0,200, 0,1, 0x03,0xe8, 0x01,0x5e, 0,0, 1, 'g',  //線の長さ(0.1-100.0)
    0, 0,44, 12, 0,200, 0,0, 0x03,0xe8, 0,200, 0,0, 1, 'h',      //線の長さランダム(0.0-100.0)
    2, 0,45,  5, 3, 0x04,0xb0, 0, 'i',  //描画タイプ
    1, 0,0, 3, 0,46, 1,     //簡易プレビュー
    4, 0,0, 2, 0,47,        //ヘルプ
    255
};

const BYTE g_filter_comicBetaFlash[] = {
    4|8|16, 0,0, 0,0,
    0, 0,37, 12, 0,200, 0,10, 0x0f,0xa0, 0,130, 0,0, 0, 'a',    //半径(10-4000)
    0, 0,38, 12, 0,200, 0xfc,0x18, 0x03,0xe8, 0,0, 0,0, 3, 'b', //縦横比
    0, 0,39, 12, 0,200, 0,2, 0x01,0x2c, 0,80, 0,0, 0, 'c',   //密度(2-300)
    0, 0,40, 12, 0,200, 0,0, 0,99, 0,20, 0,0, 0, 'd',       //間隔ランダム(0-99)
    0, 0,41, 12, 0,200, 0,5, 0x07,0xd0, 0,60, 0,0, 1, 'e',   //線の太さ(0.5-200.0)
    0, 0,42, 12, 0,200, 0,0, 0x03,0xe8, 0,20, 0,0, 1, 'f',
    0, 0,43, 12, 0,200, 0,1, 0x03,0xe8, 0,250, 0,0, 1, 'g',         //線の長さ(0.1-100.0)
    0, 0,44, 12, 0,200, 0,0, 0x03,0xe8, 0x01,0x2c, 0,0, 1, 'h',     //線の長さランダム(0.0-100.0)
    2, 0,45,  5, 3, 0x04,0xb0, 0, 'i',  //描画タイプ
    1, 0,0, 3, 0,46, 1,     //簡易プレビュー
    4, 0,0, 2, 0,47,        //ヘルプ
    255
};

const BYTE g_filter_comicUniFlash[] = {
    4|8|16, 0,0, 0,0,
    0, 0,37, 12, 0,200, 0,10, 0x0f,0xa0, 0,160, 0,0, 0, 'a',    //半径(10-4000)
    0, 0,38, 12, 0,200, 0xfc,0x18, 0x03,0xe8, 0,0, 0,0, 3, 'b', //縦横比
    0, 0,39, 12, 0,200, 0,2, 0x01,0x2c, 0,110, 0,0, 0, 'c',   //密度(2-300)
    0, 0,40, 12, 0,200, 0,0, 0,99, 0,0, 0,0, 0, 'd',       //間隔ランダム(0-99)
    0, 0,41, 12, 0,200, 0,5, 0x07,0xd0, 0,60, 0,0, 1, 'e',  //線の太さ(0.5-200.0)
    0, 0,42, 12, 0,200, 0,0, 0x03,0xe8, 0,0, 0,0, 1, 'f',
    0, 0,43, 12, 0,200, 0,1, 0x03,0xe8, 0x01,0x2c, 0,0, 1, 'g', //線の長さ(0.1-100.0)
    0, 0,44, 12, 0,200, 0,0, 0x03,0xe8, 0,0, 0,0, 1, 'h',     //線の長さランダム(0.0-100.0)
    2, 0,45,  5, 3, 0x04,0xb0, 0, 'i',  //描画タイプ
    1, 0,0, 3, 0,46, 1,     //簡易プレビュー
    4, 0,0, 2, 0,47,        //ヘルプ
    255
};

const BYTE g_filter_comicUniFlashWave[] = {
    4|8|16, 0,0, 0,0,
    0, 0,37, 12, 0,200, 0,10, 0x0f,0xa0, 0,160, 0,0, 0, 'a',   //半径(10-4000)
    0, 0,38, 12, 0,200, 0xfc,0x18, 0x03,0xe8, 0,0, 0,0, 3, 'b', //縦横比
    0, 0,39, 12, 0,200, 0,8, 0x01,0x2c, 0,30, 0,0, 0, 'c',   //密度(2-300)
    0, 0,48, 12, 0,200, 0,1, 0x03,0xe8, 0,200, 0,0, 1, 'd',  //波の長さ(0.1-100.0)
    0, 0,41, 12, 0,200, 0,5, 0x07,0xd0, 0,30, 0,0, 1, 'e',  //線の太さ(0.5-200.0)
    0, 0,43, 12, 0,200, 0,1, 0x03,0xe8, 0x01,0x2c, 0,0, 1, 'f',  //線の長さ(0.1-100.0)
    0, 0,49, 12, 0,200, 0,0, 0,100, 0,0, 0,0, 0, 'g',    //抜きの最小(0-100)
    2, 0,45,  5, 3, 0x04,0xb0, 0, 'h',  //描画タイプ
    1, 0,0, 3, 0,46, 1,     //簡易プレビュー
    4, 0,0, 2, 0,47,        //ヘルプ
    255
};

//他

const BYTE g_filter_layercol[] = {
    0, 0,0, 0,0,
    3, 0,17, 0,
    2, 0,18, 3, 7, 0x04,0x4c,
    255
};

const BYTE g_filter_mozaic[] = {
    1, 0,220, 0,220,
    0, 0,13, 6, 0,120, 0,2, 0,100,
    0x80,
    255
};

const BYTE g_filter_crystal[] = {
    1, 0,220, 0,220,
    0, 0,13, 6, 0,140, 0,5, 0,200,
    1, 0,0, 2, 0,14,
    0x80,
    255
};

const BYTE g_filter_noise[] = {
    1, 0,220, 0,220,
    0, 0,15, 6, 0,100, 0,1, 0,32,
    0, 0,13, 6, 0,100, 0,1, 0,20,
    1, 0,0, 3, 0,16, 1,
    255
};

const BYTE g_filter_edge[] = {
    1, 0,220, 0,220,
    0, 0,19, 6, 0,100, 0,1, 0,24,
    3, 0,20, 0,
    1, 0,0, 2, 0,21,
    255
};

const BYTE g_filter_halftone[] = {
    1, 0,220, 0,220,
    0, 0,13, 11, 0,180, 0,10, 0x07,0xd0, 0,50, 0,0, 1,
    0, 0,23, 10, 0,180, 0xff,0x4c, 0,0xb4, 0,45, 0,0,
    1, 0,0, 3, 0,36, 1,
    255
};
