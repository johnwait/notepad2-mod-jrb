/******************************************************************************
*
*
* Notepad2
*
* Helpers.c
*   General helper functions
*   Parts taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
*   MinimizeToTray (c) 2000 Matthew Ellis
*
* See Readme.txt for more information about this source code.
* Please send me your comments to this work.
*
* See License.txt for details about distribution and modification.
*
*                                              (c) Florian Balmer 1996-2011
*                                                  florian.balmer@gmail.com
*                                               http://www.flos-freeware.ch
*
*
******************************************************************************/

// clang-format off
#include "StdAfx.h" // platform includes
#include "Scintilla.h"
#include "Helpers.h"
#include "resource.h"
// clang-format on

//=============================================================================
//
//  Manipulation of (cached) ini file sections
//
int IniSectionGetStringW(
    LPCWSTR lpwCachedIniSection,
    LPCWSTR lpwName,
    LPCWSTR lpwDefault,
    LPWSTR lpwReturnedString,
    int cchReturnedString)
{
    WCHAR* p = (WCHAR*)lpwCachedIniSection;
    WCHAR wchIniLine[MRUINI_LINE_MAXLEN];
    int iChCount;

    if (p) {
        lstrcpyW(wchIniLine, lpwName);
        lstrcatW(wchIniLine, L"=");
        iChCount = lstrlenW(wchIniLine);

        while (*p) {
            if (StrCmpNIW(p, wchIniLine, iChCount) == 0) {
                lstrcpynW(lpwReturnedString, p + iChCount, cchReturnedString);
                return (lstrlenW(lpwReturnedString));
            } else
                p = StrEndW(p) + 1;
        }
    }
    lstrcpynW(lpwReturnedString, lpwDefault, cchReturnedString);
    return (lstrlenW(lpwReturnedString));
}

int IniSectionGetIntW(
    LPCWSTR lpwCachedIniSection,
    LPCWSTR lpwName,
    int iDefault)
{
    WCHAR* p = (WCHAR*)lpwCachedIniSection;
    WCHAR wch[MRUINI_LINE_MAXLEN];
    int ich;
    int i;

    if (p) {
        lstrcpyW(wch, lpwName);
        lstrcatW(wch, L"=");
        ich = lstrlenW(wch);

        while (*p) {
            if (StrCmpNIW(p, wch, ich) == 0) {
                if (swscanf(p + ich, L"%i", &i) == 1)
                    return (i);
                else
                    return (iDefault);
            } else
                p = StrEndW(p) + 1;
        }
    }
    return (iDefault);
}

BOOL IniSectionSetStringW(LPWSTR lpwCachedIniSection, LPCWSTR lpwName, LPCWSTR lpwString)
{
    WCHAR wch[MRUINI_LINE_MAXLEN];
    WCHAR* p = lpwCachedIniSection;

    if (p) {
        while (*p) {
            p = StrEndW(p) + 1;
        }
        wsprintfW(wch, L"%s=%s", lpwName, lpwString);
        lstrcpyW(p, wch);
        p = StrEndW(p) + 1;
        *p = 0;
        return (TRUE);
    }
    return (FALSE);
}

//=============================================================================
//
//  PrivateIsAppThemed()
//
extern HMODULE hModUxTheme;
BOOL PrivateIsAppThemed()
{
    FARPROC pfnIsAppThemed;
    BOOL bIsAppThemed = FALSE;

    if (hModUxTheme) {
        pfnIsAppThemed = GetProcAddress(hModUxTheme, "IsAppThemed");

        if (pfnIsAppThemed)
            bIsAppThemed = (BOOL)pfnIsAppThemed();
    }
    return bIsAppThemed;
}

//=============================================================================
//
//  PrivateSetCurrentProcessExplicitAppUserModelIDW()
//
HRESULT PrivateSetCurrentProcessExplicitAppUserModelIDW(PCWSTR AppID)
{
    FARPROC pfnSetCurrentProcessExplicitAppUserModelID;

    if (lstrlenW(AppID) == 0)
        return (S_OK);

    if (lstrcmpiW(AppID, L"(default)") == 0)
        return (S_OK);

    pfnSetCurrentProcessExplicitAppUserModelID = GetProcAddress(GetModuleHandleA("shell32.dll"), "SetCurrentProcessExplicitAppUserModelID");

    if (pfnSetCurrentProcessExplicitAppUserModelID)
        return ((HRESULT)pfnSetCurrentProcessExplicitAppUserModelID(AppID));

    else
        return (S_OK);
}

//=============================================================================
//
//  IsElevated()
//
BOOL IsElevated()
{

    BOOL bIsElevated = FALSE;
    HANDLE hToken = NULL;

    if (!IsVista())
        return (FALSE);

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {

        struct {
            DWORD TokenIsElevated;
        } /*TOKEN_ELEVATION*/ te;
        DWORD dwReturnLength = 0;

        if (GetTokenInformation(hToken, /*TokenElevation*/ 20, &te, sizeof(te), &dwReturnLength)) {
            if (dwReturnLength == sizeof(te))
                bIsElevated = te.TokenIsElevated;
        }
        CloseHandle(hToken);
    }
    return bIsElevated;
}

//=============================================================================
//
//  SetExplorerTheme()
//
//BOOL SetExplorerTheme(HWND hwnd)
//{
//  FARPROC pfnSetWindowTheme;
//
//  if (IsVista()) {
//    if (hModUxTheme) {
//      pfnSetWindowTheme = GetProcAddress(hModUxTheme,"SetWindowTheme");
//
//      if (pfnSetWindowTheme)
//        return (S_OK == pfnSetWindowTheme(hwnd,L"Explorer",NULL));
//    }
//  }
//  return FALSE;
//}

