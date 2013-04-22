@echo off
REM This script links all necessary files from iC to create the directory for iC4FSL
REM As the files are linked, all changes WILL AFFECT THE GIT REPOSITORY DIRECTLY
cls
SET ROOT_DIR=C:\Etherios4Kinetis
SET EC_LIB_DIR=%ROOT_DIR%\Etherios_CC
SET CW_WORKSPACE=%ROOT_DIR%\ecc_cw_project
SET IAR_WORKSPACE=%ROOT_DIR%\ecc_iar_project
REM BASE_DIR must point to the repository's root directory
SET BASE_DIR=..\..\..

REM We need an absolute path to do the linking properly...
REM Save the current directory
pushd .
cd "%BASE_DIR%"
SET BASE_DIR=%CD%
REM Restore directory
popd

RMDIR /S /Q "%EC_LIB_DIR%"
RMDIR /S /Q "%CW_WORKSPACE%"
RMDIR /S /Q "%IAR_WORKSPACE%"

MKDIR %ROOT_DIR%
MKDIR %ROOT_DIR%\utils
MKDIR "%EC_LIB_DIR%"
MKDIR "%EC_LIB_DIR%\private"
MKDIR "%EC_LIB_DIR%\include"
MKDIR "%EC_LIB_DIR%\platform\"
MKDIR "%EC_LIB_DIR%\platform\mqx"

MKDIR "%CW_WORKSPACE%"
MKDIR "%CW_WORKSPACE%\etherios_app"
MKDIR "%CW_WORKSPACE%\etherios_app\Sources"
MKDIR "%CW_WORKSPACE%\etherios_app\Sources\fs_devices"

MKDIR "%IAR_WORKSPACE%"
MKDIR "%IAR_WORKSPACE%\etherios_app"
MKDIR "%IAR_WORKSPACE%\etherios_app\Sources"
MKDIR "%IAR_WORKSPACE%\etherios_app\Sources\fs_devices"


REM Link the library
REM echo Linking "%BASE_DIR%\private\*.*" into "%EC_LIB_DIR%\private\"
for %%F in ("%BASE_DIR%\private\*.*") do (
	mklink /H "%EC_LIB_DIR%\private\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\public\include\*.h" into "%EC_LIB_DIR%\platform\mqx\"
for %%F in ("%BASE_DIR%\public\include\*.h") do (
	mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\public\run\platforms\freescale\*.*" into "%EC_LIB_DIR%\platform\mqx\"
for %%F in ("%BASE_DIR%\public\run\platforms\freescale\*.*") do (
	if NOT %%~nxF == main.c	(
		if NOT %%~nxF == main.h (
			mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
		)
	)
)

REM echo Linking "%BASE_DIR%\kits\kinetis\mqx\source\*.c" into "%EC_LIB_DIR%\platform\mqx\"
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\source\*.c") do (
	mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\kinetis\mqx\include\*.h" into "%EC_LIB_DIR%\include\"
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\include\*.h") do (
	mklink /H "%EC_LIB_DIR%\include\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\common\source\*.c" into "%EC_LIB_DIR%\platform\mqx\"
for %%F in ("%BASE_DIR%\kits\common\source\*.c") do (
	mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\common\include\*.h" into "%EC_LIB_DIR%\include\"
for %%F in ("%BASE_DIR%\kits\common\include\*.h") do (
	mklink /H "%EC_LIB_DIR%\include\%%~nxF" "%%F"
)

mklink /H "%EC_LIB_DIR%\platform\mqx\config.rci" "%BASE_DIR%\public\run\samples\simple_remote_config\config.rci"
mklink /H "%EC_LIB_DIR%\platform\mqx\remote_config.h" "%BASE_DIR%\public\run\samples\simple_remote_config\remote_config.h"
mklink /H "%EC_LIB_DIR%\platform\mqx\remote_config_cb.c" "%BASE_DIR%\public\run\samples\simple_remote_config\remote_config_cb.c"
mklink /H "%EC_LIB_DIR%\platform\mqx\remote_config_cb.h" "%BASE_DIR%\public\run\samples\simple_remote_config\remote_config_cb.h"
mklink /H "%EC_LIB_DIR%\platform\mqx\system.c" "%BASE_DIR%\public\run\samples\simple_remote_config\system.c"
mklink /H "%EC_LIB_DIR%\platform\mqx\gps_stats.c" "%BASE_DIR%\public\run\samples\simple_remote_config\gps_stats.c"


REM Link the template application in CodeWarrior workspace.
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\codewarrior\etherios_app\*.*") do (
	mklink /H "%CW_WORKSPACE%\etherios_app\%%~nxF" "%%F"
)
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\codewarrior\etherios_app\Sources\*.*") do (
	mklink /H "%CW_WORKSPACE%\etherios_app\Sources\%%~nxF" "%%F"
)

mklink /H "%CW_WORKSPACE%\etherios_app\Sources\main.c" "%BASE_DIR%\public\run\platforms\freescale\main.c"
mklink /H "%CW_WORKSPACE%\etherios_app\Sources\main.h" "%BASE_DIR%\public\run\platforms\freescale\main.h"
mklink /H "%CW_WORKSPACE%\etherios_app\Sources\application.c" "%BASE_DIR%\kits\kinetis\mqx\samples\send_data\application.c"

for %%F in ("%BASE_DIR%\kits\kinetis\mqx\source\fs_devices\*.*") do (
	mklink /H "%CW_WORKSPACE%\etherios_app\Sources\fs_devices\%%~nxF" "%%F"
)

REM Link the template application in IAR workspace.
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\iar\etherios_app\*.*") do (
	mklink /H "%IAR_WORKSPACE%\etherios_app\%%~nxF" "%%F"
)
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\iar\etherios_app\Sources\*.*") do (
	mklink /H "%IAR_WORKSPACE%\etherios_app\Sources\%%~nxF" "%%F"
)
mklink /H "%IAR_WORKSPACE%\etherios_app\Sources\main.c" "%BASE_DIR%\public\run\platforms\freescale\main.c"
mklink /H "%IAR_WORKSPACE%\etherios_app\Sources\main.h" "%BASE_DIR%\public\run\platforms\freescale\main.h"
mklink /H "%IAR_WORKSPACE%\etherios_app\Sources\application.c" "%BASE_DIR%\kits\kinetis\mqx\samples\send_data\application.c"

for %%F in ("%BASE_DIR%\kits\kinetis\mqx\source\fs_devices\*.*") do (
	mklink /H "%IAR_WORKSPACE%\etherios_app\Sources\fs_devices\%%~nxF" "%%F"
)

mklink /H "%ROOT_DIR%\utils\FWUpdateIntFlash.lcf" "%BASE_DIR%\kits\kinetis\mqx\projects\misc\FWUpdateIntFlash.lcf"

:end
pause
