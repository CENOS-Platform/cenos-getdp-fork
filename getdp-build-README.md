### 29/08/2022 CENOS (c)
### CENOS GetDP 64 threads optimized ###


# CENOS threaded GetDP:
    This release is with max 64 threads and optimized for microarchitectures.
    ***This release already contains wide character/spaces fix.***
    
    To set up Your system for multithreaded CENOS usage :

    !!! Ask for current GetDP release in slack #dev channel !!!
    
    Open Control Panel->System and Security->System->Advanced system settings->Environment Variables->New

        VARIABLE NAME: OPENBLAS_NUM_THREADS
        VARIABLE VALUE: max_thread_count_to_use

        VARIABLE NAME: OPENBLAS_CORETYPE
        VARIABLE VALUE: your_cpu_microarchitecture_name

        *** Set this to see which cpu Architecture we are using ***
        VARIABLE NAME: OPENBLAS_VERBOSE
        VARIABLE VALUE: 2
     
        For example :

        VARIABLE NAME: OPENBLAS_NUM_THREADS
        VARIABLE VALUE: 8

        VARIABLE NAME: OPENBLAS_CORETYPE
        VARIABLE VALUE: HASWELL

        VARIABLE NAME: OPENBLAS_VERBOSE
        VARIABLE VALUE: 2

        Press OK
        Press OK
        Press OK

    4) Now Your system is set to use chosen thread limit.
    5) Fire up CENOS and observe CPU usage in calculation phase. 
    6) Please create multithreading related issues or comments, especially for different CPU types, good/bad/unsatisfying.
    
    List of optimized microarchitectures :
    Intel CPU:
        P2
        KATMAI
        COPPERMINE
        NORTHWOOD
        PRESCOTT
        BANIAS
        YONAH
        CORE2
        PENRYN
        DUNNINGTON
        NEHALEM
        SANDYBRIDGE
        HASWELL
        SKYLAKEX
        ATOM
        COOPERLAKE

    AMD CPU:
        ATHLON
        OPTERON
        OPTERON_SSE3
        BARCELONA
        SHANGHAI
        ISTANBUL
        BOBCAT
        BULLDOZER
        PILEDRIVER
        STEAMROLLER
        EXCAVATOR
        ZEN


    NOTE : If You specify OPENBLAS_CORETYPE that exists but is NOT Your cpu type, getdp.exe may or may not crash. 
    NOTE : Make sure You type the OPENBLAS_CORETYPE correctly !
    NOTE : If Your cpu is not listed, try CORE2, that is default fallback for unlisted scenario.
    
    TODO : !!! publish already working multithreaded code for faster postprocessing !!!



### 28/03/2021 CENOS (c)
### CENOS GetDP 64 threads initial release ###


# CENOS threaded GetDP:
    This is a threaded GetDP executable.
    Initial release is with max 64 threads.
    ***Initial release already contains wide character/spaces fix.***
    
    To set up Your system for multithreaded CENOS usage :

    Download files here :
        https://gitlab.com/cenos/cenos-getdp/-/blob/master/getdp-cenos-64threads.tar.gz
    
    1) Backup getdp.exe in Your CENOS GetDP folder.
    2) Uncompress getdp-cenos-64threads.tar.gz archive and copy its contents/files into Your CENOS GetDP folder.
    3) Open Control Panel->System and Security->System->Advanced system settings->Environment Variables->New

        VARIABLE NAME: OPENBLAS_NUM_THREADS
        VARIABLE VALUE: max_thread_count_to_use
     
        For example :

        VARIABLE NAME: OPENBLAS_NUM_THREADS
        VARIABLE VALUE: 8

        Press OK
        Press OK
        Press OK

    4) Now Your system is set to use chosen thread limit.
    5) Fire up CENOS and observe CPU usage in calculation phase. 
    6) Please create multithreading related issues or comments, especially for different CPU types, good/bad/unsatisfying.
    
    TODO : !!! publish already working multithreaded code for faster postprocessing !!!

### 06/09/2021 CENOS (c)
### Building our own GetDP.exe ###

    This file also contains step by step build process of every component.
    It is based on excellent getdp build instructions by Christophe Geuzaine, 
        https://gitlab.onelab.info/geuzaine

    Christophe Geuzaine getdp.exe build instructions can be found here :
    https://gitlab.onelab.info/getdp/getdp/-/wikis/GetDP-compilation


### TARGET :

    For this build we will be using cygwin on windows, with several install options - instructions below.
    Also we will need source code for all libraries listed - instructions below.

    And for each library built we will have a set of specific options - which have to be set properly in 
    order to have a successful build.
    
    A result will be multithreaded GetDP.exe we can use on windows platform.
    
    **** Please adjust all the paths to Your specific locations ****

### Tools used :
    Cygwin : https://cygwin.com/install.html -> https://cygwin.com/setup-x86_64.exe
        Options installed, full list with version info : 
