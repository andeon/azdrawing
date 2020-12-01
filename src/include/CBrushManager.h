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

#ifndef _AZDRAW_BRUSHMANAGER_H_
#define _AZDRAW_BRUSHMANAGER_H_

#include "AXDef.h"

class CBrushImgList;
class CBrushTreeItem;
class AXString;
class AXTreeViewItem;
class AXTreeView;
class AXFileWriteBuf;
class AXBuf;

class CBrushManager
{
public:
    static CBrushManager *m_pSelf;

    enum { REGITEM_NUM = 5 };

protected:
    CBrushImgList   *m_pImgList;

    CBrushTreeItem  *m_pEditItem,
                    *m_pSelItem,
                    *m_pRegItem[REGITEM_NUM];

protected:
    void _freeImage(CBrushTreeItem *p);
    void _loadImage(CBrushTreeItem *p,BOOL bForce);
    void _saveFileBrush(AXFileWriteBuf *pfile,CBrushTreeItem *p);
    BOOL _loadFileBrush(AXBuf *pbuf,CBrushTreeItem *p);

public:
    CBrushManager();
    ~CBrushManager();

    CBrushTreeItem *getSelBrush() const { return m_pSelItem; }
    CBrushTreeItem *getEdit() { return m_pEditItem; }
    CBrushTreeItem *getRegBrush(int no) { return m_pRegItem[no]; }

    CBrushTreeItem *newBrush();
    CBrushTreeItem *cloneBrush(const CBrushTreeItem *pSrc);
    CBrushTreeItem *newBrushFromText(const AXString &str);
    void delBrush(CBrushTreeItem *p);

    BOOL changeSelBrush(CBrushTreeItem *p);
    void saveBrushManual();
    int getRegBrushNo(CBrushTreeItem *p);
    void setRegBrush(CBrushTreeItem *p,int no);

    BOOL isDatLink();
    BOOL isEditBrushErase();

    //

    void changeDatType(int type);
    void changeBrushSize(int size,BOOL bDirect=FALSE);
    void changeBrushSizeMax(int size,int max);
    void changeVal(int val);
    void changeHoseiType(int type);
    void changeHoseiStr(int str);
    void changePixType(int type);
    void changeSizeMin(int pers);
    void changeValMin(int pers);
    void changeInterval(int interval);
    void changeRandomSize(int pers);
    void changeRandomPos(int len);
    void changeRandomPosType(int type);
    void changeBrushImg(const AXString &str);
    void changeBrushImgRotType(int type);
    void changeBrushImgAngle(int angle);
    void changeBrushImgRandom(int w);
    void changeTextureImg(const AXString &str);
    void changeAntiAlias(BOOL bOn);
    void changeCurve(BOOL bOn);
    void changePressSize(int val);
    void changePressVal(int val);

    //

    void saveFile(const AXString &filename,AXTreeViewItem *pTop,AXTreeViewItem *pSel);
    void loadFile(const AXString &filename,AXTreeView *ptree);
};

#define BRUSHMAN        (CBrushManager::m_pSelf)
#define BRUSHMANEDIT    ((CBrushManager::m_pSelf)->getEdit())

#endif
