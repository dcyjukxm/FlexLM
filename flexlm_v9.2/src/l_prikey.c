/******************************************************************************

	    COPYRIGHT (c) 1990, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/******************************************************************************
 *
 *
 *	NOTE:	The purchase of FLEXlm source does not give the purchaser
 *		the right to run FLEXlm on any platform of his choice.
 *		Modification of this, or any other file with the intent 
 *		to run on an unlicensed platform is a violation of your 
 *		license agreement with Macrovision Corporation.
 *
 *
 *	I M P O R T A N T
 *	At the bottom of this file is documentation about SIGN= and CRO
 *
 *****************************************************************************/
/*	
 *	Module: $Id: l_prikey.c,v 1.85 2003/01/14 21:46:58 kmaclean Exp $
 *
 *	Description: 	generate public/private key headers
 *
 *	D. Birns
 *	January, 2000
 *
 *	Last changed:  12/9/98
 *
 */
 
    /*
     * Include Files
     */
#include "l_pubkey.h"
#include "lmachdep.h"
#include "l_time.h"
#include <sys/types.h>
#include <sys/stat.h>
#ifdef PC
#include <winsock.h>
#endif /* PC */
/*
 *	L_F and L_G:
 *
 *	2 functions, F(x) and G(x), which are inverse:  
 *			F(G(x)) = x and G(F(x)) = x
 *	These are used to obfuscate the public key.  The public-key that's 
 *	seen by the ISV is obfuscated and not the real public key.
 */
#define L_F(x, len, vname) \
	{\
		int Li;\
		char *cp = vname;\
		for (Li = 0; Li < len; Li++)\
		{\
			if (!*cp) cp = vname;\
			if (!(Li % 2)) x[Li] +=  *cp++;\
			else if (!(Li % 3)) x[Li] ^=  *cp++;\
			else x[Li] -= *cp++;\
		}\
	}
#define L_G(x, len, vname) \
	{\
		int Gi;\
		char *cp = vname;\
		for (Gi = 0; Gi < len; Gi++)\
		{\
			if (!*cp) cp = vname;\
			if (!(Gi % 2)) x[Gi] -=  *cp++;\
			else if (!(Gi % 3)) x[Gi] ^=  *cp++;\
			else x[Gi] += *cp++;\
		}\
	}
#define OPENBRACE 0x7b
#define CLOSEBRACE 0x7d


#ifndef RELEASE_VERSION
static char *debug = (char *)-1;
#define DEBUG_INIT if (debug == (char *)-1) {\
	  char c[256];\
		strncpy(c, __FILE__, strlen(__FILE__) -2); \
		c[strlen(__FILE__) - 2] = '\0';\
		debug = (char *)l_real_getenv(c);\
	}


#define DEBUG(x) if (debug) printf x
#else
#define DEBUG_INIT 
#define DEBUG(x) 
#endif /* RELEASE_VERSION */
typedef struct _verify_mem {
	void (*cleanup)(char *);
	char *global_data;
	char *heap_space;
	sb_OptionsList Options;
	size_t heapSize;
	size_t dataSize;
	sb_PublicKey publicKey;
	int strength;
	int sign_level;
} VERIFY_MEM;
	
	

static void displayErrorMsg( const int status);
static int genkeys(unsigned int *seed, 
	int pubkey_strength,
	sb_PrivateKey *privateKey,
	sb_PublicKey *publicKey);

void
gen_pkey_headers( unsigned int *seed, 
		int pubkey_strength, LM_VENDORCODE_PUBKEYINFO *public, 
		LM_VENDORCODE_PUBKEYINFO *private,
		char *vname) 
{
  sb_PrivateKey privateKey;
  sb_PublicKey publicKey;
  int offset = pubkey_strength - 2;

	memset(&privateKey, 0, sizeof(privateKey));
	memset(&publicKey, 0, sizeof(publicKey));
	genkeys( seed, pubkey_strength, &privateKey, &publicKey);
	L_F(publicKey.key, publicKey.size, vname);
	public->pubkeysize[offset] = publicKey.size;
	memcpy(public->pubkey[offset], publicKey.key, publicKey.size);
	private->pubkeysize[offset] = privateKey.size;
	memcpy(private->pubkey[offset], privateKey.key, privateKey.size);
}
	


void
l_gen_pkey_headers( 
	unsigned int seeds[4][3],
	int pubkey_strength, VENDORCODE *public,
	char *vname)
{
  int i, j;
  FILE *prifp, *pubfp; /* header files */
  VENDORCODE private;
  unsigned int *seed;
  int seedcnt;
  int thisseed;

	printf("Generating header files lmpubkey.h, lmprikey.h\n");
	if (!(prifp = fopen("lmprikey.h", "w")))
	{
		perror("Can't open lmprikey.h, exiting\n");
		exit(1);
	}
	if (!(pubfp = fopen("lmpubkey.h", "w")))
	{
		perror("Can't open lmpubkey.h, exiting\n");
		exit(1);
	}
	if (pubkey_strength < LM_STRENGTH_PUBKEY)
	{
		fprintf(pubfp, "#define LM_KEY_CALLBACK 0\n");
		fprintf(pubfp, "\
		\nstatic int l_pubseedcnt = 0;\
		\nstatic int lm_pubsize[%d][%d] = {{0}};\
		\nstatic unsigned char lm_pubkey[1][%d][%d] = {{{0}}};\n",
			LM_MAXSIGNS, LM_PUBKEYS,
			LM_PUBKEYS, LM_MAXPUBKEYSIZ);
		fprintf(prifp, "#include \"lmclient.h\"\
		\nlm_extern int *l_prikey_sign(void);\
		\nstatic int l_priseedcnt = 0;\
		\nstatic int lm_prisize[%d][%d] = {{0}};\
		\nstatic unsigned char lm_prikey[1][%d][%d] = {{{0}}};\n",
			LM_MAXSIGNS, LM_PUBKEYS,
			LM_PUBKEYS, LM_MAXPUBKEYSIZ);
		fclose(pubfp);
		fclose(prifp);
		return;
	}
	for (seedcnt = 0; seeds[seedcnt][0] ; seedcnt++)
		;
		
	fprintf(pubfp, "\
			\n#include \"lmclient.h\"\
			\n#define LM_PUBLIC_KEY\
			\n#define LM_KEY_CALLBACK l_pubkey_verify\
			\nlm_extern int l_pubkey_verify();\
			\nstatic int l_pubseedcnt = %d;\
			\nstatic unsigned char lm_pubkey[%d][%d][%d] = {", 
				seedcnt, seedcnt, 
				LM_PUBKEYS, LM_MAXPUBKEYSIZ);
	fprintf(prifp, "#include \"lmclient.h\"\
			\nlm_extern int *l_prikey_sign(void);\
			\nstatic int l_priseedcnt = %d;\
			\nstatic unsigned char lm_prikey[%d][%d][%d] = {", 
				seedcnt, seedcnt, 
				LM_PUBKEYS, LM_MAXPUBKEYSIZ);
	memset(&private, 0, sizeof(private));
	memset(public, 0, sizeof(*public));

/* 
 *	If it's not public key, init to 0 and exit
 */
	for (thisseed = 0; seeds[thisseed][0]; thisseed++)
	{
		seed = seeds[thisseed];
		fprintf(pubfp, "%s{", thisseed ? ",\n\t" : "");
/*
 *		Set all 3 strengths for thisseed
 */
		for (i = LM_STRENGTH_PUBKEY; i <= 
					LM_STRENGTH_VERYHIGH; i++)
			gen_pkey_headers( seed, i, 
				&public->pubkeyinfo[thisseed], 
				&private.pubkeyinfo[thisseed], 
				vname);

/*
 *		P5186 -- all extern's changed to lm_extern and 
 *		lmclient.h included
 */
		for (i = 0; i < LM_PUBKEYS; i++)
		{
			fprintf(pubfp, "%s{", i ? ",\n\t" : "");
			for (j = 0; j < 
				public->pubkeyinfo[thisseed].pubkeysize[i]; j++)
			{
				fprintf(pubfp, "%s0x%x", j ? ", ": "", 
					public->pubkeyinfo[thisseed].pubkey[i][j]);
			}
			fprintf(pubfp, "}");
		}
		fprintf(pubfp, "}\n");
		fprintf(prifp, "%s{", thisseed ? ",\n\t" : "");
		for (i = 0; i < LM_PUBKEYS; i++)
		{
			fprintf(prifp, "%s{", i ? ",\n\t" : "");
			for (j = 0; j < private.pubkeyinfo[thisseed].pubkeysize[i]; j++)
			{
				fprintf(prifp, "%s0x%x", j ? ", ": "", 
					private.pubkeyinfo[thisseed].pubkey[i][j]);
			}
			fprintf(prifp, "}");
		}
		fprintf(prifp, "}\n");
	}
	fprintf(pubfp, "};\n");
	fprintf(prifp, "};\n");
	fprintf(pubfp, "\
		\nstatic unsigned int lm_pubsize[%d][%d] = {", seedcnt, LM_PUBKEYS);
	fprintf(prifp, "\
		\nstatic unsigned int lm_prisize[%d][%d] = {", LM_MAXSIGNS, LM_PUBKEYS);
	for (thisseed = 0; seeds[thisseed][0]; thisseed++)
	{
		fprintf(pubfp, "%s{", thisseed ? ",\n\t" : "");
		fprintf(prifp, "%s{", thisseed ? ",\n\t" : "");
		for (i = 0;i < LM_PUBKEYS; i++)
		{
			fprintf(pubfp, "%s0x%x", i ? ", " : "", public->pubkeyinfo[thisseed].pubkeysize[i]);
			fprintf(prifp, "%s0x%x", i ? ", " : "", private.pubkeyinfo[thisseed].pubkeysize[i]);
		}
		fprintf(pubfp, "}\n");
		fprintf(prifp, "}\n");
	}
	fprintf(pubfp, "};\n");
	fprintf(prifp, "};\n");
	fclose(pubfp);
	fclose(prifp);
} 

