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

#include <sys/types.h>
#include <dirent.h>

#include "AXDir.h"
#include "AXByteString.h"
#include "AXFileStat.h"
#include "AXUtilFile.h"


/*!
    @class AXDir
    @brief ディレクトリ読み込みクラス

    @ingroup etc
*/


AXDir::AXDir()
{
    m_pDir = NULL;
}

AXDir::~AXDir()
{
    close();
}

//! 閉じる

void AXDir::close()
{
    if(m_pDir)
    {
        ::closedir((DIR *)m_pDir);
        m_pDir = NULL;
    }
}

//! 開く

BOOL AXDir::open(const AXString &strDir)
{
    AXByteString str;

    close();

    //

    strDir.toLocal(&str);

    m_pDir = (LPVOID)::opendir(str);
    if(!m_pDir) return FALSE;

    m_strDir = strDir;
    m_strCurName.empty();

    return TRUE;
}

//! 次のファイル取得
/*!
    @return FALSEで終了
*/

BOOL AXDir::read()
{
    struct dirent *p;

    p = ::readdir((DIR *)m_pDir);
    if(!p) return FALSE;

    m_strCurName.setLocal(p->d_name);

    return TRUE;
}

//! 現在のファイル名取得

void AXDir::getFileName(AXString *pstr,BOOL bFullPath)
{
    if(bFullPath)
    {
        *pstr = m_strDir;
        pstr->path_add(m_strCurName);
    }
    else
        *pstr = m_strCurName;
}

//! 現在のファイルの情報取得

BOOL AXDir::getFileStat(AXFILESTAT *pdst)
{
    AXString str;

    getFileName(&str, TRUE);

    return AXGetFILESTAT(str, pdst);
}

//! 現在のファイル名が特殊なパス（"." ".."）か

BOOL AXDir::isSpecPath()
{
    return (m_strCurName == "." || m_strCurName == "..");
}
