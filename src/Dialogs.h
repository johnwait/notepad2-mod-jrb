/******************************************************************************
*
*
* Notepad2
*
* Dialogs.h
*   Definitions for Notepad2 dialog boxes
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


#define MBINFO         0
#define MBWARN         1
#define MBYESNO        2
#define MBYESNOWARN    3
#define MBYESNOCANCEL  4
#define MBFATAL        5
#define MBOKCANCEL     6
#define MBOKCANCELWARN 7

#define MB_TITLE_MAXLEN  64
#define MB_MSG_MAXLEN  1024

int _MsgBoxW(int iType, UINT uIdMsg, ...);
#ifdef FEAT_REPLACE_MSGBOX_BY_TASKDLG
int TaskBox(int iMBType, UINT uIdMsg, ...);
inline int MsgBoxW(int iType, UINT uIdMsg, ...) { return TaskBox(iType, uIdMsg); };
#else
inline int MsgBoxW(int iType, UINT uIdMsg, ...) { return _MsgBoxW(iType, uIdMsg); };
#endif
void DisplayCmdLineHelp(HWND hwnd);
BOOL GetDirectory(HWND,int,LPTSTR,LPCTSTR,BOOL);
INT_PTR CALLBACK AboutDlgProc(HWND,UINT,WPARAM,LPARAM);
void RunDlg(HWND,LPCTSTR);
BOOL OpenWithDlg(HWND,LPCTSTR);
BOOL FavoritesDlg(HWND,LPTSTR);
BOOL AddToFavDlg(HWND,LPCTSTR,LPCTSTR);
BOOL FileMRUDlg(HWND,LPTSTR);
BOOL ChangeNotifyDlg(HWND);
BOOL ColumnWrapDlg(HWND,UINT,int *);
BOOL WordWrapSettingsDlg(HWND,UINT,int *);
BOOL LongLineSettingsDlg(HWND,UINT,int *);
BOOL TabSettingsDlg(HWND,UINT,int *);
BOOL SelectDefEncodingDlg(HWND,int *);
BOOL SelectEncodingDlg(HWND,int *);
BOOL RecodeDlg(HWND,int *);
BOOL SelectDefLineEndingDlg(HWND,int *);
INT_PTR InfoBox(int,LPCTSTR,int,...);

#ifdef FEAT_REPLACE_MSGBOX_BY_TASKDLG

#define TASKDLGBOX_STR_MAXLEN		1024
#define TASKDLGBOX_ERR_OUTOFMEMORY	(-1)
#define TASKDLGBOX_ERR_INVALIDARG	(-2)
#define TASKDLGBOX_ERR_FAIL			(-3)
#define TASKDLGBOX_ERR_UNKNOWN		(-4)

/*
 * Signature for TASKDIALOG_BUTTON:
 *
 * typedef struct _TASKDIALOG_BUTTON {
 *     int     nButtonID;
 *     PCWSTR  pszButtonText;
 * } TASKDIALOG_BUTTON;
 *
 */

typedef struct _TASKDIALOGBOX_BTN_ARRAY {
	int			             nBtnCount;
	const TASKDIALOG_BUTTON* ptdbButtons;
	int                      nDefaultBtn;
} TASKDIALOGBOX_BTN_ARRAY, *PTASKDIALOGBOX_BTN_ARRAY;