/*
 * l_genkeys is only used for testing, and is an interface to genkeys
 */
int
l_genkeys( unsigned int lmseed1, 
	unsigned int lmseed2, 
	unsigned int lmseed3, 
	int pubkey_strength,
	int *prikey_size,
	char *prikey,
	int *pubkey_size,
	char *pubkey)
{
  sb_PrivateKey privateKey;
  sb_PublicKey publicKey;
  int ret;
  unsigned int seed[3];
	
	memset(seed, 0, sizeof(seed));
	seed[0] = lmseed1;
	seed[1] = lmseed2;
	seed[2] = lmseed3;
	
	if (!(ret = genkeys(seed, pubkey_strength, &privateKey, &publicKey)))
	{
		*prikey_size = privateKey.size;
		memcpy(prikey, privateKey.key, privateKey.size);
		*pubkey_size = publicKey.size;
		memcpy(pubkey, publicKey.key, publicKey.size);
	}
	return ret;
}

static 
int
genkeys(unsigned int *seed,
	int pubkey_strength,
	sb_PrivateKey *privateKey,
	sb_PublicKey *publicKey)
{
  const ellipticCurveParameters *ellipticCurve;
  sb_OptionsList sb_Options;
  unsigned char seedValue[SB_MAX_SEED_LEN];
  void *global_data;
  size_t dataSize;
  size_t heapSize = 0;
  void *heap_space;
  int ret, i;

	switch(pubkey_strength)
	{
	case LM_STRENGTH_LICENSE_KEY: return 0;
	case LM_STRENGTH_113BIT: ellipticCurve = &LM_PUBKEY_CURVE113BIT; break;
	case LM_STRENGTH_163BIT: ellipticCurve = &LM_PUBKEY_CURVE163BIT; break;
	case LM_STRENGTH_239BIT: ellipticCurve = &LM_PUBKEY_CURVE239BIT; break;
	default:
		{
			fprintf(stderr, 
			"LM_STRENGTH in lm_code.h has invalid value %d\n", 
							pubkey_strength);
			fprintf(stderr, 
				"Use only LM_STRENGTH_[113|163|239]BIT, LM_STRENGTH_DEFAULT, OR LM_STRENGTH_LICENSE_KEY, exiting\n");
			exit(1);
			
		}
	}
	if ((ret = sb_dataSize(ellipticCurve, &dataSize))!=SB_SUCCESS) 
	{
		displayErrorMsg(ret);
		exit(1);
	}

/*
 * 	Allocate memory space for the operations of Security Builder.
 */
	global_data = calloc(1, dataSize);


/*
 * 	Obtain the size of the heap space.
 */
	ret = sb_heapSize(ellipticCurve, &heapSize);
	if (ret != SB_SUCCESS) 
	{ 
		displayErrorMsg(ret);
		exit(1);
	}

/*
 * 	Attempt to allocate memory space for heap.
 */
	heap_space = calloc(1, heapSize);
/*
 * 	Set the initialization options.
 * 	The selected options are:
 * 	1) point compression is on.
 * 	2) use new point compression.
 * 	3) maximum length of the random number generator seed is
 *    		SB_MAX_SEED_LEN.
 * 	4) random number generator used is FIPS186.
 */
	sb_Options.pointCompressionFlag = SB_ON;
	sb_Options.ecesCompressionFlag = SB_P1363_13FEB1998;
	sb_Options.rngInfo.seed.size = SB_MAX_SEED_LEN;
	sb_Options.rngInfo.type = SB_FIPS_RNG;

/*
 * 	Set seed to sb_Options using the value contained in seedValue.
 * 	Note that it is assumed that the value in seedValue is obtained
 * 	from a random source.  In practice, a real random value from
 * 	a secure random source must be set here.
 */
	memset(seedValue, 0, sizeof(seedValue));
#if 0 /* P5308 */
	for (i = 0;i < 4; i++)
		seedValue[i] = (seed3 >> (1 << (i * 8)) & 0xff);
	for (i = 0;i < 4; i++)
		seedValue[i + 4] = (seed4 >> (1 << (i * 8)) & 0xff);
#endif /* P5308 */
	for (i = 0;i < 4; i++)
		seedValue[i] = ((seed[0] >> (i * 8)) & 0xff);
	for (i = 0;i < 4; i++)
		seedValue[i + 4] = ((seed[1] >> (i * 8)) & 0xff);
	if (seed[2]) /* added 3rd int in v8.1 */
	{
		for (i = 0;i < 4; i++)
			seedValue[i + 8] = ((seed[2] >> (i * 8)) & 0xff);
		/* And added strength into seed */
		for (i = 0;i < sizeof(pubkey_strength); i++)
			seedValue[i + 12] = ((pubkey_strength >> (i * 8)) & 0xff);
	}
	memcpy( sb_Options.rngInfo.seed.value, seedValue, SB_MAX_SEED_LEN);

	ret = sb_initialize(
		ellipticCurve,
		&sb_Options,
		dataSize,
		heapSize,
		global_data,
		heap_space
	);
	if (ret != SB_SUCCESS) 
	{
		displayErrorMsg(ret);
		exit(1);
	}


/*
 * 2. Generate ECC key pairs.
 */
	ret = sb_genKeyPair(
		global_data,
		privateKey,
		publicKey
	);
	if (ret != SB_SUCCESS) 
	{
		displayErrorMsg(ret); 
		exit(1);
	}
/*
 * End Security Builder
 */
	ret = sb_end(global_data);
	if (ret != SB_SUCCESS) 
	{
		displayErrorMsg(ret); 
		exit(1);
	}

/*
 * Free the memory space
 */
	free(heap_space);
	free(global_data);
	return 0;
}



