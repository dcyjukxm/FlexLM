#include "lmclient.h"
#include "testsuite.h"
int
ts_ds_gen(job, input, len, ret, retlen)
char *job;
char *input;
unsigned int len;
unsigned char *ret;
unsigned int *retlen;
{
  int max, i;
	if (*retlen < 64)
	{
		*retlen = 64;
		return 1;
	}
	*retlen = 64;
	max = len;
	if (*retlen > max) max = *retlen;
	memset(ret, 0, 64);
  	for (i = 0; i < max; i++)
	{
		if (ret[i%64] == (input[i%len] + i))
			ret[i%64] += input[i%len] + i;
		else ret[i%64] ^= (input[i%len] + i);
	}
	return 0;

}
