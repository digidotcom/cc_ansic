@echo off
REM This script links all necessary files from iC to create the directory for iC4FSL
REM As the files are linked, all changes WILL AFFECT THE GIT REPOSITORY DIRECTLY
cls
SET ROOT_DIR=C:\Etherios4Kinetis
SET EC_LIB_DIR=%ROOT_DIR%\Etherios_CC
SET PROJECTS_DIR=%ROOT_DIR%\Projects
SET MQX_CW_WORKSPACE=%PROJECTS_DIR%\MQX\CW10 Projects
SET MQX_IAR_WORKSPACE=%PROJECTS_DIR%\MQX\IAR Projects
SET UCOS_IAR_WORKSPACE=%PROJECTS_DIR%\uCOS
REM UCOS_REPO_DIR must point to the ucos repository's root directory
SET UCOS_REPO_DIR=C:\Git_root_c\ucos
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
RMDIR /S /Q "%MQX_CW_WORKSPACE%"
RMDIR /S /Q "%MQX_IAR_WORKSPACE%"
RMDIR /S /Q "%UCOS_IAR_WORKSPACE%"

MKDIR "%ROOT_DIR%"
MKDIR "%ROOT_DIR%\Tools"
MKDIR "%ROOT_DIR%\Python_scripts"
MKDIR "%EC_LIB_DIR%"
MKDIR "%EC_LIB_DIR%\private"
MKDIR "%EC_LIB_DIR%\include"
MKDIR "%EC_LIB_DIR%\platform\"
MKDIR "%EC_LIB_DIR%\platform\mqx"
MKDIR "%EC_LIB_DIR%\platform\ucos"
MKDIR "%EC_LIB_DIR%\platform\ucos\ucos-iii"

MKDIR "%PROJECTS_DIR%"
MKDIR "%PROJECTS_DIR%\MQX"
MKDIR "%MQX_CW_WORKSPACE%"
MKDIR "%MQX_IAR_WORKSPACE%"
MKDIR "%UCOS_IAR_WORKSPACE%"

MKDIR "%MQX_CW_WORKSPACE%"
MKDIR "%MQX_CW_WORKSPACE%\etherios_app"
MKDIR "%MQX_CW_WORKSPACE%\etherios_app\Sources"
MKDIR "%MQX_CW_WORKSPACE%\etherios_app\Sources\fs_devices"

MKDIR "%MQX_IAR_WORKSPACE%"
MKDIR "%MQX_IAR_WORKSPACE%\etherios_app"
MKDIR "%MQX_IAR_WORKSPACE%\etherios_app\Sources"
MKDIR "%MQX_IAR_WORKSPACE%\etherios_app\Sources\fs_devices"

MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\etherios_projects\etherios_app"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\ucos-iii"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\Freescale"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\uCTCPIP-V2"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\FS"

MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\etherios_projects\etherios_app"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\ucos-iii"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\Freescale"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\uCTCPIP-V2"
MKDIR "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\FS"


