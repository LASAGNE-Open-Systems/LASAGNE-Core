@setlocal
@echo off
set base=%~dp0
set SCRIPT_DIR=%base:~0,-1%

call "%SCRIPT_DIR%\build_check_env.bat"

if not exist "%ACE_ROOT%\ace\config.h" (
	echo ***************************************
	echo  ACE config.h is missing copying one for you 	
	echo ***************************************
	copy "%SCRIPT_DIR%\build\ace\config.h" "%ACE_ROOT%\ace\config.h"		
)

if not exist "%ACE_ROOT%\MPC\config\default.features" (
	echo ***************************************
	echo  MPC default.features is missing copying one for you 	
	echo ***************************************
	copy "%SCRIPT_DIR%\build\ace\default.features" "%ACE_ROOT%\MPC\config\default.features"	
)

if not exist "%ACE_ROOT%\MPC\config\base.cfg" (
	echo ***************************************
	echo  MPC config\base.cfg needs configuring for DAF
	echo ***************************************
	echo %DAF_ROOT% = $DAF_ROOT/MPC/config >> "%ACE_ROOT%\MPC\config\base.cfg"	
    
    REM add entire Drive to include DAF_ROOT configuration for MPC
    echo %DAF_ROOT:~0,3% = $DAF_ROOT/MPC/config >> "%ACE_ROOT%\MPC\config\base.cfg"	
    REM add current Drive to include DAF_ROOT configuratoin for MPC
    echo %~d0 = $DAF_ROOT/MPC/config >> "%ACE_ROOT%\MPC\config\base.cfg"
)

:: Propagate Error
if not "%ERRORLEVEL%" == "0" exit /B %ERRORLEVEL%


@echo on
@endlocal