//=============================================================================
//
//  BitmapMergeAlpha()
//  Merge alpha channel into color channel
//
BOOL BitmapMergeAlpha(HBITMAP hbmp, COLORREF crDest)
{
    BITMAP bmp;
    if (GetObject(hbmp, sizeof(BITMAP), &bmp)) {

        if (bmp.bmBitsPixel == 32) {

            int x, y;
            RGBQUAD* prgba = bmp.bmBits;

            for (y = 0; y < bmp.bmHeight; y++) {
                for (x = 0; x < bmp.bmWidth; x++) {
                    BYTE alpha = prgba[x].rgbReserved;
                    prgba[x].rgbRed = ((prgba[x].rgbRed * alpha) + (GetRValue(crDest) * (255 - alpha))) >> 8;
                    prgba[x].rgbGreen = ((prgba[x].rgbGreen * alpha) + (GetGValue(crDest) * (255 - alpha))) >> 8;
                    prgba[x].rgbBlue = ((prgba[x].rgbBlue * alpha) + (GetBValue(crDest) * (255 - alpha))) >> 8;
                    prgba[x].rgbReserved = 0xFF;
                }
                prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
            }
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================
//
//  BitmapAlphaBlend()
//  Perform alpha blending to color channel only
//
BOOL BitmapAlphaBlend(HBITMAP hbmp, COLORREF crDest, BYTE alpha)
{
    BITMAP bmp;
    if (GetObject(hbmp, sizeof(BITMAP), &bmp)) {

        if (bmp.bmBitsPixel == 32) {

            int x, y;
            RGBQUAD* prgba = bmp.bmBits;

            for (y = 0; y < bmp.bmHeight; y++) {
                for (x = 0; x < bmp.bmWidth; x++) {
                    prgba[x].rgbRed = ((prgba[x].rgbRed * alpha) + (GetRValue(crDest) * (255 - alpha))) >> 8;
                    prgba[x].rgbGreen = ((prgba[x].rgbGreen * alpha) + (GetGValue(crDest) * (255 - alpha))) >> 8;
                    prgba[x].rgbBlue = ((prgba[x].rgbBlue * alpha) + (GetBValue(crDest) * (255 - alpha))) >> 8;
                }
                prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
            }
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================
//
//  BitmapGrayScale()
//  Gray scale color channel only
//
BOOL BitmapGrayScale(HBITMAP hbmp)
{
    BITMAP bmp;
    if (GetObject(hbmp, sizeof(BITMAP), &bmp)) {

        if (bmp.bmBitsPixel == 32) {

            int x, y;
            RGBQUAD* prgba = bmp.bmBits;

            for (y = 0; y < bmp.bmHeight; y++) {
                for (x = 0; x < bmp.bmWidth; x++) {
                    prgba[x].rgbRed = prgba[x].rgbGreen = prgba[x].rgbBlue = (((BYTE)((prgba[x].rgbRed * 38 + prgba[x].rgbGreen * 75 + prgba[x].rgbBlue * 15) >> 7) * 0x80) + (0xD0 * (255 - 0x80))) >> 8;
                }
                prgba = (RGBQUAD*)((LPBYTE)prgba + bmp.bmWidthBytes);
            }
            return TRUE;
        }
    }
    return FALSE;
}

//=============================================================================
//
//  VerifyContrast()
//  Check if two colors can be distinguished
//
BOOL VerifyContrast(COLORREF cr1, COLORREF cr2)
{
    BYTE r1 = GetRValue(cr1);
    BYTE g1 = GetGValue(cr1);
    BYTE b1 = GetBValue(cr1);
    BYTE r2 = GetRValue(cr2);
    BYTE g2 = GetGValue(cr2);
    BYTE b2 = GetBValue(cr2);

    return (
        ((abs((3 * r1 + 5 * g1 + 1 * b1) - (3 * r2 + 6 * g2 + 1 * b2))) >= 400) || ((abs(r1 - r2) + abs(b1 - b2) + abs(g1 - g2)) >= 400));
}

//=============================================================================
//
//  IsFontAvailable()
//  Test if a certain font is installed on the system
//
int CALLBACK EnumFontsProcW(CONST LOGFONT* plf, CONST TEXTMETRIC* ptm, DWORD FontType, LPARAM lParam)
{
    *((PBOOL)lParam) = TRUE;
    return (FALSE);
}

BOOL IsFontAvailableW(LPCWSTR lpszFontName)
{
    BOOL fFound = FALSE;

    HDC hDC = GetDC(NULL);
    EnumFontsW(hDC, lpszFontName, EnumFontsProcW, (LPARAM)&fFound);
    ReleaseDC(NULL, hDC);

    return (fFound);
}

//=============================================================================
//
//  SetWindowTitle()
//
BOOL bFreezeAppTitle = FALSE;

BOOL SetWindowTitle(HWND hwnd, UINT uIDAppName, BOOL bIsElevated, UINT uIDUntitled,
    LPCTSTR lptszFile, int iFormat, BOOL bModified,
    UINT uIDReadOnly, BOOL bReadOnly, LPCTSTR lptszExcerpt)
{

    TCHAR tszUntitled[128];
	TCHAR tszExcrptQuot[256];
	TCHAR tszExcrptFmt[32];
	TCHAR tszAppName[128];
	TCHAR tszElevatedAppName[128];
	TCHAR tszReadOnly[32];
	TCHAR tszTitle[512];
    static TCHAR tszCachedFile[MAX_PATH];
    static TCHAR tszCachedDisplayName[MAX_PATH];
    static const TCHAR* ptszSep = _T(" - ");
    static const TCHAR* ptszMod = _T("* ");

    if (bFreezeAppTitle)
        return FALSE;

    if (!GetString_(uIDAppName, tszAppName, COUNTOF(tszAppName)) || !GetString_(uIDUntitled, tszUntitled, COUNTOF(tszUntitled)))
        return FALSE;

    if (bIsElevated) {
        FormatString(tszElevatedAppName, COUNTOF(tszElevatedAppName), IDS_APPTITLE_ELEVATED, tszAppName);
        StrCpyN(tszAppName, tszElevatedAppName, COUNTOF(tszAppName));
    }

    if (bModified)
        lstrcpy(tszTitle, ptszMod);
    else
        lstrcpy(tszTitle, _T(""));

    if (lstrlen(lptszExcerpt)) {
        GetString_(IDS_TITLEEXCERPT, tszExcrptFmt, COUNTOF(tszExcrptFmt));
        wsprintf(tszExcrptQuot, tszExcrptFmt, lptszExcerpt);
        StrCat(tszTitle, tszExcrptQuot);
    }

    else if (lstrlen(lptszFile)) {
        if (iFormat < 2 && !PathIsRoot(lptszFile)) {
            if (lstrcmp(tszCachedFile, lptszFile) != 0) {
                SHFILEINFO shfi;
                lstrcpy(tszCachedFile, lptszFile);
                if (SHGetFileInfo2(lptszFile, 0, &shfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
                    lstrcpy(tszCachedDisplayName, shfi.szDisplayName);
                else
                    lstrcpy(tszCachedDisplayName, PathFindFileName(lptszFile));
            }
            lstrcat(tszTitle, tszCachedDisplayName);
            if (iFormat == 1) {
                TCHAR tchPath[MAX_PATH];
                StrCpyN(tchPath, lptszFile, COUNTOF(tchPath));
                PathRemoveFileSpec(tchPath);
                StrCat(tszTitle, _T(" ["));
                StrCat(tszTitle, tchPath);
                StrCat(tszTitle, _T("]"));
            }
        } else
            lstrcat(tszTitle, lptszFile);
    }

    else {
        lstrcpy(tszCachedFile, _T(""));
        lstrcpy(tszCachedDisplayName, _T(""));
        lstrcat(tszTitle, tszUntitled);
    }

    if (bReadOnly && GetString_(uIDReadOnly, tszReadOnly, COUNTOF(tszReadOnly))) {
        lstrcat(tszTitle, _T(" "));
        lstrcat(tszTitle, tszReadOnly);
    }

    lstrcat(tszTitle, ptszSep);
    lstrcat(tszTitle, tszAppName);

    return SetWindowText(hwnd, tszTitle);
}

//=============================================================================
//
//  SetWindowTransparentMode()
//
void SetWindowTransparentMode(HWND hwnd, BOOL bTransparentMode)
{
    FARPROC fp;
    int iAlphaPercent;
    BYTE bAlpha;

    if (bTransparentMode) {
        if (fp = GetProcAddress(GetModuleHandle(L"User32"), "SetLayeredWindowAttributes")) {
            SetWindowLongPtr(hwnd, GWL_EXSTYLE,
                GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);

            // get opacity level from registry
            iAlphaPercent = IniGetIntW(L"Settings2", L"OpacityLevel", 75);
            if (iAlphaPercent < 0 || iAlphaPercent > 100)
                iAlphaPercent = 75;
            bAlpha = iAlphaPercent * 255 / 100;

            fp(hwnd, 0, bAlpha, LWA_ALPHA);
        }
    }

    else
        SetWindowLongPtr(hwnd, GWL_EXSTYLE,
            GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
}

//=============================================================================
//
//  CenterDlgInParent()
//
void CenterDlgInParent(HWND hDlg)
{

    RECT rcDlg;
    HWND hParent;
    RECT rcParent;
    MONITORINFO mi;
    HMONITOR hMonitor;

    int xMin, yMin, xMax, yMax, x, y;

    GetWindowRect(hDlg, &rcDlg);

    hParent = GetParent(hDlg);
    GetWindowRect(hParent, &rcParent);

    hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

    xMin = mi.rcWork.left;
    yMin = mi.rcWork.top;

    xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
    yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

    if ((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left) > 20)
        x = rcParent.left + (((rcParent.right - rcParent.left) - (rcDlg.right - rcDlg.left)) / 2);
    else
        x = rcParent.left + 70;

    if ((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top) > 20)
        y = rcParent.top + (((rcParent.bottom - rcParent.top) - (rcDlg.bottom - rcDlg.top)) / 2);
    else
        y = rcParent.top + 60;

    SetWindowPos(hDlg, NULL, max(xMin, min(xMax, x)), max(yMin, min(yMax, y)), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

//=============================================================================
//
//  GetDlgPos()
//
void GetDlgPos(HWND hDlg, LPINT xDlg, LPINT yDlg)
{

    RECT rcDlg;
    HWND hParent;
    RECT rcParent;

    GetWindowRect(hDlg, &rcDlg);

    hParent = GetParent(hDlg);
    GetWindowRect(hParent, &rcParent);

    // return positions relative to parent window
    *xDlg = rcDlg.left - rcParent.left;
    *yDlg = rcDlg.top - rcParent.top;
}

//=============================================================================
//
//  SetDlgPos()
//
void SetDlgPos(HWND hDlg, int xDlg, int yDlg)
{

    RECT rcDlg;
    HWND hParent;
    RECT rcParent;
    MONITORINFO mi;
    HMONITOR hMonitor;

    int xMin, yMin, xMax, yMax, x, y;

    GetWindowRect(hDlg, &rcDlg);

    hParent = GetParent(hDlg);
    GetWindowRect(hParent, &rcParent);

    hMonitor = MonitorFromRect(&rcParent, MONITOR_DEFAULTTONEAREST);
    mi.cbSize = sizeof(mi);
    GetMonitorInfo(hMonitor, &mi);

    xMin = mi.rcWork.left;
    yMin = mi.rcWork.top;

    xMax = (mi.rcWork.right) - (rcDlg.right - rcDlg.left);
    yMax = (mi.rcWork.bottom) - (rcDlg.bottom - rcDlg.top);

    // desired positions relative to parent window
    x = rcParent.left + xDlg;
    y = rcParent.top + yDlg;

    SetWindowPos(hDlg, NULL, max(xMin, min(xMax, x)), max(yMin, min(yMax, y)), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
}

//=============================================================================
//
//  Resize Dialog Helpers()
//
#define WNDPROP_RESIZE_DLG _T("ResizeDlg")

typedef struct _resizedlg {
    int cxClient;
    int cyClient;
    int cxFrame;
    int cyFrame;
    int mmiPtMinX;
    int mmiPtMinY;
} RESIZEDLG, *PRESIZEDLG;

void ResizeDlg_Init(HWND hwnd, int cxFrame, int cyFrame, int nIdGrip)
{
    RECT rc;
    WCHAR wch[64];
    int cGrip;
    RESIZEDLG* pm = LocalAlloc(LPTR, sizeof(RESIZEDLG));

    GetClientRect(hwnd, &rc);
    pm->cxClient = rc.right - rc.left;
    pm->cyClient = rc.bottom - rc.top;

    pm->cxFrame = cxFrame;
    pm->cyFrame = cyFrame;

    AdjustWindowRectEx(&rc, GetWindowLong(hwnd, GWL_STYLE) | WS_THICKFRAME, FALSE, 0);
    pm->mmiPtMinX = rc.right - rc.left;
    pm->mmiPtMinY = rc.bottom - rc.top;

    if (pm->cxFrame < (rc.right - rc.left))
        pm->cxFrame = rc.right - rc.left;
    if (pm->cyFrame < (rc.bottom - rc.top))
        pm->cyFrame = rc.bottom - rc.top;

    SetProp(hwnd, WNDPROP_RESIZE_DLG, (HANDLE)pm);

    SetWindowPos(hwnd, NULL, rc.left, rc.top, pm->cxFrame, pm->cyFrame, SWP_NOZORDER);

    SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) | WS_THICKFRAME);
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);
    GetMenuString(GetSystemMenu(GetParent(hwnd), FALSE), SC_SIZE, wch, COUNTOF(wch), MF_BYCOMMAND);
    InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_STRING | MF_ENABLED, SC_SIZE, wch);
    InsertMenu(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_SEPARATOR, 0, NULL);

    SetWindowLongPtr(GetDlgItem(hwnd, nIdGrip), GWL_STYLE,
        GetWindowLongPtr(GetDlgItem(hwnd, nIdGrip), GWL_STYLE) | SBS_SIZEGRIP | WS_CLIPSIBLINGS);
    cGrip = GetSystemMetrics(SM_CXHTHUMB);
    SetWindowPos(GetDlgItem(hwnd, nIdGrip), NULL, pm->cxClient - cGrip, pm->cyClient - cGrip, cGrip, cGrip, SWP_NOZORDER);
}

void ResizeDlg_Destroy(HWND hwnd, int* cxFrame, int* cyFrame)
{
    RECT rc;
    PRESIZEDLG pm = GetProp(hwnd, WNDPROP_RESIZE_DLG);

    GetWindowRect(hwnd, &rc);
    *cxFrame = rc.right - rc.left;
    *cyFrame = rc.bottom - rc.top;

    RemoveProp(hwnd, WNDPROP_RESIZE_DLG);
    LocalFree(pm);
}

void ResizeDlg_Size(HWND hwnd, LPARAM lParam, int* cx, int* cy)
{
    PRESIZEDLG pm = GetProp(hwnd, WNDPROP_RESIZE_DLG);

    *cx = LOWORD(lParam) - pm->cxClient;
    *cy = HIWORD(lParam) - pm->cyClient;
    pm->cxClient = LOWORD(lParam);
    pm->cyClient = HIWORD(lParam);
}

void ResizeDlg_GetMinMaxInfo(HWND hwnd, LPARAM lParam)
{
    PRESIZEDLG pm = GetProp(hwnd, WNDPROP_RESIZE_DLG);

    LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;
    lpmmi->ptMinTrackSize.x = pm->mmiPtMinX;
    lpmmi->ptMinTrackSize.y = pm->mmiPtMinY;
}

HDWP DeferCtlPos(HDWP hdwp, HWND hwndDlg, int nCtlId, int dx, int dy, UINT uFlags)
{
    RECT rc;
    HWND hwndCtl = GetDlgItem(hwndDlg, nCtlId);
    GetWindowRect(hwndCtl, &rc);
    MapWindowPoints(NULL, hwndDlg, (LPPOINT)&rc, 2);
    if (uFlags & SWP_NOSIZE)
        return (DeferWindowPos(hdwp, hwndCtl, NULL, rc.left + dx, rc.top + dy, 0, 0, SWP_NOZORDER | SWP_NOSIZE));
    else
        return (DeferWindowPos(hdwp, hwndCtl, NULL, 0, 0, rc.right - rc.left + dx, rc.bottom - rc.top + dy, SWP_NOZORDER | SWP_NOMOVE));
}

//=============================================================================
//
//  MakeBitmapButton()
//
void MakeBitmapButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, UINT uBmpId)
{
    HWND hwndCtl = GetDlgItem(hwnd, nCtlId);
    BITMAP bmp;
    BUTTON_IMAGELIST bi;
    HBITMAP hBmp = LoadImage(hInstance, MAKEINTRESOURCE(uBmpId), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);
    GetObject(hBmp, sizeof(BITMAP), &bmp);
    bi.himl = ImageList_Create(bmp.bmWidth, bmp.bmHeight, ILC_COLOR32 | ILC_MASK, 1, 0);
    ImageList_AddMasked(bi.himl, hBmp, CLR_DEFAULT);
    DeleteObject(hBmp);
    SetRect(&bi.margin, 0, 0, 0, 0);
    bi.uAlign = BUTTON_IMAGELIST_ALIGN_CENTER;
    SendMessage(hwndCtl, BCM_SETIMAGELIST, 0, (LPARAM)&bi);
}

//=============================================================================
//
//  MakeColorPickButton()
//
void MakeColorPickButton(HWND hwnd, int nCtlId, HINSTANCE hInstance, COLORREF crColor)
{
    HWND hwndCtl = GetDlgItem(hwnd, nCtlId);
    BUTTON_IMAGELIST bi;
    HIMAGELIST himlOld = NULL;
    HBITMAP hBmp;
    COLORMAP colormap[2];

    if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi))
        himlOld = bi.himl;

    if (IsWindowEnabled(hwndCtl) && crColor != -1) {
        colormap[0].from = RGB(0x00, 0x00, 0x00);
        colormap[0].to = GetSysColor(COLOR_3DSHADOW);
    } else {
        colormap[0].from = RGB(0x00, 0x00, 0x00);
        colormap[0].to = RGB(0xFF, 0xFF, 0xFF);
    }

    if (IsWindowEnabled(hwndCtl) && crColor != -1) {
        if (crColor == RGB(0xFF, 0xFF, 0xFF))
            crColor = RGB(0xFF, 0xFF, 0xFE);

        colormap[1].from = RGB(0xFF, 0xFF, 0xFF);
        colormap[1].to = crColor;
    } else {
        colormap[1].from = RGB(0xFF, 0xFF, 0xFF);
        colormap[1].to = RGB(0xFF, 0xFF, 0xFF);
    }

    hBmp = CreateMappedBitmap(hInstance, IDB_PICK, 0, colormap, 2);

    bi.himl = ImageList_Create(10, 10, ILC_COLORDDB | ILC_MASK, 1, 0);
    ImageList_AddMasked(bi.himl, hBmp, RGB(0xFF, 0xFF, 0xFF));
    DeleteObject(hBmp);

    SetRect(&bi.margin, 0, 0, 4, 0);
    bi.uAlign = BUTTON_IMAGELIST_ALIGN_RIGHT;

    SendMessage(hwndCtl, BCM_SETIMAGELIST, 0, (LPARAM)&bi);
    InvalidateRect(hwndCtl, NULL, TRUE);

    if (himlOld)
        ImageList_Destroy(himlOld);
}

//=============================================================================
//
//  DeleteBitmapButton()
//
void DeleteBitmapButton(HWND hwnd, int nCtlId)
{
    HWND hwndCtl = GetDlgItem(hwnd, nCtlId);
    BUTTON_IMAGELIST bi;
    if (SendMessage(hwndCtl, BCM_GETIMAGELIST, 0, (LPARAM)&bi))
        ImageList_Destroy(bi.himl);
}

//=============================================================================
//
//  StatusSetText()
//
BOOL StatusSetText(HWND hwnd, UINT nPart, LPCTSTR lptszText)
{

    UINT uFlags = (nPart == 255) ? nPart | SBT_NOBORDERS : nPart;
    return (BOOL)SendMessage(hwnd, SB_SETTEXT, uFlags, (LPARAM)lptszText);
}

//=============================================================================
//
//  SendWMSize()
//
LRESULT SendWMSize(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);
    return (SendMessage(hwnd, WM_SIZE, SIZE_RESTORED,
        MAKELPARAM(rc.right, rc.bottom)));
}

//=============================================================================
//
//  StatusSetTextID()
//
BOOL StatusSetTextID(HWND hwnd, UINT nPart, UINT uID)
{

    TCHAR tchText[256];
    UINT uFlags = (nPart == 255) ? nPart | SBT_NOBORDERS : nPart;

    if (!uID) {
        SendMessage(hwnd, SB_SETTEXT, uFlags, 0);
        return TRUE;
    }

    if (!GetString(uID, tchText, 256))
        return FALSE;

    return (BOOL)SendMessage(hwnd, SB_SETTEXT, uFlags, (LPARAM)tchText);
}

//=============================================================================
//
//  StatusCalcPaneWidth()
//
int StatusCalcPaneWidth(HWND hwnd, LPCTSTR lptsz)
{
    SIZE size;
    HDC hdc = GetDC(hwnd);
    HFONT hfont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
    HFONT hfold = SelectObject(hdc, hfont);
    int mmode = SetMapMode(hdc, MM_TEXT);

    GetTextExtentPoint32(hdc, lptsz, lstrlen(lptsz), &size);

    SetMapMode(hdc, mmode);
    SelectObject(hdc, hfold);
    ReleaseDC(hwnd, hdc);

    return (size.cx + 9);
}

//=============================================================================
//
//  Toolbar_Get../SetButtons()
//
int Toolbar_GetButtons(HWND hwnd, int cmdBase, LPTSTR lptszButtons, int cchButtons)
{
    TCHAR tchButtons[512];
    TCHAR tchItem[32];
    int i, c;
    TBBUTTON tbb;

    lstrcpy(tchButtons, _T(""));
    c = min(50, (int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));

    for (i = 0; i < c; i++) {
        SendMessage(hwnd, TB_GETBUTTON, (WPARAM)i, (LPARAM)&tbb);
        wsprintf(tchItem, _T("%i "),
            (tbb.idCommand == 0) ? 0 : tbb.idCommand - cmdBase + 1);
        lstrcat(tchButtons, tchItem);
    }
    TrimString(tchButtons);
    lstrcpyn(lptszButtons, tchButtons, cchButtons);
    return (c);
}

int Toolbar_SetButtons(HWND hwnd, int cmdBase, LPCTSTR lptszButtons, LPCTBBUTTON ptbb, int ctbb)
{
    TCHAR tchButtons[512];
    TCHAR* p;
    int i, c;
    int iCmd;

    ZeroMemory(tchButtons, COUNTOF(tchButtons) * sizeof(tchButtons[0]));
    lstrcpyn(tchButtons, lptszButtons, COUNTOF(tchButtons) - 2);
    TrimString(tchButtons);
    while (p = StrStr(tchButtons, _T("  ")))
        MoveMemory((TCHAR*)p, (TCHAR*)p + 1, (lstrlen(p) + 1) * sizeof(TCHAR));

    c = (int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0);
    for (i = 0; i < c; i++)
        SendMessage(hwnd, TB_DELETEBUTTON, 0, 0);

    for (i = 0; i < COUNTOF(tchButtons); i++)
        if (tchButtons[i] == _T(' '))
            tchButtons[i] = 0;

    p = tchButtons;
    while (*p) {
        if (swscanf(p, _T("%i"), &iCmd) == 1) {
            iCmd = (iCmd == 0) ? 0 : iCmd + cmdBase - 1;
            for (i = 0; i < ctbb; i++) {
                if (ptbb[i].idCommand == iCmd) {
                    SendMessage(hwnd, TB_ADDBUTTONS, (WPARAM)1, (LPARAM)&ptbb[i]);
                    break;
                }
            }
        }
        p = StrEnd_(p) + 1;
    }
    return ((int)SendMessage(hwnd, TB_BUTTONCOUNT, 0, 0));
}

//=============================================================================
//
//  IsCmdEnabled()
//
BOOL IsCmdEnabled(HWND hwnd, UINT uId)
{

    HMENU hmenu;
    UINT ustate;

    hmenu = GetMenu(hwnd);

    SendMessage(hwnd, WM_INITMENU, (WPARAM)hmenu, 0);

    ustate = GetMenuState(hmenu, uId, MF_BYCOMMAND);

    if (ustate == 0xFFFFFFFF)
        return TRUE;

    else
        return (!(ustate & (MF_GRAYED | MF_DISABLED)));
}

//=============================================================================
//
//  FormatString()
//
int FormatString(LPTSTR lptOutput, int nOutput, UINT uIdFormat, ...)
{

    TCHAR* p = LocalAlloc(LPTR, sizeof(TCHAR) * nOutput);

    if (GetString(uIdFormat, p, nOutput))
        wvsprintf(lptOutput, p, (LPVOID)((PUINT_PTR)&uIdFormat + 1));

    LocalFree(p);

    return lstrlen(lptOutput);
}

//=============================================================================
//
//  PathRelativeToAppA() & ..W()
//
//  2020-04-21: Converted to a A/W() pair since MRU_*() require methods that
//              always work with WCHARs
//
/*
void PathRelativeToApp(
    LPTSTR lpszSrc, LPTSTR lpszDest, int cchDest, BOOL bSrcIsFile,
    BOOL bUnexpandEnv, BOOL bUnexpandMyDocs)
{

	TCHAR tchAppPath[MAX_PATH];
	TCHAR tchWinDir[MAX_PATH];
	TCHAR tchUserFiles[MAX_PATH];
	TCHAR tchPath[MAX_PATH];
	TCHAR tchResult[MAX_PATH];
    DWORD dwAttrTo = (bSrcIsFile) ? 0 : FILE_ATTRIBUTE_DIRECTORY;

    GetModuleFileName(NULL, tchAppPath, COUNTOF(tchAppPath));
    PathRemoveFileSpec(tchAppPath);
    GetWindowsDirectory(tchWinDir, COUNTOF(tchWinDir));
    SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, tchUserFiles);

    if (
            bUnexpandMyDocs &&
            !PathIsRelative(lpszSrc) &&
            !PathIsPrefix(tchUserFiles, tchAppPath) &&
            PathIsPrefix(tchUserFiles, lpszSrc) &&
            PathRelativePathTo(tchPath, tchUserFiles, FILE_ATTRIBUTE_DIRECTORY, lpszSrc, dwAttrTo)
    ) {
        lstrcpy(tchUserFiles, _T("%CSIDL:MYDOCUMENTS%"));
        PathAppend(tchUserFiles, tchPath);
        lstrcpy(tchPath, tchUserFiles);
    } else if (PathIsRelative(lpszSrc) || PathCommonPrefix(tchAppPath, tchWinDir, NULL))
        lstrcpyn(tchPath, lpszSrc, COUNTOF(tchPath));
    else {
        if (!PathRelativePathTo(tchPath, tchAppPath, FILE_ATTRIBUTE_DIRECTORY, lpszSrc, dwAttrTo))
            lstrcpyn(tchPath, lpszSrc, COUNTOF(tchPath));
    }

    if (bUnexpandEnv) {
        if (!PathUnExpandEnvStrings(tchPath, tchResult, COUNTOF(tchResult)))
            lstrcpyn(tchResult, tchPath, COUNTOF(tchResult));
    } else
        lstrcpyn(tchResult, tchPath, COUNTOF(tchResult));

    if (lpszDest == NULL || lpszSrc == lpszDest)
        lstrcpyn(lpszSrc, tchResult, (cchDest == 0) ? MAX_PATH : cchDest);
    else
        lstrcpyn(lpszDest, tchResult, (cchDest == 0) ? MAX_PATH : cchDest);
}
*/
void PathRelativeToAppA(
    LPSTR lpszSrc, LPSTR lpszDest, int cchDest, BOOL bSrcIsFile,
    BOOL bUnexpandEnv, BOOL bUnexpandMyDocs)
{

    char chAppPath[MAX_PATH];
    char chWinDir[MAX_PATH];
    char chUserFiles[MAX_PATH];
    char chPath[MAX_PATH];
    char chResult[MAX_PATH];
    DWORD dwAttrTo = (bSrcIsFile) ? 0 : FILE_ATTRIBUTE_DIRECTORY;

    GetModuleFileNameA(NULL, chAppPath, COUNTOF(chAppPath));
    PathRemoveFileSpecA(chAppPath);
    GetWindowsDirectoryA(chWinDir, COUNTOF(chWinDir));
    SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, chUserFiles);

    if (
        bUnexpandMyDocs &&
        !PathIsRelativeA(lpszSrc) &&
        !PathIsPrefixA(chUserFiles, chAppPath) &&
        PathIsPrefixA(chUserFiles, lpszSrc) &&
        PathRelativePathToA(chPath, chUserFiles, FILE_ATTRIBUTE_DIRECTORY, lpszSrc, dwAttrTo)
        ) {
        lstrcpyA(chUserFiles, "%CSIDL:MYDOCUMENTS%");
        PathAppendA(chUserFiles, chPath);
        lstrcpyA(chPath, chUserFiles);
    }
    else if (PathIsRelativeA(lpszSrc) || PathCommonPrefixA(chAppPath, chWinDir, NULL))
        lstrcpynA(chPath, lpszSrc, COUNTOF(chPath));
    else {
        if (!PathRelativePathToA(chPath, chAppPath, FILE_ATTRIBUTE_DIRECTORY, lpszSrc, dwAttrTo))
            lstrcpynA(chPath, lpszSrc, COUNTOF(chPath));
    }

    if (bUnexpandEnv) {
        if (!PathUnExpandEnvStringsA(chPath, chResult, COUNTOF(chResult)))
            lstrcpynA(chResult, chPath, COUNTOF(chResult));
    }
    else
        lstrcpynA(chResult, chPath, COUNTOF(chResult));

    if (lpszDest == NULL || lpszSrc == lpszDest)
        lstrcpynA(lpszSrc, chResult, (cchDest == 0) ? MAX_PATH : cchDest);
    else
        lstrcpynA(lpszDest, chResult, (cchDest == 0) ? MAX_PATH : cchDest);
}

void PathRelativeToAppW(
    LPWSTR lpwszSrc, LPWSTR lpwszDest, int cchDest, BOOL bSrcIsFile,
    BOOL bUnexpandEnv, BOOL bUnexpandMyDocs)
{

    WCHAR wchAppPath[MAX_PATH];
    WCHAR wchWinDir[MAX_PATH];
    WCHAR wchUserFiles[MAX_PATH];
    WCHAR wchPath[MAX_PATH];
    WCHAR wchResult[MAX_PATH];
    DWORD dwAttrTo = (bSrcIsFile) ? 0 : FILE_ATTRIBUTE_DIRECTORY;

    GetModuleFileNameW(NULL, wchAppPath, COUNTOF(wchAppPath));
    PathRemoveFileSpecW(wchAppPath);
    GetWindowsDirectoryW(wchWinDir, COUNTOF(wchWinDir));
    SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, wchUserFiles);

    if (
        bUnexpandMyDocs &&
        !PathIsRelativeW(lpwszSrc) &&
        !PathIsPrefixW(wchUserFiles, wchAppPath) &&
        PathIsPrefixW(wchUserFiles, lpwszSrc) &&
        PathRelativePathToW(wchPath, wchUserFiles, FILE_ATTRIBUTE_DIRECTORY, lpwszSrc, dwAttrTo)
        ) {
        lstrcpyW(wchUserFiles, L"%CSIDL:MYDOCUMENTS%");
        PathAppendW(wchUserFiles, wchPath);
        lstrcpyW(wchPath, wchUserFiles);
    }
    else if (PathIsRelativeW(lpwszSrc) || PathCommonPrefixW(wchAppPath, wchWinDir, NULL))
        lstrcpynW(wchPath, lpwszSrc, COUNTOF(wchPath));
    else {
        if (!PathRelativePathToW(wchPath, wchAppPath, FILE_ATTRIBUTE_DIRECTORY, lpwszSrc, dwAttrTo))
            lstrcpynW(wchPath, lpwszSrc, COUNTOF(wchPath));
    }

    if (bUnexpandEnv) {
        if (!PathUnExpandEnvStringsW(wchPath, wchResult, COUNTOF(wchResult)))
            lstrcpynW(wchResult, wchPath, COUNTOF(wchResult));
    }
    else
        lstrcpynW(wchResult, wchPath, COUNTOF(wchResult));

    if (lpwszDest == NULL || lpwszSrc == lpwszDest)
        lstrcpynW(lpwszSrc, wchResult, (cchDest == 0) ? MAX_PATH : cchDest);
    else
        lstrcpynW(lpwszDest, wchResult, (cchDest == 0) ? MAX_PATH : cchDest);
}

//=============================================================================
//
//  PathAbsoluteFromAppA() & ..W()
//
//  2020-04-21: Converted to a A/W() pair since MRU_*() require methods that
//              always work with WCHARs
//
void PathAbsoluteFromAppA(LPSTR lpszSrc, LPSTR lpszDest, int cchDest, BOOL bExpandEnv)
{

    char chPath[MAX_PATH];
    char chResult[MAX_PATH];

    if (StrCmpNIA(lpszSrc, "%CSIDL:MYDOCUMENTS%", CSTRLEN("%CSIDL:MYDOCUMENTS%")) == 0) {
        SHGetFolderPathA(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, chPath);
        PathAppendA(chPath, lpszSrc + CSTRLEN("%CSIDL:MYDOCUMENTS%"));
    }
    else
        lstrcpynA(chPath, lpszSrc, COUNTOF(chPath));

    if (bExpandEnv)
        ExpandEnvironmentStringsExA(chPath, COUNTOF(chPath));

    if (PathIsRelativeA(chPath)) {
        GetModuleFileNameA(NULL, chResult, COUNTOF(chResult));
        PathRemoveFileSpecA(chResult);
        PathAppendA(chResult, chPath);
    }
    else
        lstrcpynA(chResult, chPath, COUNTOF(chResult));

    PathCanonicalizeExA(chResult);
    if (PathGetDriveNumberA(chResult) != -1)
        CharUpperBuffA(chResult, 1);

    if (lpszDest == NULL || lpszSrc == lpszDest)
        lstrcpynA(lpszSrc, chResult, (cchDest == 0) ? MAX_PATH : cchDest);
    else
        lstrcpynA(lpszDest, chResult, (cchDest == 0) ? MAX_PATH : cchDest);
}

void PathAbsoluteFromAppW(LPWSTR lpwszSrc, LPWSTR lpwszDest, int cchDest, BOOL bExpandEnv)
{

    WCHAR wchPath[MAX_PATH];
    WCHAR wchResult[MAX_PATH];

    if (StrCmpNIW(lpwszSrc, L"%CSIDL:MYDOCUMENTS%", CSTRLEN(L"%CSIDL:MYDOCUMENTS%")) == 0) {
        SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, wchPath);
        PathAppendW(wchPath, lpwszSrc + CSTRLEN(L"%CSIDL:MYDOCUMENTS%"));
    } else
        lstrcpynW(wchPath, lpwszSrc, COUNTOF(wchPath));

    if (bExpandEnv)
        ExpandEnvironmentStringsExW(wchPath, COUNTOF(wchPath));

    if (PathIsRelativeW(wchPath)) {
        GetModuleFileNameW(NULL, wchResult, COUNTOF(wchResult));
        PathRemoveFileSpecW(wchResult);
        PathAppendW(wchResult, wchPath);
    } else
        lstrcpynW(wchResult, wchPath, COUNTOF(wchResult));

    PathCanonicalizeExW(wchResult);
    if (PathGetDriveNumberW(wchResult) != -1)
        CharUpperBuffW(wchResult, 1);

    if (lpwszDest == NULL || lpwszSrc == lpwszDest)
        lstrcpynW(lpwszSrc, wchResult, (cchDest == 0) ? MAX_PATH : cchDest);
    else
        lstrcpynW(lpwszDest, wchResult, (cchDest == 0) ? MAX_PATH : cchDest);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathIsLnkFile()
//
//  Purpose: Determine wheter pszPath is a Windows Shell Link File by
//           comparing the filename extension with L".lnk"
//
//  Manipulates:
//
BOOL PathIsLnkFile(LPCTSTR ptszPath)
{

    ///TCHAR *pszExt;

    TCHAR tchResPath[256];

    if (!ptszPath || !*ptszPath)
        return FALSE;

 /*
    pszExt = StrRChr(pszPath, NULL, _T('.'));

    if (!pszExt)
        return FALSE;

    if (!lstrcmpi(pszExt, _T(".lnk")))
        return TRUE;

    else
        return FALSE;
*/

    if (lstrcmpi(PathFindExtension(ptszPath), _T(".lnk")))
        return FALSE;

    else
        return PathGetLnkPath(ptszPath, tchResPath, COUNTOF(tchResPath));
}

///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathGetLnkPath()
//
//  Purpose: Try to get the path to which a lnk-file is linked
//
//
//  Manipulates: pszResPath
//
BOOL PathGetLnkPath(LPCTSTR ptszLnkFile, LPTSTR ptszResPath, int cchResPath)
{

    IShellLink* psl;
    WIN32_FIND_DATA fd;
    BOOL bSucceeded = FALSE;

    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IShellLink, &psl))) {
        IPersistFile* ppf;

        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf))) {
            TCHAR tsz[MAX_PATH];

            /* MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED, pszLnkFile,-1,wsz,MAX_PATH);*/
            lstrcpy(tsz, ptszLnkFile);

            if (SUCCEEDED(ppf->lpVtbl->Load(ppf, tsz, STGM_READ))) {
                if (NOERROR == psl->lpVtbl->GetPath(psl, ptszResPath, cchResPath, &fd, 0))
                    bSucceeded = TRUE;
            }
            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    // This additional check seems reasonable
    if (!lstrlen(ptszResPath))
        bSucceeded = FALSE;

    if (bSucceeded) {
        ExpandEnvironmentStringsEx(ptszResPath, cchResPath);
        PathCanonicalizeEx(ptszResPath);
    }

    return (bSucceeded);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathIsLnkToDirectory()
//
//  Purpose: Determine wheter pszPath is a Windows Shell Link File which
//           refers to a directory
//
//  Manipulates: pszResPath
//
BOOL PathIsLnkToDirectory(LPCTSTR ptszPath, LPTSTR ptszResPath, int cchResPath)
{

    TCHAR tchResPath[MAX_PATH];

    if (PathIsLnkFile(ptszPath)) {

        if (PathGetLnkPath(ptszPath, tchResPath, sizeof(WCHAR) * COUNTOF(tchResPath))) {

            if (PathIsDirectory(tchResPath)) {

                lstrcpyn(ptszResPath, tchResPath, cchResPath);
                return (TRUE);
            }

            else
                return FALSE;
        }

        else
            return FALSE;
    }

    else
        return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathCreateDeskLnk()
//
//  Purpose: Modified to create a desktop link to Notepad2
//
//  Manipulates:
//
BOOL PathCreateDeskLnk(LPCTSTR ptszDocument)
{

    TCHAR tchExeFile[MAX_PATH];
    TCHAR tchDocTemp[MAX_PATH];
    TCHAR tchArguments[MAX_PATH + 16];
    TCHAR tchLinkDir[MAX_PATH];
    TCHAR tchDescription[64];

    TCHAR tchLnkFileName[MAX_PATH];

    IShellLink* psl;
    BOOL bSucceeded = FALSE;
    BOOL fMustCopy;

    if (!ptszDocument || lstrlen(ptszDocument) == 0)
        return TRUE;

    // init strings
    GetModuleFileName(NULL, tchExeFile, COUNTOF(tchExeFile));

    lstrcpy(tchDocTemp, ptszDocument);
    PathQuoteSpaces(tchDocTemp);

    lstrcpy(tchArguments, _T("-n "));
    lstrcat(tchArguments, tchDocTemp);

    SHGetSpecialFolderPath(NULL, tchLinkDir, CSIDL_DESKTOPDIRECTORY, TRUE);

    GetString(IDS_LINKDESCRIPTION, tchDescription, COUNTOF(tchDescription));

    // Try to construct a valid filename...
    if (!SHGetNewLinkInfo(ptszDocument, tchLinkDir, tchLnkFileName, &fMustCopy, SHGNLI_PREFIXNAME))
        return (FALSE);

    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IShellLink, &psl))) {
        IPersistFile* ppf;

        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf))) {
            TCHAR tsz[MAX_PATH];

            /* MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED, tchLnkFileName, -1, wsz, MAX_PATH);*/
            lstrcpy(tsz, tchLnkFileName);

            psl->lpVtbl->SetPath(psl, tchExeFile);
            psl->lpVtbl->SetArguments(psl, tchArguments);
            psl->lpVtbl->SetDescription(psl, tchDescription);

            if (SUCCEEDED(ppf->lpVtbl->Save(ppf, tsz, TRUE)))
                bSucceeded = TRUE;

            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    return (bSucceeded);
}

