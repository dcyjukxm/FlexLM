

Build instructions for FLEXlm ultralite

$Id: readme.txt,v 1.1 2002/12/12 17:44:30 kmaclean Exp $

Last changed:
	Kirk MacLean
	Dec 12, 2002

Ultralite is messed up. Read the docs/README file to start with. It is in 
the main CVS branch.

The last release of ultralite was done from the v7-0c-1 branch. As of 
12/12/02 ultralite in the main branch is updated to compile and work
for VxWorks. If you can't get it to work from the main branch then try the 
v7-0c-1 branch. Some of the docs can only be found in the main branch.

There are two build methods. The old way and the new way.

Old way:
	Do not add new platforms to this method.
	This only works on UNIX. Use the 'makefile' in the src directory.
		make linksrc
		ln -s ../utils/ul_platargs .
		chmod a+x ul_platargs
		make
		make lmlite
	TO make the binary kit use the shell scripts in the mfg directory.

The new way:
	This has only been tested on the PC but should work on UNIX. If it 
	doesn't	work let me know and I'll fix it.
	Use GNU make. 
		cd src	
		make -f ulite.mak GPLATFORM=xyz
	Where xyz is one of the valid system names that used to come from 
	the gplatargs script.

	There are further instructions in the makefile ulite.mak.
	To build a release and the binary kit:
		make -f ulite.mak GPLATFORM=xyz clean
		make -f ulite.mak GPLATFORM=xyz release
	This will create a new directory 'ulite_<xyz>' in the top directory.
	The docs will need to be copied in manualy.

	As of 12/09/2002 this suystem has been tested with the following:
	GPLATFORM=
		vx_simpc  -  The VxWorks simulator on the PC
			     Currently there are no releas enotes or
			     docs for this version.	    

