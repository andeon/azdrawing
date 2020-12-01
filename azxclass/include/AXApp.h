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

#ifndef _AX_APP_H
#define _AX_APP_H

#include "AXString.h"

class AXWindow;
class AXRootWindow;
class AXAppWinHash;
class AXAppRes;
class AXAppAtom;
class AXAppRun;
class AXList;
class AXTranslation;
class AXClipboard;
class AXAppIM;
class AXAppIC;
class AXDND;
class AXMutex;
class AXCursor;

//------------------

class AXApp
{
    friend class AXWindow;
    friend class AXTopWindow;

public:
    enum CUSORTYPE
    {
        CURSOR_HSPLIT,
        CURSOR_VSPLIT,

        CURSOR_NUM
    };

protected:
    LPVOID      m_pDisp;
    ULONG       m_timeLast,
                m_timeUser;

private:
    ULONG       m_idRoot,
                m_idColmap;
    LPVOID      m_pVisual;
    int         m_nScreen,
                m_nConnection,
                m_nDepth;

    BOOL        m_bUpdate;
    UINT        m_uSupportFlag;

    AXAppAtom       *m_pAtom;
    AXAppWinHash    *m_pWinTbl;
    AXAppRes        *m_pRes;
    AXList          *m_pTimerList,
                    *m_pNotifyList;
    AXTranslation   *m_ptransDef,
                    *m_ptransApp;
    AXClipboard     *m_pClipboard;
    AXDND           *m_pDND;
    AXAppIM         *m_pIM;
    AXCursor        *m_pCursor[CURSOR_NUM];

    AXMutex         *m_pMutex;
    int             m_fdPipe[2],
                    m_nMaxFD;

    AXRootWindow    *m_pwinRoot;
    AXAppRun        *m_pNowRun;
    AXWindow        *m_pDrawTop,
                    *m_pDrawBottom,
                    *m_pReConfTop,
                    *m_pReConfBottom,
                    *m_pwinGrab;

    int         m_nXIopcode;
    int         m_nRedLShift,
                m_nGreenLShift,
                m_nBlueLShift,
                m_nRedRShift,
                m_nGreenRShift,
                m_nBlueRShift;

    char        m_cLang[6];
    AXString    m_strResDir,
                m_strConfDir;

public:
    static AXApp    *m_pApp;

private:
    static ULONG    m_dbcWin;
    static ULONG    m_dbcTime;
    static int      m_dbcX,m_dbcY;
    static UINT     m_dbcButton;

protected:
    enum SUPPORTEDFLAG
    {
        SUPPORT_XSHM            = 0x0001,
        SUPPORT_USERTIME        = 0x0002,
        SUPPORT_NETACTIVEWINDOW = 0x0004
    };

private:
    void addWindow(AXWindow *pwin);
    void removeWindow(AXWindow *pwin);
    BOOL grabWindow(AXWindow *pwin);
    void ungrabWindow();

    BOOL _main(BOOL bIdle);
    BOOL _wait();
    BOOL _event();
    void _event_XI2(LPVOID pev);
    void _event_time(LPVOID pev);
    void _draw();

    UINT _convButtonState(UINT state);
    void _event_ButtonPress(AXWindow *pwin,LPVOID pev,LPVOID phd);
    void _compEvent_Configure(LPVOID pev);
    void _sendPing(long *pdat);
    void _checkSupport();
    void _updateUserTime(AXWindow *pwin,ULONG time);

    void addDraw(AXWindow *pwin,int x,int y,int w,int h);
    void removeDraw(AXWindow *pwin);
    AXAppIC *createIC(AXWindow *pwin);

    void addNotify(AXWindow *pwin,AXWindow *pwinFrom,UINT uNotify,ULONG lParam);
    void addCommand(AXWindow *pwin,UINT uID,ULONG lParam,int from);
    void removeNotify(AXWindow *pwin);
    BOOL _notify();

    void addReconfig(AXWindow *pwin);
    void removeReconfig(AXWindow *pwin);
    void _reconfig();

