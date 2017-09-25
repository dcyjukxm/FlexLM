/******************************************************************************
 *
 *           COPYRIGHT (c) 1998, 2003 by Macrovision Corporation.
 *       This software has been provided pursuant to a License Agreement
 *       containing restrictions on its use.  This software contains
 *       valuable trade secrets and proprietary information of
 *       Macrovision Corporation and is protected by law.  It may
 *       not be copied or distributed in any form or medium, disclosed
 *       to third parties, reverse engineered or used in any manner not
 *       provided for in said License Agreement except with the prior
 *       written authorization from Macrovision Corporation.
 *
 *****************************************************************************
 *
 *      Module: $Id: l_getenv.c,v 1.78 2003/05/30 16:50:29 sluu Exp $
 *
 *       Function: l_getenv.c
 *
 *       Description: functions for setting license paths and files
 *
 *                      In this file, getenv() is the real getenv()
 *
 *	 See end of File for additional comments
 *       Author: Blane
 *       10/17/98
 *
 ******************************************************************************/
#include "lmachdep.h"
#include "lmclient.h"
#include "l_prot.h"
#include "flex_file.h"
#include "flex_utils.h"
#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef ANSI
#include <unistd.h>
#endif /* ANSI */
#endif /* UNIX */

#ifdef VXWORKS
#include <sys/fcntlcom.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#if defined(UNIX) || defined(VXWORKS)
static void outputreg(LM_HANDLE *job, int fd, char *key, char *value, int len);
#endif

#include <string.h>

#ifdef NLM
char *
l_getenv(
	LM_HANDLE *	job,
	char *		name)
{
	return NULL;
}
#else


#if !defined(PC16) && !defined(NLM)  && !defined(OS2)
#ifdef getenv
#undef getenv
#endif
API_ENTRY  char * l_getnextfile lm_args((char * ));
static int check_path_in_license lm_args((LM_HANDLE_PTR , char *, char * ));
API_ENTRY char * l_getlastfile lm_args((char * ));
static char * regname lm_args(( LM_HANDLE *, int));

#if defined(UNIX) || defined(VXWORKS)
static void read_rc lm_args((LM_HANDLE *, int));
char * get_next_registry lm_args((char *, char *, int *));
#endif /* UNIX */

/*
 *	l_rcfilename and l_borrowname allow us to rename the
 *	registry locations for testing purposes
 */
char *l_rcfilename = NULL;
char *l_borrowname = NULL;


API_ENTRY
int
lc_install_license_path(
	LM_HANDLE *	job,
	char *		value,
	char *		vendor)
{
	if (LM_API_ERR_CATCH)
		return job->lm_errno;
	LM_API_RETURN(int, l_update_license_path(job,value,vendor))
}

/*
 *  l_update_license_path
 *  adds the paths to the end of the license string if not already in them
 *  Places it in reverse order.
*/
API_ENTRY
int
l_update_license_path(
	LM_HANDLE *	job,
	char *		value,
	char *		vendor)
{
	char *	n = NULL;
	int		ret = 0;
	char *	val_cpy = NULL;

	if (job->flags & LM_FLAG_LMDIAG)
		return 0;

	if (!vendor)
		vendor = job->vendor;
	if (!value || !strlen(value))
		return 1;
	val_cpy=l_malloc(job,strlen(value)+1);
	n=val_cpy;
	strcpy(val_cpy,value);

	while (val_cpy)
	{
		val_cpy = l_getlastfile(n);
		if ( val_cpy )
			ret = l_update_license_file(job,val_cpy,vendor);
		else
			ret = l_update_license_file(job,n,vendor);
	}
	free(n);
	return ret;
}

/*
 *      This function looks at the LM_LICENSE_FILE and VENDOR_LICENSE_FILE
 *      paths, and adds the new path
 *      To the old path if it is not already there
 */
API_ENTRY
int
l_update_license_file(
	LM_HANDLE *	job,
	char *		value,
	char *		vendor)
{
	char *	old_registry = NULL;
	char *	new_registry = NULL;
	char	vendor_license_file[40] = {'\0'};


	if ( !vendor || !value )
	{
		LM_SET_ERRNO(job, LM_BADPARAM, 470, 0);
		return 1;
	}
	if (job->flags & LM_FLAG_LMDIAG)
		return 0;

	if (L_STREQ(job->options->behavior_ver, LM_BEHAVIOR_V5_1))
		strcpy(vendor_license_file,LM_DEFAULT_ENV_SPEC);
	else
	{
		sprintf(vendor_license_file,"%s_LICENSE_FILE", vendor);
		l_uppercase(vendor_license_file);
	}


	/* First Check all values to see if is there already, if it is, just return  */

	if (job->options->flags & LM_OPTFLAG_CKOUT_INSTALL_LIC)
	{
		if ( ! l_get_registry(job, LM_DEFAULT_ENV_SPEC, &old_registry, 0, 0))
		{
			if (check_path_in_license(job, old_registry, value))
				return 0;       /* already in path */
		}

		if ( ! l_get_registry(job, vendor_license_file, &old_registry, 0, 0))
		{
			if (check_path_in_license(job, old_registry, value))
				return 0;       /* already in path */
		}

	}
	else
		return 1; /* old style, and env LM_LICENSE_FILE is not set */


    if (!old_registry)
    {
		/* nothing is set, set it to path passed in */
		if (l_set_registry(job, vendor_license_file, value, 0, 0))
			 return 1; /* error in writing */
		else
			return 0;
    }
/* It was previously set, concatenate the new path to it */

    new_registry=l_malloc(job,(strlen(value) + strlen(old_registry) + 10));

    sprintf(new_registry,"%s%c%s", value, PATHSEPARATOR,old_registry);
    l_set_registry(job,vendor_license_file,new_registry, 0, 0);


    free(new_registry);

    return 0;
}




