@ECHO OFF
rem ******************************************************************************
rem *
rem * Notepad2-mod
rem *
rem * make_zip.bat
rem *   Batch file for creating the zip packages
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
SET "l_installer_naming_scheme=$PRJNAME$_v$VERMAJOR$.$VERMINOR$.$VERBUILD$.$VERREV$"
SET "l_zip_common_scheme=$PRJNAME$_v$VERMAJOR$.$VERMINOR$.$VERBUILD$.$VERREV$"
SET "l_zip_naming_scheme=$PRJNAME$_v$VERMAJOR$.$VERMINOR$.$VERBUILD$.$VERREV$_$ARCH$"
SET "l_project_devenv=VS2017" & REM <= Still needed?
SET "l_project_binary=Notepad2-jrb.exe"
SET "l_path_output_x86=..\bin\Release_x86"
SET "l_path_output_x64=..\bin\Release_x64"
SET "l_readme_path=..\"
SET "l_readme_file=Readme-mod-jrb.txt"
SET "l_readme2_file=Readme-mod.txt"
SET "l_readme3_file=Readme.txt"
SET "l_license_path=..\"
SET "l_license_file=License.txt"
SET "l_info_path=..\"
SET "l_info_file=Notepad2.txt"
SET "l_config_path=..\distrib\"
SET "l_config_file=Notepad2.ini"
SET "l_zip_everything=1"
SET "l_project_zip=$PRJNAME$_v$VERMAJOR$.$VERMINOR$.$VERBUILD$.$VERREV$.zip"

rem Check for the help switches
IF /I "%~1" == "help"   GOTO SHOWHELP
IF /I "%~1" == "/help"  GOTO SHOWHELP
IF /I "%~1" == "-help"  GOTO SHOWHELP
IF /I "%~1" == "--help" GOTO SHOWHELP
IF /I "%~1" == "/?"     GOTO SHOWHELP


rem Check for the first switch
IF "%~1" == "" (
    rem Set default value
    SET "COMPILER=%l_project_devenv%"
) ELSE (
    IF /I "%~1" == "%l_project_devenv%"   (SET "COMPILER=%l_project_devenv%" & GOTO START)
    IF /I "%~1" == "/%l_project_devenv%"  (SET "COMPILER=%l_project_devenv%" & GOTO START)
    IF /I "%~1" == "-%l_project_devenv%"  (SET "COMPILER=%l_project_devenv%" & GOTO START)
    IF /I "%~1" == "--%l_project_devenv%" (SET "COMPILER=%l_project_devenv%" & GOTO START)

    ECHO:
    ECHO:Unsupported commandline switch!
    ECHO:Run "%~nx0 help" for details about the commandline switches.
    CALL :SubMsg "ERROR" "Compilation failed!"
)


:START
IF EXIST "..\signinfo.txt" SET "SIGN=True"

IF /I NOT "%COMPILER%" == "%l_project_devenv%" SET SUFFIX=_%COMPILER%
SET "TEMP_NAME=temp_zip%SUFFIX%"

IF NOT EXIST "%l_path_output_x86%\%l_project_binary%" CALL :SubMsg "ERROR" "Compile %l_project_name% x86 first!"
IF NOT EXIST "%l_path_output_x64%\%l_project_binary%" CALL :SubMsg "ERROR" "Compile %l_project_name% x64 first!"

CALL :SubGetVersion
CALL :SubDetectSevenzipPath

IF /I "%SEVENZIP%" == "" CALL :SubMsg "ERROR" "7za wasn't found!"

IF /I "%SIGN%" == "True" CALL :SubSign "%l_path_output_x86%"
IF /I "%SIGN%" == "True" CALL :SubSign "%l_path_output_x64%"

CALL :SubZipFiles "%l_path_output_x86%" x86
CALL :SubZipFiles "%l_path_output_x64%" x64

IF NOT "%l_zip_everything%"=="1" GOTO :END

CALL :SubMsg "INFO" "Creating ZIP archive of everything..."

rem Compress everything into a single ZIP file
PUSHD "packages"
IF EXIST "%l_project_zip%"  DEL "%l_project_zip%"
IF EXIST "%TEMP_NAME%"      RD /S /Q "%TEMP_NAME%"
IF NOT EXIST "%TEMP_NAME%"  MD "%TEMP_NAME%"

SET "INSTALLER_FNAME=%l_installer_naming_scheme%"
SET "ZIP_CMN_FNAME=%l_zip_common_scheme%"

rem Expand name fields
Call :SubExpandFields INSTALLER_FNAME
Call :SubExpandFields ZIP_CMN_FNAME

IF EXIST "%INSTALLER_FNAME%*.exe" COPY /Y /V "%INSTALLER_FNAME%*.exe" "%TEMP_NAME%\" >NUL
IF EXIST "%ZIP_CMN_FNAME%*.zip" COPY /Y /V "%ZIP_CMN_FNAME%*.zip" "%TEMP_NAME%\" >NUL

PUSHD "%TEMP_NAME%"

rem Expand name fields
Call :SubExpandFields l_project_zip

"%SEVENZIP%" a -tzip -mx=9 "%l_project_zip%" * >NUL
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!"

CALL :SubMsg "INFO" "%l_project_zip% created successfully!"

MOVE /Y "%l_project_zip%" ".." >NUL

POPD
IF EXIST "%TEMP_NAME%" RD /S /Q "%TEMP_NAME%"

POPD


:END
TITLE Finished!
ECHO:Finished!
ECHO:
PAUSE
ENDLOCAL
GOTO :EOF


