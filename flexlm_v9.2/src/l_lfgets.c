/******************************************************************************

	    COPYRIGHT (c) 1992, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_lfgets.c,v 1.40 2003/06/13 17:17:36 sluu Exp $
 *
 *	Function:	l_lfgets(job, string, max, lfptr, numlines)
 *			l_lfseek(lfptr, where, how)
 *			l_lfclose(lfptr)
 *
 *	Description: 	l_lfgets Gets the next line from the license file.
 *			l_lfseek "seeks" the license file ptr.
 *			l_lfclose closes the pointer if it is a file.
 *
 *	Parameters:	(int) max - Maximum size of license file line
 *			(LICENSE_FILE *) lfptr - License file pointer
 *			(ptr to int)numlines - return number of actual
 *					text lines read, or ignore if NULL
 *
 *	Return:		(char *) string - Filled in - next license file line
 *
 *	M. Christiano
 *	6/12/92
 *
 *	Last changed:  11/6/98
 *
 */

#include <string.h>
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "l_openf.h"

static char * lfgets_tokens lm_args(( LM_HANDLE *, char *, int, LICENSE_FILE *, int *));
static void lfptr_eof lm_args((LM_HANDLE *, LICENSE_FILE *));
static void l_lf_put_token lm_args(( LM_HANDLE *, char *, LICENSE_FILE *));
static int line_continuation(int this_char,int prev_char,int next_char);

#define L_STRING_CONCAT '-'

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

/*
 *	-------------------------------------------------------------
 *	get_token stuff
 */
#define L_TOKEN_MAIN 		1
#define L_TOKEN_REGULAR 	2
#define L_TOKEN_COMMENT 	3
#define L_TOKEN_NEWLINE 	4
#define L_TOKEN_DECIMAL		5


static char *l_main_tokens[] = {
	"BORROW_LOWWATER",
	"COMM_TRANSPORT",
	"DAEMON",
	"DEBUGLOG",
	"END_LICENSE",
	"EXCLUDE",
	"EXCLUDE_BORROW",
	"EXCLUDEALL",
	"FEATURE",
	"FEATURESET",
	"GROUP",
	"HOST_GROUP",
	"INCLUDE",
	"INCLUDE_BORROW",
	"INCLUDEALL",
	"INCREMENT",
	"LINGER",
	"MAX",
	"MAX_BORROW_HOURS",
	"MAX_OVERDRAFT",
	"NOLOG",
	"PACKAGE",
	"REPORTLOG",
	"RESERVE",
	"SERVER",
	"START_LICENSE",
	"TIMEOUT",
	"TIMEOUTALL",
	"TRANSPORT",
	"UPGRADE",
	"USE_SERVER",
	"USER_GROUP",
	"VENDOR",
	0
};
static int ismaintoken(LM_HANDLE *job, char *string)
{
	char **cpp;

	for (cpp = l_main_tokens; *cpp; cpp++)
	{
		if (l_keyword_eq(job, string, *cpp))
			return 1;
	}
	return 0;
}


/***********************************************************************
 * Get the next word from the license file taking into account
 * line continueations. return the type of token the word is.
 *
 * Parameters:
 * 		job			- the job handle
 * 		string      - Put the next token/word here on return
 * 		max			- max number of chars to read
 * 		lfptr		- license file
 * 		numlines	- return the number of actual lines read
 *
 * Return:
 * 		the type of token found. one of the L_TOKEN_ #defines
 * 		0 - on an error
 *
 ************************************************************************/
