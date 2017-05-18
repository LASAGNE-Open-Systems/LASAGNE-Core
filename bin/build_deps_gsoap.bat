@setlocal
@echo off
set base=%~dp0
set SCRIPT_DIR=%base:~0,-1%

call "%SCRIPT_DIR%\vsconfig.bat"
call "%SCRIPT_DIR%\build_check_env.bat"

if not "%ERRORLEVEL%" == "0" (
	echo Error during configuration.. %ERRORLEVEL%
	exit /B %ERRORLEVEL%
)

if not exist "%GSOAP_HOME%\gsoap\bin\win32\wsdl2h.exe" goto error
if not exist "%GSOAP_HOME%\gsoap\bin\win32\soapcpp2.exe" goto error

goto good

:error
echo *****************************************
echo SORRY gSOAP under Windows. You are on your own. The required binaries should
echo come with the gSOAP distribution. Maybe your GSOAP_HOME env var is wrong 
echo *****************************************
exit /B 1


:good


@echo on
@endlocal