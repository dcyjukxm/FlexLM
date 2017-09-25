#include "\library\common\basic.h"
#include <winreg.h>
//#include <stdlib.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int ValidateProductSuite(char *suiteName) //Adapted from Microsoft function.
{
   int validated=0;
   long result;
   HKEY hKey=NULL;
   DWORD type=0;
   DWORD size=0;
   char *productSuites=NULL;
   char *suite;
   result=RegOpenKey(HKEY_LOCAL_MACHINE,"System\\CurrentControlSet\\Control\\ProductOptions",&hKey);
   if (result != ERROR_SUCCESS)
      goto done;
   result=RegQueryValueEx(hKey,"ProductSuite",NULL,&type,NULL,&size);
   if (result != ERROR_SUCCESS || !size)
      goto done;
   productSuites=new char[size];
   result=RegQueryValueEx(hKey,"ProductSuite",NULL,&type,(LPBYTE)productSuites,&size);
   if (result != ERROR_SUCCESS || type != REG_MULTI_SZ)
      goto done;
   suite=productSuites;
   while (*suite)
   {
      if (strcmp(suite,suiteName)==0)
      {
         validated=TRUE;
         break;
      }
      suite+=strlen(suite)+1;
   }
done:
   if (productSuites)
      delete productSuites;
   if (hKey)
      RegCloseKey(hKey);
   return validated;
}

int WinframeIsRegistered(void)
{
   int validated=0;
   long result;
   HKEY hKey=NULL;
   result=RegOpenKey(HKEY_LOCAL_MACHINE,"System\\CurrentControlSet\\Control\\Citrix",&hKey);
   if (result == ERROR_SUCCESS)
      validated=TRUE;
   if (hKey)
      RegCloseKey(hKey);
   return validated;
}

int IsHydra(void)
{
   if (ValidateProductSuite("Terminal Server"))
      return TRUE;
   if (getenv("WINSTATIONNAME")) //Check for Citrix WinFRAME for NT 3.51.
      return TRUE;
   if (WinframeIsRegistered())
      return TRUE;
   return FALSE;
}

