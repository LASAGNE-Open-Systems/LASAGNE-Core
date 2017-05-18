@setlocal
@echo off
set base=%~dp0
set SCRIPT_DIR=%base:~0,-1%

call "%SCRIPT_DIR%\vsconfig.bat"
call "%SCRIPT_DIR%\build_check_env.bat"

if not exist "%SSL_ROOT%" (
	echo ***************************************
	echo  SSL_ROOT is not defined... Where is your OpenSSL development install?
	echo ***************************************
	exit
)


if not "%ERRORLEVEL%" == "0" (
	echo Error during configuration.. %ERRORLEVEL%
	exit /B %ERRORLEVEL%
)

pushd "%SSL_ROOT%"	
    echo OpenSSL - Configuring....
	perl Configure VC-WIN32 no-asm --prefix=%SSL_ROOT%
	if not "%ERRORLEVEL%" == "0" (
		echo Error During OpenSSL Configuration... %ERRORLEVEL%
		exit /B %ERRORLEVEL%
	)
	
	call ms\do_ms
	if not "%ERRORLEVEL%" == "0" (
		echo Error During OpenSSL Microsoft Configuration... %ERRORLEVEL%
		exit /B %ERRORLEVEL%
	)
	
	nmake -f ms\ntdll.mak clean
	nmake -f ms\nt.mak clean
	
	nmake -f ms\ntdll.mak 
	nmake -f ms\nt.mak
	
	nmake -f ms\ntdll.mak test
	
	
popd

:: Copy the dlls into DAF_ROOT/bin to ensure that runs with the correctly built DLLs.
xcopy "%SSL_ROOT%\out32dll\libeay32.dll" "%DAF_ROOT%\bin"
xcopy "%SSL_ROOT%\out32dll\ssleay32.dll" "%DAF_ROOT%\bin"
:: Propagate Error
if not "%ERRORLEVEL%" == "0" exit /B %ERRORLEVEL%


@echo on
@endlocal