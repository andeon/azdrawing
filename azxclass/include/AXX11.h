
#include <X11/Xlib.h>

#ifdef _AX_X11_ATOM
#include <X11/Xatom.h>
#endif

#ifdef _AX_X11_UTIL
#include <X11/Xutil.h>
#endif

#ifdef _AX_X11_LOCALE
#include <X11/Xlocale.h>
#endif

#ifdef _AX_X11_XSHM
#include <X11/extensions/XShm.h>
#endif

#ifdef _AX_X11_XFT
#include <X11/Xft/Xft.h>
#endif

#if defined(_AX_X11_XI2) && defined(_AX_OPT_XI2)
#include <X11/extensions/XInput2.h>
#endif
