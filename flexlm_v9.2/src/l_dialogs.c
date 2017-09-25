/******************************************************************************

            COPYRIGHT (c) 1988, 2003 by Macrovision Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of 
        Macrovision Corporation and is protected by law.  It may 
        not be copied or distributed in any form or medium, disclosed 
        to third parties, reverse engineered or used in any manner not 
        provided for in said License Agreement except with the prior 
        written authorization from Macrovision Corporation.

 *****************************************************************************/
/*
 *      Module: $Id: l_dialogs.c,v 1.39 2003/05/27 16:59:03 sluu Exp $
 *
 *      Function: l_dialogs.c
 *
 *      Description: Provides Wizard functions for obtaining the license file
 *                      when the user has not specified them
 *      Blane Eisenberg
 *      7/4/97
 *
 *      Last changed:  12/16/98
 *
 */

/*

To Do:
	Disable main dialog when the wait dialog shows??? 
*/

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>

#include "lmachdep.h"
#include "lmclient.h"
#include "l_privat.h"
#include "lm_attr.h"
#include "lmgrrsrc.h"
#include "lmclient.h"
#include "l_winrsrc.h"
#include "flex_utils.h"

//
// Constants
//

#define WM_NEXT_PRESSED WM_USER+1770
#define WM_BACK_PRESSED WM_USER+1771
#define WM_ABOUT_TO_SHOW WM_USER+1772
#define CHOOSE_FIND_PAGE 0
#define FIND_FILE_PAGE 1
#define FIND_SERVER_PAGE 2
#define END_PAGE 3
#define DO_EXIT_PAGE 4
#define ID_FLEXLOCK 13961

//
// Types
//

typedef struct _FindWizData {
	HINSTANCE hInst;
	LM_HANDLE_PTR DialogJob;
	char FLEXlm_Install_Feature[MAX_FEATURE_LEN+1];
	BOOL lcm_enable;
	BOOL demo_enable;
	BOOL skip_locate;
	int exit_code;
	char ReturnFileName[MAX_PATH];
	char button_info[3];
#define FWD_ENABLE 0x001
#define FWD_TEXT 0x002
	char button_prompts[3][50];
} FindWizData;

#define FWD_INIT(x) memset((x),0,sizeof(FindWizData))
#define FWD_SETBUTTON(x,a,y,z) {if(y) (x)->button_info[(a)] |= FWD_ENABLE;    \
				else (x)->button_info[(a)] &= (~FWD_ENABLE);  \
				if(z) {strcpy((x)->button_prompts[(a)], (z)); \
					(x)->button_info[(a)] |= FWD_TEXT;}   \
				else (x)->button_info[(a)] &= (~FWD_TEXT); }
#define FWD_BUTTONSTATE(x,a) ((x)->button_info[(a)] & FWD_ENABLE)
#define FWD_ISBUTTONTEXTSET(x,a) ((x)->button_info[(a)] & FWD_TEXT)
#define FWD_CLEARBUTTONTEXT(x,a) (x)->button_info[(a)] &= (~FWD_TEXT)

#define FWD_CANCEL 2
#define FWD_BACK 1
#define FWD_NEXT 0

//**********************************************************************
//
// Static functions.
//
//
//**********************************************************************

/****************************************************************************/
/**	@brief	Wrapper function for getting dialog text in UTF-8 format.
 *
 *	@param	hDlg		HANDLE of desired dialog box
 *	@param	iID			ID of control
 *	@param	szData		Buffer to put UTF-8 data into
 *	@param	iSize		Size of buffer to put data into
 *
 *	@return	Number of bytes copied into buffer or zero on error.
 ****************************************************************************/