```
_autorebase                             001007-1
adwaita-icon-theme                      3.26.1-1
alternatives                            1.3.30c-10
at-spi2-core                            2.26.2-1
autoconf                                13-1
autoconf2.1                             2.13-12
autoconf2.5                             2.69-4
automake                                11-1
automake1.10                            1.10.3-3
automake1.11                            1.11.6-3
automake1.12                            1.12.6-3
automake1.13                            1.13.4-2
automake1.14                            1.14.1-3
automake1.15                            1.15.1-2
automake1.16                            1.16.1-1
automake1.9                             1.9.6-11
base-cygwin                             3.8-1
base-files                              4.3-2
bash                                    4.4.12-3
binutils                                2.36.1-2
bzip2                                   1.0.8-1
ca-certificates                         2.40-1
clang                                   8.0.1-1
cmake                                   3.20.0-1
cmake-debuginfo                         3.20.0-1
compiler-rt                             8.0.1-1
coreutils                               8.26-2
crypto-policies                         20190218-1
csih                                    0.9.11-1
cygrunsrv                               1.62-1
cygutils                                1.4.16-2
cygwin                                  3.2.0-1
cygwin-debuginfo                        3.2.0-1
cygwin-devel                            3.2.0-1
cygwin32                                2.10.0-1
cygwin32-binutils                       2.29-1
cygwin32-default-manifest               6.4-1
cygwin32-gcc-core                       6.4.0-1
cygwin32-gcc-fortran                    6.4.0-1
cygwin32-gcc-g++                        6.4.0-1
cygwin32-w32api-headers                 4.0.4-1
cygwin32-w32api-runtime                 4.0.4-1
dash                                    0.5.9.1-1
dconf-service                           0.26.1-1
dejavu-fonts                            2.37-1
desktop-file-utils                      0.23-1
diffutils                               3.5-2
dri-drivers                             19.1.6-1
editrights                              1.03-1
expat                                   2.2.6-1
file                                    5.39-1
findutils                               4.6.0-1
gamin                                   0.1.10-15
gawk                                    5.1.0-1
gcc-core                                10.2.0-1
gcc-fortran                             10.2.0-1
gcc-g++                                 10.2.0-1
gcovr                                   4.0-1
gdb                                     9.2-1
gdk-pixbuf2.0-svg                       2.40.20-1
gendef                                  1.0-svn2931-1
getent                                  2.18.90-4
girepository-GLib2.0                    1.54.1-2
git                                     2.31.1-1
glib2.0-networking                      2.54.1-1
gnupg                                   1.4.23-1
grep                                    3.0-2
groff                                   1.22.4-1
gsettings-desktop-schemas               3.24.1-1
gtk-update-icon-cache                   3.22.28-1
gzip                                    1.8-1
hicolor-icon-theme                      0.15-1
hostname                                3.13-1
info                                    6.7-1
ipc-utils                               1.0-2
less                                    563-1
libarchive13                            3.5.1-1
libargp                                 20110921-3
libassuan0                              2.5.3-1
libatk-bridge2.0_0                      2.26.1-1
libatk1.0_0                             2.26.1-1
libatomic1                              10.2.0-1
libatspi0                               2.26.2-1
libattr1                                2.4.48-2
libblkid1                               2.33.1-2
libboost_regex1.66                      1.66.0-1
libbrotlicommon1                        1.0.9-1
libbrotlidec1                           1.0.9-1
libbz2_1                                1.0.8-1
libc++-devel                            8.0.1-1
libc++1                                 8.0.1-1
libc++abi-devel                         8.0.1-1
libc++abi1                              8.0.1-1
libcairo2                               1.17.4-1
libcares2                               1.14.0-1
libcbor                                 0.5.0-1
libclang8                               8.0.1-1
libcom_err2                             1.44.5-1
libcroco0.6_3                           0.6.12-1
libcrypt-devel                          4.4.4-1
libcrypt0                               2.1-1
libcrypt2                               4.4.4-1
libcurl4                                7.76.0-1
libdatrie1                              0.2.8-1
libdb5.3                                5.3.28-2
libdbus1_3                              1.10.22-1
libdeflate0                             1.7-1
libedit0                                20130712-1
libepoxy0                               1.4.3-1
libevent2.0_5                           2.0.22-1
libexpat1                               2.2.6-1
libext2fs2                              1.44.5-1
libfam0                                 0.1.10-15
libfdisk1                               2.33.1-2
libffi6                                 3.2.1-2
libfido2                                1.5.0-1
libfontconfig-common                    2.13.1-2
libfontconfig1                          2.13.1-2
libfreetype6                            2.10.4-2
libgc1                                  8.0.4-1
libgcc1                                 10.2.0-1
libgdbm6                                1.18.1-1
libgdbm_compat4                         1.18.1-1
libgdk_pixbuf2.0_0                      2.36.11-1
libgfortran4                            7.4.0-1
libgfortran5                            10.2.0-1
libgirepository1.0_1                    1.54.1-2
libGL1                                  19.1.6-1
libglapi0                               19.1.6-1
libglib2.0_0                            2.54.3-1
libgmp10                                6.2.1-1
libgnutls30                             3.6.9-1
libgomp1                                10.2.0-1
libgpg-error0                           1.37-1
libgpgme11                              1.9.0-1
libgraphite2_3                          1.3.14-1
libgssapi_krb5_2                        1.15.2-2
libgtk3_0                               3.22.28-1
libguile2.2_1                           2.2.7-1
libharfbuzz0                            2.7.4-1
libhogweed4                             3.4.1-1
libhwloc15                              2.0.3-1
libiconv                                1.16-2
libiconv2                               1.16-2
libicu61                                61.1-1
libidn2_0                               2.2.0-1
libimagequant0                          2.10.0-1
libintl8                                0.19.8.1-2
libisl13                                0.14.1-1
libisl22                                0.22.1-2
libjasper4                              2.0.14-1
libjbig2                                2.1-1
libjpeg8                                2.0.6-1
libjson-glib1.0_0                       1.4.2-1
libjsoncpp24                            1.9.4-1
libk5crypto3                            1.15.2-2
libkrb5_3                               1.15.2-2
libkrb5support0                         1.15.2-2
liblapack0                              3.9.1-1
liblcms2_2                              2.12-1
libllvm8                                8.0.1-1
libltdl7                                2.4.6-7
liblz4_1                                1.7.5-1
liblzma5                                5.2.4-1
liblzo2_2                               2.10-2
libmetalink3                            0.1.2-1
libmetis-devel                          5.1.0-1
libmetis0                               5.1.0-1
libmpc3                                 1.2.1-1
libmpfr4                                3.1.6-1p1
libmpfr6                                4.1.0-1
libncursesw10                           6.1-1.20190727
libnettle6                              3.4.1-1
libnghttp2_14                           1.37.0-1
libopenblas                             0.3.14-1
libOpenCL-devel                         2.2.12-1
libOpenCL1                              2.2.12-1
libopenldap2_4_2                        2.4.58-1
libp11-kit0                             0.23.20-1
libpango1.0_0                           1.40.14-1
libpcre1                                8.44-2
libpcre2_8_0                            10.36-1
libpipeline1                            1.5.3-1
libpixman1_0                            0.40.0-1
libpkgconf3                             1.6.3-1
libpng16                                1.6.37-1
libpocl-common                          1.3-1
libpocl2                                1.3-1
libpolly8                               8.0.1-1
libpopt-common                          1.18-1
libpopt0                                1.18-1
libproxy1                               0.4.14-2
libpsl5                                 0.21.0-1
libquadmath0                            10.2.0-1
libreadline7                            7.0.3-3
librest0.7_0                            0.8.1-1
librhash0                               1.4.0-1
librsvg2_2                              2.40.20-1
libsasl2_3                              2.1.27-1
libsigsegv2                             2.10-2
libslang2                               2.3.2-2
libsmartcols1                           2.33.1-2
libsoup-gnome2.4_1                      2.60.3-1
libsoup2.4_1                            2.60.3-1
libsource-highlight-common              3.1.8-6
libsource-highlight4                    3.1.8-6
libsqlite3_0                            3.34.0-1
libssh2_1                               1.7.0-1
libssl1.0                               1.0.2t-1
libssl1.1                               1.1.1f-1
libstdc++6                              10.2.0-1
libtasn1_6                              4.14-1
libthai0                                0.1.26-1
libtiff6                                4.2.0-1
libunistring2                           0.9.10-1
libunwind-devel                         8.0.1-1
libunwind1                              8.0.1-1
libusb0                                 1.2.6.0-2
libuuid-devel                           2.33.1-2
libuuid1                                2.33.1-2
libuv1                                  1.41.0-1
libwebp7                                0.6.1-2
libwebpdemux2                           0.6.1-2
libwebpmux3                             0.6.1-2
libX11-xcb1                             1.7.0-1
libX11_6                                1.7.0-1
libXau6                                 1.0.9-1
libxcb-glx0                             1.14-1
libxcb-render0                          1.14-1
libxcb-shm0                             1.14-1
libxcb1                                 1.14-1
libXcomposite1                          0.4.5-1
libXcursor1                             1.2.0-1
libXdamage1                             1.1.5-1
libXdmcp6                               1.1.3-1
libXext6                                1.3.4-1
libXfixes3                              5.0.3-1
libXft2                                 2.3.3-1
libXi6                                  1.7.10-1
libXinerama1                            1.1.4-1
libxml2                                 2.9.10-2
libXrandr2                              1.5.2-1
libXrender1                             0.9.10-1
libXtst6                                1.2.3-1
libxxhash0                              0.8.0-1
libzstd1                                1.4.9-1
login                                   1.13-1
m4                                      1.4.18-1
make                                    4.3-1
make-debuginfo                          4.3-1
man-db                                  2.9.4-1
mc                                      4.8.26-1
mingw64-x86_64-boost                    1.66.0-1
mingw64-x86_64-bzip2                    1.0.6-4
mingw64-x86_64-crypt                    2.1-1
mingw64-x86_64-expat                    2.2.2-1
mingw64-x86_64-gcc-core                 10.2.0-1
mingw64-x86_64-gcc-fortran              10.2.0-1
mingw64-x86_64-gcc-g++                  10.2.0-1
mingw64-x86_64-headers                  8.0.0-1
mingw64-x86_64-icu                      57.1-2
mingw64-x86_64-libffi                   3.2.1-2
mingw64-x86_64-libgcrypt                1.8.2-1
mingw64-x86_64-libgnurx                 2.5-3
mingw64-x86_64-libgpg-error             1.28-1
mingw64-x86_64-ncurses                  6.0-12.20171125
mingw64-x86_64-openssl                  1.0.2o-1
mingw64-x86_64-pthreads                 20100619-5
mingw64-x86_64-runtime                  8.0.0-1
mingw64-x86_64-sqlite3                  3.34.0-1
mingw64-x86_64-tcl                      8.6.11-1
mingw64-x86_64-tk                       8.6.11-1
mingw64-x86_64-win-iconv                0.0.6-2
mingw64-x86_64-windows-default-manifest 6.4-1
mingw64-x86_64-winpthreads              8.0.0-1
mingw64-x86_64-xz                       5.2.3-1
mingw64-x86_64-zlib                     1.2.11-1
mintty                                  3.4.7-1
ncurses                                 6.1-1.20190727
openssh                                 8.5p1-1
openssh-debuginfo                       8.5p1-1
openssl                                 1.1.1f-1
p11-kit                                 0.23.20-1
p11-kit-trust                           0.23.20-1
perl                                    5.32.1-1
perl-Error                              0.17029-2
perl-Scalar-List-Utils                  1.56-1
perl-TermReadKey                        2.38-3
perl-Test-Harness                       3.42-3
perl-TimeDate                           2.33-2
perl-Unicode-Normalize                  1.26-3
perl_autorebase                         5.32.1-1
perl_base                               5.32.1-1
pkg-config                              1.6.3-1
pkgconf                                 1.6.3-1
publicsuffix-list-dafsa                 20200326-1
python2                                 2.7.18-4
python27                                2.7.18-4
python27-pip                            20.3.3-2
python27-setuptools                     41.2.0-1
python3                                 3.8.6-1
python3-devel                           3.8.6-1
python3-wx-common                       4.0.7.post2-4
python36                                3.6.13-1
python36-jinja2                         2.11.2-1
python36-markupsafe                     1.1.1-2
python36-pip                            21.0-1
python36-setuptools                     51.1.1-1
python38                                3.8.8-1
python38-alabaster                      0.7.12-2
python38-attrs                          20.3.0-1
python38-babel                          2.9.0-1
python38-backcall                       0.2.0-1
python38-cairo                          1.19.1-1
python38-cffi                           1.14.4-1
python38-chardet                        4.0.0-1
python38-crypto                         2.6.1-3
python38-cryptography                   3.3.1-1
python38-cycler                         0.10.0-1
python38-cython                         0.29.21-3
python38-dateutil                       2.8.1-1
python38-decorator                      4.4.2-1
python38-devel                          3.8.8-1
python38-distlib                        0.3.1-1
python38-docutils                       0.16-1
python38-gi                             3.36.1-1
python38-idna                           3.1-1
python38-imagesize                      1.2.0-1
python38-imaging                        8.1.2-1
python38-ipython                        7.19.0-1
python38-ipython_genutils               0.2.0-2
python38-jinja2                         2.11.2-1
python38-kiwisolver                     1.3.1-1
python38-markupsafe                     1.1.1-2
python38-matplotlib                     3.3.3-1
python38-nose                           1.3.7-3
python38-numpy                          1.19.4-1
python38-olefile                        0.46-2
python38-openssl                        20.0.1-1
python38-packaging                      20.4-1
python38-pexpect                        4.8.0-1
python38-pickleshare                    0.7.5-2
python38-pip                            21.0-1
python38-ply                            3.11-2
python38-prompt_toolkit                 3.0.10-1
python38-ptyprocess                     0.6.0-2
python38-pycparser                      2.20-1
python38-pygments                       2.7.3-1
python38-pyparsing                      2.4.7-1
python38-pytest                         6.2.1-1
python38-pytz                           2020.5-1
python38-requests                       2.25.1-1
python38-setuptools                     51.1.1-1
python38-six                            1.15.0-1
python38-snowballstemmer                2.0.0-1
python38-sphinx                         3.4.3-1
python38-sphinx_rtd_theme               0.4.3-2
python38-sphinxcontrib-applehelp        1.0.2-1
python38-sphinxcontrib-devhelp          1.0.2-1
python38-sphinxcontrib-htmlhelp         1.0.3-1
python38-sphinxcontrib-jsmath           1.0.1-1
python38-sphinxcontrib-qthelp           1.0.3-1
python38-sphinxcontrib-serializinghtml  1.1.4-1
python38-sphinxcontrib-websupport       1.2.4-1
python38-sqlalchemy                     1.3.22-1
python38-testpath                       0.4.4-1
python38-traitlets                      5.0.5-1
python38-urllib3                        1.26.2-1
python38-wcwidth                        0.2.5-1
python38-whoosh                         2.7.4-3
python38-wx                             4.0.7.post2-4
rebase                                  4.4.4-1
rsync                                   3.2.3+20200903+git9f9240b-4
run                                     1.3.4-2
sed                                     4.4-1
shared-mime-info                        2.1-1
tar                                     1.34-1
terminfo                                6.1-1.20190727
terminfo-extra                          6.1-1.20190727
texinfo                                 6.7-1
tzcode                                  2021a-1
tzdata                                  2021a-1
unzip                                   6.0-17
util-linux                              2.33.1-2
vim-minimal                             8.2.0486-1
w32api-headers                          8.0.0-1
w32api-runtime                          8.0.0-1
wget                                    1.21.1-1
which                                   2.20-2
windows-default-manifest                6.4-1
xz                                      5.2.4-1
zlib0                                   1.2.11-1
zstd                                    1.4.9-1
```

