#########################################################################
#            COPYRIGHT (c) 2003 by Macrovision Corporation.
#        This software has been provided pursuant to a License Agreement
#        containing restrictions on its use.  This software contains
#        valuable trade secrets and proprietary information of 
#        Macrovision Corporation and is protected by law.  It may 
#        not be copied or distributed in any form or medium, disclosed 
#        to third parties, reverse engineered or used in any manner not 
#        provided for in said License Agreement except with the prior 
#        written authorization from Macrovision Corporation.
##########################################################################
#       Module: $Id: pc.mak,v 1.88 2003/06/10 23:06:25 jwong Exp $
#
##########################################################################

FLEXVER=v9.2
FLEXDIR="$(MAKEDIR)\\"
RELEASE_LOCATION=K:\releases\flexlm

!ifdef NOTRELEASE
RELEASE="RELEASE_VERSION=1"
DEBUG="NOTDEBUG=1"
!else
RELEASE="NOTRELEASE=1"
DEBUG="DEBUG=1"
!endif
#CERTDIR = \certicom
CERTDIR = $(CERTICOM)

!ifdef DLL
STATIC="NOTSTATIC=1"
!else
STATIC="STATIC=1"
!endif
!if "$(OS)" == "Windows_NT"
COPYARG = 
!else
COPYARG=/y
!endif
PLATFORM=i86_n3

!ifndef FLEXLM_MAKE_PLATFORM
FLEXLM_MAKE_PLATFORM = i86_n3
!endif

#!ifndef PROCESSOR_ARCHITECTURE
#PROCESSOR_ARCHITECTURE=INTEL
#PLAT=INTEL
#!endif

#
# DEFAULT platform 
#
!ifndef FLEXLM_MAKE_PLATFORM
FLEXLM_MAKE_PLATFORM = i86_n3
!endif

PLAT = $(FLEXLM_MAKE_PLATFORM)
!if "$(PLAT)"=="i86_n3"
PLAT = INTEL
PLAT86 = 1
!endif

!if "$(PLAT)"=="INTEL"
PROCESSOR_DIRECTORY=i86_n3
!else if "$(PLAT)"=="ALPHA"
PROCESSOR_DIRECTORY=alpha_n3
!else if "$(PLAT)"=="i64_n5"
PROCESSOR_DIRECTORY=it64_n

!endif

!ifdef 16BIT
PROCESSOR_DIRECTORY=i86_w3
!endif

MAKE = $(MAKE) /nologo CERTDIR=$(CERTDIR)


all:    $(PROCESSOR_DIRECTORY)\libcrvs.lib \
	$(PROCESSOR_DIRECTORY)\libsb.lib \
	machind\lm_code.h \
        $(PROCESSOR_DIRECTORY)\libsb_md.lib \
	$(PROCESSOR_DIRECTORY)\libcrvs_md.lib \
	build 
#	testsuitedir

$(PROCESSOR_DIRECTORY)\lmclient.exe:	$(PROCESSOR_DIRECTORY)\lmgr.lib
	copy utils\lmrand1.exe $(PLATFORM)
	copy utils\lmnewgen.obj $(PLATFORM)
	cd $(PLATFORM) 
	set FLEXLM_NO_CRO=1 
	$(MAKE) lmclient.exe
	cd ..
	set FLEXLM_NO_CRO=

$(PLATFORM)\lmgr.lib:	src/lmgr.lib
	copy src\lmgr.lib $@

$(PLATFORM)\lmtools.exe:
	cd lmtools
	$(MAKE) /f pc.mak
	cp lmtools.exe ..\$(PLATFORM)
	cd ..

$(PLATFORM)\lmutil.exe:	utils\lmutil.exe
	cp utils\lmutil.exe $(PLATFORM)
	@echo make everything now
	cd $(PLATFORM)
	set FLEXLM_NO_CRO=1 
	$(MAKE)
	set FLEXLM_NO_CRO=1 
	$(MAKE) clean

$(PLATFORM)\lmgrd.exe:	master\lmgrd.exe
	copy master\lmgrd $@
	
$(PLATFORM)\lmgras.lib:	app\lmgras.lib
	copy app\lmgras.lib $@

$(PLATFORM)/demo.exe:	server\lmgrs.lib app\lmgras.lib
	cd $(PLATFORM)
	set FLEXLM_NO_CRO=1 
	$(MAKE) demo.exe
	cd ..
	set FLEXLM_NO_CRO=

server\lmgrs.lib:
	cd server
	$(MAKE) 
	cd ..

app\lmgras.lib:
	cd app
	$(MAKE) 
	cd ..
	

build:
        
        cd src
        copy ..\flexlm-java-tng\c\i86_n3\lGetNativeHostId.obj .
        $(MAKE) /f pc.mak 
        cd ..\server
        $(MAKE) /f pc.mak 
        cd ..\app
        $(MAKE) /f pc.mak 
        cd ..\master
        $(MAKE) /f pc.mak 
        cd ..\utils
        $(MAKE) /f pc.mak 
        cd ..\test
        $(MAKE) /f pc.mak 
        cd ..\vendor
        $(MAKE) /f pc.mak 
        cd ..
        $(MAKE) /f pc.mak kit 