///////////////////////////////////////////////////////////////////////////////
//
//
//  Name: PathCreateFavLnk()
//
//  Purpose: Modified to create a Notepad2 favorites link
//
//  Manipulates:
//
BOOL PathCreateFavLnk(LPCTSTR ptszName, LPCTSTR ptszTarget, LPCTSTR ptszDir)
{

    TCHAR tchLnkFileName[MAX_PATH];

    IShellLink* psl;
    BOOL bSucceeded = FALSE;

    if (!ptszName || lstrlen(ptszName) == 0)
        return TRUE;

    lstrcpy(tchLnkFileName, ptszDir);
    PathAppend(tchLnkFileName, ptszName);
    lstrcat(tchLnkFileName, _T(".lnk"));

    if (PathFileExists(tchLnkFileName))
        return FALSE;

    if (SUCCEEDED(CoCreateInstance(&CLSID_ShellLink, NULL,
            CLSCTX_INPROC_SERVER,
            &IID_IShellLink, &psl))) {
        IPersistFile* ppf;

        if (SUCCEEDED(psl->lpVtbl->QueryInterface(psl, &IID_IPersistFile, &ppf))) {
            TCHAR tsz[MAX_PATH];

            /* MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, tchLnkFileName, -1, wsz, MAX_PATH);*/
            lstrcpy(tsz, tchLnkFileName);

            psl->lpVtbl->SetPath(psl, ptszTarget);

            if (SUCCEEDED(ppf->lpVtbl->Save(ppf, tsz, TRUE)))
                bSucceeded = TRUE;

            ppf->lpVtbl->Release(ppf);
        }
        psl->lpVtbl->Release(psl);
    }

    return (bSucceeded);
}