You can try following command after `cd` to where you have `setup-x86_64.exe`. Command line does not allow to choose version so for specific versions you may have to select it in the installation wizard.
```
setup-x86_64.exe -nq -P  _autorebase,adwaita-icon-theme,alternatives,at-spi2-core,autoconf,autoconf2.1,autoconf2.5,automake,automake1.10,automake1.11,automake1.12,automake1.13,automake1.14,automake1.15,automake1.16,automake1.9,base-cygwin,base-files,bash,binutils,bzip2,ca-certificates,clang,cmake,cmake-debuginfo,compiler-rt,coreutils,crypto-policies,csih,cygrunsrv,cygutils,cygwin,cygwin-debuginfo,cygwin-devel,cygwin32,cygwin32-binutils,cygwin32-default-manifest,cygwin32-gcc-core,cygwin32-gcc-fortran,cygwin32-gcc-g++,cygwin32-w32api-headers,cygwin32-w32api-runtime,dash,dconf-service,dejavu-fonts,desktop-file-utils,diffutils,dri-drivers,editrights,expat,file,findutils,gamin,gawk,gcc-core,gcc-fortran,gcc-g++,gcovr,gdb,gdk-pixbuf2.0-svg,gendef,getent,girepository-GLib2.0,git,glib2.0-networking,gnupg,grep,groff,gsettings-desktop-schemas,gtk-update-icon-cache,gzip,hicolor-icon-theme,hostname,info,ipc-utils,less,libarchive13,libargp,libassuan0,libatk-bridge2.0_0,libatk1.0_0,libatomic1,libatspi0,libattr1,libblkid1,libboost_regex1.66,libbrotlicommon1,libbrotlidec1,libbz2_1,libc++-devel,libc++1,libc++abi-devel,libc++abi1,libcairo2,libcares2,libcbor,libclang8,libcom_err2,libcroco0.6_3,libcrypt-devel,libcrypt0,libcrypt2,libcurl4,libdatrie1,libdb5.3,libdbus1_3,libdeflate0,libedit0,libepoxy0,libevent2.0_5,libexpat1,libext2fs2,libfam0,libfdisk1,libffi6,libfido2,libfontconfig-common,libfontconfig1,libfreetype6,libgc1,libgcc1,libgdbm6,libgdbm_compat4,libgdk_pixbuf2.0_0,libgfortran4,libgfortran5,libgirepository1.0_1,libGL1,libglapi0,libglib2.0_0,libgmp10,libgnutls30,libgomp1,libgpg-error0,libgpgme11,libgraphite2_3,libgssapi_krb5_2,libgtk3_0,libguile2.2_1,libharfbuzz0,libhogweed4,libhwloc15,libiconv,libiconv2,libicu61,libidn2_0,libimagequant0,libintl8,libisl13,libisl22,libjasper4,libjbig2,libjpeg8,libjson-glib1.0_0,libjsoncpp24,libk5crypto3,libkrb5_3,libkrb5support0,liblapack0,liblcms2_2,libllvm8,libltdl7,liblz4_1,liblzma5,liblzo2_2,libmetalink3,libmetis-devel,libmetis0,libmpc3,libmpfr4,libmpfr6,libncursesw10,libnettle6,libnghttp2_14,libopenblas,libOpenCL-devel,libOpenCL1,libopenldap2_4_2,libp11-kit0,libpango1.0_0,libpcre1,libpcre2_8_0,libpipeline1,libpixman1_0,libpkgconf3,libpng16,libpocl-common,libpocl2,libpolly8,libpopt-common,libpopt0,libproxy1,libpsl5,libquadmath0,libreadline7,librest0.7_0,librhash0,librsvg2_2,libsasl2_3,libsigsegv2,libslang2,libsmartcols1,libsoup-gnome2.4_1,libsoup2.4_1,libsource-highlight-common,libsource-highlight4,libsqlite3_0,libssh2_1,libssl1.0,libssl1.1,libstdc++6,libtasn1_6,libthai0,libtiff6,libunistring2,libunwind-devel,libunwind1,libusb0,libuuid-devel,libuuid1,libuv1,libwebp7,libwebpdemux2,libwebpmux3,libX11-xcb1,libX11_6,libXau6,libxcb-glx0,libxcb-render0,libxcb-shm0,libxcb1,libXcomposite1,libXcursor1,libXdamage1,libXdmcp6,libXext6,libXfixes3,libXft2,libXi6,libXinerama1,libxml2,libXrandr2,libXrender1,libXtst6,libxxhash0,libzstd1,login,m4,make,make-debuginfo,man-db,mc,mingw64-x86_64-boost,mingw64-x86_64-bzip2,mingw64-x86_64-crypt,mingw64-x86_64-expat,mingw64-x86_64-gcc-core,mingw64-x86_64-gcc-fortran,mingw64-x86_64-gcc-g++,mingw64-x86_64-headers,mingw64-x86_64-icu,mingw64-x86_64-libffi,mingw64-x86_64-libgcrypt,mingw64-x86_64-libgnurx,mingw64-x86_64-libgpg-error,mingw64-x86_64-ncurses,mingw64-x86_64-openssl,mingw64-x86_64-pthreads,mingw64-x86_64-runtime,mingw64-x86_64-sqlite3,mingw64-x86_64-tcl,mingw64-x86_64-tk,mingw64-x86_64-win-iconv,mingw64-x86_64-windows-default-manifest,mingw64-x86_64-winpthreads,mingw64-x86_64-xz,mingw64-x86_64-zlib,mintty,ncurses,openssh,openssh-debuginfo,openssl,p11-kit,p11-kit-trust,perl,perl-Error,perl-Scalar-List-Utils,perl-TermReadKey,perl-Test-Harness,perl-TimeDate,perl-Unicode-Normalize,perl_autorebase,perl_base,pkg-config,pkgconf,publicsuffix-list-dafsa,python2,python27,python27-pip,python27-setuptools,python3,python3-devel,python3-wx-common,python36,python36-jinja2,python36-markupsafe,python36-pip,python36-setuptools,python38,python38-alabaster,python38-attrs,python38-babel,python38-backcall,python38-cairo,python38-cffi,python38-chardet,python38-crypto,python38-cryptography,python38-cycler,python38-cython,python38-dateutil,python38-decorator,python38-devel,python38-distlib,python38-docutils,python38-gi,python38-idna,python38-imagesize,python38-imaging,python38-ipython,python38-ipython_genutils,python38-jinja2,python38-kiwisolver,python38-markupsafe,python38-matplotlib,python38-nose,python38-numpy,python38-olefile,python38-openssl,python38-packaging,python38-pexpect,python38-pickleshare,python38-pip,python38-ply,python38-prompt_toolkit,python38-ptyprocess,python38-pycparser,python38-pygments,python38-pyparsing,python38-pytest,python38-pytz,python38-requests,python38-setuptools,python38-six,python38-snowballstemmer,python38-sphinx,python38-sphinx_rtd_theme,python38-sphinxcontrib-applehelp,python38-sphinxcontrib-devhelp,python38-sphinxcontrib-htmlhelp,python38-sphinxcontrib-jsmath,python38-sphinxcontrib-qthelp,python38-sphinxcontrib-serializinghtml,python38-sphinxcontrib-websupport,python38-sqlalchemy,python38-testpath,python38-traitlets,python38-urllib3,python38-wcwidth,python38-whoosh,python38-wx,rebase,rsync,run,sed,shared-mime-info,tar,terminfo,terminfo-extra,texinfo,tzcode,tzdata,unzip,util-linux,vim-minimal,w32api-headers,w32api-runtime,wget,which,windows-default-manifest,xz,zlib0,zstd,

```


