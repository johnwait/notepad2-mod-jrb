@ECHO OFF
rem ******************************************************************************
rem *
rem * Notepad2-mod
rem *
rem * build_vs2017.bat
rem *   Batch file used to build Notepad2 with MSVC2017
rem *
rem * See License.txt for details about distribution and modification.
rem *
rem *                                     (c) XhmikosR 2010-2015, 2017
rem *                                     https://github.com/XhmikosR/notepad2-mod
rem *
rem ******************************************************************************

SETLOCAL ENABLEEXTENSIONS ENABLEDELAYEDEXPANSION

CD /D "%~dp0"

SET "DEFAULT_BUILDTYPE=Build"     & REM One of { Build, Clean, Rebuild }
SET "DEFAULT_ARCH=all"            & REM One of { x86, x64, all }
SET "DEFAULT_CONFIG=Release"      & REM One of { Debug, Release, all }

rem Check for the help switches
IF /I "%~1" == "help"   GOTO SHOWHELP
IF /I "%~1" == "/help"  GOTO SHOWHELP
IF /I "%~1" == "-help"  GOTO SHOWHELP
IF /I "%~1" == "--help" GOTO SHOWHELP
IF /I "%~1" == "/?"     GOTO SHOWHELP

rem 2019-08-04: Make sure env variable VS_PATH is set
IF NOT "%VS_PATH%"=="" GOTO VSPATHDEFINED
rem VS_PATH not defined; we'll need to check/search for file "vsdevcmd.bat"
SET "l_vspath_save=.\vs.path"
SET l_vscmd_file=vsdevcmd.bat
SET l_vscmd_subpath=\Common7\Tools
rem Check if we saved a previously-chosem path to a separate file
IF NOT EXIST "%l_vspath_save%" GOTO FINDVSPATHS
rem Load as path the file's content and then test it
FOR /F "tokens=* delims=" %%a IN (%l_vspath_save%) DO SET "VS_PATH=%%a"
IF EXIST "%VS_PATH%%l_vscmd_subpath%\%l_vscmd_file%" GOTO VSPATHFOUND

:FINDVSPATHS
ECHO:WARNING: Environment variable VS_PATH not defined.
ECHO:         Searching %SystemDrive% for possible Visual Studio paths...
ECHO:
PUSHD %SystemDrive%\
SET l_vspath_count=
FOR /F "tokens=2 delims=:" %%a IN ('DIR /s %l_vscmd_file% ^| FIND "%l_vscmd_subpath%"') DO CALL :VSPATHCANDIDATE "%SystemDrive%%%a"
POPD
IF "%l_vspath_count%"=="1" (
    ECHO:Search ended with a single match.
    ECHO:
    SET "l_vspath=%l_vspath1%"
    GOTO FINALIZEVSPATH
)
:MULTIVSPATHS
rem Show the list of paths to choose from
SET l_enum_firstitem=1
SET l_enum_start=%l_enum_firstitem%
SET l_menu_pagesize=9
:VSPATHSELECT
SET /A l_enum_end=%l_enum_start%+%l_menu_pagesize%-1
IF %l_enum_end% GTR %l_vspath_count% SET l_enum_end=%l_vspath_count%
CLS
ECHO:More than one candidate for %%VS_PATH%% have been found. Select below
ECHO:the one to use for building the project:
ECHO:
SET l_menu_idx=1
SET l_menu_count=0
SET l_choices=
set l_choices_verbose=
If %l_enum_start% GTR %l_enum_firstitem% (
    SET "l_choices=P"
    SET "l_choices_verbose=P,"
    ECHO:  P^) ^<^< Show previous entries
    SET /A l_menu_count=%l_menu_count%+1
)
FOR /L %%i IN (%l_enum_start%,1,%l_enum_end%) DO (
    ECHO:  !l_menu_idx!^) "!l_vspath%%i!"
    SET "l_choices=!l_choices!!l_menu_idx!"
    SET "l_choices_verbose=!l_choices_verbose!!l_menu_idx!,"
    SET /A l_menu_idx=!l_menu_idx!+1
    SET /A l_menu_count=!l_menu_count!+1
)
IF "%l_choices:~0,1%"=="P" SET l_choices=%l_choices:~1%%l_choices:~0,1%
IF %l_enum_end% LSS %l_vspath_count% (
    SET "l_choices=%l_choices%N"
    SET "l_choices_verbose=%l_choices_verbose%N,"
    ECHO:  N^) Show next entries  ^>^>
    SET /A l_menu_count=%l_menu_count%+1
)
:: 2019-08-04: Added Cancel option
IF 1 GTR 0 (
    SET "l_choices=%l_choices%C"
    SET "l_choices_verbose=%l_choices_verbose%C"
    ECHO:
    ECHO:  C^) Cancel (abort script^)
    SET /A l_menu_count=%l_menu_count%+1
)
ECHO:
CHOICE /C %l_choices% /N /M "VS Path to use for builing [%l_choices_verbose%]: "
SET l_zbidx=
FOR /l %%c IN (%l_menu_count%,-1,1) DO IF "!l_zbidx!"=="" IF ERRORLEVEL %%c (
    SET /A l_zbidx=%%c-1
)
CALL SET "l_choice=%%l_choices:~%l_zbidx%,1%%"
IF "%l_choice%"=="C" CALL :SUBMSG "ERROR" "Script aborted!"
IF "%l_choice%"=="P" GOTO MENU_PREV
IF "%l_choice%"=="N" GOTO MENU_NEXT
:: Use chosen value
SET /A l_idx=%l_enum_start%+%l_choice%-1
CALL SET "l_vspath=%%l_vspath%l_idx%%%"
ECHO:
GOTO FINALIZEVSPATH
:MENU_PREV
SET /A l_enum_start=%l_enum_start%-%l_menu_pagesize%
IF !l_enum_start! LSS %l_enum_firstitem% SET l_enum_start=%l_enum_firstitem%
GOTO VSPATHSELECT
:MENU_NEXT
SET /A l_enum_start=%l_enum_start%+%l_menu_pagesize%
IF !l_enum_start! GTR %l_vspath_count% SET /A l_enum_start=%l_vspath_count%-%l_menu_pagesize%+1
GOTO VSPATHSELECT

