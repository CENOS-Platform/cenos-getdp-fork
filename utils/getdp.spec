Summary: A General environment for the treatment of Discrete Problems
Name: getdp
Version: 0.81
Source: getdp-0.81.tar.gz
Release: 1
Copyright: distributable
Group: Applications/Engineering
URL: http://www.geuz.org/getdp/
Packager: Christophe.Geuzaine@AdValvas.be
Prereq: /sbin/install-info
Buildroot: /var/tmp/%{name}-buildroot

%description 
GetDP is a scientific software environment for the numerical solution
of integro-differential equations, open to the coupling of physical
problems (electromagnetic, thermal, mechanical, etc) as well as of
numerical methods (finite element method, integral methods, etc). It
can deal with such problems of various dimensions (1D, 2D, 2D
axisymmetric or 3D) and time states (static, transient or
harmonic). The main feature of GetDP is the closeness between the
organization of data defining discrete problems (written by the user
in ASCII data files) and the symbolic mathematical expressions of
these problems.

Install GetDP if you need a finite element solver for multiphysic
problems.

%prep

%setup -c -q

%build
make linux
make compress_bin
make doc-pdf
make doc-info
rm -rf CVS */CVS */*/CVS

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/usr/bin
mkdir -p $RPM_BUILD_ROOT/usr/man/man1
mkdir -p $RPM_BUILD_ROOT/usr/info

install -m 755 bin/getdp $RPM_BUILD_ROOT/usr/bin/getdp
install -m 644 doc/getdp.1 $RPM_BUILD_ROOT/usr/man/man1/getdp.1
install -m 644 doc/texinfo/getdp.info* $RPM_BUILD_ROOT/usr/info/

%post
/sbin/install-info /usr/info/getdp.info /usr/info/dir

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc doc/getdp.pdf doc/VERSIONS doc/FAQ doc/BUGS doc/CONTRIBUTORS demos
/usr/bin/getdp
/usr/man/man1/getdp*
/usr/info/getdp*