### Libraries needed & have to be built : we will be building in this order
    OpenBLAS    : git clone https://github.com/xianyi/OpenBLAS.git
    LAPACK      : git clone https://github.com/Reference-LAPACK/lapack.git
    MUMPS       : git clone https://bitbucket.org/petsc/pkg-mumps.git
    PETSc       : git clone -b release https://gitlab.com/petsc/petsc.git petsc
    gmsh        : git clone http://gitlab.onelab.info/gmsh/gmsh.git
    GetDP       : git clone https://gitlab.onelab.info/getdp/getdp.git

### *** PART I ***

    cd /bin
    cp gcc.exe gcc.exe-old
    cp g++.exe g++.exe-old
    cp gfortran.exe gfortran.exe-old
    rm gfortran.exe 
    rm gcc.exe
    rm g++.exe
    rm gfortran
    rm gcc
    rm g++
    cp x86_64-pc-cygwin-gcc.exe gcc.exe
    cp x86_64-pc-cygwin-g++.exe g++.exe

    cd /usr/bin
    cp gcc.exe gcc.exe-old
    cp g++.exe g++.exe-old
    cp gfortran.exe gfortran.exe-old
    rm gcc.exe
    rm g++.exe
    rm gfortran.exe 
    rm gfortran
    rm gcc
    rm g++
    cp x86_64-pc-cygwin-gcc.exe gcc.exe
    cp x86_64-pc-cygwin-g++.exe g++.exe



    Edit Your ~/.bash_profile file.
    Add these lines to the end of file:

    export PATH="${PATH}:/usr/x86_64-w64-mingw32/sys-root/mingw/bin:/usr/local/bin:"
    export CXX=/bin/x86_64-w64-mingw32-g++
    export CC=/bin/x86_64-w64-mingw32-gcc
    export FC=/bin/x86_64-w64-mingw32-gfortran

    export PETSC_DIR=/home/User/petsc
    export PETSC_ARCH=complex_mumps_seq
    
    
    *** MAKE SURE PETSC LOCATION IS CORRECT ***
    *** RESTART CYGWIN TERMINAL/CONSOLE *** 