:SubZipFiles
SET "BINPATH=%~1"
SET "ARCH=%~2"
SET "ZIP_FNAME=%l_zip_naming_scheme%.zip"

rem Expand name fields
Call :SubExpandFields ZIP_FNAME

TITLE Creating %ZIP_FNAME%...
CALL :SubMsg "INFO" "Creating %ZIP_FNAME%..."

IF EXIST "%TEMP_NAME%"     RD /S /Q "%TEMP_NAME%"
IF NOT EXIST "%TEMP_NAME%" MD "%TEMP_NAME%"
IF NOT EXIST "packages"    MD "packages"

FOR %%A IN ("%l_license_path%%l_license_file%" "%1\%l_project_binary%"^
    "%l_config_path%%l_config_file%" "%l_info_path%%l_info_file%"^
    "%l_readme_path%%l_readme_file%" "%l_readme_path%%l_readme2_file%"^
    "%l_readme_path%%l_readme3_file%"
) DO COPY /Y /V "%%A" "%TEMP_NAME%\"

PUSHD "%TEMP_NAME%"
"%SEVENZIP%" a -tzip -mx=9^
    "%ZIP_FNAME%" "%l_license_file%" "%l_project_binary%"^
    "%l_config_file%" "%l_info_file%" "%l_readme_file%"^
    "%l_readme2_file%" "%l_readme3_file%" >NUL
IF %ERRORLEVEL% NEQ 0 CALL :SubMsg "ERROR" "Compilation failed!"

CALL :SubMsg "INFO" "%ZIP_FNAME% created successfully!"

MOVE /Y "%ZIP_FNAME%" "..\packages" >NUL
POPD
IF EXIST "%TEMP_NAME%" RD /S /Q "%TEMP_NAME%"
GOTO:EOF


:SubDetectSevenzipPath
FOR %%G IN (7z.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & GOTO :EOF)

FOR %%G IN (7za.exe) DO (SET "SEVENZIP_PATH=%%~$PATH:G")
IF EXIST "%SEVENZIP_PATH%" (SET "SEVENZIP=%SEVENZIP_PATH%" & GOTO :EOF)

FOR /F "tokens=2*" %%A IN (
  'REG QUERY "HKLM\SOFTWARE\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ" ^|^|
   REG QUERY "HKLM\SOFTWARE\Wow6432Node\7-Zip" /v "Path" 2^>NUL ^| FIND "REG_SZ"') DO SET "SEVENZIP=%%B\7z.exe"
GOTO :EOF


:SubGetVersion
REM Get the version
FOR /F "tokens=3,4 delims= " %%K IN (
    'FINDSTR /I /L /C:"define VERSION_MAJOR" "..\src\Version.h"') DO (SET "VerMajor=%%K")
FOR /F "tokens=3,4 delims= " %%K IN (
    'FINDSTR /I /L /C:"define VERSION_MINOR" "..\src\Version.h"') DO (SET "VerMinor=%%K")
FOR /F "tokens=3,4 delims= " %%K IN (
    'FINDSTR /I /L /C:"define VERSION_BUILD" "..\src\Version.h"') DO (SET "VerBuild=%%K")
FOR /F "tokens=3,4 delims= " %%K IN (
    'FINDSTR /I /L /C:"define VERSION_REV " "..\src\VersionRev.h"') DO (SET "VerRev=%%K")

SET BIN_VER=%VerMajor%.%VerMinor%.%VerBuild%.%VerRev%
GOTO :EOF


:SubExpandFields
SET "TARGET_VAR=%~1"
CALL SET "%TARGET_VAR%=%%%TARGET_VAR%:$PRJNAME$=%l_project_name%%%"
CALL SET "%TARGET_VAR%=%%%TARGET_VAR%:$ARCH$=%ARCH%%%"
CALL SET "%TARGET_VAR%=%%%TARGET_VAR%:$VERMAJOR$=%VerMajor%%%"
CALL SET "%TARGET_VAR%=%%%TARGET_VAR%:$VERMINOR$=%VerMinor%%%"
CALL SET "%TARGET_VAR%=%%%TARGET_VAR%:$VERBUILD$=%VerBuild%%%"
CALL SET "%TARGET_VAR%=%%%TARGET_VAR%:$VERREV$=%VerRev%%%"
GOTO :EOF


:SubSign
SET "BIN_PATH=%~1"
IF %ERRORLEVEL% NEQ 0 GOTO :EOF

CALL "%~dp0sign.bat" "..\%BIN_PATH%\%l_project_binary%" || (CALL :SubMsg "ERROR" "Problem signing ..\%BIN_PATH%\%l_project_binary%" & GOTO Break)
CALL :SubMsg "INFO" "..\%BIN_PATH%\%l_project_binary% signed successfully."


:Break
GOTO :EOF


:SHOWHELP
TITLE %~nx0 %1
ECHO:
ECHO:
ECHO:Usage:  %~nx0 [%l_project_devenv%]
ECHO:
ECHO:Notes:  You can also prefix the commands with "-", "--" or "/".
ECHO:        The arguments are not case sensitive.
ECHO:
ECHO:Executing %~nx0 without any arguments is equivalent to "%~nx0 %l_project_devenv%"
ECHO:
ECHO:
ENDLOCAL
GOTO :EOF


:SubMsg
ECHO. & ECHO:______________________________
ECHO:[%~1] %~2
ECHO:______________________________ & ECHO:
IF /I "%~1" == "ERROR" (
    CHOICE /C "YN" /N /M "Build failed; do you wish to retry? [y,n]: "
    IF ERRORLEVEL 2 EXIT
    GOTO START
) ELSE (
    EXIT /B
)
