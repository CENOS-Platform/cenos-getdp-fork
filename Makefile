# $Id: Makefile,v 1.132 2003-03-18 01:37:52 geuzaine Exp $

include variables

GETDP_MAJOR_VERSION = 0
GETDP_MINOR_VERSION = 88
GETDP_PATCH_VERSION = 0

GETDP_VERSION_FILE = include/GetDPVersion.h
GETDP_RELEASE = ${GETDP_MAJOR_VERSION}.${GETDP_MINOR_VERSION}.${GETDP_PATCH_VERSION}
GETDP_ARCHIVE = archives/getdp-`date "+%Y.%m.%d"`
GETDP_SRCRPM = getdp-${GETDP_RELEASE}

GETDP_SOURCES = `find . \( ! -name "*.tar*" -a ! -name "*.tgz" \
                        -a ! -name "*.o"    -a ! -name "*.a"   -a ! -name "*.cut"   \
                        -a ! -name "*.pos"  -a ! -name "*.pre" -a ! -name "*.res" \
                        -a ! -name "*.bak"  -a ! -name "getdp" -a ! -name "getdp-*" \
                        -a ! -type d       \)`

all: variables initialtag compile link

compile: variables initialtag
	@for i in ${GETDP_DIRS}; do (cd $$i && ${MAKE}); done

link: variables
	${LINKER} -o bin/getdp ${GETDP_LIBS}

install: variables
	-mkdir -p ${bindir}
	-cp bin/getdp${EXEEXT} ${bindir}
	-chmod 755 ${bindir}/getdp${EXEEXT}
	-mkdir -p ${mandir}/man1
	-cp doc/getdp.1 ${mandir}/man1

variables: configure
	@echo "********************************************************************"
	@echo "Please configure GetDP by running ./configure"
	@echo "For help, type ./configure --help"
	@echo "********************************************************************"
	@exit 1

tag:
	rm -f ${GETDP_VERSION_FILE}
	echo "#define GETDP_MAJOR_VERSION ${GETDP_MAJOR_VERSION}" >  ${GETDP_VERSION_FILE}
	echo "#define GETDP_MINOR_VERSION ${GETDP_MINOR_VERSION}" >> ${GETDP_VERSION_FILE}
	echo "#define GETDP_PATCH_VERSION ${GETDP_PATCH_VERSION}" >> ${GETDP_VERSION_FILE}
	echo "#define GETDP_VERSION  ((double)GETDP_MAJOR_VERSION + \\" >> ${GETDP_VERSION_FILE}
	echo "                 0.01 * (double)GETDP_MINOR_VERSION + \\" >> ${GETDP_VERSION_FILE}
	echo "               0.0001 * (double)GETDP_PATCH_VERSION)"     >> ${GETDP_VERSION_FILE}
	echo "#define GETDP_DATE     \"`date`\""      >> ${GETDP_VERSION_FILE}
	echo "#define GETDP_HOST     \"`hostname`\""  >> ${GETDP_VERSION_FILE}
	echo "#define GETDP_PACKAGER \"`whoami`\""    >> ${GETDP_VERSION_FILE}
	echo "#define GETDP_OS       \"`uname -sr`\"" >> ${GETDP_VERSION_FILE}

initialtag:
	@if [ ! -r ${GETDP_VERSION_FILE} ]; then ${MAKE} tag ; fi

tags:
	gtags
	htags

parser:
	cd Parser && ${MAKE} parser

depend:
	for i in ${GETDP_DIRS}; do (cd $$i && ${MAKE} depend); done

nodepend:
	for i in ${GETDP_DIRS}; do (cd $$i && ${MAKE} nodepend); done

clean:
	for i in lib doc/texinfo ${GETDP_DIRS}; do (cd $$i && ${MAKE} clean ); done
	rm -f ${GETDP_VERSION_FILE}

doc-info:
	cd doc/texinfo && ${MAKE} info
	cp doc/texinfo/getdp.info* doc/

doc-ps:
	cd doc/texinfo && ${MAKE} ps
	cp doc/texinfo/getdp.ps doc/

doc-pdf:
	cd doc/texinfo && ${MAKE} pdf
	cp doc/texinfo/getdp.pdf doc/