static
DWORD
sGetDlgItemText(
	HWND	hDlg,
	int		iID,
	char *	szData,
	int		iSize)
{
	int			rv = 0;
	int			len = 0;
	char *		pszData = NULL;
	wchar_t *	pwszBuffer = NULL;


	if( (hDlg == NULL) || (iID == 0) || (szData == NULL) || (iSize == 0) )
		goto done;

	/*
	 *	Initialize returned data
	 */
	memset(szData, 0, iSize);

	if(l_getOSFamily() == OS_95_98_ME)
	{
		if(GetDlgItemTextA(hDlg, iID, szData, iSize - 1) == 0)
		{
			szData[0] = '\0';
		}
		else
		{
			/*
			 *	Convert to UTF-8
			 */
			pszData = l_convertStringMBToUTF8(NULL, szData, &len);
			if(pszData == NULL)
			{
				szData[0] = '\0';
			}
			else
			{
				if(len > iSize)
				{
					len = iSize - 1;
				}
				strncpy(szData, pszData, len);
				szData[len] = '\0';
				rv = len;
			}
		}
	}
	else
	{
		/*
		 *	Allocate buffer to hold data
		 */
		pwszBuffer = l_malloc(NULL, sizeof(wchar_t) * iSize);
		if(pwszBuffer)
		{
			if(GetDlgItemTextW(hDlg, iID, pwszBuffer, iSize - 1) == 0)
			{
				szData[0] = '\0';
			}
			else
			{
				/*
				 *	Convert to UTF-8
				 */
				pszData = l_convertStringWCToUTF8(NULL, pwszBuffer, &len);
				if(pszData == NULL)
				{
					szData[0] = '\0';
				}
				else
				{
					if(len > iSize)
					{
						len = iSize - 1;
					}
					strncpy(szData, pszData, len);
					szData[len] = '\0';
					rv = len;
				}
			}
		}
	}
done:
	if(pszData)
	{
		l_free(pszData);
		pszData = NULL;
	}
	if(pwszBuffer)
	{
		l_free(pwszBuffer);
		pwszBuffer = NULL;
	}
	return rv;
}

static
BOOL
FileDialog(HWND parent, char * SelectedFileName)
{
	static OPENFILENAME ofn;

	char FileName[_MAX_PATH + 1] = {'\0'};
	char FileTitle [_MAX_PATH + 1] = {'\0'};
	BOOL status;
	static char szFilter[] = "License Files(*.dat,*.lic)\0*.dat;*.lic\0"
				"All Files(*.*)\0*.*\0\0";

	ofn.lStructSize       = sizeof (OPENFILENAME);;
	ofn.hwndOwner         = parent; //hdlg;
	ofn.hInstance         = NULL;
	ofn.lpstrFilter       = szFilter;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 0;
	ofn.nMaxFile          = _MAX_PATH;
	ofn.nMaxFileTitle     = _MAX_PATH; //_MAX_FNAME + _MAX_EXT ;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrTitle        = "Choose License File"; //NULL;
	ofn.nFileOffset       = 0;
	ofn.nFileExtension    = 0;
	ofn.lpstrDefExt       = "txt";
	ofn.lCustData         = 0;
	ofn.lpfnHook          = NULL;
	ofn.lpTemplateName    = NULL;
	ofn.lpstrFile         = &FileName[0];
	ofn.lpstrFileTitle    = FileTitle;
	ofn.Flags             = OFN_HIDEREADONLY | OFN_CREATEPROMPT | OFN_NOCHANGEDIR;

	memset( FileName,0,sizeof(FileName) );
	memset( FileTitle,0,sizeof(FileTitle) );

	status = GetOpenFileName (&ofn);

	if( status )
		strcpy(SelectedFileName,FileName);

	return status;
}

static
void 
SetButtonCheck (HWND hwndDlg, int CtrlID, BOOL bCheck)
{
	HWND hwndCtrl = GetDlgItem (hwndDlg, CtrlID) ;
	if (bCheck)
	{
		Button_SetCheck (hwndCtrl, BST_CHECKED) ;
	}
	else
	{
		Button_SetCheck (hwndCtrl, BST_UNCHECKED) ;
	}

}

static
int
QueryButtonCheck (HWND hwndDlg, int count, ...)
{
	BOOL found;
	int cntrlId, i;
	va_list vArgs;

	va_start( vArgs, count );

	for( i = 0, found = FALSE; i < count; i++ )
	{
		HWND hwndCtrl;

		cntrlId = va_arg( vArgs, int );
		hwndCtrl = GetDlgItem (hwndDlg, cntrlId);
		if( hwndCtrl && (BST_CHECKED == Button_GetCheck( hwndCtrl ))
			&& (WS_VISIBLE & GetWindowLong( hwndDlg, GWL_STYLE )) )
		{
			found = TRUE;
			break;
		}
	}

	if( !found )
		cntrlId = 0;

	return cntrlId;
}