/*
 *      Gets an item out of the registry
 *      Warning:  value is only good until the next call to this function
 *      Return: 0 == success, !=0 == failure
 *	If (borrow) use a different registry name
 */

API_ENTRY
int
lc_get_registry(
	LM_HANDLE *	job,
	char *		name,
	char **		value)
{
	if (LM_API_ERR_CATCH)
		return job->lm_errno;
	LM_API_RETURN(int, l_get_registry(job, name, value, 0, 0))
}

API_ENTRY
int
l_get_registry(
	LM_HANDLE *	job,
	char *		name,
	char **		value,
	int *		len,	/* can be 0 if the data is ascii text */
	int			borrow)	/* flag */
{
#ifdef WINNT
	long errs = 0;
	DWORD val_type = 0, buf_len = 0;
	HKEY hkey = NULL;
	char * rn = NULL;
	HKEY where = borrow ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;

	if( (job == NULL) || (name == NULL) )
	{
		goto done;
	}

	if (len)
	{
		*len = 0;
		*value=0;
	}
	/* if not see if it is defined in registry        */
	rn = regname(job, borrow);

	errs = RegOpenKeyEx(where, rn, (DWORD)0, KEY_QUERY_VALUE, &hkey);
	free(rn);
	rn = NULL;

    if (errs == ERROR_SUCCESS)
    {
		if(job && job->curr_registry)
		{
			free(job->curr_registry);
			job->curr_registry = NULL;
		}
		/*
		 *	Read data, if type is REG_SZ, data returned will be in UTF-8 format
		 */
		job->curr_registry = l_regQueryValue(job, hkey, name, &val_type, &buf_len);
		RegCloseKey(hkey);
		*value = job->curr_registry ? job->curr_registry : NULL;
		if(len)
		{
			*len = job->curr_registry ? buf_len : 0;
		}
		return job->curr_registry ? 0 : 1;

	}
	else
	{
		if (errs == ERROR_FILE_NOT_FOUND )
			return 1;

		if (errs == ERROR_ACCESS_DENIED  && job )
			LM_SET_ERRNO(job, NOTLICADMIN, 403, 0);
	}
done:
	return 1;
#else /* UNIX */
	char * cp = NULL;
	int keylen = 0;
	char *buf = 0;
	int ret = 1; /* assume failure */
	int buf_len = 0;
	char **regfile = borrow ? &job->borrfile : &job->rcfile;
	long *reglen = borrow ? &job->borrfile_s  : &job->rcfile_s;

	if( (job == NULL) || (name == NULL) )
	{
		goto done;
	}

	keylen = strlen(name);


	if (job->curr_registry)
		free(job->curr_registry);
	job->curr_registry = 0;
	*value = 0;
	read_rc(job, borrow);
	if (!*regfile)
		return 1;
	buf = l_malloc(job, *reglen + 1);
	for (cp = get_next_registry(*regfile, buf, &buf_len); *buf;
			cp = get_next_registry(cp, buf, &buf_len))
	{
		if (l_keyword_eq_n(job, name, buf, keylen) &&
							((buf[keylen] == ' ') || (buf[keylen] == '=')))
		{
			/* skip to start of value */
			cp = buf + keylen;
			while (*cp && *cp != '=')
			{
				cp++;
				buf_len--;
			}
			if (!*cp)
			{
				ret = 1;
				continue;
			}
			cp++;
			buf_len--; /* skip = */

			/* skip trailing spaces */
			while (isspace(*cp))
			{
				cp++;
				buf_len--;
			}
			buf_len -= keylen;

			*value = job->curr_registry = l_malloc(job, buf_len + 1);
			memcpy(*value, cp, buf_len);
			ret = 0;
			break;
		}
	}
	if (buf)
		free(buf);
	if (len)
		*len = buf_len;
done:
	return ret;
#endif /* WINNT */
}

/*
 *	Sets an item into the registry
 */
API_ENTRY
int
lc_set_registry(
	LM_HANDLE *	job,
	char *		key,
	char *		value)
{
	if (LM_API_ERR_CATCH)
		return job->lm_errno;
	LM_API_RETURN(int, l_set_registry(job, key, value, 0, 0))
}

