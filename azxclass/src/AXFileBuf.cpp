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

#include <string.h>

#include "AXFileBuf.h"


/*!
    @class AXFileBuf
    @brief ファイルまたはバッファからの読み込み操作クラス

    @ingroup etc
*/


AXFileBuf::AXFileBuf()
{
    m_pBufNow = NULL;
}

//! 閉じる

void AXFileBuf::close()
{
    m_file.close();

    m_pBufNow = NULL;
}

//! ファイルを開く

BOOL AXFileBuf::openFile(const AXString &filename)
{
    close();

    if(!m_file.openRead(filename)) return FALSE;

    m_dwSize = m_file.getSize();

    return TRUE;
}

//! バッファを開く

void AXFileBuf::openBuf(LPVOID pBuf,DWORD dwSize)
{
    close();

    //

    m_pBufTop = (LPBYTE)pBuf;
    m_pBufNow = m_pBufTop;
    m_dwSize  = dwSize;
}

//! 位置を相対移動

void AXFileBuf::seek(int pos)
{
    if(m_pBufNow)
    {
        m_pBufNow += pos;

        if(m_pBufNow < m_pBufTop)
            m_pBufNow = m_pBufTop;
        else if((DWORD)(m_pBufNow - m_pBufTop) > m_dwSize)
            m_pBufNow = m_pBufTop + m_dwSize;
    }
    else
        m_file.seekCur(pos);
}

//! 位置を移動

void AXFileBuf::setPos(DWORD pos)
{
    if(m_pBufNow)
    {
        if(pos > m_dwSize) pos = m_dwSize;
        m_pBufNow = m_pBufTop + pos;
    }
    else
        m_file.seekTop(pos);
}

//! 読み込み

BOOL AXFileBuf::read(LPVOID pBuf,DWORD dwSize)
{
    if(m_pBufNow)
    {
        if(m_pBufNow - m_pBufTop + dwSize > m_dwSize)
        {
            m_pBufNow = m_pBufTop + m_dwSize;

            return FALSE;
        }
        else
        {
            ::memcpy(pBuf, m_pBufNow, dwSize);
            m_pBufNow += dwSize;

            return TRUE;
        }
    }
    else
    {
        return m_file.readSize(pBuf, dwSize);
    }
}
