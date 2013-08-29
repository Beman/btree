@echo off
chg %1 "<" "&lt;"
chg %1 ">" "&gt;"
chg %1 "\q" "&quot;"
chg %1 "'" "&#39;"
chg %1 "\r\n///" ""
