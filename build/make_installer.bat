@ECHO OFF
rem ******************************************************************************
rem *
rem * Notepad2-mod
rem *
rem * make_installer.bat
rem *   Batch file for building the installer for Notepad2-mod
rem *
rem * See License.txt for details about distribution and modification.
rem *
rem *                                     (c) XhmikosR 2010-2017
rem *                                     https://github.com/XhmikosR/notepad2-mod
rem *
rem ******************************************************************************

SETLOCAL ENABLEEXTENSIONS
CD /D "%~dp0"

:: Configuration
SET "l_project_name=Notepad2-mod-jrb"
SET "l_project_devenv=VS2017" & REM <= Still needed?
SET "l_project_binary=Notepad2-jrb.exe"
SET "l_path_output_x86=..\bin\Release_x86"
SET "l_path_output_x64=..\bin\Release_x64"

rem Check for the help switches
IF /I "%~1" == "help"   GOTO SHOWHELP
IF /I "%~1" == "/help"  GOTO SHOWHELP
IF /I "%~1" == "-help"  GOTO SHOWHELP
IF /I "%~1" == "--help" GOTO SHOWHELP
IF /I "%~1" == "/?"     GOTO SHOWHELP

rem You can set here the Inno Setup path if for example you have Inno Setup Unicode
rem installed and you want to use the ANSI Inno Setup which is in another location
IF NOT DEFINED InnoSetupPath SET "InnoSetupPath="

rem Check the building environment
CALL :SubDetectInnoSetup

rem Check for the first switch
IF "%~1" == "" (
    rem Continue
    ECHO>NUL
) ELSE (
    ECHO:
    ECHO:Unsupported commandline switch!
    ECHO:Run "%~nx0 help" for details about the commandline switches.
    CALL :SUBMSG "ERROR" "Compilation failed!"
)


:START
IF EXIST "..\signinfo.txt" SET "SIGN=True"

IF NOT EXIST "%l_path_output_x86%\%l_project_binary%" CALL :SUBMSG "ERROR" "Compile %l_project_name% x86 first!"
IF NOT EXIST "%l_path_output_x64%\%l_project_binary%" CALL :SUBMSG "ERROR" "Compile %l_project_name% x64 first!"

IF /I "%SIGN%" == "True" CALL :SubSign "%l_path_output_x86%"
IF /I "%SIGN%" == "True" CALL :SubSign "%l_path_output_x64%"

CALL :SubInstaller


:END
TITLE Finished!
ECHO:
ENDLOCAL
GOTO :EOF


:SubInstaller
TITLE Building installer...
CALL :SUBMSG "INFO" "Building installer using %InnoSetupPath%\ISCC.exe..."

"%InnoSetupPath%\ISCC.exe" /SMySignTool="cmd /c "%~dp0sign.bat" $f" /Q /O"packages" "..\distrib\notepad2_setup.iss" /D%1
IF %ERRORLEVEL% NEQ 0 CALL :SUBMSG "ERROR" "Packaging failed!"

GOTO :EOF


:SubDetectInnoSetup
rem Detect if we are running on 64bit Windows and use Wow6432Node since Inno Setup is
rem a 32-bit application, and set the registry key of Inno Setup accordingly
IF DEFINED PROGRAMFILES(x86) (
  SET "l_reg_uninstall_root=HKLM\SOFTWARE\Wow6432Node\Microsoft\Windows\CurrentVersion\Uninstall"
) ELSE (
  SET "l_reg_uninstall_root=HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall"
)

IF DEFINED InnoSetupPath IF NOT EXIST "%InnoSetupPath%" (
  CALL :SUBMSG "INFO" ""%InnoSetupPath%" wasn't found on this machine! I will try to detect Inno Setup's path from the registry..."
)

IF NOT EXIST "%InnoSetupPath%" (
  FOR /F "delims=" %%a IN (
    'REG QUERY "%l_reg_uninstall_root%\Inno Setup 5_is1" /v "Inno Setup: App Path"2^>Nul^|FIND "REG_"') DO (
    SET "InnoSetupPath=%%a" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
)
IF NOT EXIST "%InnoSetupPath%" (
  FOR /F "delims=" %%a IN (
    'REG QUERY "%l_reg_uninstall_root%\Inno Setup 6_is1" /v "Inno Setup: App Path"2^>Nul^|FIND "REG_"') DO (
    SET "InnoSetupPath=%%a" & CALL :SubInnoSetupPath %%InnoSetupPath:*Z=%%)
)

IF NOT EXIST "%InnoSetupPath%" CALL :SUBMSG "ERROR" "Inno Setup wasn't found!"
GOTO :EOF


:SubInnoSetupPath
SET "InnoSetupPath=%*"
GOTO :EOF


:SubSign
IF %ERRORLEVEL% NEQ 0 GOTO :EOF
REM %1 is the subfolder

CALL "%~dp0sign.bat" "..\%1\%l_project_binary%" || (CALL :SUBMSG "ERROR" "Problem signing ..\%1\%l_project_binary%" & GOTO Break)

CALL :SUBMSG "INFO" "..\%1\%l_project_binary% signed successfully."

:Break
GOTO :EOF


:SHOWHELP
TITLE %~nx0 %1
ECHO:
ECHO:
ECHO:Usage:  %~nx0
ECHO:
ECHO:Notes:  You can also prefix the commands with "-", "--" or "/".
ECHO         The arguments are not case sensitive.
ECHO:
ECHO         You can use another Inno Setup location by defining %%InnoSetupPath%%.
ECHO         This is usefull if you have the Unicode build installed
ECHO         and you want to use the ANSI one.
ECHO:
ECHO:
ECHO:
ENDLOCAL
GOTO :EOF


:SUBMSG
ECHO. & ECHO:______________________________
ECHO:[%~1] %~2
ECHO:______________________________ & ECHO:
IF /I "%~1" == "ERROR" (
  PAUSE
  EXIT
) ELSE (
  GOTO :EOF
)