//
// The event processing loop for the first page of the dialog.
//

static
BOOL CALLBACK 
ChooseDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static FindWizData *wiz_data_ptr;
	BOOL msg_used = TRUE;

    switch (msg)
    {
        case WM_INITDIALOG: {
			HWND hwndCtrl;
			RECT r,s;
			POINT p;
			int default_button;

			wiz_data_ptr = (FindWizData*) lParam;

			SetButtonCheck (hwndDlg, IDC_LFILE, 0) ;
			SetButtonCheck (hwndDlg, IDC_SERVER, 0) ;
			SetButtonCheck (hwndDlg, IDC_WEBKEY, 0) ;
			SetButtonCheck (hwndDlg, IDC_FLEXLOCK, 0) ;
			default_button = IDC_WEBKEY;

			if ( !wiz_data_ptr->skip_locate )
				default_button = IDC_SERVER;
			else
			{
				hwndCtrl = GetDlgItem ( hwndDlg, IDC_LFILE );
				GetWindowRect( hwndCtrl,&r );
		 		EnableWindow( hwndCtrl, FALSE );

				hwndCtrl = GetDlgItem (hwndDlg, IDC_SERVER );
				GetWindowRect(hwndCtrl,&r);
		 		EnableWindow( hwndCtrl, FALSE );
			}
		
			if ( !wiz_data_ptr->lcm_enable )
			{
				hwndCtrl = GetDlgItem (hwndDlg, IDC_WEBKEY) ;
				GetWindowRect(hwndCtrl,&r);
		 		ShowWindow(hwndCtrl,SW_HIDE);

		 		hwndCtrl = GetDlgItem (hwndDlg, IDC_FLEXLOCK) ;
				GetWindowRect(hwndCtrl,&s);
				p.x=r.left; p.y=r.top;
				ScreenToClient(hwndDlg,&p);
				MoveWindow(hwndCtrl,p.x,p.y, s.right-s.left, s.bottom - s.top ,TRUE );

				if( IDC_WEBKEY == default_button )
					default_button = IDC_FLEXLOCK;
			}

			if ( !wiz_data_ptr->demo_enable )
			{
				hwndCtrl = GetDlgItem (hwndDlg, IDC_FLEXLOCK) ;
				GetWindowRect(hwndCtrl,&r);
		 		ShowWindow(hwndCtrl,SW_HIDE);

				//
				// It should be possible for this to be true since
				// it would mean there were no options available.
				//

				if( IDC_FLEXLOCK == default_button )
					default_button = IDC_SERVER;
			}

			SetButtonCheck (hwndDlg, default_button, 1);
                
			msg_used = FALSE;  // SetFocus() was not called.
			break;
        }

		case WM_ABOUT_TO_SHOW:
			FWD_SETBUTTON( wiz_data_ptr, FWD_CANCEL, TRUE, NULL );
			FWD_SETBUTTON( wiz_data_ptr, FWD_BACK,   FALSE, NULL );
			FWD_SETBUTTON( wiz_data_ptr, FWD_NEXT,   TRUE, "Next>" );
			break;

        case WM_COMMAND :
        {
			WORD wNotifyCode = HIWORD (wParam);
			WORD wID = LOWORD (wParam);

			switch (wID)
			{ 
				case IDC_LFILE :
					SetButtonCheck (hwndDlg, IDC_LFILE, 1) ;
					SetButtonCheck (hwndDlg, IDC_SERVER, 0) ;
					SetButtonCheck (hwndDlg, IDC_WEBKEY, 0) ;
					SetButtonCheck (hwndDlg, IDC_FLEXLOCK, 0) ;
					break;

				case IDC_SERVER :
					SetButtonCheck (hwndDlg, IDC_LFILE, 0) ;
					SetButtonCheck (hwndDlg, IDC_SERVER, 1) ;
					SetButtonCheck (hwndDlg, IDC_WEBKEY, 0) ;
					SetButtonCheck (hwndDlg, IDC_FLEXLOCK, 0) ;
					break;

				case IDC_WEBKEY :
					SetButtonCheck (hwndDlg, IDC_LFILE, 0) ;
					SetButtonCheck (hwndDlg, IDC_SERVER, 0) ;
					SetButtonCheck (hwndDlg, IDC_WEBKEY, 1) ;
					SetButtonCheck (hwndDlg, IDC_FLEXLOCK, 0) ;
					break;

				case IDC_FLEXLOCK :
					SetButtonCheck (hwndDlg, IDC_LFILE, 0) ;
					SetButtonCheck (hwndDlg, IDC_SERVER, 0) ;
					SetButtonCheck (hwndDlg, IDC_WEBKEY, 0) ;
					SetButtonCheck (hwndDlg, IDC_FLEXLOCK, 1) ;
					break;
			}
			break;
		}

        case WM_HELP:
			break;

	//
	// This is actually a message passed to us directly (i.e. not through
	// the windows dispatching code but rather we're being called right
	// from another of our routines).  We respond TRUE if we want to go
	// on and put the page number into LPARAM (pointer to an int).
	//

        case WM_NEXT_PRESSED:
        {
			int *next_page_ptr = (int*) lParam;
			int status;

			switch( QueryButtonCheck( hwndDlg, 4, IDC_LFILE,
					IDC_SERVER, IDC_WEBKEY, IDC_FLEXLOCK ) )
			{
			case IDC_LFILE:
				*next_page_ptr = FIND_FILE_PAGE;
				msg_used = TRUE;
				break;

			case IDC_SERVER:
				*next_page_ptr = FIND_SERVER_PAGE;
				msg_used = TRUE;
				break;

			case IDC_WEBKEY:
			{
				char* UrlString;

				//
				// Run the LCM.
				//

				l_get_attr( wiz_data_ptr->DialogJob, LM_A_LCM_URL, &UrlString );
				EnableWindow( GetParent( hwndDlg ), FALSE );
							status = lcm_executefulfill(
						wiz_data_ptr->DialogJob,
						wiz_data_ptr->FLEXlm_Install_Feature,
						UrlString,
						wiz_data_ptr->ReturnFileName );
				EnableWindow( GetParent( hwndDlg ), TRUE );

				//
				// If the operation either failed or was canceled, we
				// won't move off this dialog.
				//

				if( 0 == status )
					msg_used = FALSE;
				else
				{
					*next_page_ptr = END_PAGE;
					msg_used = TRUE;
					wiz_data_ptr->exit_code = IDOK;

					//
					// When using LCM, we don't allow the user to
					// go backwards from the end.
					//

					FWD_SETBUTTON( wiz_data_ptr, FWD_BACK,   FALSE, NULL );
				}

				break;
			}

			case IDC_FLEXLOCK:
				*next_page_ptr = END_PAGE;
				msg_used = TRUE;
				wiz_data_ptr->exit_code = ID_FLEXLOCK;
				break;

			} // End switch on QueryButtons
			break;

        } // End case WM_NEXT_PRESSED

		case WM_BACK_PRESSED:
			//
			// Can't ever go back from here.
			//
			msg_used = FALSE;
			break;


        default :
			msg_used = FALSE;
	}

	return msg_used;
}