purge:
	for i in . lib include doc archives ${GETDP_DIRS}; \
        do (cd $$i && rm -f *~ *~~ \#*\#); done

tgz:
	if (test -f ${GETDP_ARCHIVE}.tar.gz); \
	then mv -f ${GETDP_ARCHIVE}.tar.gz ${GETDP_ARCHIVE}.tar.gz~; \
	fi
	tar cvf ${GETDP_ARCHIVE}.tar ${GETDP_SOURCES}
	gzip ${GETDP_ARCHIVE}.tar
	chmod 640 ${GETDP_ARCHIVE}.tar.gz

zip:
	zip ${GETDP_ARCHIVE}.zip ${GETDP_SOURCES}

minizip:
	tar jcvf getdp-`date "+%Y.%m.%d"`.tar.bz2\
            `ls Makefile */Makefile */*.[cfFhyl] */*.cpp */*.txt */*.pro */*.par */*.geo`

tgzdoc:
	tar cvf getdp-texi.tar doc/texinfo
	gzip getdp-texi.tar

package-unix:
	rm -rf getdp-${GETDP_RELEASE}
	mkdir getdp-${GETDP_RELEASE}
	strip bin/getdp
	cp bin/getdp getdp-${GETDP_RELEASE}
	cp doc/VERSIONS doc/FAQ doc/CONTRIBUTORS getdp-${GETDP_RELEASE}
	cp -R demos getdp-${GETDP_RELEASE}
	rm -rf getdp-${GETDP_RELEASE}/*/CVS
	rm -f getdp-${GETDP_RELEASE}/*/*.pre
	rm -f getdp-${GETDP_RELEASE}/*/*.res
	rm -f getdp-${GETDP_RELEASE}/*/*.pos
	rm -f getdp-${GETDP_RELEASE}/*/*.cut
	rm -f getdp-${GETDP_RELEASE}/*/*~
	tar cvf getdp-${GETDP_RELEASE}-${UNAME}.tar getdp-${GETDP_RELEASE}
	gzip getdp-${GETDP_RELEASE}-${UNAME}.tar
	mv getdp-${GETDP_RELEASE}-${UNAME}.tar.gz getdp-${GETDP_RELEASE}-${UNAME}.tgz

package-windows:
	rm -rf getdp-${GETDP_RELEASE}
	mkdir getdp-${GETDP_RELEASE}
	strip bin/getdp.exe
	cp /usr/bin/cygwin1.dll getdp-${GETDP_RELEASE}
	cp bin/getdp.exe getdp-${GETDP_RELEASE}
	cp doc/README.txt getdp-${GETDP_RELEASE}
	cp doc/VERSIONS getdp-${GETDP_RELEASE}/VERSIONS.txt
	cp doc/FAQ getdp-${GETDP_RELEASE}/FAQ.txt
	cp doc/CONTRIBUTORS getdp-${GETDP_RELEASE}/CONTRIBUTORS.txt
	cd utils && unix2dos ../getdp-${GETDP_RELEASE}/*.txt
	cp -R demos getdp-${GETDP_RELEASE}
	rm -rf getdp-${GETDP_RELEASE}/*/CVS
	rm -f getdp-${GETDP_RELEASE}/*/*.pre
	rm -f getdp-${GETDP_RELEASE}/*/*.res
	rm -f getdp-${GETDP_RELEASE}/*/*.pos
	rm -f getdp-${GETDP_RELEASE}/*/*.cut
	rm -f getdp-${GETDP_RELEASE}/*/*~
	cd utils && unix2dos ../getdp-${GETDP_RELEASE}/demos/*
	cd getdp-${GETDP_RELEASE} && zip -r getdp-${GETDP_RELEASE}-Windows.zip *
	mv getdp-${GETDP_RELEASE}/getdp-${GETDP_RELEASE}-Windows.zip .

source:
	rm -rf getdp-${GETDP_RELEASE}
	tar zcvf getdp.tgz `ls README* configure *.in Makefile */Makefile\
                            */*.[chylfF] */*.[ch]pp */*.opt */*.spec` doc demos
	mkdir getdp-${GETDP_RELEASE}
	cd getdp-${GETDP_RELEASE} && tar zxvf ../getdp.tgz
	rm -f getdp.tgz
	cd getdp-${GETDP_RELEASE}/demos && ${MAKE} clean
	cd getdp-${GETDP_RELEASE}/doc && ${MAKE} clean
	cd getdp-${GETDP_RELEASE} && rm -rf Scattering trash utils doc/slides\
                                            ${GETDP_VERSION_FILE} CVS */CVS */*/CVS
	tar zcvf getdp-${GETDP_RELEASE}-source.tgz getdp-${GETDP_RELEASE}

distrib-unix: clean all package-unix

distrib-windows: clean all package-windows

rpmold:
	tar zcvf /usr/src/redhat/SOURCES/${GETDP_SRCRPM}.tar.gz ${GETDP_SOURCES}
	rpm -bb --define 'getdpversion ${GETDP_RELEASE}' utils/getdp.spec
	cp /usr/src/redhat/RPMS/i386/${GETDP_SRCRPM}-1.i386.rpm .
	cp /usr/src/redhat/BUILD/${GETDP_SRCRPM}/getdp-${GETDP_RELEASE}-${UNAME}.tgz .

rpm:
	tar zcvf /usr/src/redhat/SOURCES/${GETDP_SRCRPM}.tar.gz ${GETDP_SOURCES}
	rpmbuild -bb --define 'getdpversion ${GETDP_RELEASE}' utils/getdp.spec
	cp /usr/src/redhat/RPMS/i386/${GETDP_SRCRPM}-1.i386.rpm .
	cp /usr/src/redhat/BUILD/${GETDP_SRCRPM}/getdp-${GETDP_RELEASE}-${UNAME}.tgz .

blackbox: initialtag
	@for i in ${GETDP_DIRS}; \
        do (cd $$i && ${MAKE} \
           "C_FLAGS=${C_FLAGS} -D_BLACKBOX" \
        ); done
	${LINKER} -o bin/getdp-box ${GETDP_LIBS}

compile-scat: initialtag
	@for i in ${GETDP_DIRS} Scattering; do (cd $$i && ${MAKE} \
           "CC=${CC}" \
           "CXX=${CXX}" \
           "F77=${F77}" \
           "RANLIB=${RANLIB}" \
           "C_FLAGS=${COPTFLAGS} -I${FFTW_PREFIX}/include -Wall -Wno-uninitialized" \
           "F77_FLAGS=${FOPTFLAGS}" \
           "SOLVER=-D_PETSC ${PETSCFLAGS} ${PETSC_INCLUDE}" \
        ); done
link-scat:
	${CLINKER} -o bin/hf lib/libScattering.a lib/libDofData.a lib/libNumeric.a\
                   lib/libDataStr.a lib/libNR.a  ${PETSC_SLES_LIB}\
                   -L${FFTW_PREFIX}/lib -lfftw -lm
scat: compile-scat link-scat
