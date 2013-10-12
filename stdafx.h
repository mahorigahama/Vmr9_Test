#pragma once
#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// #define D3D_DEBUG_INFO
// Windows Header Files:
#include <windows.h>
// C RunTime Header Files
#include <tchar.h>

// ATL
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS// some CString constructors will be explicit
#include <atlbase.h>
#include <atlwin.h>
#include <atlstr.h>

// STL
#include <vector>

// Direct3D 9, DirecShow

// #include <streams.h>

#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <d3dx9.h>
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dxguid.lib")
#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")
#else
#pragma comment(lib, "d3dx9.lib")
#endif

// Direct3D 10
#include <d3d10.h>
#include <d3dx10.h>
#include <dxerr.h>
#pragma comment(lib,"d3d10.lib")
#pragma comment(lib,"dxerr.lib")
#ifdef _DEBUG
#pragma comment(lib,"d3dx10d.lib")
#else
#pragma comment(lib,"d3dx10.lib")
#endif