//
// Processor for File Panel Dialog
//

static
BOOL CALLBACK 
FileDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static FindWizData *wiz_data_ptr;
	char SelectedFileName[MAX_PATH];
	BOOL msg_used = TRUE;
	int *next_page_ptr;

	switch (msg)
	{
	case WM_INITDIALOG:
		wiz_data_ptr = (FindWizData*) lParam;
		msg_used = FALSE;  // No SetFocus();
		break;

	case WM_ABOUT_TO_SHOW:
		FWD_SETBUTTON( wiz_data_ptr, FWD_BACK, TRUE, NULL );
		FWD_SETBUTTON( wiz_data_ptr, FWD_NEXT, TRUE, "Next>" );
		break;

	case WM_COMMAND:
	{
		WORD wNotifyCode = HIWORD (wParam);
		WORD wID = LOWORD (wParam);

		if( BN_CLICKED != wNotifyCode )	// Only care if they press a button.
		{
			msg_used = FALSE;
			break;
		}

		switch (wID)
		{
			case IDC_BROWSE:

				//
				// Put up a file dialog.  If the user enters one,
				// then put it back into the display.
				//

				if( FileDialog( hwndDlg, &SelectedFileName[0] ) )
									 SetDlgItemText( hwndDlg, IDC_FILE, SelectedFileName );
				break ;
		}
		break;
	}

    case WM_HELP:
		break;

	case WM_NEXT_PRESSED:
		//
		// Make sure something was entered.
		//

		if(sGetDlgItemText(hwndDlg, IDC_FILE, SelectedFileName, sizeof(SelectedFileName)) == 0)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			msg_used = FALSE;
			break;
		}
		strcpy( wiz_data_ptr->ReturnFileName, SelectedFileName );
		next_page_ptr = (int*) lParam;
		msg_used = TRUE;
		*next_page_ptr = END_PAGE;
		wiz_data_ptr->exit_code = IDOK;
		break;

	case WM_BACK_PRESSED:
		//
		// Just go back to the start page.
		//

		next_page_ptr = (int*) lParam;
		msg_used = TRUE;
		*next_page_ptr = CHOOSE_FIND_PAGE;
		break;

        default:
                msg_used = FALSE;
	}

	return msg_used;
}

