#define LM_INTERNAL
#include "lmachdep.h"
#include "lmclient.h"
#include "lm_comm.h"
#include "lm_attr.h"
#include "l_prot.h"
#include "lm_code.h"

#ifdef PC
#define LICPATH "@localhost;license.dat;."
#else
#define LICPATH "@localhost:license.dat:."
#endif /* PC */

LM_CODE(code, ENCRYPTION_SEED1, ENCRYPTION_SEED2, VENDOR_KEY1,
	VENDOR_KEY2, VENDOR_KEY3, VENDOR_KEY4, VENDOR_KEY5);

main()
{
  LM_HANDLE *lm_job;
  char * ret;


	if (lc_init((LM_HANDLE *)0, VENDOR_NAME, &code, &lm_job))
	{
		lc_perror(lm_job, "lc_init failed");
		exit(lc_get_errno(lm_job));
	}

	if (!(ret = lc_vsend(lm_job, "flush")) || strcmp(ret, "flushed"))
		lc_perror(lm_job, "flush error");
	if (!(ret = lc_vsend(lm_job, "test")) || strcmp(ret, "ok"))
		lc_perror(lm_job, "flush error");
}
