/******************************************************************************

	    COPYRIGHT (c) 1995, 2003 by Macrovision Corporation.
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
 *	Module: $Id: lsfilter.c,v 1.15 2003/05/07 18:46:07 daniel Exp $
 *
 *	D. Birns
 *	7/30/95
 *
 *	Last changed:  9/2/97
 *
 */
/*-
 *	The documentation of this needs to be internal to Macrovision
 *	What's tough about this is:
 *	1) There's 4 streams --
 *		a) comments, preceded with '#'
 *		b) CLEAR text, preceded with 'Z'
 *		c) Dictionary entries, that is entries which have
 *		   non-hex characters
 *		d) Entries which are only hex.
 *		e) 8-bit Dictionary entries, preceded with 'X'
 *		We distinguish the streams c and d by the copyright
 *		string.  Each string is prefaced with the next character
 *		from the copyright string.  When we change streams,
 *		we skip 2 characters.
 *	   Using the copyright string.  When the stream changes.
 *	2) Flushing  -- Normally the compressed lines are buffered
 *	   until 70 output characters are accumuluted.  However,
 *	   we also need to be able to flush.
 *	3) The compression technique is:
 *		3 characters to 2.
 *		input (that can be compressed) uses 20-characters:
 *			16 hex + ' ', 'g', '\n' and NO_CHAR
 *		These 3 characters are converted to a number, pretending
 *		that the 20 characters are digits in a base 20 system.
 *		The result is then printed using the 94 characters
 *		above ' ' (all the printable characters aside from ' '),
 *		pretending these are each digits in base 94.
 *		In 2 base 94 numbers, you have 94^2 = 8836 numbers, and
 *		with 3 base 20 numbers you have 20^3 = 8000.  836 to spare!
 */
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "lsserver.h"
#include "ls_log.h"
#include "lsreplog.h"
#include <stdio.h>
#include <errno.h>
#define F_NL	'\n'
#define F_SPC 	' '
#ifndef REP_DEBUG
#define REP_DEBUG 0
#endif /*REP_DEBUG*/

#ifdef SYS_ERRLIST_NOT_IN_ERRNO_H
  extern char *sys_errlist[];
#endif

static int linecont; /* when encounter a line continuation character */
static char hex[3];
static int num[3] = { -1, -1, -1};
static int filter lm_args(( char *, char *));
static void append_replog lm_args((char *));
static char * ascii_filter lm_args(( char *));
static void incr_currc lm_args((lm_noargs));
char *curra; /* This is used by the ascii_filter */

/* kmaclean Jan 13, 2003
 * Do not change this copyright to Macrovision. In fact don't change it at all.
 * It is used to encrypt and decrypt the log file */
static char kcopyright[] = "KCopyright_1988-1996_Globetrotter_Software_Inc_";

static char *copyright = &kcopyright[1];
static char *currc;
static int curr_mode;
#define MODE_DICT  1
#define MODE_EVENT 2
#define COPYLEN (sizeof(kcopyright) - 2)


/*-
 *	ls_lf -- obscure name on purpose
 *	Given a line -- it's added to the output streams
 *	NOTE:  We MUST use fputs here, since printf attempts to
 *	interpret any '%' signs in the output
 *	returns true if line was an "event"
 */