#ifdef PC
int l_make_regs_testsuite;
static
void
make_regs(int borrow)
{
	HKEY hcpl = NULL, happ = NULL, *h = NULL, other = NULL, h1 = NULL, h2 = NULL, h3 = NULL;
	DWORD dwDisp = 0;
	HKEY where = borrow ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;


	if (RegOpenKeyEx(where, "SOFTWARE", 0,KEY_WRITE,&hcpl) != ERROR_SUCCESS)
		return;
	if (RegOpenKeyEx(where, "SOFTWARE\\FLEXlm License Manager", 0,
				KEY_WRITE,&happ) != ERROR_SUCCESS)
	{
		if (RegCreateKeyEx(hcpl, "FLEXlm License Manager",0, "",
			   REG_OPTION_NON_VOLATILE,KEY_WRITE,
			   NULL,&happ, &dwDisp) != ERROR_SUCCESS)
		{
			RegCloseKey(hcpl);
			return; /* failed */
		}
	}
	if (RegCreateKeyEx(happ, "Borrow", 0, "",
			REG_OPTION_NON_VOLATILE, KEY_WRITE,
			NULL, &other, &dwDisp) !=
			ERROR_SUCCESS)
	{
		RegCloseKey(hcpl);
		RegCloseKey(happ);
		return; /* failed */
	}
	RegCloseKey(hcpl);
	RegCloseKey(happ);
	RegCloseKey(other);



	if (l_make_regs_testsuite)
	{
		if (RegOpenKeyEx(where, "SOFTWARE\\FLEXlm License Manager\\flexlmrc",
						(DWORD)0, KEY_READ, &h1 ) != ERROR_SUCCESS)
		{
			if (RegOpenKeyEx(where,"SOFTWARE\\FLEXlm License Manager", 0,KEY_WRITE,&h2) == ERROR_SUCCESS)
			{
				if (RegCreateKeyEx(h2, "flexlmrc", 0, "",
					   REG_OPTION_NON_VOLATILE,KEY_WRITE,
					   NULL,&h3, &dwDisp) != ERROR_SUCCESS)
				{
					printf("Error, can't write to registry 2!\n");
				}
				RegCloseKey(h2);
				RegCloseKey(h3);
			}

			else
				printf("Error, can't write to registry!\n");
		}
		else
			RegCloseKey(h1);
	}
}
#endif /* PC */

API_ENTRY int
l_set_registry(
	LM_HANDLE *	job,
	char *		key,
	char *		value,
	int			len,	/* if 0, it's a null-terminated string */
	int			borrow)	/* flag, use a different file/registry location if true */
{
#ifdef WINNT
	HKEY h = NULL, where = borrow ? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE;
	DWORD dwDisp = 0;
	char * rn = NULL;

	rn = regname(job, borrow);

	if (RegOpenKeyEx(where, rn, 0,KEY_WRITE,&h) != ERROR_SUCCESS)
	{
		make_regs(0);
		make_regs(1);

		if (RegOpenKeyEx(where, rn, 0,KEY_WRITE,&h) != ERROR_SUCCESS)
		{
			free(rn);
			if (job)
			{
				LM_SET_ERRNO(job, NOTLICADMIN, 478, 0);
			}
			return 1;
		}
	}
	free(rn);


	/* Set the value */
	if (!value || (!len && !strlen(value)))
	{
		RegDeleteValue(h,key);
	}
	else
	{
		l_regSetValue(job, h, key, len ? REG_BINARY : REG_SZ, value,
						len ? len : (strlen(value) + 1));
	}
	RegCloseKey(h);

	return 0;
#else
	char *	path = NULL;
	int		fd = -1;
	int		olen = 0;

	char *	cp = NULL;
	char *	buf = NULL;
	int ret = 1; /* assume failure */
	long *	s = borrow ? &job->borrfile_s : &job->rcfile_s;
	char **	f = borrow ? &job->borrfile : &job->rcfile;

	l_get_registry(job, key, &cp, &olen, borrow);
	if ((!value && !cp) || ((value && cp && (olen == len) &&
		!memcmp(cp, value, olen))))
	{
		return 0; /* already set */
	}
    path = regname(job, borrow);
	l_flexUnlink(job, path);
	if ((fd = l_flexOpen(job, path, O_WRONLY|O_CREAT, 0777)) < 0)
	{
		goto err_exit_lc_set_registry;
	}
    free(path); path = 0;
    if (value)
		outputreg(job, fd, key, value, len);
	len = strlen(key);
	if (*f)
	{
		buf = l_malloc(job, *s + 1);
		for (cp = get_next_registry(*f, buf, &olen); *buf;
                                        cp = get_next_registry(cp, buf, &olen))
		{
			if (!strcmp(buf, "\n"))
				continue;
			if (!(l_keyword_eq_n(job, key, buf, len) &&
					((buf[len] == ' ') || (buf[len] == '='))))
			{
				int keep = 1;
#ifdef RELEASE_VERSION
#define L_WEEK (60 * 60 * 24 * 7)
#else
#define L_WEEK (60 * 60) /* one hour */
#endif

				char * v = strchr(buf, '=');
				long exp = 0;
				long t = (long) time(0);

				sscanf(v + 5, "%08lx", &exp);	/* overrun checked */
				if (t > (exp + L_WEEK) && strncmp(buf, "borrow", strlen("borrow")) == 0)
					keep = 0;

				if (keep)
					outputreg(job, fd, 0, buf, olen);
			}

		}
	}
    close(fd);
    read_rc(job, borrow);
    ret = 0;
err_exit_lc_set_registry:
    if (fd >=0 )
		close(fd);
    if (buf)
		free(buf);
    if (path)
		free(path);
    return ret;
#endif /* WINNT  */
}