//=============================================================================
//
//  StrLTrim()
//
BOOL StrLTrim(LPTSTR ptszSource, LPCTSTR ptszTrimChars)
{
    LPTSTR ptsz;

    if (!ptszSource || !*ptszSource)
        return FALSE;

    ptsz = ptszSource;
    while (StrChrI(ptszTrimChars, *ptsz))
        ptsz++; // compare on a byte-to-byte basis

    MoveMemory(ptszSource, ptsz, sizeof(TCHAR) * (lstrlen(ptsz) + 1));

    return TRUE;
}

//=============================================================================
//
//  TrimString()
//
BOOL TrimString(LPTSTR lptString)
{

    LPTSTR ptsz;

    if (!lptString || !*lptString)
        return FALSE;

    // Trim left
    ptsz = lptString;

    while (*ptsz == _T(' '))
        ptsz = CharNext(ptsz);

    MoveMemory(lptString, ptsz, sizeof(TCHAR) * (lstrlen(ptsz) + 1));

    // Trim right
    ptsz = StrEnd_(lptString);

    while (*(ptsz = CharPrev(lptString, ptsz)) == _T(' '))
        *ptsz = _T('\0');

    return TRUE;
}

//=============================================================================
//
//  ExtractFirstArgument()
//
BOOL ExtractFirstArgument(LPCTSTR lptArgs, LPTSTR lptArg1, LPTSTR lptArg2)
{

    LPTSTR ptsz;
    BOOL bQuoted = FALSE;

    lstrcpy(lptArg1, lptArgs);
    if (lptArg2)
        *lptArg2 = _T('\0');

    if (!TrimString(lptArg1))
        return FALSE;

    if (*lptArg1 == _T('\"')) {
        *lptArg1 = _T(' ');
        TrimString(lptArg1);
        bQuoted = TRUE;
    }

    if (bQuoted)
        ptsz = StrChr(lptArg1, _T('\"'));
    else
        ptsz = StrChr(lptArg1, _T(' '));
    ;

    if (ptsz) {
        *ptsz = _T('\0');
        if (lptArg2)
            lstrcpy(lptArg2, ptsz + 1);
    }

    TrimString(lptArg1);

    if (lptArg2)
        TrimString(lptArg2);

    return TRUE;
}