int
ls_lf(line)
char *line;
{
  static char out[500];
  static char *outcp = out;
  int stat;
  int i;
  int cmd;
  int event = 0;
  int ascii = 0;
#define ASCII 1
#define CLEAR 2
  extern int ls_replog_ascii[];
  extern int ls_replog_clear[];
  char *newline;
#ifndef RELEASE_VERSION
  static int compress = -1;
#define COMPRESS compress

	if (compress == -1) compress = !(int)getenv("NO_COMPRESS");
#else
#define COMPRESS 1
#endif /* RELEASE_VERSION */

	if (!line) /*- flush */
	{
#if REP_DEBUG
		puts("ls_lf flush" );
#endif
		if (COMPRESS && (outcp != out))
		{
		  char buf[MAX_REPFILE_LEN];

			filter (0, outcp);
			if (curr_mode != MODE_EVENT)
			{
				if (curr_mode) incr_currc();
				curr_mode = MODE_EVENT;
				incr_currc();
			}
			*buf = *currc;
			incr_currc();
			strcpy(&buf[1], out);
			append_replog(buf);
			*out = '\0';
			outcp = out;
		}
		return 0;
	}
#if REP_DEBUG
	printf("ls_lf %s", line);
#endif

	if (linecont)
	{

		linecont = 0;
		if (line[strlen(line) -2] == '\\')
			linecont = 1;
		newline = ascii_filter(line);
		append_replog(newline);
		return 0;
	}
	if (*line == '#') /* comment */
	{
		append_replog(line);
		return 0;
	}

/*-
 *	Differentiate CLEAR and ASCII
 */
	if (line[1] == ' ' || line[2] == ' ' || line[2] == '\n')
	{
		hex[0] = line[0];
		hex[1] = line[1];
		cmd = 0;
		sscanf(hex, "%x", &cmd);	/* overrun checked */
		for (i = 0; ls_replog_ascii[i] != -1; i++)
			if (cmd == ls_replog_ascii[i]) ascii = ASCII;
		if (!ascii)
			for (i = 0; ls_replog_clear[i] != -1; i++)
				if (cmd == ls_replog_clear[i]) ascii = CLEAR;
		if (ascii == ASCII)
		{
			if (line[strlen(line) -2] == '\\')
				linecont = 1;
#if REP_DEBUG
			{
		        int t = 0;
			if (*line == '7')
			{
				t = 1;
				printf("HERE1: \"%s\"", line);
			}
#endif
			newline = ascii_filter(line);

#if REP_DEBUG
			if (t) printf("HERE2: \"%s\"", newline);
			}
#endif
			append_replog(newline);
			return 0;

		}
		else if (ascii == CLEAR)
		{
		  char buf[MAX_REPFILE_LEN];

			sprintf(buf, "Z%s", line); /* Prepend 'Z' */
			append_replog(buf);
			if (cmd == LL_START)
			{
				curr_mode = 0;
				currc = 0;
				curra = 0;
			}
			return 0;
		}
		else if (cmd ==  LL_CHECKIN) event = 1;
		else if (cmd ==  LL_CKSUM)
		{
			currc = kcopyright;
			curr_mode = MODE_EVENT;
			curra = 0;
		}
	}
/*-
 *	filter stream
 *	filter may be called more than once for input lines longer than can
 *	be compressed on a single output line (FILTER_LEN)
 */
	stat = 1;
	if (COMPRESS)
	{
		while (stat)
		{
			stat = filter (line, outcp);
			if (strchr(outcp, '\n'))
			{
			  char buf[MAX_REPFILE_LEN];
				if (curr_mode != MODE_EVENT)
				{
					if (curr_mode)incr_currc();
					curr_mode = MODE_EVENT;
					incr_currc();
				}
				*buf = *currc;
				incr_currc();
				strcpy(&buf[1], out);
				append_replog(buf);
				outcp = out;
			}
			else
				outcp += strlen(outcp);
		}
	}
	else
		append_replog(line);
	return event;
}


static
int
filter(str, ret)
char *str;
char *ret;
{
#define SQ20 20 * 20

  char tmp[500];
  char *cp;
  char *outcp = ret;
  static int cnt, idx, outcnt;
  int i;

	if (!str) /*- flush */
	{
		if (num[0] == -1) num[0] = 19;  /* 19 is the special NO_CHAR */
		if (num[1] == -1) num[1] = 19;
		if (num[2] == -1) num[2] = 19;
		idx = cnt = 0;
		i = num[0] + (num[1] * 20) + (num[2] * SQ20);
		*outcp++ = i%FILTER_OBASE + FILTER_START;
		*outcp++ = i/FILTER_OBASE + FILTER_START;
		*outcp++ = '\n';
		*outcp = '\0';
		outcnt = 0;
		return (0);
	}
	for (cp = str; *cp; cp++)
	{
		idx = cnt++ % 3;
		switch (*cp)
		{
		case LL_EQUAL: 		num[idx] = 16; break;
		case F_SPC: 		num[idx] = 17; break;
		case F_NL: 		num[idx] = 18; break;
		default:		*hex = *cp;
					hex[1] = 0;
					sscanf(hex, "%x", &num[idx]) ;	/* overrun checked */
		}
		if (idx == 2)
		{
			i = num[0] + (num[1] * 20) + (num[2] * SQ20);
			num[0] = num[1] = num[2] = -1;
			*outcp++ = i%FILTER_OBASE + FILTER_START;
			*outcp++ = i/FILTER_OBASE + FILTER_START;
			outcnt += 2;
			if (outcnt >= FILTER_LEN) /*- return what we have */
			{
				outcnt = 0;
				*outcp++ = '\n';
				*outcp = '\0';
				cp++;
/*-
 *				move the remainder of the string to
 *				the beginning, to finish it later
 */
				*tmp = 0;
				if (*cp)
				{
					strcpy(tmp, cp);
					strcpy(str, tmp);
				}
				return (*tmp); /*- more to come? */
			}
		}
	}
	*outcp = '\0';
	return 0; /*- need more to fill buffer */
}
char cksumbuf [CKSUMBUF_SIZE];
int buflen;