/*
*  Retrieve an item from Registry or Environment variable
*  If the  item is LM_LICENSE_FILE or VENDOR_LICENSE file, concatenate the
*  Envorioment and Registry settings. Else, just return the first thing set
*  either Environment, or Registry
*
*/
char *
l_getEnvUTF8(
	LM_HANDLE *	job,
	char *		name,
	char *		buffer,		/* buffer to copy data into */
	int			iSize)		/* size of buffer */
{
	char *	buf_ptr = NULL;
	char *	lm_env = NULL;
	char *	lm_reg = NULL;
	char	ps[2] = {'\0'};
	int     call_type = 0, env_len = 0, reg_len = 0;
#define LM_LICENSE_CASE 1
#define LM_VENDOR_CASE 2
#define LM_ANY_OTHER  3


	if (L_STREQ(name, "LM_LICENSE_FILE"))
		call_type = LM_LICENSE_CASE;
	else if (strlen(name) >= sizeof("_LICENSE_FILE") &&
		L_STREQ(&name[strlen(name) - (sizeof("_LICENSE_FILE") - 1)], "_LICENSE_FILE"))
	{
		call_type = LM_VENDOR_CASE;
	}
	else
		call_type = LM_ANY_OTHER ;
	switch ( call_type)
	{
	case LM_ANY_OTHER:
		/*
		 * first see if there is a corresponding environment
		 * variable set
		 */


		if(l_getEnvironmentVariable(job, name, buffer, iSize))
		{
			return buffer;
		}

		/* if not see if it is defined in registry/ .flexlmrc      */
		if (strcmp(name, "LM_BORROW") == 0)
		{
			/* try HKEY_LOCAL_MACHINE first*/
			l_get_registry(job, name, &buf_ptr, 0, 0);
			if (buf_ptr == NULL)
			{
				/* try HKEY_CURRENT_USER */
				l_get_registry(job, name, &buf_ptr, 0, 1);
			}
		}
		else
			l_get_registry(job, name, &buf_ptr, 0, 0);
		return buf_ptr;

	case LM_LICENSE_CASE:
		if (job->path_env)
		{
			free(job->path_env);
			job->path_env = 0;
		}
		if(l_getEnvironmentVariable(job, name, buffer, iSize))
		{
			env_len = strlen(buffer) + 1;
			lm_env = buffer;
		}
		else
		{
			env_len = 0;
			lm_env = NULL;
		}

		l_get_registry(job, name, &lm_reg, 0, 0);
		if (lm_reg)
			reg_len = strlen(lm_reg)+1;
		else
		{
			reg_len=0;
			lm_reg="";
		}

		if( !reg_len && !env_len)
			return 0;

		if ((job->path_env = calloc(1, reg_len + env_len + 1 )) == NULL)
			return 0;


		sprintf(ps, "%c", PATHSEPARATOR);
		sprintf(job->path_env, "%s%s%s", lm_env ? lm_env : "",
			(lm_reg && lm_env) ? ps : "",
			lm_reg ? lm_reg : "" );

		return job->path_env;

	case LM_VENDOR_CASE:
		l_uppercase(name); /* P5002 */
		if (job->vd_path_env)
		{
			free(job->vd_path_env);
			job->vd_path_env = 0;
		}

		if(l_getEnvironmentVariable(job, name, buffer, iSize))
		{
			env_len = strlen(buffer) + 1;
			lm_env = buffer;
		}
		else
		{
			env_len = 0;
			lm_env = NULL;
		}


		lm_reg = 0;
		if (!(job->flags & LM_FLAG_IS_VD))
			l_get_registry(job, name, &lm_reg, 0, 0);

		if (lm_reg)
			reg_len = strlen(lm_reg)+1;
		else
		{
			reg_len=0;
			lm_reg="";
		}


		if( !reg_len && !env_len)
			return 0;

		if ((job->vd_path_env = calloc(1, reg_len + env_len + 3 )) == NULL)
			return 0;


		sprintf(ps, "%c", PATHSEPARATOR);
		sprintf(job->vd_path_env, "%s%s%s",
				lm_env ? lm_env : "",
				(lm_reg  &&  lm_env ) ? ps : "",
				lm_reg ? lm_reg : "" );

		return job->vd_path_env;

	}
	return 0;
}

