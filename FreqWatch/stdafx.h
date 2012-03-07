// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER		0x0500
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200
#define _WTL_NO_WTYPES
#include <atlbase.h>
#include <atlstr.h>
#include <atltypes.h>
#include <atlapp.h>
#include <vector>
#include <map>
#include <atlmisc.h>
#include <atlimage.h>
#include <atlscrl.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlctrlw.h>
#include <atlcrack.h>

extern CAppModule _Module;

#include <atlwin.h>
#include <atldlgs.h>

#include <extools.h>
#include <CSqlite.h>
#include <JsonLib.h>

#ifdef _DEBUG
#pragma comment(lib,"ExLibd.lib")
#else
#pragma comment(lib,"ExLibInline.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib,"sqliteD.lib")
#else
#pragma comment(lib,"sqlite.lib")
#endif

#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mfuuid.lib")
#pragma comment(lib,"mf.lib")
#ifdef _DEBUG
#pragma comment(lib,"..\\Debug\\WavSink.lib")
#else
#pragma comment(lib,"..\\release\\WavSink.lib")
#endif

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
