::
:: This is not to be called directly
:: But is called from anot
::


echo Building %VC%
set "MSBUILD="
if "%VC%" =="vc12" set MSBUILD=1
if "%VC%" =="vc11" set MSBUILD=1
if "%VC%" =="vc10" set MSBUILD=1
	
echo Solution File %VC_SLN%
	
if defined MSBUILD (
REM	echo MSBuild Building (%VC%) DEBUG...
REM	msbuild %MPC_SLN% /t:build /p:Configuration=Debug /p:platform=win32	
	
	msbuild %VC_SLN% /maxcpucount:2 /t:build /p:Configuration=Release /p:platform=win32
) else (
	echo VCBuild
	vcbuild /useenv %VC_SLN% "Release|Win32"
)

if not "%ERRORLEVEL%" == 0 exit /B %ERRORLEVEL%