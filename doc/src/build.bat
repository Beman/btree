@echo off
rem Copyright Beman Dawes 2013
rem Distributed under the Boost Software License, Version 1.0.

echo Extracting code snippets from example .cpp files

mmp int_set.cpp.extract int_set.cpp.html
call htmlize int_set.cpp.html

echo Generating tutorial.html...
del tutorial.html 2>nul

rem ' dir="ltr"' is an artifact of editing mistakes.
chg tutorial_source.html " dir=\qltr\q" ""

mmp tutorial_source.html tutorial.html
type \bgd\util\crnl.txt
tutorial.html

echo Run "hoist" to hoist tutorial.html to the parent directory
