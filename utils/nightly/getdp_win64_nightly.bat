@echo off

C:
chdir C:\cygwin\bin

bash --login -c "cd ${HOME}/src/getdp/utils/nightly && ctest -S getdp_win64_nightly.ctest"
