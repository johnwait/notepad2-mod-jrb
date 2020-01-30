// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

// 2019-08-22: Now expressly targeting Win7+
#if !defined(WINVER)
#define WINVER 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x601  /*_WIN32_WINNT_WIN7*/
#endif
#if !defined(NTDDI_VERSION)
#define NTDDI_VERSION 0x06010000  /*NTDDI_WIN7*/
#endif

// In-dev feature flags; to be decided on on the next feature-set freeze
#define FEAT_RTFDLG_REGEX_SYNTAX
#define FEAT_NOTIFY_CHANGE_ON_ACTIVEAPP
#if FALSE
#define FEAT_REPLACE_MSGBOX_BY_TASKDLG
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>    // Dialogs.c, Dlapi.c, Edit.c, Helpers.c, Notepad2.c, Print.cpp, Styles.c
#include <commctrl.h>   // Dialogs.c, Dlapi.c, Edit.c, Helpers.c, Notepad2.c, Print.cpp, Styles.c
#include <shlobj.h>     // Dialogs.c, Dlapi.c, Helpers.c, Notepad2.c, Styles.c
#include <shellapi.h>   // Dialogs.c, Notepad2.c
#include <shlwapi.h>    // Dialogs.c, Dlapi.c, Edit.c, Helpers.c, Notepad2.c, Print.cpp, Styles.c
#include <commdlg.h>    // Dialogs.c, Edit.c, Notepad2.c, Print.cpp, Styles.c
#include <stdio.h>      // Edit.c, Helpers.c, Notepad2.c, Styles.c
#include <string.h>     // Dialogs.c, Dlapi.c, Edit.c, Helpers.c, Notepad2.c, Print.cpp
#include <strsafe.h>
#include <tchar.h>      // Edit.c (for _strdec(), _strninc())
#include <time.h>       // Notepad2.c
#include <limits.h>     // Edit.c
#include <uxtheme.h>    // Helpers.c

// (Re-)Implement lstrcpynW() & lstrcpynA() as wrappers around more safer StringCchCopy*()
#define lstrcpynW(dst, src, max_buffer_size) \
    (StringCchCopyW(dst, max_buffer_size, src) == S_OK ? dst : NULL)
#define lstrcpynA(dst, src, max_buffer_size) \
    {HRESULT hr = StringCchCopyA(dst, max_buffer_size, src); if (!SUCCEEDED(hr)) { DebugBreak(); }; (hr == S_OK ? dst : NULL);}

// (Re-)Implement lstrcatnW() & lstrcatnA() as wrappers around more safer StringCchCat*()

#define lstrcatnW(dst, src, max_buffer_size) \
    (StringCchCatW(dst, max_buffer_size, src) == S_OK ? dst : NULL)
#define lstrcatnA(dst, src, max_buffer_size) \
    {HRESULT hr = StringCchCatA(dst, max_buffer_size, src); if (!SUCCEEDED(hr)) { DebugBreak(); }; (hr == S_OK ? dst : NULL);}

#define lstrncatnW(dst, to_append_size, src, max_buffer_size) \
    (StringCchCatNW(dst, max_buffer_size, src, to_append_size) == S_OK ? dst : NULL)
#define lstrncatnA(dst, to_append_size, src, max_buffer_size) \
    {HRESULT hr = StringCchCatNA(dst, max_buffer_size, src, to_append_size); if (!SUCCEEDED(hr)) { DebugBreak(); }; (hr == S_OK ? dst : NULL);}

// Implement _strdec() & _strninc() when _UNICODE is used (normally in <tchar.h>)
#ifdef _UNICODE
_Check_return_ __inline char * __CRTDECL _strdec(_In_reads_z_(_Cpc2 - _Cpc1) const char * _Cpc1, _In_z_ const char * _Cpc2) { return (char *)((_Cpc1)>=(_Cpc2) ? NULL : (_Cpc2-1)); }
///_Check_return_ __inline char * __CRTDECL _strinc(_In_z_ const char * _Pc) { return (char *)(_Pc+1); }
///_Check_return_ __inline unsigned int __CRTDECL _strnextc(_In_z_ const char * _Cpc) { return (unsigned int)*(const unsigned char *)_Cpc; }
_Check_return_ __inline char * __CRTDECL _strninc(_In_reads_or_z_(_Sz) const char * _Pc, _In_ size_t _Sz) { return (char *)(_Pc+_Sz); }
#endif // _UNICODE

///   End of StdAfx.h   \\\