static
BOOL CALLBACK
NullDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	return FALSE;
}

static
BOOL CALLBACK 
ServerDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static FindWizData *wiz_data_ptr;
	BOOL msg_used = TRUE;

	switch (msg)
	{
	case WM_INITDIALOG:
		wiz_data_ptr = (FindWizData*) lParam;
		msg_used = FALSE;  // SetFocus() not called.
		break;

	case WM_ABOUT_TO_SHOW:
		FWD_SETBUTTON( wiz_data_ptr, FWD_BACK, TRUE, NULL );
		FWD_SETBUTTON( wiz_data_ptr, FWD_NEXT, TRUE, "Next>" );
		break;

	case WM_HELP:
		break;

        case WM_NEXT_PRESSED:
	{
		int *next_page_ptr = (int*) lParam;
		char server_name[MAX_PATH] = {'\0'};
		char *host_name;
		int status;
		char *errmsg;
		HWND hWaitWin;


		if(sGetDlgItemText(hwndDlg, IDC_SERVER, server_name, sizeof(server_name)) == 0)
		{
			MessageBeep(MB_ICONEXCLAMATION);
			msg_used = FALSE;
			break;
		}

		hWaitWin = l_createdialog(
				wiz_data_ptr->hInst,
				"FLEXLM_WAITPING",
				GetParent( hwndDlg ),
				NullDlgProc,
				(LPARAM) NULL );

		if( NULL == hWaitWin )
		{
			MessageBox(
				GetActiveWindow(),
				"Error at CreateDialog",
 				"FLEXlm License Manager",
				MB_OK | MB_ICONSTOP | 
				MB_APPLMODAL | MB_SETFOREGROUND );        
			break;;
		}

		//
		// Check if port@host was specified
		// if so, then strip off the port and @ number when
		// pinging
		//

		host_name = strchr( server_name, '@' );
                        
		if (host_name) 
			host_name++;
		else
 			host_name = server_name;

		status = l_do_ping_operation( host_name, &errmsg );

		//
		// Remove the dialog that announces the 5 second delay
		//

		EndDialog( hWaitWin, IDOK );

		if (status)
		{
			//
			// No error occured so clean up the name and
			// prepend @ if necessary
			//

			host_name = strchr(server_name,'@');
                        
			if (!host_name) // Prepend a @ sign
			{
				char hold[MAX_PATH];

				strcpy( hold, "@" );
				strcat( hold, server_name );
				strcpy( server_name, hold );
                        }

			SetDlgItemText( hwndDlg, IDC_SERVER, server_name );
			strcpy( wiz_data_ptr->ReturnFileName, server_name );

			msg_used = TRUE;
			*next_page_ptr = END_PAGE;
			wiz_data_ptr->exit_code = IDOK;
		}
		else    // There was an error, display it
		{
 			MessageBox( GetActiveWindow(),
				errmsg,
				"FLEXlm License Manager",
				MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND );

			msg_used = FALSE;
		}
		break;
	}  // End WM_NEXT_PRESSED

	case WM_BACK_PRESSED:
	{
		int *next_page_ptr = (int*) lParam;

		//
		// Just go back to the start page.
		//
		next_page_ptr = (int*) lParam;
		msg_used = TRUE;
		*next_page_ptr = CHOOSE_FIND_PAGE;
		break;
	}

	default:
		msg_used = FALSE;
		break;
	}

	return msg_used;
}

