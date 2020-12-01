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

#ifndef _AZDRAW_BRUSHIMGLIST_H_
#define _AZDRAW_BRUSHIMGLIST_H_

#include "AXList.h"
#include "AXString.h"
#include "CImage8.h"

class CBrushImgItem:public AXListItem
{
public:
    enum
    {
        TYPE_BRUSH,
        TYPE_TEXTURE
    };

public:
    int         m_nType,
                m_nRefCnt;  //!< 参照カウンタ
    AXString    m_strPath;
    CImage8     m_img;

public:
    CBrushImgItem *next() { return (CBrushImgItem *)m_pNext; }
};

//

class CBrushImgList:public AXList
{
protected:
    CBrushImgItem *_searchImage(int type,const AXString &strPath);

public:
    CBrushImgItem *getTopItem() { return (CBrushImgItem *)m_pTop; }

    CImage8 *loadImage(int type,const AXString &strPath);
    void freeImage(CImage8 *pimg);
};

#endif