char *
l_getenv(
	LM_HANDLE *	job,
	char *		name)
{
	char *	buf_ptr = NULL;
	char *	lm_env = NULL;
	char *	lm_reg = NULL;
	char	ps[2] = {'\0'};
	int     call_type = 0, env_len = 0, reg_len = 0;
#define LM_LICENSE_CASE 1
#define LM_VENDOR_CASE 2
#define LM_ANY_OTHER  3

	if (L_STREQ(name, "LM_LICENSE_FILE"))
		call_type = LM_LICENSE_CASE;
	else if (strlen(name) >= sizeof("_LICENSE_FILE") &&
		L_STREQ(&name[strlen(name) - (sizeof("_LICENSE_FILE") - 1)], "_LICENSE_FILE"))
	{
		call_type = LM_VENDOR_CASE;
	}
	else
		call_type = LM_ANY_OTHER ;
	switch ( call_type)
	{
	case LM_ANY_OTHER:
		/*
		 * first see if there is a corresponding environment
		 * variable set
		 */

		if (buf_ptr = getenv(name))	/* overrun checked */
			return buf_ptr;

		/* if not see if it is defined in registry/ .flexlmrc      */
		if (strcmp(name, "LM_BORROW") == 0)
		{
			/* try HKEY_LOCAL_MACHINE first*/
			l_get_registry(job,name,&buf_ptr, 0, 0);
			if (buf_ptr == NULL)
			{
				/* try HKEY_CURRENT_USER */
				l_get_registry(job,name,&buf_ptr, 0, 1);
			}
		}
		else
			l_get_registry(job,name,&buf_ptr, 0, 0);
		return buf_ptr;

	case LM_LICENSE_CASE:
		if (job->path_env)
		{
			free(job->path_env);
			job->path_env = 0;
		}
		lm_env=getenv(name);	/* overrun checked */
		if (lm_env)
			env_len=strlen(lm_env)+1;
		else
		{
			env_len=0;
			lm_env="";
		}

		l_get_registry(job,name,&lm_reg, 0, 0);
		if (lm_reg)
			reg_len=strlen(lm_reg)+1;
		else
		{
			reg_len=0;
			lm_reg="";
		}


		if( !reg_len && !env_len)
			return 0;

		if (!(job->path_env=calloc(1, reg_len + env_len + 1 )))
			return 0;


		sprintf(ps, "%c", PATHSEPARATOR);
		sprintf(job->path_env, "%s%s%s",
			lm_env ? lm_env : "",
			(lm_reg && lm_env) ? ps : "",
			lm_reg ? lm_reg : "" );

		return job->path_env;

	case LM_VENDOR_CASE:
		l_uppercase(name); /* P5002 */
		if (job->vd_path_env)
		{
			free(job->vd_path_env);
			job->vd_path_env = 0;
		}

		lm_env=getenv(name);	/* overrun checked */
		if (lm_env)
			env_len=strlen(lm_env)+1;
		else
		{
			env_len=0;
			lm_env="";
		}

		lm_reg = 0;
		if (!(job->flags & LM_FLAG_IS_VD))
			l_get_registry(job,name, &lm_reg, 0, 0);

		if (lm_reg)
			reg_len=strlen(lm_reg)+1;
		else
		{
			reg_len=0;
			lm_reg="";
		}


		if( !reg_len && !env_len)
			return 0;

		if (!(job->vd_path_env=calloc(1, reg_len + env_len + 3 )))
			return 0;


		sprintf(ps, "%c", PATHSEPARATOR);
		sprintf(job->vd_path_env, "%s%s%s",
				lm_env ? lm_env : "",
				(lm_reg  &&  lm_env ) ? ps : "",
				lm_reg ? lm_reg : "" );

		return job->vd_path_env;

	}
	return 0;
}


static int
check_path_in_license(
	LM_HANDLE *	job ,
	char *		vendor_path,
	char *		ref_path)
{
	int ret = 0;
	char * vendor_path_copy = NULL;
	char * free_this = NULL;                /* save value to free later */
	char * p = NULL;

    free_this = vendor_path_copy = l_malloc(job,strlen(vendor_path)+1);
    strcpy(vendor_path_copy,vendor_path);

    while (vendor_path_copy)
    {
        p = vendor_path_copy;
        if (vendor_path_copy = l_getnextfile(vendor_path_copy))
        {
            *vendor_path_copy = '\0'; /* null terminate p */
            vendor_path_copy++;
        }
        if (L_STREQ(ref_path, p))
        {
            ret = 1;  /* found it */
            break;
        }
    }
	if (free_this)
		free(free_this);
	return ret;
}

API_ENTRY
char *
l_getnextfile(char * str)
{
	int start_len = sizeof(LM_LICENSE_START) - 1;
	int end_len = sizeof(LM_LICENSE_END) - 1;
	char * cp = NULL;

    if (L_STREQ_N(str, LM_LICENSE_START, start_len))
    {
		for (cp = str + start_len + 1; *cp; cp++)
		{
			if (L_STREQ_N(cp, LM_LICENSE_END, end_len))
				return cp + end_len;
		}
		return 0;
    }
	if (cp = strchr(str, PATHSEPARATOR))
		return cp;
	else
		return 0;
}