static int l_lf_get_token(LM_HANDLE *job, char *string, int max, LICENSE_FILE *lfptr, int *numlines)
{
  	int ret = L_TOKEN_COMMENT;
  	int num;
  	/* P2122 */
  	int i_am_client = (int) ((job->flags & LM_FLAG_CHECKOUT) ||
  			!(job->flags &(LM_FLAG_LMUTIL | LM_FLAG_LMGRD | LM_FLAG_IS_VD)));
  	int c;
  	int prev_char = 0;
  	char *cp = string;
  	char *maxcp = string + max;
  	int start_line = (lfptr->flags & LM_LF_FLAG_LINE_START);

  	/*
  	 *	Check if last item parsed was a '\\', added to fix bug P6288 where
  	 *	if you have a line continuation and the very next line begins with a
  	 *	main token, it incorrectly handles it.  For example:
  	 *	..... ISSUER=" \
  	 *		group " ....
  	 *	We handle the continuation after ISSUER correctly but we think "group"
  	 *	is a keyword instead of a name of the issuer.
  	 */
  	int bPrevCont = (lfptr->flags & LM_LF_FLAG_CONTINUATION);
  	int bLineCont = 0;
  	int bFoundCont = 0;

	num= 0;
	*string = 0;
	DEBUG(("get_token l:%d \n", __LINE__));
	lfptr->flags &= ~( LM_LF_FLAG_LINE_START | LM_LF_FLAG_CONTINUATION);
	if (lfptr->flags & LM_LF_FLAG_EOF)
		return 0;
	if (lfptr->bufsize && lfptr->buffer && *lfptr->buffer)
	{
		strcpy(string, lfptr->buffer);
		*lfptr->buffer = 0;
	}
	else if (lfptr->type == LF_FILE_PTR)
	{
		int check_line_continuation = 0;
		/* eat white space */
		/*-
		 *               NOTE:  the \\ test following is a big problem
		 *               it will break any license file paths...  Fortunately, they
		 *               don't seem to get parsed by this routine...
		 * kmaclean 3/5/03
		 * I think I fixed the \\ test big problem
		 */
		while (((c = fgetc(lfptr->ptr.f)) != EOF) &&
			(c == ' ' || c == '\t' ||  c == '\r' ||  c == '\\'))
		{

			/* kmaclean 3/5/03
			 * P7008
			 * added line continuation check */
			if (check_line_continuation )
			{
				if ( isspace(c) )
					bFoundCont = 1;
				check_line_continuation = 0;
			}
			if ( c == '\\' && (prev_char == 0 || isspace(prev_char) ))
				check_line_continuation = 1;

			prev_char = c;
		}
		if (c == EOF && !*string)
		{
			lfptr_eof(job, lfptr);
			goto exit_l_lf_get_token;
		}
		else if ((c == 147) || (c == 148)) /* smartquotes */
		{
			c = '=';
		}
		else if (c == '\n')
		{
			num++;
			strcpy(string, "\n");
			ret = L_TOKEN_NEWLINE;
			/* kmaclean 3/5/03
			 * P7008
			 * added line continuation check */
			lfptr->flags |= bFoundCont ? LM_LF_FLAG_CONTINUATION : LM_LF_FLAG_LINE_START;
			goto exit_l_lf_get_token;
		}
		else if ((start_line) && c == '#')
		{
			/* comment */
			*string = c;
			fgets(&string[1], MAX_CONFIG_LINE - 1, lfptr->ptr.f);	/* overrun checked */
			num++;
			lfptr->flags |= LM_LF_FLAG_LINE_START;
			goto exit_l_lf_get_token;
		}

		do
		{
			*cp++ = c;
		} while ((c != EOF) &&
			((c = fgetc(lfptr->ptr.f)) != EOF) &&
			(!isspace(c)) &&
			(cp <= maxcp));

		if (c == EOF)
		{
			/*cp --;*/
			lfptr_eof(job, lfptr);
		}
		else
		{
			ungetc(c, lfptr->ptr.f);
		}
		*cp = 0; /* null terminate */
		/*-
		 *		Set EOF flag if encountered USE_SERVER, and am_client
		 *		We may have to modify this in the future.
		 *		One possibility is that we don't return EOF until we
		 *		encounter the first FEATURE/INCREMENT/UPGRADE line
		 *		after USE_SERVER -- that we read other lines that
		 *		we may need, like SEE_ALSO???
		 */
		if (*string && i_am_client &&
			l_keyword_eq(job, string, LM_RESERVED_USE_SERVER))
		{
			lfptr_eof(job, lfptr);
			lfptr->type = LF_PORT_HOST_PLUS;
		}
	}
	else if (lfptr->type == LF_STRING_PTR)
	{
  		char *s = lfptr->ptr.str.cur;

		while ((c = *s++) &&
		       (c == ' ' || c == '\t' ||  c == '\r' || (bLineCont =
			line_continuation(c, s ==
			(lfptr->ptr.str.cur + 1) ? 0 : s[-2], *s))))
		{
			if(bLineCont)
			{
				bFoundCont = 1;
				bLineCont = 0;
			}
		}

		lfptr->ptr.str.cur = s;
		if (!c)
		{
			lfptr_eof(job, lfptr);
			ret = L_TOKEN_COMMENT;
			goto exit_l_lf_get_token;
		}
		if (c == '\n')
		{
			num++;
			lfptr->flags |= bFoundCont ?
						LM_LF_FLAG_CONTINUATION :
						LM_LF_FLAG_LINE_START;
			strcpy(string, "\n");
			ret = L_TOKEN_NEWLINE;
			goto exit_l_lf_get_token;
		}
		if ((start_line || bPrevCont) && c == '#')
		{
			do
			{
				*cp++ = c;
			}
			while ((c = *s++) && (c != '\n'));
			*cp++ = c;
			num++;
			lfptr->ptr.str.cur = s;
			if (!c)
				lfptr_eof(job, lfptr);
			ret = L_TOKEN_COMMENT;
			*cp = 0; /* null terminate */
			goto exit_l_lf_get_token;
		}

		do
		{
			*cp++ = c;
		}
		while ( (c = *s++) &&
			(!isspace((unsigned char)c) &&
                                !line_continuation(c,
                                s == (lfptr->ptr.str.cur + 1) ? 0 : s[-2],
                                *s)));


		if (c == '\n')
		{
			lfptr->flags |= LM_LF_FLAG_LINE_START;
		}
		else if (!c)
		{
			lfptr_eof(job, lfptr);
		}
		*cp = 0; /* null terminate */
		lfptr->ptr.str.cur  = s - 1;
	}

	if (*string)
	{
		ret = L_TOKEN_REGULAR;

		if (start_line)
		{

			if (*string == '#')
				ret = L_TOKEN_COMMENT;
			else if (ismaintoken(job, string))
				ret = L_TOKEN_MAIN;
			else if (l_decimal_format((unsigned char *)string))
				ret = L_TOKEN_DECIMAL;
		}
	}
	else
	{
		ret = 0;
	}
exit_l_lf_get_token:

	if (numlines)
		*numlines += num;
	return ret;

}