#	cd .\genlic
#	$(MAKE) /f pc.mak 
#       cd ..

testsuitedir:
        cd testsuite
        $(MAKE) /f pc.mak 
	cd ..

$(PROCESSOR_DIRECTORY)\libcrvs.lib: $(CERTDIR)\lib\libcrvs.lib
		if not exist "$(PROCESSOR_DIRECTORY)/$(NULL)" mkdir "$(PROCESSOR_DIRECTORY)"
        copy $(CERTDIR)\lib\libcrvs.lib $(PROCESSOR_DIRECTORY)

$(PROCESSOR_DIRECTORY)\libcrvs_md.lib: $(CERTDIR)\lib\libcrvs_md.lib
        copy $(CERTDIR)\lib\libcrvs_md.lib $(PROCESSOR_DIRECTORY)

$(PROCESSOR_DIRECTORY)\libsb.lib: $(CERTDIR)\lib\libsb.lib
        copy $(CERTDIR)\lib\libsb.lib $(PROCESSOR_DIRECTORY)

$(PROCESSOR_DIRECTORY)\libsb_md.lib: $(CERTDIR)\lib\libsb_md.lib
        copy $(CERTDIR)\lib\libsb_md.lib $(PROCESSOR_DIRECTORY)

machind\lm_code.h:  h\lm_code.h
        copy h\lm_code.h machind

md: 	
	cd src
	-echo [Building MD Version]
        $(MAKE) /f pc.mak clean
        $(MAKE) /f pc.mak  ..\src\lmgr.lib FLEXLM_MAKE_MT=MD 
        copy lmgr.lib ..\$(PROCESSOR_DIRECTORY)\lmgr_md.lib
	@if exist lmgr.pdb copy lmgr.pdb ..\$(PROCESSOR_DIRECTORY)\lmgr_md.pdb
	cd ..
	-echo [Building lmnewgen_md.obj]
	cd utils
	$(MAKE) /f pc.mak FLEXLM_MAKE_MT=MD lmnewgen_md.obj
	@if exist lmnewgen_md.obj copy lmnewgen_md.obj ..\$(PROCESSOR_DIRECTORY)
	cd ..
        cd app
        $(MAKE) /f pc.mak clean
        $(MAKE) /f pc.mak BUILDMD FLEXLM_MAKE_MT=MD
        copy lmgras_md.lib ..\$(PROCESSOR_DIRECTORY)\lmgras_md.lib
	cd .. 
        cd server
        $(MAKE) /f pc.mak clean
        $(MAKE) /f pc.mak MD=1 FLEXLM_MAKE_MT=MD
        copy lmgrs_md.lib ..\$(PROCESSOR_DIRECTORY)\lmgrs_md.lib
	cd ..

kit:
        -echo [Installing MT Version]
        copy src\lmgr.lib $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy src\lmgr.res $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy server\lmgrs.lib $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy app\lmgras.lib $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy master\lmgrd.exe $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy utils\lmutil.exe $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy utils\lmrand1.exe $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy utils\lmnewgen.obj $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy utils\lmborrow.exe $(PROCESSOR_DIRECTORY) $(COPYARG)
        copy utils\lmpath.exe $(PROCESSOR_DIRECTORY) $(COPYARG)
	copy utils\lmstrip\lmstrip.exe $(PROCESSOR_DIRECTORY)\lmstrip.exe
	@echo [*** NOTE: Remeber to build LMTOOLS.EXE ***]
	@echo

releasekit: 
#        cd src
#        $(MAKE) /f pc.mak
#        cd ..
#        $(MAKE) /f pc.mak kit
#        cd src
#
#        -echo [Building DLL Version]
#        $(MAKE) /f pc.mak clean
#        $(MAKE) /f pc.mak  lmgr8b.dll FLEXLM_MAKE_DEBUGFLAG=NONDEBUG \
#                FLEXLM_MAKE_RELEASEFLAG=RELEASE \
#                FLEXLM_MAKE_DLLFLAG=DLL FLEXDIR=$(FLEXDIR)
#        copy lmgr8a.dll ..\$(PROCESSOR_DIRECTORY)
#        copy lmgr327b.lib ..\$(PROCESSOR_DIRECTORY)
#        -echo [Building MD Version]
#        $(MAKE) /f pc.mak clean
#        $(MAKE) /f pc.mak  FLEXLM_MAKE_DEBUGFLAG=NONDEBUG \
#                FLEXLM_MAKE_RELEASEFLAG=RELEASE \
#                FLEXLM_MAKE_MT=MD FLEXDIR=$(FLEXDIR)
#        copy lmgr.lib ..\$(PROCESSOR_DIRECTORY)\lmgr_md.lib
#        -echo [Rebuilding MT in src directory]
#        $(MAKE) /f pc.mak clean
#        $(MAKE) /f pc.mak
#        cd ..