//=============================================================================
//
//  PrepareFilterStr()
//
void PrepareFilterStr(LPTSTR lptFilter)
{
    LPTSTR ptsz = StrEnd_(lptFilter);
    while (ptsz != lptFilter) {
        if (*(ptsz = CharPrev(lptFilter, ptsz)) == _T('|'))
            *ptsz = _T('\0');
    }
}

//=============================================================================
//
//  StrTab2Space() - in place conversion
//
void StrTab2Space(LPTSTR lptsz)
{
    TCHAR* c = lptsz;
    while (c = StrChr(lptsz, _T('\t')))
        *c = _T(' ');
}

//=============================================================================
//
//  PathFixBackslashes() - in place conversion
//
void PathFixBackslashes(LPTSTR lptsz)
{
    TCHAR* c = lptsz;
    while (c = StrChr(c, _T('/'))) {
        if (*CharPrev(lptsz, c) == _T(':') && *CharNext(c) == _T('/'))
            c += sizeof(TCHAR);
        else
            *c = _T('\\');
    }
}

//=============================================================================
//
//  ExpandEnvironmentStringsEx()
//
//  Adjusted for Windows 95
//
void ExpandEnvironmentStringsEx(LPTSTR lptSrc, DWORD dwSrc)
{
    TCHAR tchBuf[312];

    if (ExpandEnvironmentStrings(lptSrc, tchBuf, COUNTOF(tchBuf)))
        lstrcpyn(lptSrc, tchBuf, dwSrc);
}

//=============================================================================
//
//  PathCanonicalizeEx()
//
//
void PathCanonicalizeEx(LPTSTR lptSrc)
{
    TCHAR tchDst[MAX_PATH];

    if (PathCanonicalize(tchDst, lptSrc))
        lstrcpy(lptSrc, tchDst);
}

//=============================================================================
//
//  GetLongPathNameEx()
//
//
DWORD GetLongPathNameEx(LPTSTR lptszPath, DWORD cchBuffer)
{
    DWORD dwRet = GetLongPathName(lptszPath, lptszPath, cchBuffer);
    if (dwRet) {
        if (PathGetDriveNumber(lptszPath) != -1)
            CharUpperBuff(lptszPath, 1);
        return (dwRet);
    }
    return (0);
}

//=============================================================================
//
//  SHGetFileInfo2()
//
//  Return a default name when the file has been removed, and always append
//  a filename extension
//
DWORD_PTR SHGetFileInfo2(LPCTSTR ptszPath, DWORD dwFileAttributes,
    SHFILEINFO* psfi, UINT cbFileInfo, UINT uFlags)
{

    if (PathFileExists(ptszPath)) {

        DWORD_PTR dw = SHGetFileInfo(ptszPath, dwFileAttributes, psfi, cbFileInfo, uFlags);
        if (lstrlen(psfi->szDisplayName) < lstrlen(PathFindFileName(ptszPath)))
            StrCatBuff(psfi->szDisplayName, PathFindExtension(ptszPath), COUNTOF(psfi->szDisplayName));
        return (dw);
    }

    else {
        DWORD_PTR dw = SHGetFileInfo(ptszPath, FILE_ATTRIBUTE_NORMAL, psfi, cbFileInfo, uFlags | SHGFI_USEFILEATTRIBUTES);
        if (lstrlen(psfi->szDisplayName) < lstrlen(PathFindFileName(ptszPath)))
            StrCatBuff(psfi->szDisplayName, PathFindExtension(ptszPath), COUNTOF(psfi->szDisplayName));
        return (dw);
    }
}