static BOOL CALLBACK 
EndDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static FindWizData *wiz_data_ptr;
	BOOL msg_used = TRUE;
	int *next_page_ptr;

	switch (msg)
	{
	case WM_INITDIALOG:
		wiz_data_ptr = (FindWizData*) lParam;
		msg_used = FALSE;  // SetFocus() not called.
		break;

	case WM_ABOUT_TO_SHOW:
		FWD_SETBUTTON( wiz_data_ptr, FWD_CANCEL, FALSE, NULL );
		FWD_SETBUTTON( wiz_data_ptr, FWD_NEXT,   TRUE, "Finish" );
		break;

	case WM_HELP:
		break;

	case WM_NEXT_PRESSED:
		//
		// Go to the exit page.  This just triggers
		// an end dialog.
		//

		next_page_ptr = (int*) lParam;
		msg_used = TRUE;
		*next_page_ptr = DO_EXIT_PAGE;
		break;

	case WM_BACK_PRESSED:
		//
		// Just go back to the start page.
		//

		next_page_ptr = (int*) lParam;
		msg_used = TRUE;
		*next_page_ptr = CHOOSE_FIND_PAGE;
		break;

	default:
		msg_used = FALSE;
		break;
        }

	return msg_used;
}

static
void
CenterChildInWindow( HWND parent, HWND child )
{
	RECT rect;
	RECT childRect;
	POINT point;
	int indentation;
	BOOL opOK;

	//
	// Figure out the size of what we're moving and how big
	// an area we've got to put it in.  Note we need to convert
	// the child's rectangle from screen coordinates to coordinates
	// in it's parents window.
	//

	GetClientRect( parent, &rect );

	GetWindowRect( child, &childRect );
	point.x = childRect.left;
	point.y = childRect.top;
	ScreenToClient( parent, &point );
	childRect.left = point.x;
	childRect.top = point.y;

	point.x = childRect.right;
	point.y = childRect.bottom;
	ScreenToClient( parent, &point );
	childRect.right = point.x;
	childRect.bottom = point.y;

	//
	// If the child window is not as wide as the parent, we'll
	// indent it so that it's centered.
	//

	indentation = ((rect.right - rect.left)
			- (childRect.right - childRect.left)) / 2;
	if( 0 > indentation )
		indentation = 0;

	//
	// Move the window so it's centered in the tab
	// area.
	//

	opOK = MoveWindow(
		child,
		childRect.left + indentation,
		childRect.top,
		(childRect.right - childRect.left),
		childRect.bottom - childRect.top,
		FALSE );
}

//
// Wizard page dispatch table.  It's defined here so it
// lives after the routines which it catalogs.
//

static struct WizPageInfo {
	BOOL (CALLBACK * event_proc)(HWND,UINT,WPARAM,LPARAM);
	unsigned char *window_name;
	HWND hWnd;
	BOOL aligned;
} WizardPages[] = {
	{ ChooseDlgProc,   "FLEXLM_CHOOSE",  NULL, FALSE },
	{ FileDlgProc,     "FLEXLM_FILE",    NULL, FALSE },
	{ ServerDlgProc,   "FLEXLM_SERVER",  NULL, FALSE },
	{ EndDlgProc,      "FLEXLM_END",     NULL, FALSE }
};
#define NUM_WIZ_PAGES (sizeof(WizardPages)/sizeof(struct WizPageInfo))
static int ActiveWizPage = -1;

static struct WizCntrlInfo {
	int rsrc_id;
	int button_id;
} WizCntrls[] = {
	{ IDCANCEL, FWD_CANCEL },
	{ IDC_NEXT, FWD_NEXT },
	{ IDC_BACK, FWD_BACK }
};
#define NUM_CONTROLS (sizeof(WizCntrls)/sizeof(struct WizCntrlInfo))

//
// Shows a child window in the wizard.
//

