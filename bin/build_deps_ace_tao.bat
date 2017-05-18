@setlocal
@echo off
set base=%~dp0
set SCRIPT_DIR=%base:~0,-1%

call "%SCRIPT_DIR%\vsconfig.bat"
call "%SCRIPT_DIR%\config_deps_ace_mpc.bat"
@echo off

if not "%ERRORLEVEL%" == "0" (
	echo Error during configuration.. %ERRORLEVEL%
	exit /B %ERRORLEVEL%
)

pushd "%TAO_ROOT%"	
	
	
	for /f %%i in ('dir /B *_daf_builder_%VC%.sln') do set VC_SLN=%%i
	
	if not exist "%VC_SLN%" (
		echo MPC Generation %VC%
		
		perl "%ACE_ROOT%/bin/mwc.pl" -type %VC% -name_modifier *_daf_builder_%VC% -apply_project TAO_ACE.mwc
	)
	
	for /f %%i in ('dir /B *_daf_builder_%VC%.sln') do set VC_SLN=%%i
	
	if not exist "%VC_SLN%" (
		echo Error during MPC Generation, could not find a Solution file..
		popd
		exit /B 1
	)
	
	call "%SCRIPT_DIR%\build_vs.bat" %VC_SLN%
	
popd

:: Propagate Error
if not "%ERRORLEVEL%" == "0" exit /B %ERRORLEVEL%


@echo on
@endlocal