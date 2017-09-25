#include <stdio.h>
#include <stdlib.h>
#include "lmclient.h"
#include "sys/types.h"
#include "sys/stat.h"
/******************************************************************************

	COPYRIGHT (c) 1998, 2003 by Macrovision Corporation.
	This software has been provided pursuant to a License Agreement
	containing restrictions on its use.  This software contains
	valuable trade secrets and proprietary information of 
	Macrovision Corporation and is protected by law.  It may 
	not be copied or distributed in any form or medium, disclosed 
	to third parties, reverse engineered or used in any manner not 
	provided for in said License Agreement except with the prior 
	written authorization from Macrovision Corporation.

 *****************************************************************************/
/*	
 *	Module: $Id: lm_rand3.c,v 1.53 2003/04/18 23:48:07 sluu Exp $
 *
 *	D. Birns
 *	May, 1998
 *
 *	Last changed:  6/11/98
 *
 */
static int rand8 lm_args((lm_noargs));
static void srand8 lm_args((int, int, int));
static void err_exit lm_args((char *));
static void confirm(void);
static void open_files(void);
static void setup_vals(void);
static void rand_arr(int *ret, int size);

#define MAX_CRYPT_BYTES 20 /* can't exceed numnames (see below) */
#define NUM_BITS MAX_CRYPT_BYTES * 8
char *l_appfile;

char mask[NUM_BITS];
unsigned char xor_vals[MAX_CRYPT_BYTES];
char lineflags[NUM_BITS];
#define MASK_INIT_VAL 40 /* mask values are 0-7, 40 is invalid */

char *l_appfile;
char *names[] = { "bit0", 
		"bit1", 
		"bit2", 
		"bit3", 
		"bit4", 
		"bit5", 
		"bit6", 
		"bit7" };

/* 
 *	make sure there are as many numnames as MAX_CRYPT_BYTES 
 */
char *numnames[] = { 
		"num0", 
		"num1", 
		"num2", 
		"num3", 
		"num4", 
		"num5", 
		"num6", 
		"num7",
		"num8",
		"num9",
		"num10",
		"num11",
		"num12",
		"num13",
		"num14",
		"num15",
		"num16",
		"num17",
		"num18",
		"num19",
		"num20",
		};

char **staticp;
char **staticp;

#define MAX_OUT_LINE 200

void l_puts_rand lm_args(( FILE *, FILE *, int));
void l_puts_rand1 lm_args((FILE *, int, char **));
static void print_files(void);
char licoutbuf[8 * MAX_CRYPT_BYTES * MAX_OUT_LINE];
char *licoutcp = licoutbuf;
char *liclines[8 * MAX_CRYPT_BYTES];
char **liclinesp = liclines;

char appoutbuf[8 * MAX_CRYPT_BYTES * MAX_OUT_LINE];
char *appoutcp = appoutbuf;
char *applines[8 * MAX_CRYPT_BYTES];
char **applinesp = applines;

char xoroutbuf[MAX_CRYPT_BYTES * MAX_OUT_LINE];
char *xoroutcp = xoroutbuf;
char *xorlines[MAX_CRYPT_BYTES];
char **xorlinesp = xorlines;
char appxoroutbuf[MAX_CRYPT_BYTES * 8 * MAX_OUT_LINE];
char *appxoroutcp = appxoroutbuf;
char *appxorlines[8 * MAX_CRYPT_BYTES];
char **appxorlinesp = appxorlines;

char varoutbuf[MAX_CRYPT_BYTES * MAX_OUT_LINE];
char *varoutcp = varoutbuf;
char *varlines[MAX_CRYPT_BYTES];
char **varlinesp = varlines;
#ifndef RELEASE_VERSION
static char *readable_client = (char *)-1;
#endif /* RELEASE_VERSION */