static
void
ShowWizWin( HWND hWnd, FindWizData *wiz_data_ptr, int page )
{
	HWND hchild_win;
	int i;

	//
	// If the page is -99, that's a signal to
	// reset things.
	//

	if( -99 == page )
	{
		ActiveWizPage = -1;
		for( i = 0; i < NUM_WIZ_PAGES; i++ )
		{
			WizardPages[i].hWnd = NULL;
			WizardPages[i].aligned = FALSE;
		}
		return;
	}

	//
	// If the commanded page is greater than the
	// last page, it means to exit.
	//

	if( NUM_WIZ_PAGES <= page )
	{
		EndDialog( hWnd, wiz_data_ptr->exit_code );
		return;
	}

	//
	// Boundary condition checks.
	//

	if( (0 > page) || (NUM_WIZ_PAGES <= page) )
		return;

	//
	// Hide the current window if we have one.
	//

	if( (0 <= ActiveWizPage) || (NUM_WIZ_PAGES > ActiveWizPage) )
		ShowWindow( WizardPages[ActiveWizPage].hWnd, SW_HIDE );

	//
	// Make the new page the active page and make it visible.
	// Note we align it's position before we show it should it
	// need it.  Also note we let the upcoming page a chance
	// to modify the global data structure.
	//

	ActiveWizPage = page;
	if( !WizardPages[ActiveWizPage].aligned )
	{
		CenterChildInWindow( hWnd, WizardPages[ActiveWizPage].hWnd );
		WizardPages[ActiveWizPage].aligned = TRUE;
	}

	(*WizardPages[ActiveWizPage].event_proc)(
				WizardPages[ActiveWizPage].hWnd,
				WM_ABOUT_TO_SHOW,
				0, 0 );
	// set default focus item
	//SetFocus(GetDlgItem(WizardPages[ActiveWizPage].hWnd, IDC_NEXT));
	
	ShowWindow( WizardPages[ActiveWizPage].hWnd, SW_SHOW );

	//
	// Change the button states as desired.
	//

	for( i = 0; i < NUM_CONTROLS; i++ )
	{
		hchild_win = GetDlgItem( hWnd, WizCntrls[i].rsrc_id );
		if( hchild_win )
		{
			EnableWindow( hchild_win,
				FWD_BUTTONSTATE( wiz_data_ptr,
						WizCntrls[i].button_id ) );

			if( FWD_ISBUTTONTEXTSET( wiz_data_ptr,
						WizCntrls[i].button_id ) )
			{
				SetWindowText( hchild_win,
					wiz_data_ptr->button_prompts[WizCntrls[i].button_id] );
				FWD_CLEARBUTTONTEXT( wiz_data_ptr,
					WizCntrls[i].button_id );
			}
		}
	}
}	

//
// Main control routine for the wizard.
//

static
BOOL CALLBACK 
WizDlgProc (HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static FindWizData *wiz_data_ptr;
	BOOL used;
	int i;

	used = FALSE;
	switch( msg )
	{
	case WM_INITDIALOG:
		//
		// Save the wizard data pointer when it comes in on
		// this message.
		//

		wiz_data_ptr = (FindWizData*) lParam;

		//
		// Initialize all the buttons to be enabled with their
		// default text.
		//

		FWD_SETBUTTON( wiz_data_ptr, FWD_CANCEL, TRUE, NULL );
		FWD_SETBUTTON( wiz_data_ptr, FWD_NEXT,   TRUE, NULL );
		FWD_SETBUTTON( wiz_data_ptr, FWD_BACK,   TRUE, NULL );

		//
		// Create each sub page and make the first one
		// visible on our window.
		//

		ShowWizWin( hwndDlg, wiz_data_ptr, -99 ); // Resets page info.

		for( i = 0; i < NUM_WIZ_PAGES; i++ )
			WizardPages[i].hWnd = l_createdialog(
						wiz_data_ptr->hInst,
						WizardPages[i].window_name,
						hwndDlg,
						WizardPages[i].event_proc,
						(LPARAM) wiz_data_ptr );
		ShowWizWin( hwndDlg, wiz_data_ptr, 0 );
		break;

	case WM_COMMAND:
	{
		WORD cntrlId = LOWORD( wParam );
		WORD cmd = HIWORD( wParam );
		int next_page;
		BOOL go;

		if( BN_CLICKED != cmd )	// Only care about buttons.
			break;

		switch( cntrlId )
		{
		case IDCANCEL:
			used = TRUE;
			EndDialog( hwndDlg, IDCANCEL );
			break;

		case IDC_NEXT:
			used = TRUE;
			go = WizardPages[ActiveWizPage].event_proc(
					WizardPages[ActiveWizPage].hWnd,
					WM_NEXT_PRESSED,
					0,
					(LPARAM) &next_page );
			if( go )
				ShowWizWin( hwndDlg, wiz_data_ptr, next_page );
			break;

		case IDC_BACK:
			used = TRUE;
			go = WizardPages[ActiveWizPage].event_proc(
					WizardPages[ActiveWizPage].hWnd,
					WM_BACK_PRESSED,
					0,
					(LPARAM) &next_page );
			if( go )
			{
				ShowWizWin( hwndDlg, wiz_data_ptr, next_page );
				SetFocus(GetDlgItem(hwndDlg, IDC_NEXT));	// forces focus back to popup
			}
			break;
		}

		break;
	}

	default:
		used = FALSE;
		break;
	}

	return used;
}

