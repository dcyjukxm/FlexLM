#define l_crypt_private lc_crypt
#define get_ver l_get_ver
#define our_encrypt l_our_encrypt
#define shuffle l_shuffle
#define atox l_atox
#define l_movelong movelong
#define swap l_swap
#define move_in_hostid l_move_in_hostid
#define add l_add
#define addi l_addi
#define LM_CRYPT
#include "lmachdep.h"
#ifdef PC
#include <malloc.h>
#endif
#include "l_crypt.c"
