@echo off

C:
chdir C:\cygwin\bin

bash --login -c "cd ${HOME}/src/getdp/utils/nightly && ctest -S getdp_win32_nightly.ctest -VV > ${HOME}/src/getdp/nightly.log 2>&1"