//
// Initialization function for Wizard
//

static
int
DoFlexlmWizard (HWND hwndParent, FindWizData *wiz_data_ptr )
{
	int retVal;

	InitCommonControls();

	retVal = l_dialogbox(
			wiz_data_ptr->hInst,
			"FLEXLM_LICENSE_FINDER",
			hwndParent,
			WizDlgProc,
			(LPARAM) wiz_data_ptr );

	return retVal;
}

//**********************************************************************
//
// Public API.
//
//**********************************************************************

int
l_install_conf(LM_HANDLE_PTR job, CONFIG* c)
{   
	static VENDORCODE l_temp_code;
	CONFIG *t; /* have to append to end of job->line */

	for (t = job->line; t && t->next ; t = t->next)
                ;
	if (t)
		t->next = c;
	else
		job->line = c;

	l_featon(job, c->feature, c->version, 1, &l_temp_code, c );
	l_set_attr( job, LM_A_CHECK_INTERVAL, -1);
	l_set_attr( job, LM_A_RETRY_COUNT, -1);

	return 0;
}

int
l_prompt_lf(LM_HANDLE_PTR job, char* feature, char* szFile,BOOL demo_enabled, BOOL skip_locate)
{
    HANDLE hinst;
    HWND hwnd;
    int ret;
	FindWizData DialogData;
	short lcm_enable;

#ifdef FLEX_STATIC
	l_get_attr( job, LM_A_WINDOWS_MODULE_HANDLE, &hinst );
	if (!hinst)
		hinst = GetModuleHandle (NULL);
#else
	hinst = GetModuleHandle("LMGR327B");
#endif

/*
 *	Initialize the dialog data block.  This will get passed around
 *	to all components of the find dialog so they can share information.
 */

	FWD_INIT( &DialogData );
	DialogData.hInst = hinst;
	strcpy( DialogData.FLEXlm_Install_Feature, feature );
	DialogData.DialogJob = job;

	l_get_attr(job, LM_A_LCM , &lcm_enable );
	if ( lcm_enable && lcm_fulfillavailable(job) )
		DialogData.lcm_enable = TRUE;
	else
		DialogData.lcm_enable= FALSE;

	DialogData.demo_enable = demo_enabled;
	DialogData.skip_locate = skip_locate;

	DialogData.ReturnFileName[0] = 0;

/*
 *	Check to see if anything needs to be displayed in the dialog, if not, just
 *	return
 */
	if ( !demo_enabled && !DialogData.lcm_enable && skip_locate )
		return 0;

	hwnd = GetFocus();
	ret = DoFlexlmWizard( GetFocus(), &DialogData );

/*
 *	Convert from the dialog return to the FLEXlm return.
 */

	switch (ret)
	{
	case IDCANCEL:
		ret = 0;
		break;

	case IDOK:
		if (DialogData.ReturnFileName[0])
			strcpy( szFile, DialogData.ReturnFileName );
		ret = 1;
		break;

	case ID_FLEXLOCK:
		ret = 2;
		break;

	default:
		ret = 0;
		break;
  	}
	
        return ret;
}

int
l_res_ok(LM_HANDLE_PTR job)
{
	return 1;
}

//**********************************************************************
//
//
//
//**********************************************************************

void l_update_lm_license(void * job,char *szFile1,char * vendor_name)
{
	if ( l_update_license_path(job,szFile1,vendor_name,"06.0"))
		lc_perror(job,"Error in saving configuration settings");
}




