#include "lmclient.h"
#include "l_intelid.h"
#include "l_eintelid.h"

void show_id( HOSTID *idptr )
{
	if( NULL == idptr )
	{
		printf( "No CPU id\n" );
		return;
	}

	while( idptr )
	{
	  int i;

		printf( "CPU ID: " );
		for( i = 0; i < (3*4); i++ )
			printf( "%02x", (unsigned char) idptr->id.pc_cpu[i] );
		printf( "\n" );

		idptr = idptr->next;
	}
}

void main()
{
  HOSTID *idptr;
  LM_HANDLE job;

	printf( "Testing cpu id without obfuscation\n" );
	idptr = l_intelid( &job, 3 );
	show_id( idptr );

	printf( "Testing cpu id with obfuscation\n" );
	idptr = l_eintelid( &job );
	show_id( idptr );

	exit( 0 );
}