If an absolute path error occurs, specify the PETSC_DIR with /cygdrive in front.

    

### 1. OpenBLAS ###
    Makefile.rule
            VERSION = 0.0.7.CENOS--12-08-2021
            
            TARGET = CORE2
            DYNAMIC_ARCH = 1
            DYNAMIC_OLDER = 1

            # cross compiler for Windows
             CC = x86_64-w64-mingw32-gcc
             FC = x86_64-w64-mingw32-gfortran
             HOSTCC = gcc

             BINARY=64
             USE_THREAD = 1
             NUM_THREADS = 64
             INTERFACE64 = 0
             NO_AFFINITY = 1

    make CFLAGS='-O3 -static -static-libgcc -static-libstdc++ -static-libgfortran' FFLAGS='-O3 -static -static-libgfortran'

    make PREFIX=/usr/local install
    manually copy /usr/local/bin/libopenblas.dll to /usr/local/lib/

### 2. lapack ###
    CMakeLists.txt
            set(CMAKE_BUILD_TYPE Release)
            set(CMAKE_EXE_LINKER_FLAGS "-static")

    mkdir build
    cd build
    cmake -DBUILD_SHARED_LIBS=OFF ..
    cmake --build . --target install

Set the variables somewhere between version variables and first mention of build type (first few lines).

