#ifndef L_WINRSRC_H
#define L_WINRSRC_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

HWND l_createdialog(
		HINSTANCE hInstance,
		unsigned char *lpTemplate,
		HWND hWndParent,
		DLGPROC lpDialogFunc,
		LPARAM data );

int l_dialogbox(
		HINSTANCE hInstance,
		unsigned char *lpTemplate,
		HWND hWndParent,
		DLGPROC lpDialogFunc,
		LPARAM data );

#endif

