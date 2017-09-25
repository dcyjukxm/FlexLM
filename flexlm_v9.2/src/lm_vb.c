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
/*
 *	Module: $Id: lm_vb.c,v 1.5 2003/01/13 22:41:48 kmaclean Exp $
 *
 *      Description:    
 *                      
 *
 *     Blane Eisenberg 
 *     3/19/97 
 *
 *	Last changed:  9/22/98
 *
 */

#include "lmachdep.h"

#ifdef PC
#include "windows.h"
#endif
#include "lmclient.h"
#include "lmpolicy.h"

#ifdef PC
#define VB_CALL _stdcall
#else
#define VB_CALL
#endif

static VENDORCODE vb_code;
static char vb_vendorname[20];
static LM_HANDLE_PTR vb_job;

VB_CALL LP_SETUP(Name, VendorCode1, VendorCode2, VendorCode3, VendorCode4, XorValue1, XorValue2)
char * Name;
long * VendorCode1;
long *VendorCode2;
long * VendorCode3;
long *VendorCode4;
long *XorValue1;
long *XorValue2;

{
  int err=0;
	if (*Name)
		strcpy(vb_vendorname,Name);
	else
		err=1;

	vb_code.type    = 4;
	vb_code.data[0] = *XorValue1;
	vb_code.data[1] = *XorValue2;
	vb_code.keys[0] = *VendorCode1;
	vb_code.keys[1] = *VendorCode2;
	vb_code.keys[2] = *VendorCode3;
	vb_code.keys[3] = *VendorCode4;
        vb_code.flexlm_version = FLEXLM_VERSION;
        vb_code.flexlm_revision = FLEXLM_REVISION;
        strcpy(vb_code.flexlm_patch,FLEXLM_PATCH);
        strcpy(vb_code.behavior_ver,LM_BEHAVIOR_CURRENT);

	if ( !vb_code.data[0] || !vb_code.data[1] || !vb_code.keys[0] ||
		!vb_code.keys[1] ||
		!vb_code.keys[2] || !vb_code.keys[3] )
		err=1;

	return err;
}


VB_CALL  LC_HOSTID ( type,  buf, buf_length)
int type; 
char * buf ;
int * buf_length; 
{
  char  h_str[2048];
	int err;
	vb_job=0;
	err=lc_init(0,vb_vendorname,&vb_code,&vb_job);
	if (!err)
	{
		err= lc_hostid (vb_job, type, h_str);
		if ( *buf_length > (int) strlen( h_str ) )
		{	
			strcpy( buf , h_str );
			/* remove the trailing null for visual basic */
			buf[strlen(h_str)]=' ';
			*buf_length=0;
		}
		else
		{
			*buf_length=strlen( h_str ) ;
			err=LM_BADPARAM ;
		}
	}
	if (!vb_job) lc_free_job(vb_job);
	return err ;
}
#ifdef WINNT
VB_CALL  COM_LC_HOSTID ( type,  buf)
int type;
char * buf ;
{
        int err;
        vb_job=0;
        err=lc_init(0,vb_vendorname,&vb_code,&vb_job);
        if (!err)
        {
                err= lc_hostid (vb_job, type, buf);

        }
        if (!vb_job) lc_free_job(vb_job);
        return err ;
}



#endif

VB_CALL  LC_CRYPTSTR ( input_string, 
			output_string ,
			output_string_length,
			error_string,
			error_string_length,
			VendorKey5)
LM_CHAR_PTR input_string ; 
LM_CHAR_PTR output_string ;
int * output_string_length ;
LM_CHAR_PTR error_string ;
int * error_string_length ;
long VendorKey5 ;
{
  int err;
  char * flex_out_string;
  char * flex_error_string;
  int flag=2;
	vb_job=0;
	err=lc_init(0,vb_vendorname,&vb_code,&vb_job);
	if (!err)
	{
		vb_code.data[0]=vb_code.data[0] ^ VendorKey5;
		vb_code.data[1]=vb_code.data[1] ^ VendorKey5;
		err=lc_cryptstr (vb_job,input_string,&flex_out_string,
			&vb_code,flag,0,&flex_error_string);
		if ( *output_string_length > (int) strlen( flex_out_string ) )
		{	
			strcpy( output_string , flex_out_string );
			output_string[strlen(flex_out_string)]=' ';
			*output_string_length=0;
		}
		else
			*output_string_length=strlen( flex_out_string ) ;
	
		if ( err  )
		{
			if (*error_string_length >
				 (int) strlen( flex_error_string ))
			{
				strcpy( error_string , flex_error_string );
				error_string[strlen(flex_error_string)]=' ';
				*output_string_length=0;
			}
			else
				*error_string_length=strlen( flex_error_string ) ;
			
		}
		vb_code.data[0]=vb_code.data[0] ^ VendorKey5;
		vb_code.data[1]=vb_code.data[1] ^ VendorKey5;

		lc_free_mem(vb_job,flex_out_string);
		if (err) lc_free_mem(vb_job,flex_error_string);
	}
	if (!vb_job) lc_free_job(vb_job);

	return err;
}
#ifdef WINNT
VB_CALL  COM_LC_CRYPTSTR ( input_string,
                        output_string ,
                        error_string,
                        VendorKey5)
LM_CHAR_PTR input_string ;
LM_CHAR_PTR_PTR output_string ;
LM_CHAR_PTR_PTR error_string ;
long VendorKey5 ;
{
  int err;
  int flag=2;
        vb_job=0;
        err=lc_init(0,vb_vendorname,&vb_code,&vb_job);
        if (!err)
        {
                vb_code.data[0]=vb_code.data[0] ^ VendorKey5;
                vb_code.data[1]=vb_code.data[1] ^ VendorKey5;
                err=lc_cryptstr (vb_job,input_string,output_string,
                        &vb_code,flag,0,error_string);

                vb_code.data[0]=vb_code.data[0] ^ VendorKey5;
                vb_code.data[1]=vb_code.data[1] ^ VendorKey5;



        }
        if (!vb_job) lc_free_job(vb_job);

        return err;
}



#endif


VB_CALL  LP_CHECKOUT (policy, feature, version, nlic,license_path, v)
int  policy;
char * feature;
char * version; 
int  nlic;
char * license_path;
long *v;
{
  static LPCODE_HANDLE lpcode;
  int i;
  LP_HANDLE *  lp ;
	lpcode.code=&vb_code;
	lpcode.vendor_name=vb_vendorname;

	i= lp_checkout (&lpcode, policy,feature, version,nlic,license_path,&lp);
	*v=(long) lp;
	return i;

}


void VB_CALL  LP_CHECKIN (lp)
LP_HANDLE * * lp;
{
	 lp_checkin (*lp);

}

VB_CALL  LP_ERRSTRING (lp, s, s_length)
LP_HANDLE_PTR  * lp;
char * s;
int * s_length;
{
  char * t;
	 t= lp_errstring (*lp);
	 if (strlen(t) &&(*s_length > (int) strlen( t ) ))
	 {
		 strcpy(s,t);
		 s[strlen(t)]=' ';
		 return 0;
	 }
	 else
	 {
		*s_length= (int)strlen( t );
		 return 1;
	 }
}


VB_CALL  LP_HEARTBEAT (lp,  num_reconnect, num_retries)
LP_HANDLE_PTR * lp;
int * num_reconnect; 
int *num_retries;
{
  int i;
	i=  lp_heartbeat (*lp, num_reconnect,  *num_retries);
	return i;

}