#define TASKDLGBOX_1BTN_WITHIDS (btn1Text, btn1Id)                                                       { { (btn1Id), (btn1Text) } }
#define TASKDLGBOX_2BTNS_WITHIDS(btn1Text, btn1Id, btn2Text, btn2Id)                                     { { (btn1Id), (btn1Text) }, { (btn2Id), (btn2Text) } }
#define TASKDLGBOX_3BTNS_WITHIDS(btn1Text, btn1Id, btn2Text, btn2Id, btn3Text, btn3Id)                   { { (btn1Id), (btn1Text) }, { (btn2Id), (btn2Text) }, { (btn3Id), (btn3Text) } }
#define TASKDLGBOX_4BTNS_WITHIDS(btn1Text, btn1Id, btn2Text, btn2Id, btn3Text, btn3Id, btn4Text, btn4Id) { { (btn1Id), (btn1Text) }, { (btn2Id), (btn2Text) }, { (btn3Id), (btn3Text) }, { (btn4Id), (btn4Text) }  }
#define TASKDLGBOX_1BTN (btn1Text)                               TASKDLGBOX_1BTN_WITHIDS (btn1Text, 1)
#define TASKDLGBOX_2BTNS(btn1Text, btn2Text)                     TASKDLGBOX_2BTNS_WITHIDS(btn1Text, 1, btn2Text, 2)
#define TASKDLGBOX_3BTNS(btn1Text, btn2Text, btn3Text)           TASKDLGBOX_3BTNS_WITHIDS(btn1Text, 1, btn2Text, 2, btn3Text, 3)
#define TASKDLGBOX_4BTNS(btn1Text, btn2Text, btn3Text, btn4Text) TASKDLGBOX_4BTNS_WITHIDS(btn1Text, 1, btn2Text, 2, btn3Text, 3, btn4Text, 4)

#define TASKDLGBOX_BTNARRAY(defaultBtn, pbtnArray) \
	(TASKDIALOGBOX_BTN_ARRAY){ ARRAYSIZE(*(pbtnArray)), pbtnArray, (defaultBtn > 0 && defaultBtn <= ARRAYSIZE(*(pbtnArray)) ? defaultBtn : 1) }

typedef struct _TASKDIALOGBOX_XTRA_RETURN {
	int		nSelectedRadioBtn;
	BOOL	bVerifFlagChecked;
} TASKDIALOGBOX_XTRA_RETURN, *PTASKDIALOGBOX_XTRA_RETURN;

int TaskDialogEx(PCWSTR                         pwszIcon,
				 PTASKDIALOGBOX_XTRA_RETURN     xtraReturnVars,
				 PCWSTR                         pwszMainInstr,
				 PCWSTR                         pwszContent,
				 PTASKDIALOGBOX_BTN_ARRAY       ptdbaBtns, 
				 PTASKDIALOGBOX_BTN_ARRAY       ptdbaRadioBtns,
				 TASKDIALOG_COMMON_BUTTON_FLAGS dwCmnBtnFlags,
				 TASKDIALOG_FLAGS               dwFlags,
				 PCWSTR                         pwszVerificationText,
				 PCWSTR                         pwszExpandedInformation,
				 PCWSTR                         pwszExpandedControlText,
				 PCWSTR                         pwszCollapsedControlText,
				 PCWSTR                         pwszFooter,
				 UINT                           cxDialogWidth
);

inline int SimpleTaskDlg(PCWSTR                         pwszIcon,
						 PTASKDIALOGBOX_XTRA_RETURN     xtraReturnVars,
						 PCWSTR                         ppwszMainInstr,
						 PTASKDIALOGBOX_BTN_ARRAY       ptdbaBtns,
						 TASKDIALOG_COMMON_BUTTON_FLAGS dwCmnBtnFlags,
						 TASKDIALOG_FLAGS               dwFlags) {
	return TaskDialogEx(pwszIcon, xtraReturnVars, ppwszMainInstr, 0, ptdbaBtns, 0, dwCmnBtnFlags, dwFlags, 0, 0, 0, 0, 0, 0);
}

inline int SimpleTaskDlgWithRadios(PCWSTR                         pwszIcon,
								   PTASKDIALOGBOX_XTRA_RETURN     xtraReturnVars,
								   PCWSTR                         ppwszMainInstr,
								   PTASKDIALOGBOX_BTN_ARRAY       ptdbaBtns,
								   PTASKDIALOGBOX_BTN_ARRAY       ptdbaRadioBtns,
								   TASKDIALOG_COMMON_BUTTON_FLAGS dwCmnBtnFlags,
								   TASKDIALOG_FLAGS               dwFlags) {
	return TaskDialogEx(pwszIcon, xtraReturnVars, ppwszMainInstr, 0, ptdbaBtns, ptdbaRadioBtns, dwCmnBtnFlags, dwFlags, 0, 0, 0, 0, 0, 0);
}

#endif // FEAT_REPLACE_MSGBOX_BY_TASKDLG

// End of Dialogs.h