/*===========local function definition==============================*/


void
l_pubkey_err(LM_HANDLE *job, int minor, int status)
{
  char errorMsg[SB_ERROR_MESSAGE_LENGTH];

	if (sb_errorMessage(status, SB_ERROR_MESSAGE_LENGTH,
		errorMsg) == SB_SUCCESS)
	{
		LM_SET_ERROR(job, LM_PUBKEY_ERR, minor, 0, errorMsg, 
						LM_ERRMASK_ALL);
	}
	else
	{
		LM_SET_ERROR(job, LM_PUBKEY_ERR, minor, 0, "Fatal Pubkey Error",
				LM_ERRMASK_ALL);
	}
} 

int
l_prikey_sign(LM_HANDLE *job, unsigned char *input, int inputlen, 
	LM_VENDORCODE_PUBKEYINFO *private_key, char *result, int *resultlen)
{
#define MSG_BLOCK_SIZE           32

  const ellipticCurveParameters *ellipticCurve;
  sb_OptionsList sb_Options;
  unsigned char seedValue[SB_MAX_SEED_LEN];
  size_t dataSize;
  size_t heapSize;
  void *heap_space = 0, *global_data = 0;
  sb_PrivateKey privateKey;
  sb_SignContextECDSA signContext;
  sb_SignatureECDSA signature;
  int rc;
  int numberOfBlocks;
  int remainingOctets;
  int i, now; 
  int ret = 0; /* assume success */
  unsigned char pubkey_strength =  
	(unsigned char)job->L_STRENGTH_OVERRIDE ?
	(unsigned char)job->L_STRENGTH_OVERRIDE :
	private_key->strength;
  int offset = pubkey_strength - 2;
  sb_HashContext hc;
  sb_MessageDigest md;


	DEBUG_INIT
	DEBUG(("signing %s\n", input));
	memset(&sb_Options, 0, sizeof(sb_Options));
	memset(&seedValue, 0, sizeof(seedValue));
	if ((l_getattr(job, NO_EXPIRE) == NO_EXPIRE_VAL)
		&& (l_getattr(job, LM_PUBKEY_ATTR) != LM_PUBKEY_ATTR_VAL))
	{
		LM_SET_ERRNO(job, LM_NOCROSUPPORT, 536, 0);
		ret =  job->lm_errno;
		goto exit_l_prikey_sign;
	}
	switch(pubkey_strength)
	{
	case LM_STRENGTH_LICENSE_KEY: ret = 0;
		goto exit_l_prikey_sign;
	case LM_STRENGTH_113BIT: ellipticCurve = &LM_PUBKEY_CURVE113BIT; break;
	case LM_STRENGTH_163BIT: ellipticCurve = &LM_PUBKEY_CURVE163BIT; break;
	case LM_STRENGTH_239BIT: ellipticCurve = &LM_PUBKEY_CURVE239BIT; break;
	default: 
		LM_SET_ERRNO(job, LM_BADPARAM, 530, 0);
		ret = job->lm_errno;
		goto exit_l_prikey_sign;
	}
	if ((rc = sb_dataSize(ellipticCurve, &dataSize))!=SB_SUCCESS) 
	{
		l_pubkey_err(job,  10531, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
		
	}
	memset(&privateKey, 0, sizeof(privateKey));
	privateKey.size = private_key->pubkeysize[offset];
	memcpy( privateKey.key, private_key->pubkey[offset], 
		private_key->pubkeysize[offset]);

/*
 * 	Allocate memory space for the operations of Security Builder.
 */
	global_data = l_malloc(job, dataSize);


/*
 * 	Obtain the size of the heap space.
 */
	if ((rc = sb_heapSize(ellipticCurve, &heapSize)) != SB_SUCCESS) 
	{ 
		l_pubkey_err(job,  10532, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}

	heap_space = l_malloc(job, heapSize);


	/* see comment above about these options */
	sb_Options.pointCompressionFlag = SB_ON;
	sb_Options.ecesCompressionFlag = SB_P1363_13FEB1998;
	sb_Options.rngInfo.seed.size = SB_MAX_SEED_LEN;
	sb_Options.rngInfo.type = SB_FIPS_RNG;
	
	for (i = 0; i < SB_MAX_SEED_LEN && i < private_key->pubkeysize[offset]; i++)
	{
		seedValue[i] = private_key->pubkey[offset][i];
		/*fprintf(stderr, "%0x", seedValue[i]);*/
	}
	memcpy( sb_Options.rngInfo.seed.value, seedValue, SB_MAX_SEED_LEN);

/*
 * Call sb_initialize().
 */
	if ((rc = sb_initialize(
		ellipticCurve,
		&sb_Options,
		dataSize,
		heapSize,
		global_data,
		heap_space)) != SB_SUCCESS)
	{
		l_pubkey_err(job,  10533, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}
	if (rc = sb_sha1Begin(global_data, &hc))
	{
		l_pubkey_err(job,  10543, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}
	if (rc = sb_sha1Hash(global_data, inputlen, input, &hc))
	{
		l_pubkey_err(job,  10544, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}
	if (rc =  sb_ecdsaSignBegin(global_data, &signContext) != SB_SUCCESS)
	{
		l_pubkey_err(job,  10534, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}
	if (rc = sb_sha1End(global_data, &hc, &md))
	{
		l_pubkey_err(job,  10545, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}
	if (rc = sb_fipsRngOptionalInput(global_data, md.size, md.digest))
	{
		l_pubkey_err(job,  10546, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}

	numberOfBlocks = inputlen / MSG_BLOCK_SIZE;
	remainingOctets = inputlen % MSG_BLOCK_SIZE;

/*
 * Sign the full blocks.
 */
	for (i = 0; i < numberOfBlocks; i++) 
	{
		if ((rc = sb_ecdsaSign( global_data, MSG_BLOCK_SIZE,
			&input[i * MSG_BLOCK_SIZE],
			&signContext)) != SB_SUCCESS) 
		{
			l_pubkey_err(job,  10535, rc);
			ret = LM_PUBKEY_ERR;
			goto exit_l_prikey_sign;
		}
	}

	/*
	* If there is any remaining octet, sign it.
	*/
	if (remainingOctets)
	{
		if ((rc = sb_ecdsaSign( global_data, 
			(unsigned long) remainingOctets,
			&input[numberOfBlocks * 
				MSG_BLOCK_SIZE], &signContext)) 
						!= SB_SUCCESS) 
		{
			l_pubkey_err(job,  10536, rc);
			ret = LM_PUBKEY_ERR;
			goto exit_l_prikey_sign;
		}
	}

	if ((rc = sb_ecdsaSignEnd( global_data, &privateKey,
		&signature, &signContext)) != SB_SUCCESS) 
	{
		l_pubkey_err(job,  10537, rc);
		ret = LM_PUBKEY_ERR;
		goto exit_l_prikey_sign;
	}
	if (*resultlen < signature.size)
	{
		*resultlen = signature.size;
		ret = 1;
		goto exit_l_prikey_sign;
	}
	*resultlen = signature.size;
	memcpy(result, signature.signature, signature.size);

exit_l_prikey_sign:
	if (global_data) free(global_data);
	if (heap_space) free(heap_space);
	return ret;
}

static void
displayErrorMsg(
    const int status
        /* [input]
         * The error code returned by an API function.
         */
    )
{
    /*
     * errorMsg - message associated with a error code from a Security
     *            Builder API function.
     * errorStatus - status of sb_errorMessage().
     */
    char errorMsg[SB_ERROR_MESSAGE_LENGTH];
    int  errorStatus;


    errorStatus = sb_errorMessage(status, SB_ERROR_MESSAGE_LENGTH,
                                   errorMsg);

    if (errorStatus != SB_SUCCESS) {
        fprintf(stderr, "\nTerminated due to a fatal error.\n");
        fprintf(
                stderr,
                "Security Builder sb_errorMessage() error status: %d.\n",
                errorStatus
                );
    } else {

        fprintf(stderr, "\nTerminated due to a fatal error.\n");
        fprintf(
                stderr,
                "Security Builder error status %d, message:\n"
                "%s.\n", status, errorMsg
                );
    }


    return;

} /* displayErrorMsg() */


int
l_pubkey_verify (LM_HANDLE *job, 
		unsigned char * input, 
		int inputlen, 
		char *signature, 
		int siglen,
		int *pubkeysize,
		unsigned char pubkey[LM_PUBKEYS][LM_MAXPUBKEYSIZ],
		int strength, 
		int sign_level)
{
#define MSG_BLOCK_SIZE           32
/*
 * ellipticCurve - pointer to the elliptic curve object.
 * The curve, "sect113r1", is used in this application.
 */
  const ellipticCurveParameters *ellipticCurve;


/*
 * sb_Options - the ECC initialization options structure.
 */
  int ret = 0;
  int offset;   /* this is the offset into 
				   pubkeysize and pubkey */


/*
 * global_data - pointer to the data space required and used by the
 *              Security Builder for its operations.
 * dataSize - number of bytes to allocate for the data space.
 * heap_space - pointer to the heap space.
 * heapSize - number of bytes to allocate for the heap space.
 */


/*
 * privateKey - private keys.
 * publicKey - public keys.
 */
  sb_SignatureECDSA ecc_sig;



/*
 * verifyContext - ECDSA verify context buffer.
 * signature - Bob's signature.
 * verificationResult- the result of verification.
 */
 sb_VerifyContextECDSA verifyContext;
 int verificationResult;




/*
 * numberOfBlocks - the number of full message blocks to apply DES
 *                  encryption/decryption and ECDSA signing/verification
 *                  functions of Security Builder.
 * remainingOctets - the number of remaining octets when divided by
 *                   SAMPLE_BLOCK_SIZE.
 * desRemainingOctets - the number of remaining octets when divided by
 *                      SB_DES_BLOCK_SIZE.
 */
  int numberOfBlocks;
  int remainingOctets;


/*
 * returnValue - variable to receive return codes of the functions.
 */
  int returnValue;


/*
 * i - counter used in `for' loops.
 */
  int i;
  VERIFY_MEM *m;

#ifndef RELEASE_VERSION
	DEBUG_INIT
	if (debug && (debug != (char *)-1))
	{
	  char x[81];
		for (i = 0; i < 80 && i < inputlen; i++)
		{
			if (isprint(input[i]))
				x[i] = input[i];
			else
				x[i] = '.';

		}
		x[i] = 0;
		
		DEBUG(("verifying %s\n", x));
	}
#endif

	strength = 
		(int)job->L_STRENGTH_OVERRIDE ?
		(int)job->L_STRENGTH_OVERRIDE : strength;
	if ((l_getattr(job, NO_EXPIRE) == NO_EXPIRE_VAL)
		&& (l_getattr(job, LM_PUBKEY_ATTR) != LM_PUBKEY_ATTR_VAL))
	{
		LM_SET_ERRNO(job, LM_NOCROSUPPORT, 539, 0);
		return LM_FUNCNOTAVAIL;
	}
	offset = strength - 2; /* offset into pubkey and pubkeysize */
	switch(strength)
	{
	case LM_STRENGTH_LICENSE_KEY: return 0;
	case LM_STRENGTH_113BIT: ellipticCurve = &LM_PUBKEY_CURVE113BIT; break;
	case LM_STRENGTH_163BIT: ellipticCurve = &LM_PUBKEY_CURVE163BIT; break;
	case LM_STRENGTH_239BIT: ellipticCurve = &LM_PUBKEY_CURVE239BIT; break;
	default:
		{
			LM_SET_ERRNO(job, LM_BADPARAM, 531, 0);
			ret = LM_BADPARAM;
			goto exit_verify;
		}
	}
    	if (siglen > SB_ECDSA_SIGNATURE_SIZE)
	{
			LM_SET_ERRNO(job, LM_BADPARAM, 535, 0);
			ret = LM_BADPARAM;
			goto exit_verify;
	}
	memset(&ecc_sig, 0, sizeof(ecc_sig));
	ecc_sig.size = siglen;
/* 
 *	for reasons I don't understand, with some compilers, the &...[0]
 *	is required in the next statement
 */
	memcpy(&ecc_sig.signature[0], signature, siglen);


/*
 * 1. Initialize Security Builder.
 */
	if (job->L_VERIFY_MEM) 
	{
		m = (VERIFY_MEM *)job->L_VERIFY_MEM;
		if ((m->strength != strength) ||
				(sign_level != m->sign_level))
		{
			sb_end(m->global_data);
			free(m->global_data);
			free(m->heap_space);
			free(m);
			job->L_VERIFY_MEM = 0;
		}
	}
	if (!job->L_VERIFY_MEM)
	{
	  typedef void (*cleanup)(char *);
		m = (VERIFY_MEM *)(job->L_VERIFY_MEM = 
			(char *)l_malloc(job, sizeof(*m)));
		m->cleanup = (cleanup)sb_end;
		m->strength = strength;
		m->sign_level = sign_level;
		memset(&m->publicKey, 0, sizeof(m->publicKey));
		m->publicKey.size = pubkeysize[offset];
		memcpy(m->publicKey.key, pubkey[offset], pubkeysize[offset]);
		L_G(m->publicKey.key, pubkeysize[offset], job->vendor);
		returnValue = sb_dataSize(ellipticCurve, &m->dataSize);
		if (returnValue != SB_SUCCESS) 
		{
			l_pubkey_err(job, 10538, returnValue);
			ret = LM_PUBKEY_ERR;
			goto exit_verify;
		}

/*
 * 	Allocate memory space for the operations of Security Builder.
 */
		m->global_data = l_malloc(job, m->dataSize);


/*
 * 		Obtain the size of the heap space.
 */
		returnValue = sb_heapSize(ellipticCurve, &m->heapSize);
		if (returnValue != SB_SUCCESS) 
		{ 
			l_pubkey_err(job, 10539, returnValue);
			ret = LM_PUBKEY_ERR;
			goto exit_verify;
		}

		m->heap_space = l_malloc(job, m->heapSize);

/*
 * 	Set the initialization options.
 * 	The selected options are:
 * 	1) point compression is on.
 * 	2) use new point compression.
 * 	3) maximum length of the random number generator seed is
 *      	SB_MAX_SEED_LEN.
 * 	4) random number generator used is FIPS186.
 */
		m->Options.pointCompressionFlag = SB_ON;
		m->Options.ecesCompressionFlag = SB_P1363_13FEB1998;
		m->Options.rngInfo.seed.size = SB_MAX_SEED_LEN;
		m->Options.rngInfo.type = SB_FIPS_RNG;


/*
 * 	Call sb_initialize().
 */
		returnValue = sb_initialize(
			ellipticCurve,
			&m->Options,
			m->dataSize,
			m->heapSize,
			m->global_data,
			m->heap_space
		);
		if (returnValue != SB_SUCCESS) 
		{
			l_pubkey_err(job, 10540, returnValue);
			ret = LM_PUBKEY_ERR;
			goto exit_verify;
		}
	}
	else m = (VERIFY_MEM *)job->L_VERIFY_MEM;

	if ((returnValue = sb_ecdsaVerifyBegin(m->global_data, &verifyContext) !=
							SB_SUCCESS)) 
	{
		l_pubkey_err(job, 10541, returnValue);
		ret = LM_PUBKEY_ERR;
		goto exit_verify;
	}

	numberOfBlocks = inputlen / MSG_BLOCK_SIZE;
	remainingOctets = inputlen % MSG_BLOCK_SIZE;

/*
 * 	Verify the full blocks.
 */
	for (i = 0; i < numberOfBlocks; i++) 
	{
		if ((returnValue = sb_ecdsaVerify(
			m->global_data,
			MSG_BLOCK_SIZE,
			&input[i * MSG_BLOCK_SIZE],
			&verifyContext
			)) != SB_SUCCESS) 
		{
			l_pubkey_err(job, 10542, returnValue);
			ret = LM_PUBKEY_ERR;
			goto exit_verify;
		}
	}

/*
 * 	If there is any remaining octet, verify it.
 */
	if ((remainingOctets != 0) &&
		((returnValue = sb_ecdsaVerify(
			m->global_data,
			(unsigned long) remainingOctets,
			&input[numberOfBlocks * MSG_BLOCK_SIZE],
			&verifyContext
			)) != SB_SUCCESS))
	{
		l_pubkey_err(job, 10543, returnValue);
		ret = LM_PUBKEY_ERR;
		goto exit_verify;
	}

	if ((returnValue = sb_ecdsaVerifyEnd( m->global_data, &m->publicKey,
	   &ecc_sig, &verifyContext, &verificationResult)) != SB_SUCCESS) 
	{
		l_pubkey_err(job, 10544, returnValue);
		ret = LM_PUBKEY_ERR;
		goto exit_verify;
	}

	if (verificationResult == 0) 
	{
		LM_SET_ERRNO(job, LM_BADCODE, 532, 0);
		ret = LM_BADCODE;
		goto exit_verify;
	}


exit_verify:

	return ret;
} 

/*
 *	Generate random numbers from lm_seeds1-3
 *	Never use lm_seeds1-3 directly, only the random derived numbers
 *	Return value:  0 = success
 *			Set bytes_needed to number of random bytes desired
 *			returned is a char buffer at least bytes_needed
 *			in size, and it's filled with the random bytes
 *			upon successful return.
 */
int 
l_genrand(LM_HANDLE *job, 
	unsigned int seed1,
	unsigned int seed2,
	unsigned int seed3, 
	int bytes_needed,
	unsigned char *returned)
{
/*
 *	Note that the choice of elliptic curve is unimportant
 *	This code doesn't actually use an elliptic curve, but
 *	needs one for some args
 */
  const ellipticCurveParameters *ellipticCurve = &LM_PUBKEY_CURVE163BIT; 
  sb_OptionsList sb_Options;
  unsigned char seedValue[SB_MAX_SEED_LEN];
  void *global_data = 0;
  size_t dataSize;
  size_t heapSize = 0;
  void *heap_space = 0;
  int ret, i;

/*
 * 	Allocate memory space for the operations of Security Builder.
 */
	if ((ret = sb_dataSize(ellipticCurve, &dataSize))!=SB_SUCCESS) 
	{
		l_pubkey_err(job, 10547, ret);
		ret = LM_PUBKEY_ERR;
		goto exit_rand;
	}
	global_data = l_malloc(job, dataSize);
	ret = sb_heapSize(ellipticCurve, &heapSize);
	if (ret != SB_SUCCESS) 
	{ 
		l_pubkey_err(job, 10548, ret);
		ret = LM_PUBKEY_ERR;
		goto exit_rand;
	}

/*
 * 	Attempt to allocate memory space for heap.
 */
	heap_space = l_malloc(job, heapSize);
/*
 * 	Set the initialization options.
 * 	The selected options are:
 * 	1) point compression is on.
 * 	2) use new point compression.
 * 	3) maximum length of the random number generator seed is
 *    		SB_MAX_SEED_LEN.
 * 	4) random number generator used is FIPS186.
 */
	sb_Options.pointCompressionFlag = SB_ON;
	sb_Options.ecesCompressionFlag = SB_P1363_13FEB1998;
	sb_Options.rngInfo.seed.size = SB_MAX_SEED_LEN;
	sb_Options.rngInfo.type = SB_FIPS_RNG;

/*
 * 	Set seed to sb_Options using the value contained in seedValue.
 * 	Note that it is assumed that the value in seedValue is obtained
 * 	from a random source.  In practice, a real random value from
 * 	a secure random source must be set here.
 */
	memset(seedValue, 0, sizeof(seedValue));
	for (i = 0;i < 4; i++)
		seedValue[i] = ((seed1 >> (i * 8)) & 0xff);
	for (i = 0;i < 4; i++)
		seedValue[i + 4] = ((seed2 >> (i * 8)) & 0xff);
	for (i = 0;i < 4; i++)
		seedValue[i + 8] = ((seed3 >> (i * 8)) & 0xff);
	memcpy( sb_Options.rngInfo.seed.value, seedValue, SB_MAX_SEED_LEN);
	ret = sb_initialize(
		0,
		&sb_Options,
		dataSize,
		heapSize,
		global_data,
		heap_space
	);
	if (ret != SB_SUCCESS) 
	{
		l_pubkey_err(job, 10549, ret);
		ret = LM_PUBKEY_ERR;
		goto exit_rand;
	}

	ret = sb_rngFIPS186Session(
		global_data,
		bytes_needed,
		returned);
	if (ret != SB_SUCCESS) 
	{
		l_pubkey_err(job, 10550, ret);
		ret = LM_PUBKEY_ERR;
		goto exit_rand;
	}
/*
 * End Security Builder
 */
	ret = sb_end(global_data);
	if (ret != SB_SUCCESS) 
	{
		l_pubkey_err(job, 10551, ret);
		ret = LM_PUBKEY_ERR;
		goto exit_rand;
	}

exit_rand:
	if (global_data) free(global_data);
	if (heap_space) free(heap_space);
	return ret;
}
#ifndef VXWORKS
	/* kmaclean 11-19-02
	 * Don't include l_genseed() if we are building for VxWorks.
	 * l_genseed() is not used in the client lib and if
	 * VXWORKS is defined then we are only building the client lib */

/*
 *	l_genseed
 *	hardware dependent.  Generate random seeds.
 *	Exits with error message to stderr upon error
 */
int 
l_genseed( void)
{
/*
 *	Note that the choice of elliptic curve is unimportant
 *	This code doesn't actually use an elliptic curve, but
 *	needs one for some args
 */
  const ellipticCurveParameters *ellipticCurve = &LM_PUBKEY_CURVE163BIT; 
  sb_OptionsList sb_Options;
  unsigned char seedValue[SB_MAX_SEED_LEN];
  void *global_data = 0;
  size_t dataSize;
  size_t heapSize = 0;
  void *heap_space = 0;
  int ret, i, j;
  FILE *fp;
  int c;
  unsigned char returned[3*4];
#ifdef MAC10
#define MAX_SEED_LEN 10000 /*	10k	*/
#else
#define MAX_SEED_LEN 1000000 /* 1 MEGABYTE */
#endif
  unsigned char additional_seed[MAX_SEED_LEN];
  sb_HashContext hc;
  sb_MessageDigest md;
  unsigned int seed1 = 0;
  unsigned int seed2 = 0;
  unsigned int seed3 = 0;
  struct timeval nowtv;
  struct timezone tz; 
  int randcnt = 0;
  struct stat st;

	if (!stat("lmseed.dat", &st))
	{
		fprintf(stderr, "lmseed.dat already generated, exiting\n");
		exit(1);
	}
    l_gettimeofday(&nowtv, &tz);
	memcpy(&seedValue[0], &nowtv, sizeof(nowtv));
/*
 * 	Allocate memory space for the operations of Security Builder.
 */
	fprintf(stdout, "\
The machind%clm_code.h file requires LM_SEED1-3 be set to random\n\
numbers.  Once made up these numbers must *never* change.  This \n\
program will automatically generate these for you, or you can \n\
make them up in some other fashion if you prefer.\n\
\n\
The output appears on the screen, and in \"lmseed.dat\"\n\
Press Enter to generate the seeds...", 
#ifdef PC
	'\\'
#else
	'/'
#endif
	);
	fflush(stdout);
	getchar();

	if ((ret = sb_dataSize(ellipticCurve, &dataSize))!=SB_SUCCESS) 
	{
		fprintf(stderr, "Error 1: ");
		goto exit_seed;
	}
	global_data = calloc(1, dataSize);
	ret = sb_heapSize(ellipticCurve, &heapSize);
	if (ret != SB_SUCCESS) 
	{ 
		fprintf(stderr, "Error 2: ");
		goto exit_seed;
	}

/*
 * 	Attempt to allocate memory space for heap.
 */
	heap_space = calloc(1, heapSize);
/*
 * 	Set the initialization options.
 * 	The selected options are:
 * 	1) point compression is on.
 * 	2) use new point compression.
 * 	3) maximum length of the random number generator seed is
 *    		SB_MAX_SEED_LEN.
 * 	4) random number generator used is FIPS186.
 */
	sb_Options.pointCompressionFlag = SB_ON;
	sb_Options.ecesCompressionFlag = SB_P1363_13FEB1998;
	sb_Options.rngInfo.seed.size = SB_MAX_SEED_LEN;
	sb_Options.rngInfo.type = SB_FIPS_RNG;

/*
 * 	Set seed to sb_Options using the value contained in seedValue.
 * 	Note that it is assumed that the value in seedValue is obtained
 * 	from a random source.  In practice, a real random value from
 * 	a secure random source must be set here.
 */
	memset(seedValue, 0, sizeof(seedValue));
	memcpy(&seedValue[0], &nowtv, sizeof(nowtv));
    l_gettimeofday(&nowtv, &tz);
	memcpy(&seedValue[sizeof(nowtv)], &nowtv, sizeof(nowtv));
	memcpy(&seedValue[2*sizeof(nowtv)], &tz, sizeof(tz));
	memcpy( sb_Options.rngInfo.seed.value, seedValue, SB_MAX_SEED_LEN);
	ret = sb_initialize(
		0,
		&sb_Options,
		dataSize,
		heapSize,
		global_data,
		heap_space
	);
	if (ret != SB_SUCCESS) 
	{
		fprintf(stderr, "Error 3: ");
		goto exit_seed;
	}
	fprintf(stdout, "Please wait");
	do {
	  char cmd[200];
	  char letter[2];

		memset(letter, 0, sizeof(letter));
		fprintf(stdout, ".");
		fflush(stdout);
		memset(additional_seed, 0, sizeof(additional_seed));
#ifdef PC
		letter[0] = (randcnt % 25) + 'A';
		if (randcnt % 2)
			sprintf(cmd, "dir \"c:\\documents and settings\\%s*.*\" /s", randcnt > 2 ? letter : "");
		else
			sprintf(cmd, "dir %%SYSTEMROOT%%\\%s*.* /s /ta /od", randcnt > 2 ? letter : "");
		if (!(fp = _popen(cmd, "r")))
#else
		strcpy(cmd, "sh -c \"ps auxww 2>/dev/null\"");
		if (!(fp = popen(cmd, "r")))
#endif
		{
			fprintf(stderr, "Can't open c:\\documents and settings, exiting\n");
			exit(1);
		}
		i = 0;
		while (((c = fgetc(fp)) != EOF) && (i < MAX_SEED_LEN))
			additional_seed[i++] = c;
		fclose(fp);
#ifdef PC
		if (!(fp = _popen("dir \\", "r")))
#else
		if (!(fp = popen("sh -c \"ps -ef 2>/dev/null\"", "r")))
#endif
		{
			fprintf(stderr, "Can't open \\, exiting\n");
			exit(1);
		}
		while (((c = fgetc(fp)) != EOF) && (i < MAX_SEED_LEN))
			additional_seed[i++] = c;
		fclose(fp);
		if (ret = sb_sha1Begin(global_data, &hc))
		{
			fprintf(stderr, "Error 4: ");
			goto exit_seed;
		}
		if (ret = sb_sha1Hash(global_data, i, additional_seed, &hc))
		{
			fprintf(stderr, "Error 5: ");
			goto exit_seed;
		}
		if (ret = sb_sha1End(global_data, &hc, &md))
		{
			fprintf(stderr, "Error 6: ");
			goto exit_seed;
		}
		if (ret = sb_fipsRngOptionalInput(global_data, md.size, md.digest))
		{
			fprintf(stderr, "Error 7: ");
			goto exit_seed;
		}
moreseeds:
		ret = sb_rngFIPS186Session(
			global_data,
			3*4,
			returned);
		if (ret != SB_SUCCESS) 
		{
			fprintf(stderr, "Error 8: ");
			goto exit_seed;
		}
/*
 *		Convert to ints and test for reasonableness 
 */
		seed1 = seed2 = seed3 = 0;
		for (j = 0, i = 0; i < 4; i++)
			seed1 |= returned[j++] << (i * 8);
		for (i = 0; i < 4; i++)
			seed2 |= returned[j++] << (i * 8);
		for (i = 0; i < 4; i++)
			seed3 |= returned[j++] << (i * 8);

		randcnt++;
	} while ((randcnt < 5) ||
		(returned[0] % 40 )
		|| (!l_reasonable_seed(seed1) || !l_reasonable_seed(seed2) ||
						!l_reasonable_seed(seed3)));
	fprintf(stdout, "\n");
	ret = sb_end(global_data);
	if (ret != SB_SUCCESS) 
	{
		fprintf(stderr, "Error 9: ");
		goto exit_seed;
	}
	if (!(fp = fopen("lmseed.dat", "w")))
		fprintf(stderr, "Can't open lmseed.dat");
	else
	{
		fprintf(fp, 
	"\
Once set, the values for LM_SEED1-3 must be kept secret \n\
and *never* change.\n\n\
#define LM_SEED1 0x%08x\n\
#define LM_SEED2 0x%08x\n\
#define LM_SEED3 0x%08x\n", seed1, seed2, seed3);
		fclose(fp);
	}
		fprintf(stdout, 
	"\
Once set, the values for LM_SEED1-3 must be kept secret and *never* change.\n\
#define LM_SEED1 0x%08x\n\
#define LM_SEED2 0x%08x\n\
#define LM_SEED3 0x%08x\n", seed1, seed2, seed3);
	exit(0);
		

exit_seed:
	fprintf(stderr, "internal Error %d, exiting\n", ret);
	exit(1);
}
#endif  /* VXWORKS */
/*
 *	l_reasonable_seed(unsigned int seed)
 *	returns 1 if reasonable, 0 if not 
 */
l_reasonable_seed(unsigned int seed)
{
	if (!seed || 
		(seed == 1234567890) ||
		(seed == 123456789) ||
		(seed == 0x11223344) ||
		(seed == 0x00112233) ||
		(seed == 0x44556677) ||
		(seed == 0xdeadbeef) ||
		(seed == 0x12345678) ||
		(seed == 0x87654321) ||
		(seed == 0xabcdef01) ||
		(seed == 0x90abcdef) ||
		(seed == 0x01234567) ||
		(seed == 0x76543210) ||
		(seed >  0xfff00000) || /* small negative number */
		(seed <  0x00f00000) ) /* small number */
			return 0;
	return 1; /* success */
}


#if COMMENT
************************************************************

v7.1 enhancements

OVERVIEW
________

Version 7.1 enhancements are significant, and have many interdependent
features, some of which are supported but undocumented for now, including:

	o SIGN=, SIGN2= ... SIGNn=
	o v7.1 SIGN= non-CRO format, which is 12 character, but
	  a bit different/improved from license-key.
	o CRO format in 3 strengths.
	o Ability to use a phase2-like filter on the license-key (may not 
	  be supported in the future).
	o In total there are 6 different types of keys available (5 new):

		- license-key
		- SIGN= without CRO, which is very similar to the license-key
		- LM_STRENGTH_113BIT
		- LM_STRENGTH_163BIT
		- LM_STRENGTH_239BIT
		- phase-2 callback. (undocumented now)

	  The phase-2 business may be phased-out.  



There is a lot of flexibility in this:  
	
	o Every checkout call can specify/override it is required LM_STRENGTH.
	o Similarly, every FEATURE line can specify to lmcrypt what
	  LM_STRENGTH it is SIGN= should be.

	This way a company could decide to have one app at one strength and
	others at another.

	The reason for SIGN<n> is so that a vendor can migrate to 
	another set of seeds or a different key-filter (a la phase2).


The discussion is mostly about CRO.  I will discuss non-cro last.


WHAT IS NOT DOCUMENTED and WHY
_____________________________

1) The SIGN<n>= is not documented for now, but I think this is an important 
feature that may well bail out some important companies in the future.

2) The ability to use a different strength per feature is not documented.
If there is no demand for this, we may well remove it in the future.

3) The business about "key-filters" is likely to go away.  It is there for
several reasons.  First, because this was the initial way we were going to
do CRO.  Second, because we were concerned that companies might demand
extra security without wanting to pay for it, but we are charging for this
anyway.  Finally, the only possible valid reason now, is because companies
may want additional security without the added length of public-key.


BUILD
_____

During the build a number of key files get created, and it is worth
explaining these steps in detail.

The first set of dependencies are

	$(DAEMON) depends on
	lsvendor.o depends on
	lm_new.o/c is made by and depends on 
	lmnewgen depends on
	lmcode.o/c is made by lmrand1, and depends on 
	lm_code.h

These dependencies are the same as v6.1, and the only change is that
more information is put in lm_new.o.  Essentially, the lm_code.h file
is transformed into the lm_new.o file, in an obfuscated format.

The information in lm_new.o is turned into the VENDORCODE struct in
lc_new_job().

Here is what is new in the VENDORCODE struct that gets filled in by lc_new_job():

	int pubkeysize[LM_PUBKEYS];
	unsigned char pubkey[LM_PUBKEYS][LM_MAXPUBKEYSIZ];
        int strength;
	int (*pubkey_fptr)();

LM_PUBKEYS is 3, so, if CRO is used, information for 3 different CRO
strengths is stored in the VENDORCODE struct.  
pubkeysize, pubkey and strength are the required
parameters for calculating public key values:  the size, the key and
the strength.

pubkey_fptr is l_pubkey_verify() everywhere except the license-generator, 
where it is l_prikey_sign().

(The format of this was originally designed to able to be used with any
license-key "filter", like we did in v6 with phase2.  This would still work,
and is tested in the servtest, but we may never support it.)

lm_new.o then contains all of the client-side public-key info.


PUBLIC/PRIVATE KEYS
___________________

But how does lm_code.h, which contains only SEEDs and VENDORKEYs turn into 
the public/private keys?

The public-key pair is generated when lmnewgen runs.  The SEEDs 3 and 4
are used as seeds to create the public-key pair.   You can recreate
them as often as you want and you will get the public-key pair same
numbers every time.  If you were to change 1 bit in the seeds, on every
half the resulting bits in the public-key pair would change -- that is,
the change would be impossible to trace.  There is theoretically no way
to derive SEEDs3 and 4 from the public-key pair.

Therefore, we do not store the public-key pair in a way that the ISV is
particularly conscious of.  The only thing they need to keep track of
the SEEDs 3 and 4.  In addition, a license-generator like gtlicense
can, theoretically, only ask for the seeds in order to generate licenses.


Note that ENCRYPTION_SEED3 and 4 appear nowhere in the companies
product, only the public-key appears in the app/vendor daemon, and the
private-key appears only in license-generators.

lmcode.o contains the info in lm_code.h (it is generated by lmrand1).

This gets linked, with lmnewgen.o, into lmnewgen.  This generates
lm_new.c.  It calls l_gen_pkey_headers() to generate lm_new.o (which
has the public-key) and lmprikey.h (which has the private key).
l_gen_pkey_headers() eventually calls the Certicom routine
sb_genKeyPair() to generate the public-key pairs.

In summary:  lmnewgen uses the seeds 3 and 4 to make the public-key
pair.  The public-key is obfuscated in lm_new.c and the private
key goes into lmprikey.h, which is included in the license
generator source code.  

LICENSE GENERATOR
_________________

lmprikey.h is included, and is used by LM_CODE_GEN_INIT to initialize the
VENDORCODE struct.  Once the struct is initialized, it is just like the client
except that the pubkey info is only the private-key, and the callback 
pubkey_fptr is l_prikey_sign().

When generating licenses, the strength is used, unless 
SIGN=strength:<n> override is used in the license, in which case that 
strength is used.  This is done with L_CONF_STRENGTH_OVERRIDE field in the
CONFIG struct.  (Note that, for security reasons, the name of this field
is a macro defined in h/l_privat.h)

The license-generator has a number of choices:  You have a choice of
having a license-key, and/or any number of SIGN<n>= attributes, but you will
need at least one of the above.  The client specifies which one of these
it needs, and it needs exactly one.  

CLIENT/VDAEMON
______________

Theoretically, a job can have more than one LM_STRENGTH, and different
checkouts might use different strengths.  I believe this works, but
we do not document it, and we may never support it.  I mention this
because the design is such that this *could* work.

lc_new_job() calls l_init().  In this function, the pubkey info from 
the VENDORCODE struct in the job (that was initialized in lm_new.o) 
is used to call l_add_key_filter().  This adds an L_KEY_FILTER to 
the job->key_filters list of possible key-filters.  Each L_KEY_FILTER
applies to a different SIGN<n> level.


CLIENT
______

The client has 2 override possibilities:

	1) what strength it requires, 
		LM_A_STRENGTH_OVERRIDE which sets job->L_STRENGTH_OVERRIDE
		
	2) what SIGN= level it requires. 
		LM_A_SIGN_LEVEL which sets job->L_SIGN_LEVEL
		
The default strength is set in lm_code.h and is in strength in the 
VENDORCODE struct.  The default key-level is 1. A L_SIGN_LEVEL of 0 means
use the license-key, and no SIGN=.

VDAEMON
_______

The vendor-daemon has to authenticate all of the license-key/signatures
in the license.  Upon checkout client specifies which one it requires.
In order for the server to authenticate all of them, it needs parameters
for all 3 CRO strengths as well as all the usual stuff for the license-key
and non-cro SIGN= key.

SEE ALSO
________

testsuite/st_vers.c:  test 49.
The testsuite tests all of these features, and is probably the ultimate
authority for how to use any of these features from an API perspective.

utils/lmnewgen.c:
This generates the public-keys and generates lm_new.c which has the hidden 
public-key.

#endif /* COMMENT */
#ifdef V8_1_COMMENT
To: danielbirns@hotmail.com, slangford@certicom.com, bolson@certicom.com, David Znidarsic <davidz@globes.com>, Matt Christiano <matt@globes.com>
Subject: Summary of review
Cc: richard Mirabella <rich@globes.com> 

Security holes discovered 

	o Reusing Seeds3 and 4 for handshake.  We can defeat this in about 
	  a day on a fast machine. 
	o If the milliseconds does not change between signature generation, 
	  then the first *half* if the signature for 2 signatures.  If true, 
	  a knowledge hacker can immediately determine the private key.  
	  This happens on certain fast systems for 113-bit, including my own 
	  PC. 

Setup. 

	o 96-bit seed S. 
	o provide optional utility to ISVs to generate reliably random 
	  96-bit seed, or the customer makes it up. 
	o Generate from S, n more seeds using FIPS186 random number 
	  generator:  called Tn, where each Tn is 96-bit. 
	o Generating T must be done in one "session". 
	o New customers will use T2 and T3 for Seeds 1-4, so they only type 
	  in S (LM_SEED1-3). 

Public/Private Keypair Generation 

	o Use T1 for generating the public and private key 
	o Use T1 and append the number 113 to generating the 113-bit keypair 
	o Similarly append 163 and 239 for those keypairs.  
	o Ensure that keypairs are generated with seeds that are not 
	  identical, but they dont have to differ randomly, they could 
	  differ by 1. 

Signing 

	o Take time out of algorithm 
	o Seed with Private key, hash of message 
	o Signature will be guaranteed repeatable given the same FEATURE data. 
	o Requires a different certicom "context" for each signature. 

Testing 

	o Add a test to prevent the 2nd discovered problem -- 113-bit 
	  with duplicated first half of signature. 
	o Sign f1-f100 and look for dups in signatures in first half. 

Protect ISVs use of kit 

	o Brian Olson expressed concern about the way the private keys 
	  and seeds3-4 are secured on the client side.  The state of 
	  the art in this area is a hardware unit which is looks and 
	  acts something like a dongle, *except* that the seeds and the 
	  actual signing take place in the hardware unit.  This is 
	  believed to be secure and "relatively" convenient.  Were 
	  certainly not ready to implement something like this today, 
	  and it would probably be unacceptable to all but a small minority. 
	o Its my opinion that any other attempt at "real" security 
	  would lack security, or would be so inconvenient that in 
	  practice ISVs wouldnt be willing to use it, or theyd find 
	  ways to circumvent it.  For example, imagine that every time 
	  you generate licenses you have to type in a password.  This 
	  password would have to have 96-bits, or be an English phrase 
	  that is equivalent, which, given available hacks on phrases, 
	  would have to be a phrase of something like 100 or more 
	  characters in length.  Both of these are too long to realistically 
	  remember and type in every time you use it.  So, the password 
	  would ultimately wind up on disk, or a user would "login" and 
	  never log out or something which would effectively destroy the 
	  security. 
	o That said there is one step we can take today:  improve the 
	  documentation, explaining what needs to be protected.  Today, 
	  there are many files left on disk which are not explained, and 
	  which contained S, or the private keys. 

Conclusion 

	o The only likely candidate for the hack remains our (my) first 
	  guess, reusing Seeds3-4 for the handshake security.  Unless 
	  the hacker explains himself soon, we are probably going to have 
	  to presume this is it. 
	o Two more steps could be taken: 
		1) A Macrovision senior engineer could look at the kit 
		   and try to find some other hole that the hacker found.  
		   If we do this, we should give it some time limit and 
		   stop then unless we find something.  It usually takes 
		   the hackers some period of months to find this stuff, 
		   and its often a concerted effort for many hackers all 
		   over the planet. 
		2) All of the Certicom code is one one C file.  I could 
		   send the finalized revision to Certicom for review for 
		   obvious flaws.  


#endif /* V8_1_COMMENT*/