char *str0 = "\
/****************************************************************************\n\
\n\
	COPYRIGHT (c) 1998 by Macrovision Corporation.\n\
	This software has been provided pursuant to a License Agreement\n\
	containing restrictions on its use.  This software contains\n\
	valuable trade secrets and proprietary information of \n\
	Macrovision Corporation and is protected by law.  It may \n\
	not be copied or distributed in any form or medium, disclosed \n\
	to third parties, reverse engineered or used in any manner not \n\
	provided for in said License Agreement except with the prior \n\
	written authorization from Macrovision Corporation.\n\
\n\
 ***************************************************************************/\n\
#include <lmclient.h>\n";
char *statics[] = 
{ 
	"static int num0 = 0;\n" ,
	"static int num1 = 1;\n", 
	"static int num2 = 2;\n", 
	"static int num3 = 3;\n", 
	"static int num4 = 4;\n", 
	"static int num5 = 5;\n", 
	"static int num6 = 6;\n", 
	"static int num7 = 7;\n", 
	"static int num8 = 8;\n", 
	"static int num9 = 9;\n", 
	"static int num10 = 10;\n", 
	"static int num11 = 11;\n", 
	"static int num12 = 12;\n", 
	"static int num13 = 13;\n", 
	"static int num14 = 14;\n", 
	"static int num15 = 15;\n", 
	"static int num16 = 16;\n", 
	"static int num17 = 17;\n", 
	"static int num18 = 18;\n", 
	"static int num19 = 19;\n", 
	"static int num20 = 20;\n", 
	"static int bit0 = 0x2801;\n",  /* these numbers are 1 2 4 8, etc. */
	"static int bit1 = 0x102;\n",   /* with random high-order bits */
	"static int bit2 = 0x2704;\n",	/* turned on, for obfuscation */
	"static int bit3 = 0x3108;\n",
	"static int bit4 = 0x3210;\n",
	"static int bit5 = 0xa720;\n",
	"static int bit6 = 0x740;\n",
	"static int bit7 = 0xcc80;\n"
};
int num_statics = sizeof(statics)/sizeof(char *);

char *str2 = "\
void\n\
user_crypt_filter";
char *str2_ph2 = "\
void\n\
phase2";

char *str3 = "(job, inchar, idx";
char *str4 = ")\n\
LM_HANDLE *job;\n\
char *inchar;\n\
int idx;\n";

char *str5 = "\
{\n\
  unsigned char c = 0;\n\
  unsigned char in_c = *inchar;\n";

FILE *client_fp;
FILE *licgen_fp;
long seed[3];
int quiet;
int phase2 = 0;


