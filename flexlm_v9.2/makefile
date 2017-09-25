#########################################################################
#            COPYRIGHT (c) 2002 by Macrovision Corporation.
#        This software has been provided pursuant to a License Agreement
#        containing restrictions on its use.  This software contains
#        valuable trade secrets and proprietary information of 
#        Macrovision Corporation and is protected by law.  It may 
#        not be copied or distributed in any form or medium, disclosed 
#        to third parties, reverse engineered or used in any manner not 
#        provided for in said License Agreement except with the prior 
#        written authorization from Macrovision Corporation.
##########################################################################
# $Id: makefile,v 1.64 2003/03/11 00:45:54 bobm Exp $
#

XTRACFLAG = `gplatargs`  -DLM_INTERNAL $(THREADFLAG)
METER_INCLUDE = `gplatargs -imeter` 
RANLIB = `(test -f /usr/bin/ranlib || test -f /bin/ranlib) && echo ranlib || echo true`
AR = ar
STRIP = strip

THREADLIB = `gplatargs -lthreads`
XTRALIB = `gplatargs -l` 
METER_LIB = `gplatargs -lmeter`

DEBUGFLAG = -g 

LD = $(CC)

PLATFORM=`gplatform`
GPLATARGS=`gplatargs`
CERTDIR = /u/gsi/certicom/sb24ssr

DIRS = src server app master utils vendor test 

_all:	
	$(MAKE) $(PLATFORM) all \
		DEBUGFLAG="$(DEBUGFLAG)" \
		PLATFORM=$(PLATFORM) \
		XTRACFLAG="$(XTRACFLAG)" \
		RANLIB="$(RANLIB)" \
		AR="$(AR)" \
		CERTDIR="$(CERTDIR)" \
		THREADFLAG="$(THREADFLAG)" \
		THREADLIB="$(THREADLIB)"
	master/lmgrd -v
	utils/lmutil lmver src/liblmgr.a

all:	machind/lm_code.h $(PLATFORM)/makefile \
	utils/gplatform \
	$(PLATFORM)/libsb.a \
	$(PLATFORM)/libcrvs.a \
	build $(PLATFORM)/lmclient \
	$(PLATFORM)/demo \
	$(PLATFORM)/lmgrd \
	$(PLATFORM)/lmutil  \
	$(PLATFORM)/lm_nomt.o  \
	testsuitedir 
	

$(PLATFORM)/libsb.a:	$(CERTDIR)/lib/$(PLATFORM)/libsb.a 
	cp $(CERTDIR)/lib/$(PLATFORM)/libsb.a  $(PLATFORM)
	$(RANLIB) $(PLATFORM)/libsb.a
	test -f $(CERTDIR)/lib/$(PLATFORM)/libsb_pic.a  && \
		cp $(CERTDIR)/lib/$(PLATFORM)/libsb_pic.a  $(PLATFORM) && \
		$(RANLIB) $(PLATFORM)/libsb_pic.a || true

$(PLATFORM)/libcrvs.a:	$(CERTDIR)/lib/$(PLATFORM)/libcrvs.a 
	cp $(CERTDIR)/lib/$(PLATFORM)/libcrvs.a  $(PLATFORM)
	$(RANLIB) $(PLATFORM)/libcrvs.a
	test -f $(CERTDIR)/lib/$(PLATFORM)/libcrvs_pic.a  && \
		cp $(CERTDIR)/lib/$(PLATFORM)/libcrvs_pic.a  $(PLATFORM) && \
		$(RANLIB) $(PLATFORM)/libcrvs_pic.a || true

#$(PLATFORM)/liblcrvs.a:	$(CERTDIR)/lib/$(PLATFORM)/liblcrvs.a 
#	cp $(CERTDIR)/lib/$(PLATFORM)/liblcrvs.a  $(PLATFORM)

utils/gplatform: scripts/products/gplatform
	cp scripts/products/gplatform utils/gplatform

#kit:	
#	cd machind;\
#	chmod 555 *INSTALL CONFIG_DAEMON;\
#	chmod 444 *.[ch] ;\
#	chmod 664 lm_code.h *.c 
#	cd $(PLATFORM); $(MAKE) clean
#
#unkit:	
#	chmod 664 machind/*
#	-chmod 777 machind/CVS
#	cd $(PLATFORM); $(MAKE)

machind/lm_code.h:	h/lm_code.h
	grep '0x0' machind/lm_code.h > /dev/null || \
		rm -f machind/lm_code.h; cp h/lm_code.h machind

release: dorelease extralibs dotestsuite

dorelease:
	$(MAKE) DEBUGFLAG="" \
		XTRACFLAG="-DRELEASE_VERSION $(GPLATARGS) -DLM_INTERNAL" \
		XTRALIB="`gplatargs -l`" PLATFORM=`gplatform` | \
			tee `gplatform`.log

extralibs:
	if [ "`gplatargs -pic`" != "`gplatargs`" ]; then\
		cd src; $(MAKE) clean;\
			$(MAKE) DEBUGFLAG="" \
		XTRACFLAG="-DRELEASE_VERSION `gplatargs -pic` -DLM_INTERNAL" \
				XTRALIB="`gplatargs -l`" PLATFORM=`gplatform` ;\
			mv liblmgr.a ../$(PLATFORM)/liblmgr_pic.a;\
			$(MAKE) clean; \
	fi