API_ENTRY
char *
l_getlastfile(char * str)
{
	char *	cp = NULL;
        /*  If there is a pathseparator, get the last field */

	if (!(cp = strrchr(str, PATHSEPARATOR)))
		return 0;       /* only one field remains */

    *cp=0;
    cp++;
	return cp;
}

API_ENTRY
char*
l_vendor_license_file(
	LM_HANDLE *	job,
	int			vflag) /* vflag is set, we only want the vendor names, not the paths*/
{
	char *	retpath = NULL;
	char *	retcp = NULL;
	int		cnt = 0;
	int		len = 0;
	char **	ep = NULL;
	char *	cp = NULL;
	int		lflen = sizeof("_LICENSE_FILE") - 1;
	char *	buf = NULL;
	char *	curr = NULL;
#ifndef MAC10
	extern char **environ;
#endif /* MAC10 */

#ifdef WINNT
	long	errs = 0;
	DWORD	val_type = 0, buf_len = 0 , path_len = 0, num = 0;
	char	keyname[40] = {'\0'};
	char	path[MAX_CONFIG_LINE + 1] = {'\0'};
	HKEY	hkey = NULL;
	FILETIME t;
	int		have_reg = 0;



	/* if not see if it is defined in registry        */

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\FLEXlm License Manager",
						(DWORD)0, KEY_READ, &hkey ) == ERROR_SUCCESS)
        {
            have_reg = 1;
            buf_len=39; path_len=MAX_CONFIG_LINE;
			while (RegEnumValue( hkey, num, keyname, &buf_len,
                        (DWORD) 0, NULL, path, &path_len) == ERROR_SUCCESS )
			{
                num++;
                if ( strstr( keyname, "_LICENSE_FILE"   ) &&
                        !L_STREQ( keyname, "LM_LICENSE_FILE") )
                {
                    len += vflag ? strlen(keyname) : path_len;
                    cnt++;
                }
                buf_len=39; path_len=MAX_CONFIG_LINE;
			}

        }
#else

        if (!job->rcfile)
			read_rc(job, 0);
        if (curr = job->rcfile)
			buf = l_malloc(job, len = (strlen(job->rcfile) + 1));
        while ( job->rcfile && ((curr= get_next_registry(curr, buf, 0)), *buf))
        {
			if ((cp = strchr(buf, '_')) && L_STREQ_N(cp, "_LICENSE_FILE", lflen) &&
					!L_STREQ_N(buf, "LM_LICENSE_FILE", lflen + 2) &&
					((cp[lflen] == ' ') || (cp[lflen] == '=')))
            {
				len += strlen(buf); /* overestimate */
				cnt ++;
            }
        }
#endif /* WINNT */
#ifndef MAC10
        for (ep = environ; *ep; ep++)
        {
			if ((cp = strchr(*ep, '_')) &&
					L_STREQ_N(cp, "_LICENSE_FILE", lflen) &&
					!L_STREQ_N(*ep, "LM_LICENSE_FILE", lflen+2) &&
					(cp[lflen] == '='))
			{
				len += strlen(*ep);
				cnt ++;
			}
        }
#endif /* MAC10 */
        if (!cnt)
			return 0;
        retcp = retpath = l_malloc(job, len + cnt);
#if defined(UNIX) || defined(VXWORKS)
        curr = job->rcfile;
        while ( job->rcfile && ((curr= get_next_registry(curr, buf, 0)), *buf))
        {
			if ((cp = strchr(buf, '_')) &&
                        L_STREQ_N(cp, "_LICENSE_FILE", lflen) &&
                        !L_STREQ_N(buf, "LM_LICENSE_FILE", lflen + 2) &&
                        ((cp[lflen] == ' ') || (cp[lflen] == '=')))
			{
				if (retcp > retpath)
					*retcp++ = PATHSEPARATOR;
				if (!vflag)
				{
					cp = &cp[lflen];
					while ((*cp == '=') || (*cp == ' '))
						cp++;
					strcpy(retcp, cp);
				}
				else
				{
					strncpy(retcp, buf, (cp - buf) + lflen);
					retcp[(cp - buf) + lflen] = 0; /*0 terminate*/
				}
				retcp += strlen(retcp);
			}
        }
#elif defined(PC) /* UNIX (now PC) */
        if (have_reg)
        {
			num = 0;
			buf_len=39; path_len=MAX_CONFIG_LINE;
			while (RegEnumValue( hkey, num, keyname, &buf_len,
						(DWORD) 0, NULL, path, &path_len) == ERROR_SUCCESS )
			{
				num++;
				if ( strstr( keyname, "_LICENSE_FILE"   ) &&
						!L_STREQ( keyname, "LM_LICENSE_FILE") )
				{
					if (retcp > retpath)
						*retcp++ = PATHSEPARATOR;
					if (vflag)
						strcpy(retcp, keyname);
					else
						strcpy(retcp, path);
					retcp += strlen(retcp);
				}
				buf_len=39; path_len=MAX_CONFIG_LINE;
			}
			RegCloseKey(hkey);
        }
