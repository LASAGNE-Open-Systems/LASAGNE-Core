  
:: NOTE: note using @setlocal as we want this environment to be exported to other scripts
@echo off
set base=%~dp0
set SCRIPT_DIR=%base:~0,-1%

set STATE=1

if NOT DEFINED MPC_FILE (
	set MPC_FILE=%SCRIPT_DIR%\..\MPC\config\TAF.features
)

echo MPC_FILE=%MPC_FILE%

for %%i in (perl.exe) do set PRESENT=%%~$PATH:i
::echo PRESENT=%PRESENT%

if NOT EXIST "%PRESENT%" (
	echo ***************************************************************************
	echo Perl looks like its missing from the PATH
	echo ***************************************************************************	
	set STATE=0
)


if NOT DEFINED ACE_ROOT (
	echo ***************************************************************************
	echo ACE_ROOT is not defined or valid please check your development environment 
	echo ***************************************************************************	
	set STATE=0
)

if NOT DEFINED TAO_ROOT (
	echo ***************************************************************************
	echo TAO_ROOT is not defined or valid please check your development environment 
	echo ***************************************************************************	
	set STATE=0
)

:: TODO need to check the path for TAO_IDL_FE.dll or TAO_IDL_FEd.dll
set PRESENT=MISSING
set PRESENTD=MISSING
for %%i in (TAO_IDL_FE.dll) do set PRESENT=%%~$PATH:i
for %%i in (TAO_IDL_FEd.dll) do set PRESENTD=%%~$PATH:i
if NOT EXIST "%PRESENT%" if NOT EXIST "%PRESENTD%" echo WARNING: ACE_ROOT/lib missing from PATH. TAO_IDL_FE.dll NOT found

if NOT DEFINED DAF_ROOT (
	echo ***************************************************************************
	echo DAF_ROOT is not defined or valid please check your development environment 
	echo ***************************************************************************	
	set DAF_ROOT=%SCRIPT_DIR%\..
)

if NOT DEFINED TAF_ROOT (
	echo ***************************************************************************
	echo TAF_ROOT is not defined or valid please check your development environment 
	echo ***************************************************************************	
	set TAF_ROOT=%DAF_ROOT%\TAF
)


::TODO: work out how to check Features file

if NOT DEFINED DDS_ROOT (
	echo ***************************************************************************
	echo DDS_ROOT is not defined or valid please check your development environment 
	echo ***************************************************************************	
	set STATE=0
) 

:: TODO Need to check the path for DDS_ROOT, OpenDDS_Dcps.dll
set PRESENT=MISSING
set PRESENTD=MISSING
for %%i in (OpenDDS_Dcps.dll) do set PRESENT=%%~$PATH:i
for %%i in (OpenDDS_Dcpsd.dll) do set PRESENTD=%%~$PATH:i
if NOT EXIST "%PRESENT%" if NOT EXIST "%PRESENTD%" echo WARNING: DDS_ROOT/lib missing from PATH. OpenDDS_Dcps.dll NOT found	



if %STATE% == 0 (
	echo Errors were found in your configuration. Please Fix them before proceeding. 
	exit /B 1
)


::
:: Check for NDDS
:: NDDSHOME, NDDSARCHITECTURE

::
:: Check for CoreDX
::












