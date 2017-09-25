#include "lmclient.h"
#include "lmachdep.h"
#include "l_prot.h"
#undef gethostname
l_gethostname(char *x, int y)
{
	return gethostname(x, y);
}