#else

		ERROR need some code here
		or the proper define above

#endif /* end of PC */

#ifndef MAC10
        for (ep = environ; *ep; ep++)
        {
			if ((cp = strchr(*ep, '_')) &&
                        L_STREQ_N(cp, "_LICENSE_FILE", lflen) &&
                        !L_STREQ_N(*ep, "LM_LICENSE_FILE", lflen+2) &&
                        (cp[lflen] == '='))
			{
				if (retcp > retpath)
					*retcp++ = PATHSEPARATOR;
				if (!vflag)
				{
					cp = &cp[lflen];
					while ((*cp == '=') || (*cp == ' '))
						cp++;
					strcpy(retcp, cp);
				}
				else
				{
					strncpy(retcp, *ep, (cp - *ep) + lflen);
					retcp[(cp - *ep) + lflen] = 0; /*0 terminate*/
				}
				retcp += strlen(retcp);
			}
        }
#endif /* mac10 */
        if (buf)
			free(buf);
        return retpath;
}

#endif /*!defined(PC16) && !defined(NLM)  && !defined(OS2)*/

/*
 *      regname: returns name of registry: caller must free this
 */
static
char *
regname(
	LM_HANDLE *	job,
	int			borrow)	/* flag */
{
	char *	envname = NULL;
	char *	path = NULL;

	if (borrow)
	{
		if (l_borrowname == NULL)
			l_borrowname = l_real_getenv("FLEXLM_BORROWFILE");
		if (l_borrowname)
			envname = l_borrowname;
#ifdef PC
		else
			envname = "Borrow";
#endif
	}
	else
	{
		if (l_rcfilename == NULL)
			l_rcfilename = l_real_getenv("FLEXLM_RC");
		if (l_rcfilename)
			envname = l_rcfilename;
	}
#ifdef PC
	{
		char buf[500] = {'\0'};


		sprintf(buf, "SOFTWARE\\FLEXlm License Manager%s%s",
			envname ? "\\" : "", envname ? envname : "");	/* OVERRUN */
		path = (char *) l_malloc(job, strlen(buf) + 1);
		strcpy(path, buf);
		return path; /* caller must free this */
	}

#else
	{
		char *	home = NULL;
		char *	name = borrow ? LM_BORROW_FILE : LM_REGISTRY_FILE;
		if (envname)
		{
			path = l_malloc(job, strlen(envname) + 1);
			strcpy(path, envname);
		}
		else
		{
			home = getenv("HOME");
			path = l_malloc(job, (home ? strlen(home)  : 0) + strlen(name) + 2);
			sprintf(path, "%s/%s", (home ? home : "."), name);
		}
	}
#endif /* PC */
	return path;
}


#if defined(UNIX) || defined(VXWORKS)
static
void
read_rc(
	LM_HANDLE *	job,
	int			borrow)	/* flag */
{
	char *		path = NULL;
	struct stat statbuf;
	int			fd = 0;
	time_t *	t = borrow ? &job->borrfile_t : &job->rcfile_t;
	long *		s = borrow ? &job->borrfile_s : &job->rcfile_s;
	char **		n = borrow ? &job->borrfile : &job->rcfile;



	path = regname(job, borrow);
	if (stat(path, &statbuf))
	{
		free(path);
		return;
	}
	if ((statbuf.st_mtime == *t)
		&& (statbuf.st_size == *s))
	{
		free(path);
		return;
	}
	*t = statbuf.st_mtime;
	*s = statbuf.st_size;
	if (*n)
		free(*n);
	*n = 0;

	*n = (char *)l_malloc(job, statbuf.st_size + 1);
	if ((fd = l_flexOpen(job, path, O_RDONLY, 0777)) != -1)
	{
        if (fd != -1)
        {
			read(fd, *n, statbuf.st_size );
			close(fd);
        }
	}
	free(path);
}

char *
get_next_registry(
	char *	in,
	char *	out,
	int *	len)	/* can be 0 if null-terminated string */
{
	int l = 0;

	*out = 0;
	if (!in || !*in || (*in == '\n') || (*in == '\r'))
		return 0;
	while (*in && (*in != ' ') && (*in != '='))  /* skip till = */
	{
		*out++ = *in++; l++;
	}
	while (*in && ((*in == ' ') || (*in == '=')))  /* skip spaces and = */
	{
		*out++ = *in++; l++;
	}
	*out = 0; /* in case there's no more */

	if (len) *len = 0;
	for (; in && (*in != '\n') && (*in != '\r'); in++, out++, l++)
	{
		if ((*in == '\\') && (in[1] == 'x'))
		{
			int i = 0;
			char buf[4];
			/* read binary format:  "\xnn" */
			memcpy(buf, &in[2], 2);
			buf[2] = 0;
			sscanf(buf, "%x", &i);	/* overrun checked */
			*out = i;
			in += 3;
		}
		else
			*out = *in;
	}
	if (len)
		*len = l;
	while (*in && (*in == '\n' || *in == '\r'))
		in++;
	return in;
}