:FINALIZEVSPATH
rem Finalize VS_PATH value
PUSHD "%l_vspath%"
SET "l_subpath=%l_vscmd_subpath%"
:FVSP_LOOP
FOR /F "tokens=1* delims=\" %%a IN ("%l_subpath%") DO (
   CD ..
   SET l_subpath=%%b
)
IF DEFINED l_subpath GOTO :FVSP_LOOP
SET "VS_PATH=%CD%"
POPD
GOTO VSPATHFOUND

:VSPATHCANDIDATE
IF NOT EXIST "%~1\%l_vscmd_file%" EXIT /B
IF "%l_vspath_count%"=="" (
    SET l_vspath_count=1
) ELSE (
    SET /A l_vspath_count=%l_vspath_count%+1
)
SET "l_vspath%l_vspath_count%=%~1"
EXIT /B

:VSPATHFOUND
rem Save path for future callsm just in case
IF NOT EXIST "%l_vspath_save%" <nul set /p=%VS_PATH%>"%l_vspath_save%"
ECHO:Setting
ECHO:  VS_PATH=%VS_PATH%
ECHO:and continuing with build steps...
rem Cleanup after ourselves
SET l_choice=
SET l_choices=
SET l_choices_verbose=
SET l_menu_count=
SET l_menu_idx=
SET l_menu_pagesize=
SET l_enum_end=
SET l_enum_start=
SET l_enum_firstitem=
SET l_idx=
SET l_subpath=
SET l_vscmd_file=
SET l_vscmd_subpath=
SET l_vspath=
FOR /L %%i IN (1,1,%l_vspath_count%) DO SET l_vspath%%i=
SET l_vspath_count=
SET l_zbidx=
:VSPATHDEFINED


