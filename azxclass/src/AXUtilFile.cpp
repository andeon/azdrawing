/*****************************************************************************
 *  Copyright (C) 2012-2015 Azel.
 *
 *  This file is part of AzXClass.
 *
 *  This library is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this library.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>

#include "AXUtilFile.h"
#include "AXFileStat.h"

#include "AXString.h"
#include "AXByteString.h"
#include "AXDir.h"


/*!
    @defgroup AXUtilFile AXUtilFile
    ファイルシステム関連ユーティリティ

    @ingroup util

    @{
*/

//--------------------------


//! 指定パスのファイル/ディレクトリが存在するか
/*!
    @param bDir ディレクトリかどうかを判定
*/

BOOL AXIsExistFile(const AXString &filename,BOOL bDir)
{
    AXByteString str;
    struct stat st;

    filename.toLocal(&str);

    if(::stat(str, &st) != 0) return FALSE;

    if(bDir)
    {
        if(!S_ISDIR(st.st_mode)) return FALSE;
    }

    return TRUE;
}

//! ファイルサイズ取得

LONGLONG AXGetFileSize(const AXString &filename)
{
    AXByteString str;
    struct stat st;

    filename.toLocal(&str);

    if(::stat(str, &st) != 0)
        return 0;
    else
        return st.st_size;
}

//! ディレクトリ作成

BOOL AXCreateDir(const AXString &dirpath,int perm)
{
    AXByteString str;

    dirpath.toLocal(&str);

    return (::mkdir(str, perm) == 0);
}

//! ホームディレクトリ＋szPathAdd のディレクトリ作成

BOOL AXCreateDirHome(LPCSTR szPathAdd,int perm)
{
    AXString str;

    str.path_setHomePath();
    str.path_add(szPathAdd);

    return AXCreateDir(str, perm);
}

//! ファイル削除

BOOL AXDeleteFile(const AXString &filename)
{
    AXByteString str;

    filename.toLocal(&str);

    return (::unlink(str) == 0);
}

//! ディレクトリ削除
/*!
    ※ディレクトリ内は空であること
*/

BOOL AXDeleteDir(const AXString &dirpath)
{
    AXByteString str;

    dirpath.toLocal(&str);

    return (::rmdir(str) == 0);
}

//! ディレクトリ内のファイルをすべて削除して、ディレクトリ削除
/*!
    ※サブディレクトリは削除されない。
*/

void AXDeleteDirAndFiles(const AXString &dirpath)
{
    AXDir dir;
    AXFILESTAT stat;
    AXString str;

    //ファイル削除

    if(!dir.open(dirpath)) return;

    while(dir.read())
    {
        if(dir.getFileStat(&stat))
        {
            if(!stat.isDirectory())
            {
                dir.getFileName(&str, TRUE);
                AXDeleteFile(str);
            }
        }
    }

    dir.close();

    //ディレクトリ削除

    AXDeleteDir(dirpath);
}

//! 指定ファイルの情報取得

BOOL AXGetFILESTAT(const AXString &filename,AXFILESTAT *pdst)
{
    AXByteString str;
    struct stat st;

    filename.toLocal(&str);

    if(::stat(str, &st) != 0) return FALSE;

    AXStatToFILESTAT(pdst, &st);

    return TRUE;
}

//! struct stat のデータを AXFILESTAT へ
//! @param psrc  (struct stat *)

void AXStatToFILESTAT(AXFILESTAT *pdst,const void *psrc)
{
    struct stat *p = (struct stat *)psrc;

    pdst->nPerm        = p->st_mode & 0777;
    pdst->lFileSize    = p->st_size;
    pdst->timeAccess   = p->st_atime;
    pdst->timeModify   = p->st_mtime;
    pdst->timeStChange = p->st_ctime;

    pdst->uFlags = 0;
    if(S_ISREG(p->st_mode)) pdst->uFlags |= AXFILESTAT::FLAG_NORMAL;
    if(S_ISDIR(p->st_mode)) pdst->uFlags |= AXFILESTAT::FLAG_DIRECTORY;
    if(S_ISLNK(p->st_mode)) pdst->uFlags |= AXFILESTAT::FLAG_SYMLINK;
}

//! ファイルのタイムスタンプ変更
/*!
    @param timeAccess 最終アクセス時間
    @param timeMod    最終更新時間
*/

BOOL AXSetFileTimestamp(const AXString &filename,LONGLONG timeAccess,LONGLONG timeMod)
{
    struct utimbuf time;
    AXByteString str;

    time.actime  = (time_t)timeAccess;
    time.modtime = (time_t)timeMod;

    filename.toLocal(&str);

    return (::utime(str, &time) == 0);
}


//@}

/*!
    @struct AXFILESTAT
    @brief ファイル情報

    @var AXFILESTAT::FLAG_NORMAL
    @brief 通常ファイル
    @var AXFILESTAT::FLAG_DIRECTORY
    @brief ディレクトリ
    @var AXFILESTAT::FLAG_SYMLINK
    @brief シンボリックリンク

    @var AXFILESTAT::nPerm
    @brief パーミッション（8進数）
    @var AXFILESTAT::uFlags
    @brief フラグ
    @var AXFILESTAT::lFileSize
    @brief ファイルサイズ
    @var AXFILESTAT::timeAccess
    @brief 最終アクセス時間
    @var AXFILESTAT::timeModify
    @brief 最終更新時間
    @var AXFILESTAT::timeStChange
    @brief 最終状態更新時間
*/