//=============================================================================
//
//  FormatNumberStr()
//
int FormatNumberStr(LPTSTR lptNumberStr)
{
    TCHAR* c;
    TCHAR szSep[8];
    int i = 0;

    if (!lstrlen(lptNumberStr))
        return (0);

    if (!GetLocaleInfo(LOCALE_USER_DEFAULT,
            LOCALE_STHOUSAND,
            szSep,
            COUNTOF(szSep)))
        szSep[0] = _T('\'');

    c = StrEnd_(lptNumberStr);

    while ((c = CharPrev(lptNumberStr, c)) != lptNumberStr) {
        if (++i == 3) {
            i = 0;
            MoveMemory(c + 1, c, sizeof(TCHAR) * (lstrlen(c) + 1));
            *c = szSep[0];
        }
    }

    return (lstrlen(lptNumberStr));
}

//=============================================================================
//
//  SetDlgItemIntEx()
//
BOOL SetDlgItemIntEx(HWND hwnd, int nIdItem, UINT uValue)
{
    TCHAR tchBuf[64];

    wsprintf(tchBuf, _T("%u"), uValue);
    FormatNumberStr(tchBuf);

    return (SetDlgItemText(hwnd, nIdItem, tchBuf));
}

//=============================================================================
//
//  A2W: Convert Dialog Item Text form Unicode to UTF-8 and vice versa
//
UINT GetDlgItemTextA2W(UINT uCP, HWND hDlg, int nIDDlgItem, LPSTR lpString, int nMaxCount)
{
    WCHAR wsz[DLG_ITEM_MAXLEN] = L"";
    UINT uRet = GetDlgItemTextW(hDlg, nIDDlgItem, wsz, COUNTOF(wsz));
    ZeroMemory(lpString, nMaxCount);
    WCharToMBCS(uCP, wsz, lpString, nMaxCount - 2);
    return uRet;
}

UINT SetDlgItemTextA2W(UINT uCP, HWND hDlg, int nIDDlgItem, LPSTR lpString)
{
    WCHAR wsz[DLG_ITEM_MAXLEN] = L"";
    MBCSToWChar(uCP, lpString, wsz, COUNTOF(wsz));
    return SetDlgItemTextW(hDlg, nIDDlgItem, wsz);
}

LRESULT ComboBox_AddStringA2W(UINT uCP, HWND hwnd, LPCSTR lpString)
{
    WCHAR wsz[DLG_ITEM_MAXLEN] = L"";
    MBCSToWChar(uCP, lpString, wsz, COUNTOF(wsz));
    return SendMessageW(hwnd, CB_ADDSTRING, 0, (LPARAM)wsz);
}

//=============================================================================
//
//  CodePageFromCharSet()
//
UINT CodePageFromCharSet(UINT uCharSet)
{
    CHARSETINFO ci;
    if (TranslateCharsetInfo((DWORD*)(UINT_PTR)uCharSet, &ci, TCI_SRCCHARSET))
        return (ci.ciACP);
    else
        return (GetACP());
}

//=============================================================================
//
//  MRU functions
//
//  *NOTE: MRU structs & methods use WCHAR strings exclusively
//

int MRU_Cleanup(LPMRULIST pmru)
// Shifts down entries to fill empty ones (if any) and then returns their total count
{

    int i = 0, nUsed = 0;
    for (i = 0; i < pmru->iSize; i++) {
        // 2020-02-14: BUGFIX: Skip null items (uninitialized string entries)
        if (pmru->pwszItems[i]) nUsed++;
    }
    if (nUsed) {
        int nLeft = nUsed;
        i = 0;
        while (i < pmru->iSize - 1) {
            if (pmru->pwszItems[i] == 0) {
                pmru->pwszItems[i] = pmru->pwszItems[i + 1];
            } else {
                i++;
                nLeft--;
            }
            if (nLeft <= 0) break;
        }
    }
    return (nUsed);
}

LPMRULIST MRU_Create(LPCWSTR pwszRegKey, int iFlags, int iSize)
{

    LPMRULIST pmru = LocalAlloc(LPTR, sizeof(MRULIST));
    ZeroMemory(pmru, sizeof(MRULIST));
    lstrcpynW(pmru->wszRegKey, pwszRegKey, COUNTOF(pmru->wszRegKey));
    pmru->iFlags = iFlags;
    pmru->iSize = min(iSize, MRU_MAXITEMS);
    return (pmru);
}

BOOL MRU_Destroy(LPMRULIST pmru)
{

    int i;
    for (i = 0; i < pmru->iSize; i++) {
        if (pmru->pwszItems[i])
            LocalFree(pmru->pwszItems[i]);
    }
    ZeroMemory(pmru, sizeof(MRULIST));
    LocalFree(pmru);
    return (1);
}

int MRU_Compare(LPMRULIST pmru, LPCWSTR pwsz1, LPCWSTR pwsz2)
{

    if (pmru->iFlags & MRU_NOCASE)
        return (lstrcmpiW(pwsz1, pwsz2));
    else
        return (lstrcmpW(pwsz1, pwsz2));
}

BOOL MRU_Add(LPMRULIST pmru, LPCWSTR pwszNew)
{

    int i;
    for (i = 0; i < pmru->iSize; i++) {
        if (MRU_Compare(pmru, pmru->pwszItems[i], pwszNew) == 0) {
            LocalFree(pmru->pwszItems[i]);
            break;
        }
    }
    i = min(i, pmru->iSize - 1);
    for (; i > 0; i--)
        pmru->pwszItems[i] = pmru->pwszItems[i - 1];
    pmru->pwszItems[0] = StrDupW(pwszNew);
    return (1);
}


BOOL MRU_AddFile(LPMRULIST pmru, LPCWSTR pwszFile, BOOL bRelativePath, BOOL bUnexpandMyDocs)
{

    int i, nUsed = MRU_Cleanup(pmru);

    // No need to shift entries if we don't have any yet
    if (nUsed) {
        nUsed = 0;
        for (i = 0; i < pmru->iSize; i++) {
            // 2020-02-14: BUGFIX: Skip null items (uninitialized string entries)
            if (pmru->pwszItems[i]) {
                if (lstrcmpiW(pmru->pwszItems[i], pwszFile) == 0) {
                    LocalFree(pmru->pwszItems[i]);
                    pmru->pwszItems[i] = 0;
                    break;
                } else {
                    WCHAR wchItem[MAX_PATH];
                    PathAbsoluteFromAppW(pmru->pwszItems[i], wchItem, COUNTOF(wchItem), TRUE);
                    if (lstrcmpiW(wchItem, pwszFile) == 0) {
                        LocalFree(pmru->pwszItems[i]);
                        pmru->pwszItems[i] = 0;
                        break;
                    }
                }
            }
        }
        i = min(i, pmru->iSize - 1);
        for (; i > 0; i--)
            pmru->pwszItems[i] = pmru->pwszItems[i - 1];
    } // if (nUsed)

    if (bRelativePath) {
        WCHAR wchFile[MAX_PATH];
        PathRelativeToAppW((LPWSTR)pwszFile, wchFile, COUNTOF(wchFile), TRUE, TRUE, bUnexpandMyDocs);
        pmru->pwszItems[0] = StrDup(wchFile);
    } else
        pmru->pwszItems[0] = StrDup(pwszFile);

    /* notepad2-mod custom code start */
    // Needed to make W7 jump lists work when NP2 is not explicitly associated
    if (IsW7())
        SHAddToRecentDocs(SHARD_PATHW, pwszFile);
    /* notepad2-mod custom code end */

    return (1);
}

BOOL MRU_Delete(LPMRULIST pmru, int iIndex)
{

    int i;
    if (iIndex < 0 || iIndex > pmru->iSize - 1)
        return (0);
    if (pmru->pwszItems[iIndex])
        LocalFree(pmru->pwszItems[iIndex]);
    for (i = iIndex; i < pmru->iSize - 1; i++) {
        pmru->pwszItems[i] = pmru->pwszItems[i + 1];
        pmru->pwszItems[i + 1] = NULL;
    }
    return (1);
}

BOOL MRU_DeleteFileFromStore(LPMRULIST pmru, LPCWSTR pwszFile)
{

    int i = 0;
    LPMRULIST pmruStore;
    WCHAR wchItem[MRUINI_VALUE_MAXLEN];

    pmruStore = MRU_Create(pmru->wszRegKey, pmru->iFlags, pmru->iSize);
    MRU_Load(pmruStore);

    while (MRU_Enum(pmruStore, i, wchItem, COUNTOF(wchItem)) != -1) {
        PathAbsoluteFromAppW(wchItem, wchItem, COUNTOF(wchItem), TRUE);
        if (lstrcmpi(wchItem, pwszFile) == 0)
            MRU_Delete(pmruStore, i);
        else
            i++;
    }

    MRU_Save(pmruStore);
    MRU_Destroy(pmruStore);
    return (1);
}

BOOL MRU_Empty(LPMRULIST pmru)
{

    int i;
    for (i = 0; i < pmru->iSize; i++) {
        if (pmru->pwszItems[i]) {
            LocalFree(pmru->pwszItems[i]);
            pmru->pwszItems[i] = NULL;
        }
    }
    return (1);
}

int MRU_Enum(LPMRULIST pmru, int iIndex, LPWSTR pwszItem, int cchItem)
{

    if (pwszItem == NULL || cchItem == 0) {
        int i = 0;
        while (i < pmru->iSize && pmru->pwszItems[i])
            i++;
        return (i);
    } else {
        if (iIndex < 0 || iIndex > pmru->iSize - 1 || !pmru->pwszItems[iIndex])
            return (-1);
        else {
            lstrcpynW(pwszItem, pmru->pwszItems[iIndex], cchItem);
            return (lstrlenW(pwszItem));
        }
    }
}

BOOL MRU_Load(LPMRULIST pmru)
{

    int i, n = 0;
    WCHAR wchName[MRUINI_IDX_MAXLEN];
    WCHAR wchItem[MRUINI_VALUE_MAXLEN];
    WCHAR* pwIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * MRU_MAXITEMS * MRUINI_LINE_MAXLEN);

    MRU_Empty(pmru);
    LoadIniSectionW(pmru->wszRegKey, pwIniSection, (int)LocalSize(pwIniSection) / sizeof(WCHAR));

    for (i = 0; i < pmru->iSize; i++) {
        wsprintfW(wchName, L"%.2i", i + 1);
        if (IniSectionGetStringW(pwIniSection, wchName, L"", wchItem, COUNTOF(wchItem))) {
/*
            if (pmru->iFlags & MRU_UTF8) {
                WCHAR wchItem[MRUINI_VALUE_MAXLEN];
                int cbw = MultiByteToWideChar(CP_UTF7, 0, tchItem, -1, wchItem, COUNTOF(wchItem));
                WideCharToMultiByte(CP_UTF8, 0, wchItem, cbw, tchItem, COUNTOF(tchItem), NULL, NULL);
                pmru->pwszItems[n++] = StrDup(tchItem);
            } else
*/
            pmru->pwszItems[n++] = StrDupW(wchItem);
        }
    }
    LocalFree(pwIniSection);
    return (1);
}

