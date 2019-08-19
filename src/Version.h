/******************************************************************************
*
* Notepad2-mod
*
* Version.h
*
* See License.txt for details about distribution and modification.
*
*                                       (c) XhmikosR 2010-2017
*                                       https://github.com/XhmikosR/notepad2-mod
*
*
******************************************************************************/


#ifndef NOTEPAD2_VERSION_H
#define NOTEPAD2_VERSION_H

#ifndef _T
#if !defined(ISPP_INVOKED) && (defined(UNICODE) || defined(_UNICODE))
#define _T(text) L##text
#else
#define _T(text) text
#endif
#endif // _T

#include "VersionRev.h"

#define DO_STRINGIFY(x) _T(#x)
#define STRINGIFY(x)    DO_STRINGIFY(x)

#define VERSION_MAJOR   4
#define VERSION_MINOR   2
#define VERSION_BUILD   25

#ifdef JRB_BUILD
    #define MY_APPNAME                   L"Notepad2-mod-jrb"
#else
    #define MY_APPNAME                   L"Notepad2-mod"
#endif

#define VERSION_FILEVERSION_NUM      VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD,VERSION_REV
#define VERSION_FILEVERSION          STRINGIFY(VERSION_MAJOR) L"." STRINGIFY(VERSION_MINOR) L"." \
                                     STRINGIFY(VERSION_BUILD) L"." STRINGIFY(VERSION_REV)
#define VERSION_LEGALCOPYRIGHT_SHORT L"Copyright © 2004-2017"
#define VERSION_LEGALCOPYRIGHT_LONG  L"© Florian Balmer 2004-2017"
#define VERSION_AUTHORNAME           L"Florian Balmer"
#define VERSION_WEBPAGEDISPLAY       L"flo's freeware - http://www.flos-freeware.ch"
#define VERSION_EMAILDISPLAY         L"florian.balmer@gmail.com"
#define VERSION_COMPANYNAME          L"Florian Balmer et al."
#define VERSION_MODPAGEDISPLAY       L"https://xhmikosr.github.io/notepad2-mod/"

#if defined(_WIN64)
    #define VERSION_FILEVERSION_LONG  MY_APPNAME L" (64-bit) " STRINGIFY(VERSION_MAJOR) L"." \
                                      STRINGIFY(VERSION_MINOR) L"." STRINGIFY(VERSION_BUILD) \
                                      L" r" STRINGIFY(VERSION_REV) L" (" VERSION_HASH L")"
#else
    #define VERSION_FILEVERSION_LONG  MY_APPNAME L" (32-bit) " STRINGIFY(VERSION_MAJOR) L"." \
                                      STRINGIFY(VERSION_MINOR) L"." STRINGIFY(VERSION_BUILD) \
                                      L" r" STRINGIFY(VERSION_REV) L" (" VERSION_HASH L")"
#endif

// Compiler specific
#if defined(_MSC_VER)
    // 2019-05-01: Updated build numbers using https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B#Internal_version_numbering
    #if (_MSC_VER > 1916 && _MSC_VER <= 1929)
        #define VERSION_COMPILER    L"MSVC 2019"
    #elif (_MSC_VER >= 1910 && _MSC_VER <= 1916)
        #define VERSION_COMPILER    L"MSVC 2017"
    #elif (_MSC_VER == 1900)
        #define VERSION_COMPILER    L"MSVC 2015"
    #elif (_MSC_VER == 1800)
        #define VERSION_COMPILER    L"MSVC 2013"
    #elif (_MSC_VER == 1700)
        #define VERSION_COMPILER    L"MSVC 2012"
    #elif (_MSC_VER == 1600)
        #define VERSION_COMPILER    L"MSVC 2010"
    #else
        #define VERSION_COMPILER    L"MSVC (version unknown)"
    #endif
#else
    #define VERSION_COMPILER        L"(Unknown compiler)"
#endif

#endif // NOTEPAD2_VERSION_H
