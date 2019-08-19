@echo off
set "l_sln=.\build\Notepad2.sln"
if exist "%l_sln%" (
    start %l_sln%
) else for /F %%a in ("%l_sln%") do echo:File "%%~nxA" could not be found.&pause
set l_sln=