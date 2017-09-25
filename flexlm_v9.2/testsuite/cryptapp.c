#define LM_CRYPT
#define LM_CKOUT
#define l_crypt_private l_ckout_crypt
#define l_string_key	l_ckout_string_key
#include "lmclient.h"
#include "lmachdep.h"
#include "l_prot.h"
#include "l_strkey.h"
#include "../src/l_strkey.c"
unsigned char * 
crypt_string_app(job, input, inputlen, code, len, exp)
LM_HANDLE_PTR job; 
unsigned char * input;
int inputlen; 
VENDORCODE * code; 
unsigned long len;
char *exp;
{
	return l_ckout_string_key(job, input, inputlen, code, len, exp);
}