rem Check for the first switch
IF "%DEFAULT_BUILDTYPE%"=="" SET "DEFAULT_BUILDTYPE=Build"
IF "%~1" == "" (
  SET "BUILDTYPE=%DEFAULT_BUILDTYPE%"
) ELSE (
  IF /I "%~1" == "Build"     SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Build"    SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Build"    SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Build"   SET "BUILDTYPE=Build"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "Clean"     SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Clean"    SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Clean"    SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Clean"   SET "BUILDTYPE=Clean"   & GOTO CHECKSECONDARG
  IF /I "%~1" == "Rebuild"   SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "/Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "-Rebuild"  SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG
  IF /I "%~1" == "--Rebuild" SET "BUILDTYPE=Rebuild" & GOTO CHECKSECONDARG

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:CHECKSECONDARG
rem Check for the second switch
IF "%DEFAULT_ARCH%"=="" SET "DEFAULT_ARCH=all"
IF "%~2" == "" (
  SET "ARCH=%DEFAULT_ARCH%"
) ELSE (
  IF /I "%~2" == "x86"   SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "/x86"  SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "-x86"  SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "--x86" SET "ARCH=x86" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "x64"   SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "/x64"  SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "-x64"  SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "--x64" SET "ARCH=x64" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "all"   SET "ARCH=all" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "/all"  SET "ARCH=all" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "-all"  SET "ARCH=all" & GOTO CHECKTHIRDARG
  IF /I "%~2" == "--all" SET "ARCH=all" & GOTO CHECKTHIRDARG

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:CHECKTHIRDARG
rem Check for the third switch
IF "%DEFAULT_CONFIG%"=="" SET "DEFAULT_CONFIG=all"
IF "%~3" == "" (
  SET "CONFIG=%DEFAULT_CONFIG%"
) ELSE (
  IF /I "%~3" == "Debug"     SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "/Debug"    SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "-Debug"    SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "--Debug"   SET "CONFIG=Debug"   & GOTO START
  IF /I "%~3" == "Release"   SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "/Release"  SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "-Release"  SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "--Release" SET "CONFIG=Release" & GOTO START
  IF /I "%~3" == "all"       SET "CONFIG=all"     & GOTO START
  IF /I "%~3" == "/all"      SET "CONFIG=all"     & GOTO START
  IF /I "%~3" == "-all"      SET "CONFIG=all"     & GOTO START
  IF /I "%~3" == "--all"     SET "CONFIG=all"     & GOTO START

  ECHO.
  ECHO Unsupported commandline switch!
  ECHO Run "%~nx0 help" for details about the commandline switches.
  CALL :SUBMSG "ERROR" "Compilation failed!"
)


:START
CALL :SubVSPath
IF NOT EXIST "%VS_PATH%" CALL :SUBMSG "ERROR" "Visual Studio 2017 NOT FOUND!"

IF /I "%ARCH%" == "x64" GOTO x64
IF /I "%ARCH%" == "x86" GOTO x86


:x86
CALL "%VS_PATH%\Common7\Tools\vsdevcmd" -no_logo -arch=x86

IF /I "%CONFIG%" == "all" (CALL :SUBMSVC %BUILDTYPE% Debug Win32 && CALL :SUBMSVC %BUILDTYPE% Release Win32) ELSE (CALL :SUBMSVC %BUILDTYPE% %CONFIG% Win32)

IF /I "%ARCH%" == "x86" GOTO END


:x64
CALL "%VS_PATH%\Common7\Tools\vsdevcmd" -no_logo -arch=amd64

IF /I "%CONFIG%" == "all" (CALL :SUBMSVC %BUILDTYPE% Debug x64 && CALL :SUBMSVC %BUILDTYPE% Release x64) ELSE (CALL :SUBMSVC %BUILDTYPE% %CONFIG% x64)


:END
TITLE Building Notepad2-mod with MSVC2017 - Finished!
ENDLOCAL
ECHO:
ECHO:Script "%~nx0" has completed.
PAUSE
EXIT /B


:SubVSPath
rem Check the building environment
FOR /f "delims=" %%A IN ('"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -property installationPath -latest -requires Microsoft.Component.MSBuild Microsoft.VisualStudio.Component.VC.ATLMFC Microsoft.VisualStudio.Component.VC.Tools.x86.x64') DO SET "VS_PATH=%%A"
EXIT /B


:SUBMSVC
ECHO.
TITLE Building Notepad2-mod with MSVC2017 - %~1 "%~2|%~3"...
"MSBuild.exe" /nologo Notepad2.sln /t:%~1 /p:Configuration=%~2;Platform=%~3^
 /consoleloggerparameters:Verbosity=minimal /maxcpucount /nodeReuse:true
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Compilation failed!"
EXIT /B


:SHOWHELP
TITLE %~nx0 %1
ECHO. & ECHO.
ECHO Usage: %~nx0 [Clean^|Build^|Rebuild] [x86^|x64^|all] [Debug^|Release^|all]
ECHO.
ECHO Notes: You can also prefix the commands with "-", "--" or "/".
ECHO        The arguments are not case sensitive.
ECHO. & ECHO.
ECHO Executing %~nx0 without any arguments is equivalent to "%~nx0 build all release"
ECHO.
ECHO If you skip the second argument the default one will be used.
ECHO The same goes for the third argument. Examples:
ECHO "%~nx0 rebuild" is the same as "%~nx0 rebuild all release"
ECHO "%~nx0 rebuild x86" is the same as "%~nx0 rebuild x86 release"
ECHO.
ECHO WARNING: "%~nx0 x86" or "%~nx0 debug" won't work.
ECHO.
ENDLOCAL
EXIT /B


:SUBMSG
ECHO. & ECHO ______________________________
ECHO [%~1] %~2
ECHO ______________________________ & ECHO.
IF /I "%~1" == "ERROR" (
  CHOICE /C "YN" /N /M "Build failed; do you wish to retry? [y,n]: "
  IF ERRORLEVEL 2 EXIT
  GOTO START
) ELSE (
  EXIT /B
)