static
void
append_replog(str)
char *str;
{

  extern FILE *ls_log_repfile;
  extern CLIENT_DATA *ls_log_repfile_client;

	if (*str != '#') /* comment */
	{
		strcat(&cksumbuf[buflen], str);
		buflen += strlen(str);
	}

	if (ls_log_repfile)
	{
		if (fputs(str, ls_log_repfile) <= EOF)
		{
			LOG(("Error:  Can't write to reportlog:  %s", SYS_ERRLIST(errno)));
			ls_log_repfile = 0;
		}

	}
	else if (ls_log_repfile_client)
	{
		ls_client_send_str(ls_log_repfile_client, str);
	}
}
void
ls_flush_replog()
{
  extern FILE *ls_log_repfile;

	if (ls_log_repfile) fflush(ls_log_repfile);
}
static char arr[] =
"#F7/]\042 \025J\015e1GfKiE\0272R?.&'b)\021`DO$\022!_,\024\0233\017MY-hT90j>=%AS\034:U6+\020I8B[<LZ\031H^\0265a\035*W\134g\016\033P\037@d(;4\032\014X\030N\036CcQV";

#if 0
/*********************************************************************
the above string is, after applying the offset:

"7ZKCq64)^!yE[z_}Y+FfSB:;v=%tXc8&5s@('G#amA|hMD~RQ9Ug0NiJ?$]LVoP`n-\r*Iu1>kp{"/d3Tx<OH. l,b2Wwej"

This is all printable characters in a random order. (I'll say!)
***********************************************************************/
#endif
#define ARRLEN (sizeof(arr) - 1)

#define ARROFFSET 20
static
char *
ascii_filter(str)
char *str;
{
  unsigned char *cp;
  static char arr2[200];
  int offset;
  static unsigned char buf[5000];
  int is8bit = 0;
	for (cp = (unsigned char *)str; *cp; cp++)
		if (*cp & 0x80) /* 8bit value */
		{
			is8bit = 1;
			break;
		}



	if (!curra) curra = arr2;
	if (!*arr2)
	{
		strcpy(arr2, arr);
		strcat(arr2, arr);
	}
	if (curr_mode != MODE_DICT)
	{
		if (curr_mode)incr_currc();
		curr_mode = MODE_DICT;
		incr_currc();
	}
	memset(buf, 0, sizeof(buf));
	cp = buf;
	if (is8bit)  *cp++ = 'X';
	*cp++ = (unsigned char)*currc;
	incr_currc();
	if (!is8bit)
	{
		strcpy((char *)cp, str);


		for (; *cp; cp++)
		{
			if (*cp < ' ')
				/*- don't encrypt these */
				continue;
			else
			{
				offset = *cp - ' ';
				*cp = curra[offset] + ARROFFSET;
				curra += offset;
				if (curra > (arr2 + ARRLEN)) curra -= ARRLEN;
			}
		}
	}
	else
	{
	  int byte, inibble;
	  char nibble;
		for (byte = 0; str[byte]; byte++)
		{
			if ((unsigned char)str[byte] < ' ')
			{
				*cp++ = str[byte];
				continue; /*- don't encrypt these */
			}
			/* high order halfbyte first */
			for (inibble = 1; inibble >= 0; inibble--)
			{
/*
 *	for 8bit values, we don't want an 8bit character going into the
 *	reportlog, since that's a no-no for email.
 *	So, we convert each byte into 2 bytes, each of which is guaranteed
 *	to be printable 7-bits:
 *	1) split byte into 2 nibbles (halfbytes) 11000000 becomes
 *					1100 0
 *	2) shift each nibble left by 2: 110000, 0
 *	3) OR with 1000000 = 64/0x40:  1110000 1000000
 *	4) then encode as if we had 2 characters.
 *	To unfilter, reverse it:  1110000  	0
 *	1)remove the 7th-bit(64):  110000 	0
 *	2) shift right 2:	     1100 	0
 *	3) put back together	 11000000
 *	et voilà!
 */
				nibble = (str[byte] >> (inibble * 4)) & 0xf ;
				nibble <<= 2;   /* shift left 2 bits */
				nibble |= 0x40; /* turn on 7th bit */
				offset = nibble - ' ';
				*cp++ = curra[offset] + ARROFFSET;
				curra += offset;
				if (curra > (arr2 + ARRLEN)) curra -= ARRLEN;
			}
		}
	}
#if REP_DEBUG
	printf("%s%d buf is ==>%s", __FILE__, __LINE__, buf);
#endif
	return (char *)buf;
}
static
void
incr_currc()
{
	if (!currc)
		currc = copyright;
	else
	{
		currc++;
		if (currc >= (copyright + COPYLEN))
			currc = copyright;
	}
}
