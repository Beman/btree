@echo off
rem Copyright Beman Dawes 2013
rem Distributed under the Boost Software License, Version 1.0.
chg %1 "<" "&lt;"
chg %1 ">" "&gt;"
chg %1 "\q" "&quot;"
chg %1 "'" "&#39;"
chg %1 "\r\n///" ""