dotestsuite:
	cd testsuite; utiltest | tee ../`gplatform`.regress

unrelease:
	cp h/lm_code.h machind


build:
	for i in $(DIRS) ;\
	do \
		cd $$i ;\
		$(MAKE) \
		DEBUGFLAG="$(DEBUGFLAG)" \
		PLATFORM=$(PLATFORM) \
		XTRACFLAG="$(XTRACFLAG)" \
		RANLIB="$(RANLIB)" \
		AR="$(AR)" \
		CERTDIR="$(CERTDIR)"  \
		XTRALIB="$(XTRALIB)" \
		THREADLIB="$(THREADLIB)" \
		THREADFLAG="$(THREADFLAG)" \
		LD='$(LD)' || exit;\
		cd .. ; \
	done

testsuitedir:
	cd testsuite; \
	$(MAKE) \
		DEBUGFLAG='$(DEBUGFLAG)' \
		XTRACFLAG='$(XTRACFLAG) ' \
		THREADFLAG="$(THREADFLAG) " \
		XTRALIB="$(XTRALIB) " \
		THREADLIB="$(THREADLIB)" \
		LD='$(LD)' 

$(PLATFORM):
	-mkdir $(PLATFORM)

$(PLATFORM)/lmclient:	$(PLATFORM)/liblmgr.a 
	cp utils/lmrand1 utils/lmnewgen.o $(PLATFORM)
	cd $(PLATFORM); FLEXLM_NO_CRO=1 $(MAKE) lmclient

$(PLATFORM)/lm_nomt.o:	src/lm_nomt.o
	cp src/lm_nomt.o $@
	if [ "`gplatargs -pic`" != "`gplatargs`" ] ; \
	then \
		cp src/lm_nomt_pic.o $(PLATFORM); \
	fi

$(PLATFORM)/liblmgr.a:	src/liblmgr.a
	cp src/liblmgr.a $@
	$(RANLIB) $@

$(PLATFORM)/liblmgr_s.a:	server/liblmgr_s.a
	cp server/liblmgr_s.a $@
	$(RANLIB) $@

$(PLATFORM)/liblmgr_as.a:	app/liblmgr_as.a
	cp app/liblmgr_as.a $@
	$(RANLIB) $@

$(PLATFORM)/liblmutil.a:	utils/liblmutil.a
	cp utils/liblmutil.a $@
	$(RANLIB) $@

$(PLATFORM)/liblmgrd.a:	master/liblmgrd.a
	cp master/liblmgrd.a $@
	$(RANLIB) $@

$(PLATFORM)/demo:	server/liblmgr_s.a app/liblmgr_as.a
	cp server/liblmgr_s.a $(PLATFORM)
	cp app/liblmgr_as.a $(PLATFORM)
	$(RANLIB) $(PLATFORM)/liblmgr_s.a $(PLATFORM)/liblmgr_as.a
	cd $(PLATFORM); FLEXLM_NO_CRO=1 $(MAKE) demo

$(PLATFORM)/lmgrd:	master/lmgrd 
	echo "FLEXlm VERSION"
	master/lmgrd -v
	cp master/lmgrd $@
	cp master/liblmgrd.a $(PLATFORM)
	$(RANLIB) $(PLATFORM)/liblmgrd.a
	
$(PLATFORM)/lmutil:	utils/lmutil
	cp utils/liblmutil.a utils/lmutil $(PLATFORM)
	$(RANLIB) $(PLATFORM)/liblmutil.a
	@echo make everything now
	cd $(PLATFORM); FLEXLM_NO_CRO=1 $(MAKE); $(MAKE) eval

$(PLATFORM)/makefile:	utils/bin.makefile
	cd $(PLATFORM) ;\
	CFLAGS="$(XTRACFLAG) $(THREADFLAG)"; \
	sed "s/XTRACFLAG =.*/XTRACFLAG = $(XTRACFLAG)/" < \
		../utils/bin.makefile | \
	sed "s/XTRALIB =.*/XTRALIB = $(XTRALIB) $(XTRALIB)/" | \
	sed "s/THREADLIB =.*/THREADLIB = $(THREADLIB) $(THREADLIB)/" | \
	sed "s/ld -shared/ld `gplatargs -ldshar`/" | \
	sed "s/	strip/	$(STRIP)/" > makefile 

cvs_move:
	tar cf - `find . -name CVS -type d -print` | \
		( cd /u/releases/flexlm/cvs.sav; tar xf -)
cvs_restore:
	( cd /u/releases/flexlm/cvs.sav; tar cf - *) | tar xf -

cvs_clean:
	rm -rf `find . -name CVS -type d -print` 

clean:
	for i in $(DIRS) testsuite ;\
	do \
		cd $$i ;\
		make clean ;\
		cd .. ; \
	done
	rm -rf $(PLATFORM)

ctags:	
	cd src; \
	ctags ../src/*.c ../server/*.[hc] ../app/*.[hc] \
		../vendor/*.[hc] ../master/*.[hc] ../h/*.h ../utils/*.[ch] 
			
