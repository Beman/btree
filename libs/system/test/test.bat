set BOOST_ROOT=c:\bgd\boost
set BOOST_AUX_ROOT=c:\boost\site
set BOOST_BUILD_PATH=c:\boost\site\tools/build/v1

set SYSTEM_LOCATE_ROOT=f:\system-regr
mkdir %SYSTEM_LOCATE_ROOT% 2>nul
echo Begin test processing...
bjam --dump-tests "-sALL_LOCATE_TARGET=%SYSTEM_LOCATE_ROOT%" %* >bjam.log 2>&1
echo Begin log processing...
process_jam_log %SYSTEM_LOCATE_ROOT% <bjam.log
start bjam.log
echo Begin compiler status processing...
compiler_status --locate-root %SYSTEM_LOCATE_ROOT% %BOOST_ROOT% test_status.html test_links.html
start test_status.html
