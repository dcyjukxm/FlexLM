#include "lmclient.h"
#include "lmachdep.h"
#include "l_prot.h"
#include "l_strkey.h"
#include "../src/l_strkey.c"
unsigned char * 
crypt_string_key(job, input, inputlen, code, len)
LM_HANDLE_PTR job; 
unsigned char * input;
int inputlen; 
VENDORCODE * code; 
unsigned long len;
{
	return l_string_key(job, input, inputlen, code, len);
}