### 3. mumps ### 
    Makefile.inc
            OPTC    =-static -static-libgcc -static-libstdc++ -fno-stack-protector -O3 -fopenmp
            OPTF    =-static -static-libgcc -static-libstdc++ -ffree-line-length-0 -O3 -fopenmp -fallow-argument-mismatch
            OPTL    =-static-libgcc -static-libstdc++ -fopenmp -Wall -Wwrite-strings -Wno-strict-aliasing -Wno-unknown-pragmas -fno-stack-protector -fvisibility=hidden

    make clean; make CFLAGS='-static -static-libgcc -static-libstdc++ -O3' FFLAGS='-static' all

As a result, lib folder should contain several lib*.a files

### Contents of Makefile.inc
    *** DO NOT COPY, JUST ADJUST FLAGS ***

    LPORDDIR   = $(topdir)/PORD/lib/
    IPORD      = -I$(topdir)/PORD/include/
    LPORD      = -L$(LPORDDIR) -lpord
    PLAT       =
    ORDERINGSC = -Dpord
    ORDERINGSF = -Dpord
    LORDERINGS  = $(LPARMETIS) $(LMETIS) $(LPORD) $(LSCOTCH)
    IORDERINGSC = $(IPARMETIS) $(IMETIS) $(IPORD) $(ISCOTCH)
    IORDERINGSF = $(ISCOTCH)
    RM = /bin/rm -f
    CC = x86_64-w64-mingw32-gcc
    OPTC    = -static -static-libgcc -static-libstdc++ -fno-stack-protector -O3 -fopenmp
    OUTC = -o
    FC = x86_64-w64-mingw32-gfortran
    FL = x86_64-w64-mingw32-gfortran
    OPTF    =-static -static-libgcc -static-libstdc++ -ffree-line-length-0 -O3 -fopenmp -fallow-argument-mismatch
    OPTF   += -DBLR_MT
    OUTF = -o
    CDEFS   = -DAdd_
    AR      = /usr/bin/ar cr 
    LIBEXT  = .a
    RANLIB  = /usr/bin/ranlib
    SCALAP  =
    INCPAR  = -I../libseq
    INCSEQ  = -I$(topdir)/libseq
    LIBSEQ  =  $(LAPACK) -L$(topdir)/libseq -lmpiseq
    LIBBLAS = -Wl,-rpath,/usr/local/lib -L/usr/local/lib -llapack -Wl,-rpath,/usr/local/lib -L/usr/local/lib -lopenblas -lstdc++    -lgfortran -L/usr/lib/gcc/x86_64-w64-mingw32/10 -L/usr/x86_64-w64-mingw32/lib -L/usr/x86_64-w64-mingw32/sys-root/mingw/lib -lgfortran  -lmingw32 -lgcc_s -lmoldname -lmingwex -lmsvcrt -lquadmath -lm -lpthread -ladvapi32 -lshell32 -luser32
    OPTL    = -static-libgcc -static-libstdc++ -fopenmp -Wall -Wwrite-strings -Wno-strict-aliasing -Wno-unknown-pragmas -fno-stack-protecto
    r -fvisibility=hidden -O3
    INCS = $(INCPAR)
    LIBS = $(LIBPAR)
    LIBSEQNEEDED = libseqneeded
    LIBS = $(LIBSEQ)

