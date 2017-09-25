#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

//
// The name for the "get" routine can be set in res2c.exe to be whatever
// is desired.
//

unsigned char *l_findrsrcdata( char *name );

static unsigned char* l_get_resource( unsigned char *lpTemplate )
{
	char numbered_resource_name[255];
	unsigned char *rsrc_ptr;

	//
	// See if the resource is specified by a number, or a string.
	//

	if( 0 == HIWORD(lpTemplate) )
	{
		sprintf( numbered_resource_name, "intrsrc%d", LOWORD(lpTemplate) );
		lpTemplate = numbered_resource_name;
	}

	//
	// Get the resource
	//

	rsrc_ptr = l_findrsrcdata( lpTemplate );
	if( NULL == rsrc_ptr )
		SetLastError( ERROR_RESOURCE_NAME_NOT_FOUND );

	return rsrc_ptr;
}

HWND l_createdialog( HINSTANCE hInstance, unsigned char *lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM data )
{
	HWND hWnd;
	unsigned char *rsrc_ptr;

	rsrc_ptr = l_get_resource( lpTemplate );
	if( NULL == rsrc_ptr )
		return NULL;

	//
	// Create the dialog indirectly using this resource.
	//

	hWnd = CreateDialogIndirectParam(
			hInstance,
			(LPDLGTEMPLATE) rsrc_ptr,
			hWndParent,
			lpDialogFunc,
			data );

	return hWnd;
}

int l_dialogbox( HINSTANCE hInstance, unsigned char *lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM data )
{
	int retVal;
	unsigned char *rsrc_ptr;

	rsrc_ptr = l_get_resource( lpTemplate );
	if( NULL == rsrc_ptr )
		return -1;

	//
	// Create the dialog indirectly using this resource.
	//

	retVal = DialogBoxIndirectParam(
			hInstance,
			(LPDLGTEMPLATE) rsrc_ptr,
			hWndParent,
			lpDialogFunc,
			data );

	return retVal;
}
