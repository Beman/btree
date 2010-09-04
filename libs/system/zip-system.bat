echo create zip file...

if $%1 == $ goto error

rmdir /s \tmp\%1 2>nul
pushd .
mkdir \tmp\%1
cd \tmp\%1
md boost\system
md libs\utility\doc
md libs\utility\test
md libs\system\build
md libs\system\doc
md libs\system\src
md libs\system\test
popd
copy ..\..\boost\cerrno.hpp \tmp\%1\boost
copy ..\..\boost\identifier.hpp \tmp\%1\boost

copy ..\..\boost\system\config.hpp \tmp\%1\boost\system
copy ..\..\boost\system\error_code.hpp \tmp\%1\boost\system
copy ..\..\boost\system\system_error.hpp \tmp\%1\boost\system

copy ..\..\libs\utility\doc\identifier.html \tmp\%1\libs\utility\doc
copy ..\..\libs\utility\test\Jamfile \tmp\%1\libs\utility\test
copy ..\..\libs\utility\test\identifier_test.cpp \tmp\%1\libs\utility\test

copy ..\..\libs\system\build\Jamfile \tmp\%1\libs\system\build
copy ..\..\libs\system\doc\error_code.html \tmp\%1\libs\system\doc
copy ..\..\libs\system\doc\system_error.html \tmp\%1\libs\system\doc
copy ..\..\libs\system\src\error_code.cpp \tmp\%1\libs\system\src
copy ..\..\libs\system\test\Jamfile \tmp\%1\libs\system\test
copy ..\..\libs\system\test\error_code_test.cpp \tmp\%1\libs\system\test
copy ..\..\libs\system\test\error_code_user_test.cpp \tmp\%1\libs\system\test
copy ..\..\libs\system\test\system_error_test.cpp \tmp\%1\libs\system\test

pushd \tmp
zip -r %1.zip %1
popd
move \tmp\%1.zip .

goto done

:error
echo usage: zip-system version
echo version will be used for both the .zip name and the highest level directory name

:done