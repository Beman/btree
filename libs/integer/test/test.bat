@echo off
echo Special version of boost_test for sandbox version of endian library.
xcopy /D %BOOST_TRUNK%\boost-build.jam ..\..\..
xcopy /D %BOOST_TRUNK%\Jamroot ..\..\..
set BOOST_BUILD_PATH=%BOOST_TRUNK%\tools\build\v2

if not $%1==$--help goto nohelp
echo Invoke: boost_test [-ts toolset] [bjam-options]
echo Default -ts is gcc-4.3,msvc-8.0,msvc-9.0express,msvc-10.0express
goto done
:nohelp
 
if $%1==$-ts goto toolset

echo Begin test processing...
bjam include=%BOOST_TRUNK% --v2 --dump-tests --toolset=gcc-4.3,msvc-8.0,msvc-9.0express,msvc-10.0express %* >bjam.log 2>&1
goto jam_log

:toolset
echo Begin test processing...
bjam include=%BOOST_TRUNK% --v2 --dump-tests --toolset=%2 %3 %4 %5 %6 %7 %8 %9 >bjam.log 2>&1

:jam_log
echo Begin log processing...
process_jam_log --v2 <bjam.log
start bjam.log

echo Begin compiler status processing...
call boost_relative_root
rem compiler_status barfs on a relative root, so convert it to absolute
dir %BOOST_RELATIVE_ROOT% | grep " Directory of " >%TEMP%\babsr.bat
%UTIL%\change %TEMP%\babsr.bat " Directory of " "set BOOST_TEST_ABS_ROOT=" >nul
%UTIL%\change %TEMP%\babsr.bat "C:" "c:" >nul
%UTIL%\change %TEMP%\babsr.bat "D:" "d:" >nul
%UTIL%\change %TEMP%\babsr.bat "E:" "e:" >nul
%UTIL%\change %TEMP%\babsr.bat "F:" "f:" >nul
call %TEMP%\babsr.bat
compiler_status --v2 %BOOST_TEST_ABS_ROOT% test_status.html test_links.html
start test_status.html

:done