API_ENTRY
l_filter_gen(argc, argv)
char **argv;
{
  char pic[5];
  char *arg;
  int i, j;


	if (argc < 5) err_exit("3 random hex seeds required\n");
	for (i = 1, j = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "-phase2"))
		{
			phase2 = 1;
			continue;
		}
		if (!strcmp(argv[i], "-filter_gen")) continue;
		if (!strcmp(argv[i], "-q")) 
		{
			quiet = 1;
			continue;
		}
		strcpy(pic, "%ld");
		arg = argv[i];
		if (argv[i][0] == '0' &&
			(argv[i][1] == 'x' || argv[i][1] == 'X'))
		{
			pic[0]='%';
			pic[1]='l';
			pic[2]=argv[i][1];
			pic[3] = 0;
			arg = &argv[i][2];
		}
		sscanf(arg, pic, &seed[j++]);	/* overrun checked */
	}
	open_files();
	if (!quiet) confirm();
	setup_vals();
	print_files();
	return 0;
}
static
void
confirm(void)
{
	printf(
"\
%s\
\n\
This generates 2 source files, which you must *never edit*:\n\
%s.c: must be linked into vendor daemon, and all applications \n\
	    calling lc_checkout().  These applications must call\n\
		lc_set_attr(lm_job, LM_A_%s, \n\
					(LM_A_VAL_TYPE)%s);\n\
	    after lc_new_job().  Also, lsvendor.c must have\n\
		extern void %s();\n\
		void (*ls_a_%s)() = %s;\n\
\n\
%s.c: this must be linked into all license generators:\n\
	    makekey, lmcrypt, programs that call lc_cryptstr().\n\
	    In these programs, after lc_init(), call\n\
		lc_set_attr(lm_job, LM_A_%s,\n\
				(LM_A_VAL_TYPE)%s);\n\
\n\
The seeds you picked are (in hex):  0x%x 0x%x 0x%x.\n\
Make sure that you save these seeds somewhere safe.\n\
You can recreate these files by rerunning this program with the same seeds.\n\
(Use -q in the future to skip this message.)\n\
Press RETURN to continue...", 
phase2 ? "**Phase-2**\n" : "** Filter-Generator: Additional security **\n", 
phase2 ? "lmph2app" : "lmappfil",
phase2 ? "PHASE2_APP" : "USER_CRYPT_FILTER",
phase2 ? "phase2_app" : "user_crypt_filter",
phase2 ? "phase2_app" : "user_crypt_filter",
phase2 ? "phase2_app" : "user_crypt_filter",
phase2 ? "phase2_app" : "user_crypt_filter",
phase2 ? "lmph2gen" : "lmkeyfil",
phase2 ? "PHASE2_GEN" : "USER_CRYPT_FILTER_GEN",
phase2 ? "phase2_gen" : "user_crypt_filter_gen",
seed[0], seed[1], seed[2]);
getchar();

}
static
void
open_files(void)
{
  struct stat st;
  char *appname = phase2 ? "lmph2app.c" : "lmappfil.c";
  char *genname = phase2 ? "lmph2gen.c" : "lmkeyfil.c";

	if (!stat(appname, &st))
	{
		err_exit(phase2 ? "lmph2app.c already exists." : 
			"lmappfil.c already exists.");
	}
	if (!stat(genname, &st))
	{
		err_exit(phase2 ? "lmph2gen.c already exists." : 
			"lmkeyfil.c already exists.");
		err_exit("lmkeyfil.c already exists.");
	}
	if (!(client_fp = (fopen(appname, "w"))))
	{
		err_exit(phase2 ? "Cannot open lmph2app.c" :
			"Cannot open lmappfil.c");
	}
	if (!(licgen_fp = (fopen(genname, "w"))))
	{
		err_exit(phase2 ? "Cannot open lmph2gen.c" :
			"Cannot open lmkeyfil.c");
	}
}
static
void
setup_vals(void)
{
/* 
 *	Setup mask
 */
  int byte;
  int bit;
  int r;

	srand8(seed[0] & 0xff, (seed[0] >> 16) & 0xff, seed[1] & 0xff );
 	
	memset(mask, MASK_INIT_VAL, sizeof(mask));
	for (byte = 0 ; byte < MAX_CRYPT_BYTES; byte++)
	{
		xor_vals[byte] = (rand8()/256) % 256;
	}
	srand8((seed[1] >> 16) & 0xff, seed[2] & 0xff, (seed[2] >> 16) & 0xff);
	for (byte = 0 ; byte < MAX_CRYPT_BYTES * 8; byte += 8)
	{
		for (bit = 0; bit < 8;)
		{
			 r = (rand8() / 256) % 8; /* between 0 and 7 */
			 if (mask[r + byte] == MASK_INIT_VAL) 
			 	mask[r + byte] = bit++;
		}
	}
}
static
void
print_files(void)
{
  int byte;
  int bit;
  int i, j;

	for (byte = 0 ; byte < MAX_CRYPT_BYTES * 8; byte += 8)
	{
		*varlinesp++ = ++varoutcp;
		sprintf(varoutcp, "static unsigned char x_%d\t= 0x%x;\n", 
			byte/8, xor_vals[byte/8]);
		varoutcp += strlen(varoutcp) + 1;

/* 
 *		license generate doesn't need tests, but app does
 */
		*xorlinesp++ = ++xoroutcp;
		sprintf(xoroutcp, "\tif (idx == %s)\tin_c ^= x_%d;\n",
			numnames[byte/8], byte/8);
		xoroutcp += strlen(xoroutcp) + 1;

/*
 *		app xor lines with test
 *		Do the XOR 1 bit at a time, and test each bit's result
 */

		for (bit = 0; bit < 8; bit++)
		{
/* 
 *			Do license generator lines
 */
			*liclinesp++ = ++licoutcp;
			sprintf(licoutcp, "\tif ((idx == %s)\t&& ", 
							numnames[byte/8]);
			licoutcp += strlen(licoutcp) ;
			sprintf(licoutcp, "\t(in_c & %s))", 
						names[mask[bit + byte]]);
			licoutcp += strlen(licoutcp) ;
			sprintf(licoutcp, "\tc |= %s;\n", names[bit]);
			licoutcp += strlen(licoutcp) + 1;
/*
 *			Do application lines
 */
			*applinesp++ = ++appoutcp;
			sprintf(appoutcp, "\tif (idx == %s)\n\t{\n", 
							numnames[byte/8]);
			appoutcp += strlen(appoutcp) ;
			sprintf(appoutcp, "\t\tif (in_c & %s)", 
				names[bit]);
			appoutcp += strlen(appoutcp) ;
			sprintf(appoutcp, "\tc |= %s;\n", 
				names[mask[bit + byte]]);
			appoutcp += strlen(appoutcp);

			sprintf(appoutcp, "\t}\n");
			appoutcp += strlen(appoutcp) + 1;
/*
 *			Do xor lines
 */
			*appxorlinesp++ = ++appxoroutcp;
			sprintf(appxoroutcp, "\tif (idx == %s) \n\t{\n\t\t", numnames[byte/8]);
			appxoroutcp += strlen(appxoroutcp);
			if (xor_vals[byte/8] & (0x1 << bit))
			{
				sprintf(appxoroutcp, "if (c & %s)\tc &= ~%s;\n\t\t",
					names[bit], names[bit]);
				appxoroutcp += strlen(appxoroutcp);
				sprintf(appxoroutcp, "else\t\tc |= %s;\n\t\t", names[bit]);
				appxoroutcp += strlen(appxoroutcp);
			}
			sprintf(appxoroutcp, 
			"if (((c & %s) ^ (expchar & %s)) & 0xff) {*inchar = c ^ %d; return;}\n\t}\n",
				names[bit], names[bit],
				(rand8()>>2) % 0xff);
			appxoroutcp += strlen(appxoroutcp) + 1;
		}
	}
	fprintf(licgen_fp, "%s", str0);
	fprintf(client_fp, "%s", str0);
	fprintf(client_fp, "\
\n\
/************ \n\
 ************ Seeds used:  0x%x 0x%x 0x%x\n\
 ************/ \n", seed[0], seed[1], seed[2]);
	fprintf(licgen_fp, "\
\n\
/************ \n\
 ************ Seeds used:  0x%x 0x%x 0x%x\n\
 ************/ \n", seed[0], seed[1], seed[2]);

	for (i = 0; i < num_statics; i++)
		fprintf(licgen_fp, "%s", statics[i]);
/*
 *	dump str1 static stuff in random order in the client code
 */
	for (j = num_statics - 1; j >= 0; j--)
	{
	  int r;
	  int k;
	  	if (j == 0) r = 0;
		else r = rand8() % (j + 1);
		if (statics[r]) fputs(statics[r], client_fp);
		for (k = r; k < j; k++)
			statics[k] = statics[k + 1];
	}
/*
 *	dump xor_val declarations in random order
 */
	for (j = MAX_CRYPT_BYTES - 1; j >= 0; j--)
	{
	  int r;
	  int k;
	  	if (j == 0) r = 0;
		else r = rand8() % (j + 1);
		if (varlines[r]) 
		{
			fputs(varlines[r], licgen_fp);
			fputs(varlines[r], client_fp);
		}
		for (k = r; k < j; k++)
			varlines[k] = varlines[k + 1];
	}
	fprintf(licgen_fp, "%s_gen%s%s%s", phase2 ? str2_ph2 : str2, 
					str3, str4, str5);
	fprintf(client_fp, 
    "%s%s%s, ec%sunsigned int ec;\n%s  unsigned char expchar = (ec & 0xff);\n", 
		phase2 ? str2_ph2 : str2, phase2 ? "_app" : "", str3, str4, str5);
/*
 *	dump xor assignments in random order
 */
	for (j = MAX_CRYPT_BYTES - 1; j >= 0; j--)
	{
	  int r;
	  int k;
	  	if (j == 0) r = 0;
		else r = rand8() % (j + 1);
		if (xorlines[r]) 
		{
			fputs(xorlines[r], licgen_fp);
		}
		for (k = r; k < j; k++)
			xorlines[k] = xorlines[k + 1];
	}
/*
 *	dump liclines in random order
 */
	for (j = NUM_BITS - 1; j >= 0; j--)
	{
	  int r;
	  int k;
	  	if (j == 0) r = 0;
		else r = rand8() % (j + 1);
		if (liclines[r]) fputs(liclines[r], licgen_fp);
		for (k = r; k < j; k++)
			liclines[k] = liclines[k + 1];
	}
/*
 *	dump applines in random order
 */
	for (j = NUM_BITS - 1; j >= 0; j--)
	{
	  int r;
	  int k;
	  	if (j == 0) r = 0;
		else r = rand8() % (j + 1);
		if (applines[r]) fputs(applines[r], client_fp);
		for (k = r; k < j; k++)
			applines[k] = applines[k + 1];
	}
/*
 *	dump app xor assignments in random order
 */
	for (j = NUM_BITS - 1; j >= 0; j--)
	{
	  int r;
	  int k;
	  	if (j == 0) r = 0;
		else r = rand8() % (j + 1);
		if (appxorlines[r]) 
		{
			fputs(appxorlines[r], client_fp);
		}
		for (k = r; k < j; k++)
			appxorlines[k] = appxorlines[k + 1];
	}
/*
 *	Finish
 */
#define CLOSEBRACE 0x7d
	fprintf(licgen_fp, "\t*inchar = c;\n%c\n", CLOSEBRACE);
	fprintf(client_fp, "\t*inchar = c;\n%c\n", CLOSEBRACE);
	fclose(licgen_fp);
	fclose(client_fp);
}
static
void
err_exit(str)
char *str;
{
	fprintf(stderr, "%s\n", str);
	fprintf(stderr, 
	"\
usage:  lmrand1 -filter_gen seed1 seed2 seed3 [-q]\n\
	This is used for extra licensing security --\n\
	Requires the Flexible API (lc_xxx).\n\
");
	exit(1);
}


