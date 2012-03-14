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
#include <HttpClient.h>
#include <Mfapi.h>

#ifdef _DEBUG
#pragma comment(lib,"ExLibd.lib")
#pragma comment(lib,"zlibd.lib")
#else
#pragma comment(lib,"ExLibInline.lib")
#pragma comment(lib,"zlib.lib")
#endif

#ifdef _DEBUG
#pragma comment(lib,"sqliteD.lib")
#else
#pragma comment(lib,"sqlite.lib")
#endif

#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mfuuid.lib")
#pragma comment(lib,"mf.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Winmm.lib")
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

#include <ByteStream.h>
#include <gzipcap2.h>
struct GZipOutput:public OutputInterface
{
	GZipOutput():OutputInterface(WriteF,WriteCharF)
	{
		ByteStream::CreateInstanse(&memstream);
		CComPtr<IStream> tempstream;
		memstream->QueryInterface(&tempstream);
		zipcap.Reset(tempstream);
	}
	void Flush()
	{
		zipcap.Finish();
	}
	GZIPCap2 zipcap;
	CComPtr<IMemoryStream> memstream;
	static void WriteF(OutputInterface*This,LPCSTR str,int strlen)
	{
		((GZipOutput*)This)->zipcap.Write(str,strlen);
	}
	static void WriteCharF(OutputInterface*This,char c)
	{
		((GZipOutput*)This)->zipcap.Write(&c,1);
	}
};
struct FreqInfo
{
	int freq;
	int time;
	double strong;
};