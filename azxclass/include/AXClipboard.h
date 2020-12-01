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

#ifndef _AX_CLIPBOARD_H
#define _AX_CLIPBOARD_H

#include "AXDef.h"

class AXString;
class AXMem;

class AXClipboard
{
    friend class AXApp;

public:
    enum TYPE
    {
        TYPE_NONE   = 0,
        TYPE_TEXT   = 1,
        TYPE_USER   = 100
    };

    typedef struct
    {
        LPBYTE  pBuf;
        UINT    uSize;
        ULONG   idSendWindow,
                atomProp,
                atomType;
        BOOL    bMultiple;
    }CALLBACK;

protected:
    UINT    m_uDatType,
            m_uDatSize;
    LPBYTE  m_pDatBuf;
    ULONG   *m_pAtomList;
    int     m_nAtomCnt;
    BOOL (*m_callback)(AXClipboard *,CALLBACK *);

protected:
    BOOL _setAtomList(const ULONG *pAtomList,int atomcnt);
    void OnSelectionRequest(LPVOID pEvent);
    void _setMultiple(ULONG window,ULONG prop);
    BOOL _sendOneDat(ULONG window,ULONG prop,ULONG type,BOOL bMultiple);
    BOOL _sendDefText(ULONG window,ULONG prop,ULONG type);

public:
    AXClipboard();
    ~AXClipboard();

    UINT getType() const { return m_uDatType; }
    BOOL isExist(UINT type) const;

    void clear();

    BOOL setDat(ULONG idWindow,UINT type,const void *pBuf,UINT size,const ULONG *pAtomList,int atomcnt,BOOL (*callback)(AXClipboard *,CALLBACK *));
    BOOL setText(ULONG idWindow,const AXString &str);
    BOOL setText(ULONG idWindow,LPCUSTR pstr,int len);

    BOOL getDat(ULONG idWindow,ULONG atomType,AXMem *pmem);
    ULONG getDat(ULONG idWindow,ULONG *pAtomList,int atomcnt,AXMem *pmem);
    BOOL getText(ULONG idWindow,AXString *pstr);

    BOOL save(ULONG idWindow,UINT type=0);

    void sendDat(CALLBACK *pDat,const void *pBuf,UINT uSize);
};

#endif
