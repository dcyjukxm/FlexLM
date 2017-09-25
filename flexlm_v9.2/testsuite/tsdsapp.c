#include "lmclient.h"
#include "testsuite.h"
/*
 *	ds_gen and ds_app
 *	Test digital signature callbacks
 *	these are not real digital signatures, but emulate what
 *	they would do.
 */

int
ts_ds_app(job, input, len, key, keylen)
char *job;
unsigned char *input;
unsigned int len;
unsigned char *key;
unsigned int keylen;
{
 char *tsbuf;
  int max, i;
  int ret ;

	if (keylen != 64)
		return 1;

	tsbuf = (char *)calloc(1, keylen);
	max = len;
	if (keylen > max) max = keylen;
  	for (i = 0; i < max; i++)
	{
		if (tsbuf[i%64] == (input[i%len] + i))
			tsbuf[i%64] += input[i%len] + i;
		else tsbuf[i%64] ^= (input[i%len] + i);
	}
	ret =  memcmp(key, tsbuf, keylen);
	free(tsbuf);
	return ret;
}