movekit:
	-echo [Moving files into kit release location: $(RELEASE_LOCATION)]
	if exist $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY) rmdir /s /q $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY)
	if exist $(RELEASE_LOCATION)\$(FLEXVER)\machind rmdir /s /q $(RELEASE_LOCATION)\$(FLEXVER)\machind
	if exist $(RELEASE_LOCATION)\$(FLEXVER)\examples rmdir /s /q $(RELEASE_LOCATION)\$(FLEXVER)\examples
	if exist $(RELEASE_LOCATION)\$(FLEXVER)\htmlman rmdir /s /q $(RELEASE_LOCATION)\$(FLEXVER)\htmlman
	if exist $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexprog rmdir /s /q $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexprog
	if exist $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexref rmdir /s /q $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexref
	if exist /q $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexuser rmdir /s /q $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexuser
	xcopy /s /e /i $(PROCESSOR_DIRECTORY)\*.* $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY)
	xcopy /s /e /i machind\*.* $(RELEASE_LOCATION)\$(FLEXVER)\machind
	xcopy /s /e /i examples\*.* $(RELEASE_LOCATION)\$(FLEXVER)\examples
	xcopy /s /e /i k:\working\lmdocs\flexprog\htmlman\*.* $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexprog	
	xcopy /s /e /i k:\working\lmdocs\flexref\htmlman\*.* $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexref	
	xcopy /s /e /i k:\working\lmdocs\flexuser\htmlman\*.* $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\flexuser
	copy k:\working\lmdocs\index.htm $(RELEASE_LOCATION)\$(FLEXVER)\htmlman	
	copy k:\working\lmdocs\flexprog\prog.pdf $(RELEASE_LOCATION)\$(FLEXVER)\machind
	copy k:\working\lmdocs\flexref\ref.pdf $(RELEASE_LOCATION)\$(FLEXVER)\machind
	copy k:\working\lmdocs\flexuser\enduser.pdf $(RELEASE_LOCATION)\$(FLEXVER)\machind
	-del $(RELEASE_LOCATION)\$(FLEXVER)\machind\lm_code.h
	copy $(RELEASE_LOCATION)\$(FLEXVER)\machind\blank_lm_code.h $(RELEASE_LOCATION)\$(FLEXVER)\machind\lm_code.h
	-del $(RELEASE_LOCATION)\$(FLEXVER)\machind\blank_lm_code.h
	-attrib -r $(RELEASE_LOCATION)\$(FLEXVER)\htmlman\*.*
!if "$(PROCESSOR_DIRECTORY))"=="it64_n"
	copy $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY)\genlic64.exe $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY)\genlic64.exe
!else
	copy $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY)\genlic32.exe $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY)\genlic.exe
!endif
#
# copy flexid installer to kit
#
#	copy $(RELEASE_LOCATION)\docs\$(FLEXVER)\*.pdf $(RELEASE_LOCATION)\$(FLEXVER)\machind
	copy $(RELEASE_LOCATION)\flexidinstall\$(PROCESSOR_DIRECTORY)\*.exe $(RELEASE_LOCATION)\$(FLEXVER)\$(PROCESSOR_DIRECTORY)

clean:
        cd src
        -$(MAKE) /f pc.mak clean

        cd ..\server
        $(MAKE) /f pc.mak clean

        cd ..\app
        $(MAKE) /f pc.mak clean

        cd ..\master
        $(MAKE) /f pc.mak clean

        cd ..\utils
        $(MAKE) /f pc.mak clean

	cd ..\test
	$(MAKE) /f pc.mak clean

        cd ..\vendor
        $(MAKE) /f pc.mak clean

        cd ..\testsuite
        $(MAKE) /f pc.mak clean

        cd ..
#        cd genlic
#        $(MAKE) /f pc.mak clean
#        cd ..

release:
	@echo [Cleaning build for $(PROCESSOR_DIRECTORY)...]
	$(MAKE) /f pc.mak clean
	@echo [Building MT for $(PROCESSOR_DIRECTORY)...]
	$(MAKE) /f pc.mak all
	@echo [Building MD for $(PROCESSOR_DIRECTORY)...]
	$(MAKE) /f pc.mak MD
	@echo [Building $(PROCESSOR_DIRECTORY)...]
	cd $(PROCESSOR_DIRECTORY)
	$(MAKE) clean
	copy ..\h\lm_code.h ..\machind
	$(MAKE)
	$(MAKE) buildmsgfile
	$(MAKE) kitclean
	cd ..
	@echo [Finished building kit! Don't forget to build lmtools utility.]
	@echo [If this is a version # change. Make sure to build FLEXlock with the GSI vendorname.]

