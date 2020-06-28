/******************************************************************************
*
*
* Notepad2
*
* Helpers.h
*   Definitions for general helper functions and macros
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

extern HINSTANCE g_hInstance;
extern UINT16 g_uWinVer;

#define COUNTOF(ar) (sizeof(ar)/sizeof(ar[0]))
#define CSTRLEN(s)  (COUNTOF(s)-1)

// Added on 2019-08-08
#define CEILING(X)      ( ((X) > 0) ? ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X)) : (int)(X) )
#define CEIL(X)          CEILING(X)
#define DLG_ITEM_MAXLEN  4096
#define SEL_MAXLEN       DLG_ITEM_MAXLEN

// Added on 2019-08-08
#define MRUINI_IDX_MAXLEN     3 // 2 == CEIL(log(MRU_MAXITEMS)/log(10))
#define MRUINI_NAME_MAXLEN   64
#define MRUINI_VALUE_MAXLEN  DLG_ITEM_MAXLEN
#define MRUINI_LINE_MAXLEN   ( MRUINI_NAME_MAXLEN + sizeof('=') + MRUINI_VALUE_MAXLEN + sizeof('\r\n') )

extern TCHAR g_tszIniFile[MAX_PATH];

#define IniGetStringW(lpwSection,lpwName,lpwDefault,lpwReturnedStr,nSize) \
    GetPrivateProfileStringW(lpwSection,lpwName,lpwDefault,lpwReturnedStr,nSize,g_tszIniFile)
#define IniGetIntW(lpwSection,lpwName,nDefault) \
    GetPrivateProfileIntW(lpwSection,lpwName,nDefault,wszIniFile)
#define IniSetStringW(lpwSection,lpwName,lpwString) \
    WritePrivateProfileString(lpwSection,lpwName,lpwString,wszIniFile)
#define IniDeleteSectionW(lpwSection) \
    WritePrivateProfileSectionW(lpwSection,NULL,wszIniFile)

__inline BOOL IniSetIntW(LPCWSTR lpwSection,LPCWSTR lpwName,int i) {
    WCHAR wch[32]; wsprintfW(wch, L"%i",i); return IniSetStringW(lpwSection, lpwName, wch);
}
#define LoadIniSectionA(lpSection,lpBuf,cchBuf) \
    GetPrivateProfileSectionA(lpSection,lpBuf,cchBuf,wszIniFile);
#define LoadIniSectionW(lpwSection,lpwBuf,cchBuf) \
    GetPrivateProfileSectionW(lpwSection,lpwBuf,cchBuf,wszIniFile);

#define SaveIniSectionW(lpwSection,lpwBuf) \
    WritePrivateProfileSectionW(lpwSection,lpwBuf,wszIniFile)

int IniSectionGetStringW(LPCWSTR,LPCWSTR,LPCWSTR,LPWSTR,int);
int IniSectionGetIntW(LPCWSTR,LPCWSTR,int);
BOOL IniSectionSetStringW(LPWSTR,LPCWSTR,LPCWSTR);
__inline BOOL IniSectionSetIntW(LPWSTR lpCachedIniSection,LPCWSTR lpName,int i) {
    WCHAR tch[32]; wsprintf(tch,L"%i",i); return IniSectionSetStringW(lpCachedIniSection,lpName,tch);
}

extern HWND hwndEdit;
__inline void BeginWaitCursor()
{
  SendMessage(hwndEdit,SCI_SETCURSOR,(WPARAM)SC_CURSORWAIT,0);
}
__inline void EndWaitCursor()
{
  POINT pt;
  SendMessage(hwndEdit,SCI_SETCURSOR,(WPARAM)SC_CURSORNORMAL,0);
  GetCursorPos(&pt);
  SetCursorPos(pt.x,pt.y);
}

#define Is2k()    (g_uWinVer >= 0x0500)
#define IsXP()    (g_uWinVer >= 0x0501)
#define IsVista() (g_uWinVer >= 0x0600)
#define IsW7()    (g_uWinVer >= 0x0601)

BOOL PrivateIsAppThemed();
HRESULT PrivateSetCurrentProcessExplicitAppUserModelID(PCTSTR);
BOOL IsElevated();
//BOOL SetExplorerTheme(HWND);

BOOL BitmapMergeAlpha(HBITMAP,COLORREF);
BOOL BitmapAlphaBlend(HBITMAP,COLORREF,BYTE);
BOOL BitmapGrayScale(HBITMAP);
BOOL VerifyContrast(COLORREF,COLORREF);
BOOL IsFontAvailableW(LPCWSTR);

BOOL SetWindowTitle(HWND,UINT,BOOL,UINT,LPCTSTR,int,BOOL,UINT,BOOL,LPCTSTR);
void SetWindowTransparentMode(HWND,BOOL);

void CenterDlgInParent(HWND);
void GetDlgPos(HWND,LPINT,LPINT);
void SetDlgPos(HWND,int,int);
void ResizeDlg_Init(HWND,int,int,int);
void ResizeDlg_Destroy(HWND,int*,int*);
void ResizeDlg_Size(HWND,LPARAM,int*,int*);
void ResizeDlg_GetMinMaxInfo(HWND,LPARAM);
HDWP DeferCtlPos(HDWP,HWND,int,int,int,UINT);
void MakeBitmapButton(HWND,int,HINSTANCE,UINT);
void MakeColorPickButton(HWND,int,HINSTANCE,COLORREF);
void DeleteBitmapButton(HWND,int);

#define StatusSetSimple(hwnd,b) SendMessage(hwnd,SB_SIMPLE,(WPARAM)b,0)
BOOL StatusSetText(HWND,UINT,LPCTSTR);
BOOL StatusSetTextID(HWND,UINT,UINT);
int  StatusCalcPaneWidth(HWND,LPCTSTR);

int Toolbar_GetButtons(HWND,int,LPTSTR,int);
int Toolbar_SetButtons(HWND,int,LPCTSTR, LPCTBBUTTON,int);

LRESULT SendWMSize(HWND);

#define EnableCmd(hmenu,id,b) EnableMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_ENABLED:MF_BYCOMMAND|MF_GRAYED)

#define CheckCmd(hmenu,id,b)  CheckMenuItem(hmenu,id,(b)\
                               ?MF_BYCOMMAND|MF_CHECKED:MF_BYCOMMAND|MF_UNCHECKED)

BOOL IsCmdEnabled(HWND, UINT);

#define GetStringA(id,pb,cb) LoadStringA(g_hInstance,id,pb,cb)
#define GetStringW(id,pb,cb) LoadStringW(g_hInstance,id,pb,cb)
#if _UNICODE
#define GetString_ GetStringW
#else
#define GetString_ GetStringA
#endif

#define StrEndA(pszStart) (pszStart + lstrlenA(pszStart))
#define StrEndW(pwszStart) (pwszStart + lstrlenW(pwszStart))
#if _UNICODE
#define StrEnd_ StrEndW
#else
#define StrEnd_ StrEndA
#endif

int FormatString(LPTSTR,int,UINT,...);

///void PathRelativeToApp(LPTSTR, LPTSTR, int, BOOL, BOOL, BOOL);
void PathRelativeToAppA(LPSTR, LPSTR, int, BOOL, BOOL, BOOL);
void PathRelativeToAppW(LPWSTR, LPWSTR, int, BOOL, BOOL, BOOL);

void PathAbsoluteFromAppA(LPSTR, LPSTR, int, BOOL);
void PathAbsoluteFromAppW(LPWSTR, LPWSTR, int, BOOL);

BOOL PathIsLnkFile(LPCTSTR);
BOOL PathGetLnkPath(LPCTSTR,LPTSTR,int);
BOOL PathIsLnkToDirectory(LPCTSTR,LPTSTR,int);
BOOL PathCreateDeskLnk(LPCTSTR);
BOOL PathCreateFavLnk(LPCTSTR,LPCTSTR,LPCTSTR);

BOOL StrLTrim(LPTSTR,LPCTSTR);
BOOL TrimString(LPTSTR);
BOOL ExtractFirstArgument(LPCWSTR, LPWSTR, LPWSTR);

void PrepareFilterStr(LPWSTR);

void StrTab2Space(LPWSTR);
void PathFixBackslashes(LPWSTR);

void  ExpandEnvironmentStringsEx(LPWSTR,DWORD);
void  PathCanonicalizeEx(LPWSTR);
DWORD GetLongPathNameEx(LPWSTR,DWORD);
DWORD_PTR SHGetFileInfo2(LPCWSTR,DWORD,SHFILEINFO*,UINT,UINT);

int  FormatNumberStr(LPWSTR);
BOOL SetDlgItemIntEx(HWND,int,UINT);

#define MBCSToWChar(c,a,w,i) MultiByteToWideChar(c,0,a,-1,w,i)
#define WCharToMBCS(c,w,a,i) WideCharToMultiByte(c,0,w,-1,a,i,NULL,NULL)

UINT    GetDlgItemTextA2W(UINT,HWND,int,LPSTR,int);
UINT    SetDlgItemTextA2W(UINT,HWND,int,LPSTR);
LRESULT ComboBox_AddStringA2W(UINT,HWND,LPCSTR);

UINT CodePageFromCharSet(UINT);

//==== MRU Functions ==========================================================
#define MRU_MAXITEMS 24
#define MRU_NOCASE    1
#define MRU_UTF8      2

typedef struct _mrulist {

  WCHAR  wszRegKey[256];
  int   iFlags;
  int   iSize;
  LPWSTR pwszItems[MRU_MAXITEMS];

} MRULIST, *PMRULIST, *LPMRULIST;

int       MRU_Cleanup(LPMRULIST);
LPMRULIST MRU_Create(LPCWSTR,int,int);
BOOL      MRU_Destroy(LPMRULIST);
BOOL      MRU_Add(LPMRULIST,LPCWSTR);
BOOL      MRU_AddFile(LPMRULIST,LPCWSTR,BOOL,BOOL);
BOOL      MRU_Delete(LPMRULIST,int);
BOOL      MRU_DeleteFileFromStore(LPMRULIST,LPCWSTR);
BOOL      MRU_Empty(LPMRULIST);
int       MRU_Enum(LPMRULIST,int,LPWSTR,int);
BOOL      MRU_Load(LPMRULIST);
BOOL      MRU_Save(LPMRULIST);
BOOL      MRU_MergeSave(LPMRULIST,BOOL,BOOL,BOOL);

//==== Themed Dialogs =========================================================
#ifndef DLGTEMPLATEEX
#pragma pack(push, 1)
typedef struct {
  WORD      dlgVer;
  WORD      signature;
  DWORD     helpID;
  DWORD     exStyle;
  DWORD     style;
  WORD      cDlgItems;
  short     x;
  short     y;
  short     cx;
  short     cy;
} DLGTEMPLATEEX;
#pragma pack(pop)
#endif

BOOL GetThemedDialogFont(LPWSTR,WORD*);
DLGTEMPLATE* LoadThemedDialogTemplate(LPCTSTR,HINSTANCE);
#define ThemedDialogBox(hInstance,lpTemplate,hWndParent,lpDialogFunc) \
  ThemedDialogBoxParam(hInstance,lpTemplate,hWndParent,lpDialogFunc,0)
INT_PTR ThemedDialogBoxParam(HINSTANCE,LPCTSTR,HWND,DLGPROC,LPARAM);
HWND    CreateThemedDialogParam(HINSTANCE,LPCTSTR,HWND,DLGPROC,LPARAM);

#ifdef JRB_BUILD
HWND CreateToolTipW(int toolID, HWND hDlg, PWSTR pwszText);
#endif

//==== UnSlash Functions ======================================================
void TransformBackslashes(char*,BOOL,UINT);

//==== MinimizeToTray Functions - see comments in Helpers.c ===================
BOOL GetDoAnimateMinimize(VOID);
VOID MinimizeWndToTray(HWND hWnd);
VOID RestoreWndFromTray(HWND hWnd);

///   End of Helpers.h   \\\