static void l_lf_put_token(LM_HANDLE *job, char *string, LICENSE_FILE *lfptr)
{
  int len = strlen(string);

	if (!string || !*string)
		return;
	if (len >= lfptr->bufsize)
	{
		if (lfptr->bufsize && lfptr->buffer)
			free(lfptr->buffer);
		lfptr->bufsize = len + 1;
		lfptr->buffer = l_malloc(job, lfptr->bufsize);
	}
	strcpy(lfptr->buffer, string);
}


/*
 *	l_lfgets() - Get the next string from the license file pointer
 */
char * API_ENTRY l_lfgets(LM_HANDLE *job,char *string,int max,LICENSE_FILE *lfptr,int *numlines)
{

#ifdef NLM
	ThreadSwitch();
#endif /* NLM */

	DEBUG_INIT
	if (numlines)
		*numlines = 0;

	if (lfptr->flags & LM_LF_FLAG_EOF)
		return 0;

	return lfgets_tokens(job, string, max, lfptr, numlines);
}

#ifdef ANSI
#include <stdio.h>
#ifdef PC
#include <io.h>
#endif
#endif /* ANSI */


/*
 *	l_lfseek() - seek the license file pointer
 */
void l_lfseek(LICENSE_FILE *lf,long where,int how)
{
	if (lf->type == LF_FILE_PTR)
	{
		fseek(lf->ptr.f, where, how);
		clearerr(lf->ptr.f);
	}
	else if (lf->type == LF_STRING_PTR)
	{
		if (how == 0)
			lf->ptr.str.cur = lf->ptr.str.s + where;

	}
	if (!how && !where)
	{
		lf->flags |= LM_LF_FLAG_LINE_START;
		lf->flags &= ~LM_LF_FLAG_EOF;
		if (lf->bufsize)
		{
			if (lf->buffer)
				free(lf->buffer);
			lf->buffer = 0;
			lf->bufsize = 0;
		}
	}
}

/*
 *	l_lfclose() - close the license file pointer
 */