from Make.inc directory, copy Makefile.inc.generic to root directory and delete the .generic extension. Then replace its contents with the commands above.

### *** PART II ***
### 4. PETSc ###

    **In case You are rebuilding** : make a fresh PETSc clone :
    
    git clone -b release https://gitlab.com/petsc/petsc.git petsc

    cd /usr/local/lib
    ln -s ~/mumps/libseq/libmpiseq.a libmpiseq.a

    First, lets make sure we can compile sowing :
`./configure CC=$CC CXX=$CXX FC=$FC --with-debugging=0 --with-mpi=0 --with-mpiuni-fortran-binding=0 --download-mumps=yes --with-mumps-serial --with-shared-libraries=0 --with-x=0 --with-ssl=0 --with-scalar-type=complex --with-openmp=1`

    After PETSc configure has successfully compiled and installed sowing, also downloaded and installed mumps :

 `./configure CC=$CC CXX=$CXX FC=$FC COPTFLAGS='-O3 -static -static-libgcc -static-libstdc++' CXXOPTFLAGS='-O3 -static -static-libgcc -static-libstdc++' FOPTFLAGS='-O3 -static -static-libgfortran' --with-debugging=0 --with-mpi=0 --with-mpiuni-fortran-binding=0 --with-mumps-serial --with-mumps-dir=/home/user/mumps/ --with-shared-libraries=0 --with-x=0 --with-ssl=0 --with-scalar-type=complex --with-openmp=1 --with-blas-lib=/usr/local/lib/libopenblas.a --with-lapack-lib=/usr/local/lib/liblapack.a`

