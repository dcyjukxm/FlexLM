#include "lmachdep.h"
#include "lmclient.h"
#ifdef WINNT
int PASCAL FAR l__gethostname(char FAR * cp, int name_len)
#else
int l_gethostname(char *cp, int name_len)
#endif
{
	char *ret;
	extern char *l_real_getenv();
	if (ret = l_real_getenv("DEBUG_HOSTNAME")) 
	{
		strncpy(cp, ret, name_len);
		return 0;	
	}
	else
	{
		strncpy(cp, "DEBUG_HOSTNAME_not_Set", name_len);
		return -1;
	}
}