void API_ENTRY l_lfclose(LICENSE_FILE *lf)
{
	if (lf->type == LF_FILE_PTR)
	{
		if (lf->ptr.f)
			fclose(lf->ptr.f);
		lf->type = LF_NO_PTR;
		if (lf->bufsize)
		{
			if (lf->buffer)
				free(lf->buffer);
			lf->buffer = 0;
			lf->bufsize = 0;
		}
	}
}
/***********************************************************************
 * read one line from the license file stripping the line continuations
 ************************************************************************/
static char *lfgets_tokens(LM_HANDLE *job,char *string,int max,LICENSE_FILE *lfptr,int *numlines)
{
  int len = 0, buflen;
  int rc;
  int cnt = 0;
  char *sav = string;
  int br = 0;
  char buf[MAX_CONFIG_LINE + 1]; /* place in buf till sure there's space */
  int i_am_client = (int) (job->flags & LM_FLAG_CHECKOUT);
  int remove_trailing_space = 0;
  int quoted_string = 0;

	*string = 0;
	while (lfptr->flags != LM_LF_FLAG_EOF)
	{
		rc = l_lf_get_token(job, buf, max, lfptr, numlines);
		if (!(buflen = strlen(buf)))
			continue;
		if ((cnt && (rc == L_TOKEN_MAIN)) ||
			(rc == L_TOKEN_COMMENT) ||
			(rc == L_TOKEN_DECIMAL) ||
			((len + buflen + 1) >= max))
		{
			br = 1; /* we're going to break from loop */
			if (cnt)
			{
				l_lf_put_token(job, buf, lfptr);
				*buf = 0;
			}
		}
		remove_trailing_space = 0;
		if (!br || !cnt)
		{
		  char *token = buf;
			strcat(string, token);
			len += buflen;
			string += buflen;
			if (strchr(token, '"'))
				quoted_string = ~quoted_string;
		}
		while (string > sav && isspace((unsigned char)string[-1]))
		{
			string--; /* remove trailing spaces */
			len--;
		}
		if (br)
		{
			/* eat trailing newline */
			if (rc == L_TOKEN_DECIMAL)
			{
				if ((rc = l_lf_get_token(job, buf, max, lfptr, numlines) ) != L_TOKEN_NEWLINE)
					l_lf_put_token(job, buf, lfptr);
			}
			*string = 0;
			break;
		}
		strcpy(string, " ");
		len++;
		string++;
		cnt++;
		remove_trailing_space = 1;
		if ((rc == L_TOKEN_MAIN) && i_am_client &&
			l_keyword_eq(job, buf, LM_RESERVED_USE_SERVER))
		{
			lfptr_eof(job, lfptr);
			if (lfptr->ptr.str.s &&
				(lfptr->type == LF_STRING_PTR) /*P5980*/)
			{
				free(lfptr->ptr.str.s); /* leak, fixed 7.1e */
				lfptr->ptr.str.s = 0;
			}
			lfptr->type = LF_PORT_HOST_PLUS;
			break;
		}
	}
	if (remove_trailing_space)
		string[-1] = 0;

	lfptr->flags |= LM_LF_FLAG_LINE_START;
	if ((lfptr->flags & LM_LF_FLAG_EOF) && !*sav)
		return 0;
	DEBUG(("get_toksn returning %s\n", sav));
	return sav;
}
static void lfptr_eof(LM_HANDLE *job,LICENSE_FILE *lfptr)
{
	lfptr->flags |= LM_LF_FLAG_EOF;
	if (lfptr->bufsize)
	{
		if (lfptr->buffer)
			free(lfptr->buffer);
		lfptr->buffer = 0;
		lfptr->bufsize = 0;
	}
}

/*
 *	line_continuation:  determine if a backslash is a line
 *	continuation character.  It could also be a filename
 *	Be careful with this function, and be sure to test with utcrypt.
 */
static int line_continuation(int this_char,int prev_char,int next_char)
{
        if ((this_char == '\\') &&
                        (((!prev_char || isspace((unsigned char)prev_char)) &&
                        (!next_char || isspace((unsigned char)next_char)))

			||
			((!prev_char || !isspace((unsigned char)prev_char)) &&
			(!next_char ||
				(next_char == '\n') || (next_char == '\r')))))
                return 1;
        else return 0;
}