BOOL MRU_Save(LPMRULIST pmru)
{

    int i;
    WCHAR wchName[MRUINI_IDX_MAXLEN];
    WCHAR* pwIniSection = LocalAlloc(LPTR, sizeof(WCHAR) * MRU_MAXITEMS * MRUINI_LINE_MAXLEN);

    //IniDeleteSection(pmru->wszRegKey);

    for (i = 0; i < pmru->iSize; i++) {
        if (pmru->pwszItems[i]) {
            wsprintfW(wchName, L"%.2i", i + 1);
/*
            if (pmru->iFlags & MRU_UTF8) {
                WCHAR tchItem[MRUINI_VALUE_MAXLEN];
                WCHAR wchItem[MRUINI_VALUE_MAXLEN];
                int cbw = MultiByteToWideChar(CP_UTF8, 0, pmru->pwszItems[i], -1, wchItem, COUNTOF(wchItem));
                WideCharToMultiByte(CP_UTF7, 0, wchItem, cbw, tchItem, COUNTOF(tchItem), NULL, NULL);
                IniSectionSetString(pIniSection, tchName, tchItem);
            } else
*/
            IniSectionSetStringW(pwIniSection, wchName, pmru->pwszItems[i]);
        }
    }
    SaveIniSectionW(pmru->wszRegKey, pwIniSection);
    LocalFree(pwIniSection);
    return (1);
}

BOOL MRU_MergeSave(LPMRULIST pmru, BOOL bAddFiles, BOOL bRelativePath, BOOL bUnexpandMyDocs)
{

    int i;
    LPMRULIST pmruBase;

    pmruBase = MRU_Create(pmru->wszRegKey, pmru->iFlags, pmru->iSize);
    MRU_Load(pmruBase);

    if (bAddFiles) {
        for (i = pmru->iSize - 1; i >= 0; i--) {
            if (pmru->pwszItems[i]) {
                WCHAR wchItem[MAX_PATH];
                PathAbsoluteFromAppW(pmru->pwszItems[i], wchItem, COUNTOF(wchItem), TRUE);
                MRU_AddFile(pmruBase, wchItem, bRelativePath, bUnexpandMyDocs);
            }
        }
    }

    else {
        for (i = pmru->iSize - 1; i >= 0; i--) {
            if (pmru->pwszItems[i])
                MRU_Add(pmruBase, pmru->pwszItems[i]);
        }
    }

    MRU_Save(pmruBase);
    MRU_Destroy(pmruBase);
    return (1);
}

/*

  Themed Dialogs
  Modify dialog templates to use current theme font
  Based on code of MFC helper class CDialogTemplate

*/

BOOL GetThemedDialogFont(LPTSTR lptFaceName, WORD* wSize)
{
    HDC hDC;
    int iLogPixelsY;
    HMODULE hModUxTheme;
    HTHEME hTheme;
    LOGFONT lf;
    BOOL bSucceed = FALSE;

    hDC = GetDC(NULL);
    iLogPixelsY = GetDeviceCaps(hDC, LOGPIXELSY);
    ReleaseDC(NULL, hDC);

    if (hModUxTheme = GetModuleHandleW(L"uxtheme.dll")) {
        if ((BOOL)(GetProcAddress(hModUxTheme, "IsAppThemed"))()) {
            hTheme = (HTHEME)(INT_PTR)(GetProcAddress(hModUxTheme, "OpenThemeData"))(NULL, L"WINDOWSTYLE;WINDOW");
            if (hTheme) {
                if (S_OK == (HRESULT)(GetProcAddress(hModUxTheme, "GetThemeSysFont"))(hTheme, /*TMT_MSGBOXFONT*/ 805, &lf)) {
                    if (lf.lfHeight < 0)
                        lf.lfHeight = -lf.lfHeight;
                    *wSize = (WORD)MulDiv(lf.lfHeight, 72, iLogPixelsY);
                    if (*wSize == 0)
                        *wSize = 8;
                    StrCpyN(lptFaceName, lf.lfFaceName, LF_FACESIZE);
                    bSucceed = TRUE;
                }
                (GetProcAddress(hModUxTheme, "CloseThemeData"))(hTheme);
            }
        }
    }

/*
    if (!bSucceed) {
        NONCLIENTMETRICS ncm;
        ncm.cbSize = sizeof(NONCLIENTMETRICS);
        SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0);
        if (ncm.lfMessageFont.lfHeight < 0)
            ncm.lfMessageFont.lfHeight = -ncm.lfMessageFont.lfHeight;
        *wSize = (WORD)MulDiv(ncm.lfMessageFont.lfHeight, 72, iLogPixelsY);
        if (*wSize == 0)
            *wSize = 8;
        StrCpyN(lptFaceName, ncm.lfMessageFont.lfFaceName, LF_FACESIZE);
    }
*/
    return (bSucceed);
}

__inline BOOL DialogTemplate_IsDialogEx(const DLGTEMPLATE* pTemplate)
    { return ((DLGTEMPLATEEX*)pTemplate)->signature == 0xFFFF; }

__inline BOOL DialogTemplate_HasFont(const DLGTEMPLATE* pTemplate)
    { return (DS_SETFONT & (DialogTemplate_IsDialogEx(pTemplate) ? ((DLGTEMPLATEEX*)pTemplate)->style : pTemplate->style)); }

__inline int DialogTemplate_FontAttrSize(BOOL bDialogEx)
    { return (int)sizeof(WORD) * (bDialogEx ? 3 : 1); }

__inline BYTE* DialogTemplate_GetFontSizeField(const DLGTEMPLATE* pTemplate)
{
    BOOL bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
    WORD* pw;

    if (bDialogEx)
        pw = (WORD*)((DLGTEMPLATEEX*)pTemplate + 1);
    else
        pw = (WORD*)(pTemplate + 1);

    if (*pw == (WORD)-1)
        pw += 2;
    else
        while (*pw++)
            ;

    if (*pw == (WORD)-1)
        pw += 2;
    else
        while (*pw++)
            ;

    while (*pw++)
        ;

    return (BYTE*)pw;
}

DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR lptDialogTemplateID, HINSTANCE hInstance)
{

    HRSRC hRsrc;
    HGLOBAL hRsrcMem;
    DLGTEMPLATE* pRsrcMem;
    DLGTEMPLATE* pTemplate;
    UINT dwTemplateSize = 0;
    TCHAR tchFaceName[LF_FACESIZE];
    WORD wFontSize;
    BOOL bDialogEx;
    BOOL bHasFont;
    int cbFontAttr;
    int cbNew;
    int cbOld;
    BYTE* pbNew;
    BYTE* pb;
    BYTE* pOldControls;
    BYTE* pNewControls;
    WORD nCtrl;

    hRsrc = FindResource(hInstance, lptDialogTemplateID, RT_DIALOG);
    if (hRsrc == NULL)
        return (NULL);

    hRsrcMem = LoadResource(hInstance, hRsrc);
    pRsrcMem = (DLGTEMPLATE*)LockResource(hRsrcMem);
    dwTemplateSize = (UINT)SizeofResource(hInstance, hRsrc);

    if ((dwTemplateSize == 0) || (pTemplate = LocalAlloc(LPTR, dwTemplateSize + LF_FACESIZE * 2)) == NULL) {
        UnlockResource(hRsrcMem);
        FreeResource(hRsrcMem);
        return (NULL);
    }

    CopyMemory((BYTE*)pTemplate, pRsrcMem, (size_t)dwTemplateSize);
    UnlockResource(hRsrcMem);
    FreeResource(hRsrcMem);

    if (!GetThemedDialogFont(tchFaceName, &wFontSize))
        return (pTemplate);

    bDialogEx = DialogTemplate_IsDialogEx(pTemplate);
    bHasFont = DialogTemplate_HasFont(pTemplate);
    cbFontAttr = DialogTemplate_FontAttrSize(bDialogEx);

    if (bDialogEx)
        ((DLGTEMPLATEEX*)pTemplate)->style |= DS_SHELLFONT;
    else
        pTemplate->style |= DS_SHELLFONT;

    cbNew = cbFontAttr + ((lstrlen(tchFaceName) + 1) * sizeof(WCHAR));
    pbNew = (BYTE*)tchFaceName;

    pb = DialogTemplate_GetFontSizeField(pTemplate);
    cbOld = (int)(bHasFont ? cbFontAttr + 2 * (lstrlen((WCHAR*)(pb + cbFontAttr)) + 1) : 0);

    pOldControls = (BYTE*)(((DWORD_PTR)pb + cbOld + 3) & ~(DWORD_PTR)3);
    pNewControls = (BYTE*)(((DWORD_PTR)pb + cbNew + 3) & ~(DWORD_PTR)3);

    nCtrl = bDialogEx ? (WORD)((DLGTEMPLATEEX*)pTemplate)->cDlgItems : (WORD)pTemplate->cdit;

    if (cbNew != cbOld && nCtrl > 0)
        MoveMemory(pNewControls, pOldControls, (size_t)(dwTemplateSize - (pOldControls - (BYTE*)pTemplate)));

    *(WORD*)pb = wFontSize;
    MoveMemory(pb + cbFontAttr, pbNew, (size_t)(cbNew - cbFontAttr));

    return (pTemplate);
}