static int s1 = 1;
static int s2 = 1;
static int s3 = 1;

#define MODMULT(a,b,c,m,s) q = s/a; s = b * (s - a * q) - c * q; \
        if(s < 0) s += m
#define R1      32363
#define R2      31727
#define R3      31657

static int rand8(lm_noargs) {
        int q;
        int z;

        MODMULT(206, 157, 21, R1, s1);
        MODMULT(217, 146, 45, R2, s2);
        MODMULT(222, 142, 133, R3, s3);

        z = s1 - s2;
        if(z > 706) z -= 32362;
        z += s3;
        if(z < 1) z += 32362;
        return(z >> 1); 
}

/*
 *	Takes 3 16-bit values 
 */
static void srand8(i1, i2, i3)
int i1;
int i2;
int i3;
{
        s1 = i1 % R1;
        s2 = i2 % R2;
        s3 = i3 % R3;
        if(s1 <= 0) s1 += R1 - 1;
        if(s2 <= 0) s2 += R2 - 1;
        if(s3 <= 0) s3 += R3 - 1;
}
#if 0

main() {
        int i;
        srand8(100, 100, 100);
        for(i = 0; i < 1000; i++) {
                printf("%#06x\n", rand8());
        }
}

#endif

void
l_puts_rand1(fp, num, cpp)
FILE *fp;
int num;
char **cpp;
{
  int j;
  int *arr;

#ifndef RELEASE_VERSION
	if (readable_client == (char *)-1)
	{
		readable_client = getenv("L_READABLE");	/* overrun checked */
	}

	if (readable_client || getenv("NORAND"))	/* overrun checked */
	{
		for (j = 0 ; (j < num) && cpp[j]; j++)
			fputs(cpp[j], fp);
		return;
	}
#endif /*- RELEASE_VERSION */
	arr = (int *) malloc(sizeof(int) * num);
	rand_arr(arr, num);
	for (j = num - 1; j >= 0; j--)
	{
		if (cpp[arr[j]]) 
		{
#ifdef RELEASE_VERSION
		  int i;
			if (!strstr(cpp[arr[j]], "goto"))
			{
				for (i = 0; cpp[arr[j]][i]; i++)
				{
					if (cpp[arr[j]][i] == '\t')
						cpp[arr[j]][i] = ' ';
					else if (cpp[arr[j]][i] == '\n' && (j%10))
						cpp[arr[j]][i] = ' ';
					/* remove some, but not all, newlines */
				}
			}

#endif /* RELEASE_VERSION */
			fputs(cpp[arr[j]], fp);
		}
	}
	free(arr);

}