Make sure --with-mumps-dir contains path to mumps. Can add /cygdrive in front if something doesn't work. 

    This should produce petsc config.

    make clean; make -j 1 -l PETSC_DIR=/home/user/getdp/petsc PETSC_ARCH=complex_mumps_seq all

Make sure the path to getdp source in PETSC_DIR is correct.

### *** PART III ***
### 5. gmsh ###
    CMakeLists.txt
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_EXE_LINKER_FLAGS "-static-libgfortran -lgfortran")
    

    mkdir build
    cd build

    cmake -DDEFAULT=0 -DENABLE_FLTK=0 -DENABLE_PLUGINS=1 -DENABLE_ALGLIB=1 -DENABLE_EIGEN=0 -DENABLE_PARSER=1 -DENABLE_POST=1 -DENABLE_ANN=1 -DENABLE_BLAS_LAPACK=1 -DENABLE_BUILD_LIB=1 -DENABLE_OPENMP=1 -DENABLE_PRIVATE_API=1 ..

    Edit file : build/CMakeFiles/gmsh.dir/link.txt => find & change libopenblas.a to libopenblas.dll.a (this is for gmsh.exe build, without it script will not install headers !!!)

    make clean; make
    make install

    cygcheck ./gmsh.exe -> make sure it depends only on libopenblas.dll for cygwin, if You are planning using it, copy libopenblas.dll with it !
    Ofcourse, make sure it executes OK on cygwin.


### 6. GetDP ###
    CMakeLists.txt
    set(BLAS_LAPACK_LIBRARIES "/usr/local/lib/libopenblas.a")
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
    set(BUILD_SHARED_LIBS OFF)
    set(CMAKE_BUILD_TYPE Release)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgfortran")

    ...
    
    opt(PYTHON "Enable Python functions" OFF)


    mkdir build
    cd build
    

    cmake -DENABLE_MPI=0 -DENABLE_BLAS_LAPACK=1 -DENABLE_OPENMP=1 -DENABLE_PETSC=1 -DBUILD_SHARED_LIBS=OFF ..
    make clean; make CFLAGS='-static -static-libgcc -static-libstdc++ -O3' FFLAGS='-static'

    **** Most likely You will have to edit python related config files.
    **** Edit /usr/include/python3.8/pyconfig.h
    **** Depending on error message, comment out corresponding code line in pyconfig.h :
    **** For example, 
    **** On `sys/select.h` error : find and comment out `#define HAVE_SYS_SELECT_H 0` line
    **** On `sys/termio.h` error : find and comment out `#define HAVE_SYS_TERMIO_H 1` line
    **** Etc until compile finishes OK.

    *** In case it will not link (undefined reference to `quadmath_snprintf' ... etc):
    *** Edit file : build/CMakeFiles/getdp.dir/link.txt => make sure there is -lgfortran at the end of the 
    line and append -lquadmath after it (` -lquadmath`) so that ` -lgfortran -lquadmath` are last two flags
    in the line.

    Make sure getdp.exe executes fine under cygwin.
    Also make sure after You run `./getdp.exe -info` it shows PetsC compiled in !
    Run cygcheck ./getdp.exe to make sure there are no external, cygwin related dependencies left.


    As of 06.09.2021, this guide should produce statically linked, multithreaded getdp.exe.
    
    I also have a strong vision that in a 6 to 12 months time from now, this guide (flags !!!) must 
    be updated according to current library updates. This guide was created in a timeframe when 
    some of libraries were not-really up to date with others because of silent changes/etc, hence the 
    odd looking flags etc.