API_ENTRY
char *
l_get_next_registry(
	char *	in,
	char *	out,
	int *	len)
{
	return get_next_registry(in, out, len);
}

API_ENTRY
int
l_delete_registry_entry(
	LM_HANDLE_PTR	job,
	char *			key,
	int				borrow)
{
	char *		pszPath = NULL;
	char *		pszData = NULL;
	char *		buf = NULL;
	int			fd = -1;
	int			iLen = 0;
	int			iKeyLen = strlen(key);

	l_get_registry(job, key, &pszData, &iLen, borrow);

	if(pszData && iLen)
	{
		/*
		 *	Data in registry, now delete it.
		 */

		/*	Delete registry file	*/
		pszPath = regname(job, borrow);
		l_flexUnlink(job, pszPath);

		/*
		 *	Now create a new file
		 */
		fd = l_flexOpen(job, pszPath, O_WRONLY | O_CREAT, 0777);
		free(pszPath);
		pszPath = NULL;
		if(fd < 0)
		{
			return -1;
		}

		buf = l_malloc(job, (borrow ? job->borrfile_s : job->rcfile_s) + 1);
		if(!buf)
			return -1;
		for(pszData = get_next_registry(borrow ? job->borrfile : job->rcfile, buf, &iLen);
					*buf; pszData = get_next_registry(pszData, buf, &iLen))
		{
			if(!strcmp(buf, "\n"))
				continue;
			if(!(l_keyword_eq_n(job, key, buf, iKeyLen) &&
					((buf[iKeyLen] == ' ') || (buf[iKeyLen] == '='))))
			{
				/*
				 *	Write out this entry
				 */
				outputreg(job, fd, 0, buf, iLen);
			}
		}
		if(buf)
			free(buf);
		close(fd);
		/*
		 *	Re-read
		 */
		read_rc(job, borrow);
	}
	return 0;
}



#endif /* UNIX */

char *
l_real_getenv(char * name)
{
	return getenv(name);	/* overrun checked */
}
#endif /* !NLM */
#if defined(UNIX) || defined(VXWORKS)
static
void
outputreg(LM_HANDLE *job, int fd, char *key, char *value, int len)
{
	unsigned char *	vcp = (unsigned char *)value;
	char *			rcp = NULL;
	char *			newvar = NULL;
	int				olen = 0;

	olen = (key ? strlen(key) : 0) + (len ? len : strlen(value)) + 3;
/*
 *		to allow for binary data, we multipley the len * 4
 *		We convert binary data to ascii:  \xXX where XX is the
 *		hex number
 */
	newvar = l_malloc(job, olen * 4);
	if (key)
	{
		sprintf(newvar, "%s=", key);
		olen = strlen(newvar);
		rcp = newvar + olen;
	}
	else
	{
		rcp = newvar;
		olen = 0;
	}
	if (!len)  /* it's a string */
	{
		/* remove trailing newline */
		if (*value && value[strlen(value) - 1] == '\n')
			value[strlen(value) - 1] = 0;
		strcpy(rcp, value);
		olen += strlen(value);
	}
	else
	{
		int i;
		for (i = 0; i < len ; i++, vcp++)
		{
/*
 *			If it's printable 7-bit, or tab, fine.
 *			embed newlines though
 */
			if ((*vcp >= ' ' && *vcp <= '~') || *vcp == '\t')
			{
				*rcp++ = *vcp;
				olen++;
			}
			else
			{
				sprintf(rcp, "\\x%02x", *vcp);
				rcp += 4;
				olen += 4;
			}
		}
	}
	write(fd, newvar, olen);
	write(fd, "\n", 1);
	free(newvar); newvar = 0;
}
#endif /* UNIX */

#if COMMENT

registry:
	The registry is real on windows, and virtual on unix

	As of v8 there are 2 registry locations, one for borrow info
	and the one for everything else.

	We also have the ability to rename these 2 locations with:
			l_rcfilename = l_real_getenv("FLEXLM_RC");
			l_borrowname = l_real_getenv("FLEXLM_BORROWFILE");
	I use the primarily for testing, but it *could* be used
	by users.  I do not see any security problem with this except
	that they could lose things they have already borrowed doing
	this.  Presumably only someone trying something they should
	not would encounter this, since it is undocumented.

	The following variables are used in the job on unix only
	to facilitate this.  This is undocumented in lmclient.h
	for security reasons.
	   char *rcfile;
		This is a buffer for the most recently read info
		in registry.
	   time_t rcfile_t;
		rcfile_t is time the file was last modified by
		this process.  If the file is discovered to have
		a time later than this, it must be reread.  Note that
		this is not "atomic" and race conditions can occur
		which might mean that another app would set it, and
		we would not have the info they wrote there.  Normally
		this would not be a problem anyway.
	   long rcfile_s;
		The size of the registry the last time we modified it.
		If the size changes, then we should reread it.  This
		handles the case where it was modified in the same
		second we modified it.
	The next 3 variables are the same, but for the borrow registry.
	   char *borrfile;
	   time_t borrfile_t;
	   long borrfile_s;

#endif /* COMMENT */