void
l_puts_rand(out, in, num)
FILE *out;
FILE *in;
int num;
{
#define	BUF_SIZE	10240
  int j;
  int *arr;
  char	buffer[BUF_SIZE];
  long	lSize = 0;
  int	iRead = 0;
	if(in)
	{
		fflush(in);
		fseek(in, 0, SEEK_END);
		/*	Determine file size	*/
		lSize = ftell(in);
	}

#ifndef RELEASE_VERSION
	if (readable_client == (char *)-1)
	{
		readable_client = getenv("L_READABLE");	/* overrun checked */
	}

	if (readable_client || getenv("NORAND"))	/* overrun checked */
	{
		for (j = 0 ; (j < num) && ((j * BUF_SIZE) < lSize); j++)
		{
			memset(buffer, 0, sizeof(buffer));
			fseek(out, j * BUF_SIZE, SEEK_SET);
			fread(buffer, BUF_SIZE, 1, in);	/* overrun checked */
			fputs(buffer, out);
		}
		return;
	}
#endif /*- RELEASE_VERSION */
	arr = (int *) malloc(sizeof(int) * num);
	rand_arr(arr, num);
	for (j = num - 1; j >= 0; j--)
	{
		if( (arr[j] * BUF_SIZE) < lSize && in)
		{
			int	i;

			memset(buffer, 0, sizeof(buffer));
			fseek(in, (arr[j] * BUF_SIZE), SEEK_SET);
			iRead = fread(buffer, BUF_SIZE, 1, in);	/* overrun checked */
#ifdef RELEASE_VERSION
			if (!strstr(buffer, "goto"))
			{
				for (i = 0; buffer[i]; i++)
				{
					if (buffer[i] == '\t')
						buffer[i] = ' ';
					else if (buffer[i] == '\n' && (j%10))
						buffer[i] = ' ';
					/* remove some, but not all, newlines */
				}
			}

#endif /* RELEASE_VERSION */
			fputs(buffer, out);
		}
	}
	free(arr);

}
/*
 *	Fill array with random numbers between 0 and size
 *	Fixed in v8.1E to be much faster
 */
static
void
rand_arr(int *ret, int size)
{
  int i;
  int *a, *b, r, j;
	a = (int *)malloc(sizeof(int) * size);
	b = (int *)malloc(sizeof(int) * size);
	memset(a, 0, sizeof(int) * size);
	for (i = 0; i < size; i++)
		a[i] = i;
	for (i = size - 1; i  >= 0; i--)
	{

		if (i) r =  rand8() % (i + 1);
		else r = 0;
		ret[i] = a[r];
		/* Remove a[r] from a, using b as temp space */
		memcpy(b, &a[r + 1], i - r);
		memcpy(&a[r], b, i - r);
	}
	free(a);
	free(b);
}
