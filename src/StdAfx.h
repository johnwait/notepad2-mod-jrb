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

#ifdef JRB_BUILD

// In-dev feature flags; to be decided on on the next feature-set freeze
#define FEAT_REGEX_SYNTAX_RTFDLG
#define FEAT_REGEX_SYNTAX_ALLOW_SELCOPY
#define FEAT_NOTIFY_CHANGE_ON_ACTIVEAPP
///#if FALSE
#define FEAT_REPLACE_MSGBOX_BY_TASKDLG
///#endif

#endif // JRB_BUILD

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
#include <windowsx.h>   // Edit.c

#define PARTIAL_SUCCESS(hr)   (((HRESULT)(hr)) == S_OK || (HRESULT)(hr) == STRSAFE_E_INSUFFICIENT_BUFFER)

// (Re-)Implement lstrcpynW() & lstrcpynA() as wrappers around more safer StringCchCopy*()
#define lstrcpynW(dst, src, max_buffer_size) \
    (PARTIAL_SUCCESS(StringCchCopyW(dst, max_buffer_size, src)) ? dst : NULL)
#define lstrcpynA(dst, src, max_buffer_size) \
    (PARTIAL_SUCCESS(StringCchCopyA(dst, max_buffer_size, src)) ? dst : NULL)

// (Re-)Implement lstrcatnW() & lstrcatnA() as wrappers around more safer StringCchCat*()
#define lstrcatnW(dst, src, max_buffer_size) \
    (PARTIAL_SUCCESS(StringCchCatW(dst, max_buffer_size, src)) ? dst : NULL)
#define lstrcatnA(dst, src, max_buffer_size) \
    (PARTIAL_SUCCESS(StringCchCatA(dst, max_buffer_size, src)) ? dst : NULL)

#define lstrncatnW(dst, to_append_size, src, max_buffer_size) \
    (PARTIAL_SUCCESS(StringCchCatNW(dst, max_buffer_size, src, to_append_size)) ? dst : NULL)
#define lstrncatnA(dst, to_append_size, src, max_buffer_size) \
    (PARTIAL_SUCCESS(StringCchCatNA(dst, max_buffer_size, src, to_append_size)) ? dst : NULL)

// Implement _strdec() & _strninc() when _UNICODE is used (normally in <tchar.h>)
#ifdef _UNICODE
_Check_return_ __inline char * __CRTDECL _strdec(_In_reads_z_(_Cpc2 - _Cpc1) const char * _Cpc1, _In_z_ const char * _Cpc2) { return (char *)((_Cpc1)>=(_Cpc2) ? NULL : (_Cpc2-1)); }
///_Check_return_ __inline char * __CRTDECL _strinc(_In_z_ const char * _Pc) { return (char *)(_Pc+1); }
///_Check_return_ __inline unsigned int __CRTDECL _strnextc(_In_z_ const char * _Cpc) { return (unsigned int)*(const unsigned char *)_Cpc; }
_Check_return_ __inline char * __CRTDECL _strninc(_In_reads_or_z_(_Sz) const char * _Pc, _In_ size_t _Sz) { return (char *)(_Pc+_Sz); }
#endif // _UNICODE


