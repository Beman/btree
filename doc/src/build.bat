@echo off
rem Copyright Beman Dawes 2013
rem Distributed under the Boost Software License, Version 1.0.

echo Extracting code snippets from example .cpp files

mmp int_set.cpp.extract int_set.cpp.html
call htmlize int_set.cpp.html

mmp bulk_load_map.cpp.extract bulk_load_map.cpp.html
call htmlize bulk_load_map.cpp.html

mmp endian_map.cpp.extract endian_map.cpp.html
call htmlize endian_map.cpp.html

mmp hetero_set.cpp.extract hetero_set.cpp.html
call htmlize hetero_set.cpp.html

mmp int_map.cpp.extract int_map.cpp.html
call htmlize int_map.cpp.html

mmp int_map_first_try.cpp.extract int_map_first_try.cpp.html
call htmlize int_map_first_try.cpp.html

mmp int_set_read.cpp.extract int_set_read.cpp.html
call htmlize int_set_read.cpp.html

mmp int_set_read_first_try.cpp.extract int_set_read_first_try.cpp.html
call htmlize int_set_read_first_try.cpp.html

mmp pack_map.cpp.extract pack_map.cpp.html
call htmlize pack_map.cpp.html

mmp string_index_map.cpp.extract string_index_map.cpp.html
call htmlize string_index_map.cpp.html

mmp string_index_set.cpp.extract string_index_set.cpp.html
call htmlize string_index_set.cpp.html

mmp string_map.cpp.extract string_map.cpp.html
call htmlize string_map.cpp.html

mmp string_set.cpp.extract string_set.cpp.html
call htmlize string_set.cpp.html

mmp string_set_first_try.cpp.extract string_set_first_try.cpp.html
call htmlize string_set_first_try.cpp.html

mmp udt_3_index_set.cpp.extract udt_3_index_set.cpp.html
call htmlize udt_3_index_set.cpp.html

mmp udt_index_set.cpp.extract udt_index_set.cpp.html
call htmlize udt_index_set.cpp.html

mmp tune_int_map.cpp.extract tune_int_map.cpp.html
call htmlize tune_int_map.cpp.html

echo Generating tutorial.html...
del tutorial.html 2>nul

rem ' dir="ltr"' is an artifact of editing mistakes.
chg tutorial_source.html " dir=\qltr\q" ""

mmp tutorial_source.html ..\tutorial.html
..\tutorial.html
