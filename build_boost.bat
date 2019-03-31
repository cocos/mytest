@echo off
if [%1] == [] (set mode=32) else (set mode=%1)
cd ext\boost
call bootstrap.bat
b2 headers
b2 address-model=%mode% link=static runtime-link=static stage --with-system --with-date_time --with-regex
cd ..\..