    void addTimer(AXWindow *pwin,UINT uTimerID,UINT uInterval,ULONG lParam);
    void delTimer(AXWindow *pwin,UINT uTimerID);
    void delTimerWin(AXWindow *pwin);
    BOOL isTimerExist(AXWindow *pwin,UINT uTimerID);
    BOOL isTimerExistWin(AXWindow *pwin);
    void _timer();

protected:
    virtual void onIdle();
    virtual void onInputEvent(AXWindow *pwin,LPVOID pEvent,BOOL bSkip);

public:

    AXApp();
    virtual ~AXApp();

    //

    LPVOID getDisp() const { return m_pDisp; }
    ULONG getRootID() const { return m_idRoot; }
    int getDefScreen() const { return m_nScreen; }
    UINT getColmap() const { return m_idColmap; }
    LPVOID getVisual() const { return m_pVisual; }
    int getDepth() const { return m_nDepth; }
    int getConnection() const { return m_nConnection; }

    AXWindow *getRootWindow() const { return (AXWindow *)m_pwinRoot; }
    AXAppRes *getRes() const { return m_pRes; }
    AXClipboard *getClipboard() const { return m_pClipboard; }
    AXCursor *getCursor(int type) const { return m_pCursor[type]; }

    ULONG getLastTime() const { return m_timeLast; }
    ULONG getUserTime() const { return m_timeUser; }

    BOOL isSupport_XSHM() const { return m_uSupportFlag & SUPPORT_XSHM; }
    BOOL isSupport_UserTime() const { return m_uSupportFlag & SUPPORT_USERTIME; }
    BOOL isSupport_NetActiveWindow() const { return m_uSupportFlag & SUPPORT_NETACTIVEWINDOW; }

    virtual BOOL isGrab();
    BOOL isModal();

    //

    void end();
    BOOL init(int argc,char **argv,BOOL bLocale=TRUE);

    void exit();
    void run();
    void runQueue();
    void runIdle();
    void runDlg(AXWindow *pDlg);
    void runPopup(AXWindow *pPopup);

    void update() { m_bUpdate = TRUE; }
    void sync();
    void flush();

    void mutexLock();
    void mutexUnlock();
    void selectOut();

    //

    ULONG atom(int no) const;
    ULONG getAtom(LPCSTR szAtomName) const;
    AXWindow *getWindowFromID(ULONG idWin) const;
    AXWindow *getModalWin();
    void getCursorPos(AXPoint *ppt) const;

    void setResourceDir(LPCSTR szPath) { m_strResDir = szPath; }
    void setConfigDir(LPCSTR szPath) { m_strConfDir = szPath; }
    void setConfigDirHome(LPCSTR szPath);
    BOOL createConfigDir() const;
    void getResourcePath(AXString *pstr,LPCSTR szAdd) const;
    void getConfigPath(AXString *pstr,LPCSTR szAdd) const;

    DWORD rgbToPix(int r,int g,int b) const;
    DWORD rgbToPix(DWORD col) const;
    int getPixRed(DWORD c) const { return ((c >> m_nRedLShift) << m_nRedRShift) & 255; }
    int getPixGreen(DWORD c) const { return ((c >> m_nGreenLShift) << m_nGreenRShift) & 255; }
    int getPixBlue(DWORD c) const { return ((c >> m_nBlueLShift) << m_nBlueRShift) & 255; }

    void loadTranslation(LPBYTE pDefLangDat,LPCSTR szLangName=NULL);
    void setTrGroup(WORD wGroupID);
    LPCUSTR getTrStr(WORD wStrID) const;
    LPCUSTR getTrStrRaw(WORD wStrID) const;
    LPCUSTR getTrStrDef(WORD wStrID) const;
    LPCUSTR getTrString(WORD wGroupID,WORD wStrID) const;
    LPCUSTR getTrStringRaw(WORD wGroupID,WORD wStrID) const;
};

//------------------

#define axapp           (AXApp::m_pApp)
#define axres           ((AXApp::m_pApp)->getRes())
#define axclipb         ((AXApp::m_pApp)->getClipboard())
#define axdisp()        ((Display *)(AXApp::m_pApp)->getDisp())
#define axatom(no)      ((AXApp::m_pApp)->atom(no))
#define _trgroup(id)    (AXApp::m_pApp)->setTrGroup(id)
#define _str(id)        (AXApp::m_pApp)->getTrStr(id)
#define _string(gid,id)    (AXApp::m_pApp)->getTrString(gid, id)

#endif
