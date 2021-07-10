/******************************************************************************
*
*
* Notepad2
*
* Notepad2.h
*   Global definitions and declarations
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



//==== Main Window ============================================================
#define WC_NOTEPAD2 L"Notepad2"


//==== Data Type for WM_COPYDATA ==============================================
#define DATA_NOTEPAD2_PARAMS 0xFB10
typedef struct np2params {

  int   flagFileSpecified;
  int   flagChangeNotify;
  int   flagLexerSpecified;
  int   iInitialLexer;
  int   flagQuietCreate;
  int   flagJumpTo;
  int   iInitialLine;
  int   iInitialColumn;
  int   iSrcEncoding;
  int   flagSetEncoding;
  int   flagSetEOLMode;
  int   flagTitleExcerpt;
  WCHAR wchData;

} NP2PARAMS, *LPNP2PARAMS;


//==== Toolbar Style ==========================================================
#define WS_TOOLBAR (WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | \
                    TBSTYLE_TOOLTIPS | TBSTYLE_FLAT | TBSTYLE_ALTDRAG | \
                    TBSTYLE_LIST | CCS_NODIVIDER | CCS_NOPARENTALIGN | \
                    CCS_ADJUSTABLE)


//==== ReBar Style ============================================================
#define WS_REBAR (WS_CHILD | WS_CLIPCHILDREN | WS_BORDER | RBS_VARHEIGHT | \
                  RBS_BANDBORDERS | CCS_NODIVIDER | CCS_NOPARENTALIGN)


//==== Ids ====================================================================
#define IDC_STATUSBAR    0xFB00
#define IDC_TOOLBAR      0xFB01
#define IDC_REBAR        0xFB02
#define IDC_EDIT         0xFB03
#define IDC_EDITFRAME    0xFB04
#define IDC_FILENAME     0xFB05
#define IDC_REUSELOCK    0xFB06


//==== Statusbar ==============================================================
#define STATUS_DOCPOS    0
#define STATUS_DOCSIZE   1
#define STATUS_CODEPAGE  2
#define STATUS_EOLMODE   3
#define STATUS_OVRMODE   4
#define STATUS_LEXER     5
#define STATUS_HELP    255

// 2020-01-29: We shouldn't use the [WM_USER .. 0x7FFF] range, but [WM_APP .. 0xBFFF] instead
//             See https://devblogs.microsoft.com/oldnewthing/20060925-02/?p=29603

//==== Callback Message from System Tray ======================================
#define WM_TRAYMESSAGE                 WM_APP + 0


//==== Change Notifications ===================================================
#define WM_CHANGENOTIFY                WM_APP + 1
#ifdef FEAT_NOTIFY_CHANGE_ON_ACTIVEAPP
#define WM_CHANGENOTIFYCLEAR           WM_APP + 2
#define WM_CHANGENOTIFYUSERPRESENT     WM_APP + 3
#endif
// 2020-01-29: Timers IDs now expressed as being WM_APP-based
#define ID_WATCHTIMER                  WM_APP + 0x2000


//==== Paste Board Timer ======================================================
#define ID_PASTEBOARDTIMER             WM_APP + 0x2001


//==== Reuse Window Lock Timeout ==============================================
#define REUSEWINDOWLOCKTIMEOUT 1000


//==== Function Declarations ==================================================
BOOL InitApplication(HINSTANCE);
HWND InitInstance(HINSTANCE,LPSTR,int);
BOOL ActivatePrevInst();
BOOL RelaunchMultiInst();
BOOL RelaunchElevated();
void SnapToDefaultPos(HWND);
void ShowNotifyIcon(HWND,BOOL);
void SetNotifyIconTitle(HWND);
void InstallFileWatching(LPCWSTR);
void CALLBACK WatchTimerProc(HWND,UINT,UINT_PTR,DWORD);
void CALLBACK PasteBoardTimer(HWND,UINT,UINT_PTR,DWORD);


void LoadSettings();
BOOL SaveSettings(BOOL);
void ParseCommandLine();
void LoadFlags();
int  CheckIniFile(LPWSTR,LPCWSTR);
int  CheckIniFileRedirect(LPWSTR,LPCWSTR);
int  FindIniFile();
int  TestIniFile();
int  CreateIniFile();
int  CreateIniFileEx(LPCWSTR);


void UpdateStatusbar();
void UpdateToolbar();
void UpdateLineNumberWidth();


BOOL FileIO(BOOL,LPCWSTR,BOOL,int*,int*,BOOL*,BOOL*,BOOL*,BOOL);
BOOL FileLoad(BOOL,BOOL,BOOL,BOOL,LPCWSTR);
BOOL FileSave(BOOL,BOOL,BOOL,BOOL);
BOOL OpenFileDlg(HWND,LPWSTR,int,LPCWSTR);
BOOL SaveFileDlg(HWND,LPWSTR,int,LPCWSTR);


LRESULT CALLBACK MainWndProc(HWND,UINT,WPARAM,LPARAM);
LRESULT MsgCreate(HWND,WPARAM,LPARAM);
void    CreateBars(HWND,HINSTANCE);
void    MsgThemeChanged(HWND,WPARAM,LPARAM);
void    MsgSize(HWND,WPARAM,LPARAM);
void    MsgInitMenu(HWND,WPARAM,LPARAM);
#ifdef FEAT_NOTIFY_CHANGE_ON_ACTIVEAPP
void    MsgChangeNotify(HWND,WPARAM,LPARAM);
#endif
LRESULT MsgCommand(HWND,WPARAM,LPARAM);
LRESULT MsgNotify(HWND,WPARAM,LPARAM);

BOOL g_bViewWhiteSpace;
BOOL g_bViewEOLs;
#ifdef FEAT_NOTIFY_CHANGE_ON_ACTIVEAPP
BOOL g_bPendingChangeNotify;
#endif

///   End of Notepad2.h   \\\