REM Link the library
REM echo Linking "%BASE_DIR%\private\*.*" into "%EC_LIB_DIR%\private\"
for %%F in ("%BASE_DIR%\private\*.*") do (
	mklink /H "%EC_LIB_DIR%\private\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\public\include\*.h" into "%EC_LIB_DIR%\include\"
for %%F in ("%BASE_DIR%\public\include\*.h") do (
	mklink /H "%EC_LIB_DIR%\include\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\public\run\platforms\freescale\*.*" into "%EC_LIB_DIR%\platform\mqx\"
for %%F in ("%BASE_DIR%\public\run\platforms\freescale\*.*") do (
	if NOT %%~nxF == main.c	(
		if NOT %%~nxF == main.h (
			mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
		)
	)
)

REM echo Linking "%BASE_DIR%\public\run\platforms\ucos\*.*" into "%EC_LIB_DIR%\platform\ucos\"
for %%F in ("%BASE_DIR%\public\run\platforms\ucos\*.*") do (
	if NOT %%~nxF == main.c	(
		if NOT %%~nxF == main.h (
			mklink /H "%EC_LIB_DIR%\platform\ucos\%%~nxF" "%%F"
		)
	)
)

REM echo Linking "%BASE_DIR%\public\run\platforms\ucos\ucos-iii\*.*" into "%EC_LIB_DIR%\platform\ucos\ucos-iii"
for %%F in ("%BASE_DIR%\public\run\platforms\ucos\ucos-iii\*.*") do (
	mklink /H "%EC_LIB_DIR%\platform\ucos\ucos-iii\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\kinetis\mqx\source\*.c" into "%EC_LIB_DIR%\platform\mqx\"
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\source\*.c") do (
	mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\kinetis\mqx\include\*.h" into "%EC_LIB_DIR%platform\mqx\"
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\include\*.h") do (
	mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\kinetis\ucos\source\*.c" into "%EC_LIB_DIR%\platform\ucos\"
for %%F in ("%BASE_DIR%\kits\kinetis\ucos\source\*.c") do (
	mklink /H "%EC_LIB_DIR%\platform\ucos\%%~nxF" "%%F"
)
REM echo Linking "%BASE_DIR%\kits\kinetis\ucos\source\ucos-iii\*.c" into "%EC_LIB_DIR%\platform\ucos\ucos-iii"
for %%F in ("%BASE_DIR%\kits\kinetis\ucos\source\ucos-iii\*.c") do (
	mklink /H "%EC_LIB_DIR%\platform\ucos\ucos-iii\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\kinetis\ucos\include\*.h" into "%EC_LIB_DIR%platform\ucos\"
for %%F in ("%BASE_DIR%\kits\kinetis\ucos\include\*.h") do (
	mklink /H "%EC_LIB_DIR%\platform\ucos\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\common\source\*.c" into "%EC_LIB_DIR%\platform\mqx\"
for %%F in ("%BASE_DIR%\kits\common\source\*.c") do (
	mklink /H "%EC_LIB_DIR%\platform\mqx\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\common\source\*.c" into "%EC_LIB_DIR%\platform\ucos\"
for %%F in ("%BASE_DIR%\kits\common\source\*.c") do (
	mklink /H "%EC_LIB_DIR%\platform\ucos\%%~nxF" "%%F"
)

REM echo Linking "%BASE_DIR%\kits\common\include\*.h" into "%EC_LIB_DIR%\include\"
for %%F in ("%BASE_DIR%\kits\common\include\*.h") do (
	mklink /H "%EC_LIB_DIR%\include\%%~nxF" "%%F"
)

REM Link the template application in CodeWarrior workspace.
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\codewarrior\etherios_app\*.*") do (
	mklink /H "%MQX_CW_WORKSPACE%\etherios_app\%%~nxF" "%%F"
)
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\codewarrior\etherios_app\Sources\*.*") do (
	mklink /H "%MQX_CW_WORKSPACE%\etherios_app\Sources\%%~nxF" "%%F"
)

mklink /H "%MQX_CW_WORKSPACE%\etherios_app\Sources\main.c" "%BASE_DIR%\public\run\platforms\freescale\main.c"
mklink /H "%MQX_CW_WORKSPACE%\etherios_app\Sources\main.h" "%BASE_DIR%\public\run\platforms\freescale\main.h"

for %%F in ("%BASE_DIR%\kits\kinetis\mqx\source\fs_devices\*.*") do (
	mklink /H "%MQX_CW_WORKSPACE%\etherios_app\Sources\fs_devices\%%~nxF" "%%F"
)

REM Link the mqx template application in IAR workspace.
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\iar\etherios_app\*.*") do (
	mklink /H "%MQX_IAR_WORKSPACE%\etherios_app\%%~nxF" "%%F"
)
for %%F in ("%BASE_DIR%\kits\kinetis\mqx\projects\iar\etherios_app\Sources\*.*") do (
	mklink /H "%MQX_IAR_WORKSPACE%\etherios_app\Sources\%%~nxF" "%%F"
)
mklink /H "%MQX_IAR_WORKSPACE%\etherios_app\Sources\main.c" "%BASE_DIR%\public\run\platforms\freescale\main.c"
mklink /H "%MQX_IAR_WORKSPACE%\etherios_app\Sources\main.h" "%BASE_DIR%\public\run\platforms\freescale\main.h"

for %%F in ("%BASE_DIR%\kits\kinetis\mqx\source\fs_devices\*.*") do (
	mklink /H "%MQX_IAR_WORKSPACE%\etherios_app\Sources\fs_devices\%%~nxF" "%%F"
)

REM Link the ucos template application in IAR workspace.
for %%F in ("%BASE_DIR%\kits\kinetis\ucos\projects\iar\etherios_app\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\etherios_projects\etherios_app\%%~nxF" "%%F"
)
for %%F in ("%BASE_DIR%\kits\kinetis\ucos\projects\iar\etherios_app\Sources\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\etherios_projects\etherios_app\%%~nxF" "%%F"
)

for %%F in ("%BASE_DIR%\kits\kinetis\ucos\projects\iar\etherios_app\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\etherios_projects\etherios_app\%%~nxF" "%%F"
)
for %%F in ("%BASE_DIR%\kits\kinetis\ucos\projects\iar\etherios_app\Sources\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\etherios_projects\etherios_app\%%~nxF" "%%F"
)

REM Following lines links all necessary folders from the Micrium official package directory
REM Micrium official package directory CAN'T be distributed to clients
SET MICRIUM_OFFICIAL_PACKAGE_DIR=%UCOS_REPO_DIR%\Micrium_Official_Packackages

mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uCOS-III" "%MICRIUM_OFFICIAL_PACKAGE_DIR%\KRN-K3XX-PKG000-X-P1\Micrium\Software\uCOS-III"
mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uC-LIB" "%MICRIUM_OFFICIAL_PACKAGE_DIR%\KRN-K3XX-PKG000-X-P1\Micrium\Software\uC-LIB"
mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uC-CPU" "%MICRIUM_OFFICIAL_PACKAGE_DIR%\KRN-K3XX-PKG000-X-P1\Micrium\Software\uC-CPU"

mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uC-TCPIP-V2" "%MICRIUM_OFFICIAL_PACKAGE_DIR%\NET-TCPX-PKG000-X-P1\Micrium\Software\uC-TCPIP-V2"
mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uC-DHCPc"    "%MICRIUM_OFFICIAL_PACKAGE_DIR%\NET-TCPX-PKG000-X-P1\Micrium\Software\uC-DHCPc"
mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uC-DNSc"     "%MICRIUM_OFFICIAL_PACKAGE_DIR%\NET-TCPX-PKG000-X-P1\Micrium\Software\uC-DNSc"

mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uC-Clk" "%MICRIUM_OFFICIAL_PACKAGE_DIR%\FSX-FSXX-PKG000-X-P1\Micrium\Software\uC-Clk"
mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uc-crc" "%MICRIUM_OFFICIAL_PACKAGE_DIR%\FSX-FSXX-PKG000-X-P1\Micrium\Software\uc-crc"
mklink /D "%UCOS_IAR_WORKSPACE%\Micrium\Software\uc-fs" "%MICRIUM_OFFICIAL_PACKAGE_DIR%\FSX-FSXX-PKG000-X-P1\Micrium\Software\uc-fs"


REM Following lines links ucos BSPs
SET MICRIUM_BSPS_DIR=%BASE_DIR%\kits\kinetis\ucos\source\ucos-iii\BSP
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK53N512\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK53N512\ucos-iii\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\ucos-iii\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK53N512\Freescale\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\Freescale\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK53N512\uCTCPIP-V2\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\uCTCPIP-V2\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK53N512\FS\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K53N512\IAR\BSP\FS\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK60N512\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK60N512\ucos-iii\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\ucos-iii\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK60N512\Freescale\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\Freescale\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK60N512\uCTCPIP-V2\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\uCTCPIP-V2\%%~nxF" "%%F"
)
for %%F in ("%MICRIUM_BSPS_DIR%\TWRK60N512\FS\*.*") do (
	mklink /H "%UCOS_IAR_WORKSPACE%\Micrium\Software\EvalBoards\Freescale\TWR-K60N512\IAR\BSP\FS\%%~nxF" "%%F"
)

mklink /H "%ROOT_DIR%\Tools\FWUpdateIntFlash.lcf" "%BASE_DIR%\kits\kinetis\mqx\projects\misc\FWUpdateIntFlash.lcf"
mklink /H "%ROOT_DIR%\Tools\FWUpdateIntFlash.icf" "%BASE_DIR%\kits\kinetis\mqx\projects\misc\FWUpdateIntFlash.icf"

REM The following links python scripts from all samples to destination
SET RETURN_DIR="%CD%"
REM Change to the dir where to look recursively
cd "%BASE_DIR%\public\run\samples\"
call :treeProcess
cd %RETURN_DIR%

:end
pause
exit /B

:treeProcess
for %%F in (*.py) do (
	mklink /H "%ROOT_DIR%\Python_scripts\%%~nxF" "%%F"
	REM echo "%ROOT_DIR%\Python_scripts\%%~nxF" "%%F"
)
for /D %%d in (*) do (
    cd %%d
    call :treeProcess
    cd ..
)
goto:eof
