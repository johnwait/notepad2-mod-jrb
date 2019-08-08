@ECHO OFF
SETLOCAL

PUSHD "%~dp0"


IF EXIST "build.user.bat" (CALL "build.user.bat")

SET "l_path_bkup=%PATH%"  & REM MinGW's find command causes a conflict later on
IF EXIST "%MSYS%\bin" SET PATH=%MSYS%\bin;%PATH%


SET FOUND=
FOR %%G IN (bash.exe) DO (SET FOUND=%%~$PATH:G)
IF NOT DEFINED FOUND GOTO MissingVar


START /B /WAIT bash.exe ./version.sh


:: Added for JRB_BUILD, with the Onigmo regex engine
:: (assuming containing folder is <root>\build\)
SET PATH=%l_path_bkup%
SET l_scint_ver=
SET l_onigmo_ver=
SET l_padd=000
SET l_dot_padd=0.0.0
IF EXIST ".\scintilla\version.txt" FOR /F "tokens=* delims=" %%a IN ('type .\scintilla\version.txt') DO SET "l_scint_ver=%%a"
IF EXIST ".\onigmo\version.txt" FOR /F "tokens=* delims=" %%a IN ('type .\onigmo\version.txt') DO SET "l_onigmo_ver=%%a"
SET l_scint_ver_len=
SET l_onigmo_ver_len=
CALL :StrLen "%l_scint_ver%" l_scint_ver_len
CALL :StrLen "%l_onigmo_ver%" l_onigmo_ver_len
IF %l_scint_ver_len% GTR 5 SET l_scint_ver=&SET "l_scint_ver_len=0"     & REM Format expected: %l_scint_ver% == M[m]m[R]R
IF %l_onigmo_ver_len% GTR 5 SET l_onigmo_ver=&SET "l_onigmo_ver_len=0"  & REM Format expected: %l_onigmo_ver% == M.m.R
:: Patching Scintilla's version string if necessary
IF %l_scint_ver_len% GTR 0 IF %l_scint_ver_len% LSS 3 (
    CALL SET ""l_scint_ver=%l_scint_ver%%%l_padd:~%l_scint_ver_len%%%"
    SET l_scint_ver_len=3
)
:: Changing Scintilla's version string to a dotted format
IF %l_scint_ver_len% GTR 0 (
    IF %l_scint_ver_len% EQU 5 IF "%l_scint_ver:~1,1%"=="1" IF "%l_scint_ver:~3,1%"=="1" SET "l_scint_ver=%l_scint_ver:~0,1%.%l_scint_ver:~1,2%.%l_scint_ver:~3,2%"
    IF %l_scint_ver_len% EQU 4 IF "%l_scint_ver:~1,1%"=="1" SET "l_scint_ver=%l_scint_ver:~0,1%.%l_scint_ver:~1,2%.%l_scint_ver:~3,1%"
    IF %l_scint_ver_len% EQU 4 IF "%l_scint_ver:~2,1%"=="1" SET "l_scint_ver=%l_scint_ver:~0,1%.%l_scint_ver:~1,1%.%l_scint_ver:~2,2%"
    IF %l_scint_ver_len% EQU 3 SET "l_scint_ver=%l_scint_ver:~0,1%.%l_scint_ver:~1,1%.%l_scint_ver:~2,1%"
)
SET l_scint_ver_len=
SET l_padd=
:: Patching Onigmo's version string if necessary
IF %l_onigmo_ver_len% GTR 0 IF %l_onigmo_ver_len% LSS 5 (
    CALL SET "l_onigmo_ver=%l_onigmo_ver%%%l_dot_padd:~%l_onigmo_ver_len%%%"
)
SET l_onigmo_ver_len=
SET l_dot_padd=
IF "%l_scint_ver%" EQU "" set "l_scint_ver=^<unknown^>"
IF "%l_onigmo_ver%" EQU "" set "l_onigmo_ver=^<unknown^>"
:: Updating .\src\VersionRev.h
SET "l_ver_rev_hdr=.\src\VersionRev.h" &SET "l_ver_rev_hdr_next=.\src\VersionRev_next.h"
IF EXIST "%l_ver_rev_hdr_next%" DEL /F /Q "%l_ver_rev_hdr_next%">NUL 2>&1
IF EXIST "%l_ver_rev_hdr_next%" GOTO END                                & REM Skip if we cannot delete intermediate file
<NUL SET /p=>"%l_ver_rev_hdr_next%"                                     & REM Create as empty file
FOR /F "tokens=* delims=" %%A IN ('TYPE .\src\VersionRev.h ^| FIND /V "VERSION_SCINTILLA" ^| FIND /V "VERSION_ONIGMO"') DO ECHO:%%A>>"%l_ver_rev_hdr_next%"
ECHO:>>"%l_ver_rev_hdr_next%"
ECHO:#define VERSION_SCINTILLA _T("%l_scint_ver%")>>"%l_ver_rev_hdr_next%"
ECHO:#define VERSION_ONIGMO    _T("%l_onigmo_ver%")>>"%l_ver_rev_hdr_next%"
IF EXIST "%l_ver_rev_hdr%" DEL /F /Q "%l_ver_rev_hdr%">NUL 2>&1
IF EXIST "%l_ver_rev_hdr%" REN "%l_ver_rev_hdr%.to_delete">NUL 2>&1
IF EXIST "%l_ver_rev_hdr%" GOTO END                                     & REM Skip if we cannot replace header file
FOR /F %%A IN ("%l_ver_rev_hdr%") DO REN "%l_ver_rev_hdr_next%" "%%~nxA">NUL 2>&1


:END
POPD
ENDLOCAL
EXIT /B


:MissingVar
COLOR 0C
TITLE ERROR
ECHO MSYS (bash.exe) wasn't found. Create a file named "build.user.bat"
ECHO under Notepad2-mod's source root containing a line like:
ECHO.
ECHO   SET "MSYS=C:\MinGW\msys\1.0"
ECHO or wherever your MSYS installation happens to be.
ECHO. & ECHO.
ECHO Press any key to exit...
PAUSE >NUL
ENDLOCAL
EXIT /B  & REM :MissingVar


:StrLen
SETLOCAL EnableDelayedExpansion
:: Syntax:  CALL :StrLen String [RtnVar]
::             -- String  The string to be measured, surround in quotes if it contains spaces.
::             -- RtnVar  An optional variable to be used to return the string length.
:: Credits: Dave Benham and others from the DosTips forum (strLen7 version),
::          via SS64.com (https://ss64.com/nt/syntax-strlen.html)
::
SET "l_str=#%~1"  & REM Let's work with 1-based indices
SET "l_len=0"
FOR %%N IN (4096 2048 1024 512 256 128 64 32 16 8 4 2 1) DO (
    IF "!l_str:~%%N,1!" NEQ "" (
        SET /A "l_len+=%%N"
        SET "l_str=!l_str:~%%N!"
     )
)
ENDLOCAL&IF "%~2" NEQ "" (SET "%~2=%l_len%") ELSE ECHO:%l_len%
EXIT /B  & REM :StrLen