/******************************************************************************

	    COPYRIGHT (c) 2001, 2003 by Macrovision Corporation.
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
 *	Module: $Id: l_rand.h,v 1.7 2003/01/13 22:13:12 kmaclean Exp $
 *
 *	Description:  rand routine used by l_strkey.c and others
 *
 *	D. Birns
 *	2/01
 *
 */

/*
 *	Rand routines for l_strkey.c and other files
 */

#define LRAND_MODMULT(a,b,c,m,s) q = s/a; s = b * (s - a * q) - c * q; \
        if(s < 0) s += m
#define LRAND_R1      32363
#define LRAND_R2      31727
#define LRAND_R3      31657

static
void 
srand16(i1, i2, i3, seeds)
int i1;
int i2; 
int i3; 
int *seeds;
{
        seeds[0] = i1 % LRAND_R1;
        seeds[1] = i2 % LRAND_R2;
        seeds[2] = i3 % LRAND_R3;
        if(seeds[0] <= 0) seeds[0] += LRAND_R1 - 1;
        if(seeds[1] <= 0) seeds[1] += LRAND_R2 - 1;
        if(seeds[2] <= 0) seeds[2] += LRAND_R3 - 1;
}

/*
 *	For reasons unknown, we wound up with 2 almost identical
 *	version of this function:  in one the return value is shifted
 *	right by one bit (presumably to avoid negative values), but not
 *	in the other.  To maintain compatibility
 *	we made it a flag
 */
static 
int 
rand16(int *seeds, 
	int shiftit) 
{
  int q;
  int z;


        LRAND_MODMULT(206, 157, 21, LRAND_R1, seeds[0]);
        LRAND_MODMULT(217, 146, 45, LRAND_R2, seeds[1]);
        LRAND_MODMULT(222, 142, 133, LRAND_R3, seeds[2]);

        z = seeds[0] - seeds[1];
        if(z > 706) z -= 32362;
        z += seeds[2];
        if(z < 1) z += 32362;
	if (shiftit) return(z>>1);
	else return(z);
}
static int
rand16_2(int *seeds)
{
#if 1
  int s2[3], s3[3], i;
	s3[0] = rand16(seeds, 1);
	s3[1] = rand16(seeds, 1);
	s3[2] = rand16(seeds, 1);
	srand16(s3[0], s3[1], s3[2], s2);
	for (i = 0; i < s3[2]%10; i++)
		rand16(s2, 1);
	for (i = 0; i < rand16(s2, 1)%10; i++)
		rand16(seeds, 1);
	return rand16(seeds, 1);
#else
	{
 	  int ret = rand16(seeds);
		/*fprintf(stdout, "%x\n", ret);*/

		return ret;
	}
	
#endif
}