INT_PTR ThemedDialogBoxParam(
    HINSTANCE hInstance,
    LPCTSTR lptTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{

    INT_PTR ret;
    DLGTEMPLATE* pDlgTemplate;

    pDlgTemplate = LoadThemedDialogTemplate(lptTemplate, hInstance);
    ret = DialogBoxIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
    if (pDlgTemplate)
        LocalFree(pDlgTemplate);

    return (ret);
}

HWND CreateThemedDialogParam(
    HINSTANCE hInstance,
    LPCTSTR lptTemplate,
    HWND hWndParent,
    DLGPROC lpDialogFunc,
    LPARAM dwInitParam)
{

    HWND hwnd;
    DLGTEMPLATE* pDlgTemplate;

    pDlgTemplate = LoadThemedDialogTemplate(lptTemplate, hInstance);
    hwnd = CreateDialogIndirectParam(hInstance, pDlgTemplate, hWndParent, lpDialogFunc, dwInitParam);
    if (pDlgTemplate)
        LocalFree(pDlgTemplate);

    return (hwnd);
}

#ifdef JRB_BUILD
HWND CreateToolTipW(int toolID, HWND hDlg, PWSTR pwszText)
{
	if (!toolID || !hDlg || !pwszText) {
		return FALSE;
	}
	// Get the window of the tool.
	HWND hwndTool = GetDlgItem(hDlg, toolID);

	if (!hwndTool) return (HWND)NULL;

	// Create the tooltip. g_hInst is the global instance handle.
	HWND hwndTip = CreateWindowExW(0, TOOLTIPS_CLASSW, NULL,
								   WS_POPUP |TTS_ALWAYSTIP,
								   CW_USEDEFAULT, CW_USEDEFAULT,
								   CW_USEDEFAULT, CW_USEDEFAULT,
								   hDlg, NULL, 
								   g_hInstance, NULL);

	if (!hwndTip) return (HWND)NULL;

	// Associate the tooltip with the tool.
	TOOLINFOW toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hwndTool;
	toolInfo.lpszText = pwszText;
	SendMessageW(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);

	return hwndTip;
}
#endif


/******************************************************************************
*
*  UnSlash functions
*  Mostly taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
*
/

/**
 * Is the character an octal digit?
 */
static BOOL IsOctalDigit(char ch)
{
    return ch >= '0' && ch <= '7';
}

/**
 * If the character is an hexa digit, get its value.
 */
static int GetHexDigit(char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    return -1;
}

/**
 * Convert C style \a, \b, \f, \n, \r, \t, \v, \xhh and \uhhhh into their indicated characters.
 */
unsigned int UnSlash(char* s, UINT cpEdit)
{
    char* sStart = s;
    char* o = s;

    while (*s) {
        if (*s == '\\') {
            s++;
            if (*s == 'a')
                *o = '\a';
            else if (*s == 'b')
                *o = '\b';
            else if (*s == 'f')
                *o = '\f';
            else if (*s == 'n')
                *o = '\n';
            else if (*s == 'r')
                *o = '\r';
            else if (*s == 't')
                *o = '\t';
            else if (*s == 'v')
                *o = '\v';
            else if (*s == 'x' || *s == 'u') {
                BOOL bShort = (*s == 'x');
                char ch[8];
                char* pch = ch;
                WCHAR val[2] = L"";
                int hex;
                val[0] = 0;
                hex = GetHexDigit(*(s + 1));
                if (hex >= 0) {
                    s++;
                    val[0] = hex;
                    hex = GetHexDigit(*(s + 1));
                    if (hex >= 0) {
                        s++;
                        val[0] *= 16;
                        val[0] += hex;
                        if (!bShort) {
                            hex = GetHexDigit(*(s + 1));
                            if (hex >= 0) {
                                s++;
                                val[0] *= 16;
                                val[0] += hex;
                                hex = GetHexDigit(*(s + 1));
                                if (hex >= 0) {
                                    s++;
                                    val[0] *= 16;
                                    val[0] += hex;
                                }
                            }
                        }
                    }
                    if (val[0]) {
                        val[1] = 0;
                        WideCharToMultiByte(cpEdit, 0, val, -1, ch, COUNTOF(ch), NULL, NULL);
                        *o = *pch++;
                        while (*pch)
                            *++o = *pch++;
                    } else
                        o--;
                } else
                    o--;
            } else
                *o = *s;
        } else
            *o = *s;
        o++;
        if (*s) {
            s++;
        }
    }
    *o = '\0';
    return (unsigned int)(o - sStart);
}

/**
 * Convert C style \0oo into their indicated characters.
 * This is used to get control characters into the regular expresion engine.
 */
unsigned int UnSlashLowOctal(char* s)
{
    char* sStart = s;
    char* o = s;
    while (*s) {
        if ((s[0] == '\\') && (s[1] == '0') && IsOctalDigit(s[2]) && IsOctalDigit(s[3])) {
            *o = (char)(8 * (s[2] - '0') + (s[3] - '0'));
            s += 3;
        } else {
            *o = *s;
        }
        o++;
        if (*s)
            s++;
    }
    *o = '\0';
    return (unsigned int)(o - sStart);
}

void TransformBackslashes(char* pszInput, BOOL bRegEx, UINT cpEdit)
{
    if (bRegEx)
        UnSlashLowOctal(pszInput);
    else
        UnSlash(pszInput, cpEdit);
}

/*

  MinimizeToTray - Copyright 2000 Matthew Ellis <m.t.ellis@bigfoot.com>

  Changes made by flo:
   - Commented out: #include "stdafx.h"
   - Moved variable declaration: APPBARDATA appBarData;

*/

// MinimizeToTray
//
// A couple of routines to show how to make it produce a custom caption
// animation to make it look like we are minimizing to and maximizing
// from the system tray
//
// These routines are public domain, but it would be nice if you dropped
// me a line if you use them!
//
// 1.0 29.06.2000 Initial version
// 1.1 01.07.2000 The window retains it's place in the Z-order of windows
//     when minimized/hidden. This means that when restored/shown, it doen't
//     always appear as the foreground window unless we call SetForegroundWindow
//
// Copyright 2000 Matthew Ellis <m.t.ellis@bigfoot.com>
/*#include "stdafx.h"*/

// Odd. VC++6 winuser.h has IDANI_CAPTION defined (as well as IDANI_OPEN and
// IDANI_CLOSE), but the Platform SDK only has IDANI_OPEN...

// I don't know what IDANI_OPEN or IDANI_CLOSE do. Trying them in this code
// produces nothing. Perhaps they were intended for window opening and closing
// like the MAC provides...
#ifndef IDANI_OPEN
#define IDANI_OPEN 1
#endif
#ifndef IDANI_CLOSE
#define IDANI_CLOSE 2
#endif
#ifndef IDANI_CAPTION
#define IDANI_CAPTION 3
#endif

#define DEFAULT_RECT_WIDTH 150
#define DEFAULT_RECT_HEIGHT 30

// Returns the rect of where we think the system tray is. This will work for
// all current versions of the shell. If explorer isn't running, we try our
// best to work with a 3rd party shell. If we still can't find anything, we
// return a rect in the lower right hand corner of the screen
static VOID GetTrayWndRect(LPRECT lpTrayRect)
{
    APPBARDATA appBarData;
    // First, we'll use a quick hack method. We know that the taskbar is a window
    // of class Shell_TrayWnd, and the status tray is a child of this of class
    // TrayNotifyWnd. This provides us a window rect to minimize to. Note, however,
    // that this is not guaranteed to work on future versions of the shell. If we
    // use this method, make sure we have a backup!
    HWND hShellTrayWnd = FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
    if (hShellTrayWnd) {
        HWND hTrayNotifyWnd = FindWindowEx(hShellTrayWnd, NULL, TEXT("TrayNotifyWnd"), NULL);
        if (hTrayNotifyWnd) {
            GetWindowRect(hTrayNotifyWnd, lpTrayRect);
            return;
        }
    }

    // OK, we failed to get the rect from the quick hack. Either explorer isn't
    // running or it's a new version of the shell with the window class names
    // changed (how dare Microsoft change these undocumented class names!) So, we
    // try to find out what side of the screen the taskbar is connected to. We
    // know that the system tray is either on the right or the bottom of the
    // taskbar, so we can make a good guess at where to minimize to
    /*APPBARDATA appBarData;*/
    appBarData.cbSize = sizeof(appBarData);
    if (SHAppBarMessage(ABM_GETTASKBARPOS, &appBarData)) {
        // We know the edge the taskbar is connected to, so guess the rect of the
        // system tray. Use various fudge factor to make it look good
        switch (appBarData.uEdge) {
        case ABE_LEFT:
        case ABE_RIGHT:
            // We want to minimize to the bottom of the taskbar
            lpTrayRect->top = appBarData.rc.bottom - 100;
            lpTrayRect->bottom = appBarData.rc.bottom - 16;
            lpTrayRect->left = appBarData.rc.left;
            lpTrayRect->right = appBarData.rc.right;
            break;

        case ABE_TOP:
        case ABE_BOTTOM:
            // We want to minimize to the right of the taskbar
            lpTrayRect->top = appBarData.rc.top;
            lpTrayRect->bottom = appBarData.rc.bottom;
            lpTrayRect->left = appBarData.rc.right - 100;
            lpTrayRect->right = appBarData.rc.right - 16;
            break;
        }

        return;
    }

    // Blimey, we really aren't in luck. It's possible that a third party shell
    // is running instead of explorer. This shell might provide support for the
    // system tray, by providing a Shell_TrayWnd window (which receives the
    // messages for the icons) So, look for a Shell_TrayWnd window and work out
    // the rect from that. Remember that explorer's taskbar is the Shell_TrayWnd,
    // and stretches either the width or the height of the screen. We can't rely
    // on the 3rd party shell's Shell_TrayWnd doing the same, in fact, we can't
    // rely on it being any size. The best we can do is just blindly use the
    // window rect, perhaps limiting the width and height to, say 150 square.
    // Note that if the 3rd party shell supports the same configuraion as
    // explorer (the icons hosted in NotifyTrayWnd, which is a child window of
    // Shell_TrayWnd), we would already have caught it above
    hShellTrayWnd = FindWindowEx(NULL, NULL, TEXT("Shell_TrayWnd"), NULL);
    if (hShellTrayWnd) {
        GetWindowRect(hShellTrayWnd, lpTrayRect);
        if (lpTrayRect->right - lpTrayRect->left > DEFAULT_RECT_WIDTH)
            lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
        if (lpTrayRect->bottom - lpTrayRect->top > DEFAULT_RECT_HEIGHT)
            lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;

        return;
    }

    // OK. Haven't found a thing. Provide a default rect based on the current work
    // area
    SystemParametersInfo(SPI_GETWORKAREA, 0, lpTrayRect, 0);
    lpTrayRect->left = lpTrayRect->right - DEFAULT_RECT_WIDTH;
    lpTrayRect->top = lpTrayRect->bottom - DEFAULT_RECT_HEIGHT;
}

// Check to see if the animation has been disabled
/*static */ BOOL GetDoAnimateMinimize(VOID)
{
    ANIMATIONINFO ai;

    ai.cbSize = sizeof(ai);
    SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);

    return ai.iMinAnimate ? TRUE : FALSE;
}

VOID MinimizeWndToTray(HWND hWnd)
{
    if (GetDoAnimateMinimize()) {
        RECT rcFrom, rcTo;

        // Get the rect of the window. It is safe to use the rect of the whole
        // window - DrawAnimatedRects will only draw the caption
        GetWindowRect(hWnd, &rcFrom);
        GetTrayWndRect(&rcTo);

        // Get the system to draw our animation for us
        DrawAnimatedRects(hWnd, IDANI_CAPTION, &rcFrom, &rcTo);
    }

    // Add the tray icon. If we add it before the call to DrawAnimatedRects,
    // the taskbar gets erased, but doesn't get redrawn until DAR finishes.
    // This looks untidy, so call the functions in this order

    // Hide the window
    ShowWindow(hWnd, SW_HIDE);
}

VOID RestoreWndFromTray(HWND hWnd)
{
    if (GetDoAnimateMinimize()) {
        // Get the rect of the tray and the window. Note that the window rect
        // is still valid even though the window is hidden
        RECT rcFrom, rcTo;
        GetTrayWndRect(&rcFrom);
        GetWindowRect(hWnd, &rcTo);

        // Get the system to draw our animation for us
        DrawAnimatedRects(hWnd, IDANI_CAPTION, &rcFrom, &rcTo);
    }

    // Show the window, and make sure we're the foreground window
    ShowWindow(hWnd, SW_SHOW);
    SetActiveWindow(hWnd);
    SetForegroundWindow(hWnd);

    // Remove the tray icon. As described above, remove the icon after the
    // call to DrawAnimatedRects, or the taskbar will not refresh itself
    // properly until DAR finished
}

///   End of Helpers.c   \\\
