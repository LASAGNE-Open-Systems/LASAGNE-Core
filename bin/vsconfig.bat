:: Try and Detect the Machine's architecture and Build for that
if DEFINED ARCH ( 
 set WIN_ARCH=%ARCH%
 
 goto :endofsearch
) 

:: Normally we try and detect the architecture
:: But for DAF/TAF we only reliably work under x86
:: due to dependencies on NDDS/CoreDX etc 
:: We've left the above in to be able to 
:: set the ARCHitecture
set WIN_ARCH=x86


:endofsearch
echo WIN_ARCH=%WIN_ARCH%

REM Check for lowercase version? 
IF "%WIN_ARCH%"=="amd64" (
 set WIN_ARCH=AMD64
)


echo Configuring for Windows %WIN_ARCH% architecture...

if DEFINED VC (
  echo Configuring for Microsoft Compiler %VC%...
   
   :: Visual Studio 2013 - UNTESTED
   if "%VC%"=="vc12" (
    if EXIST "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" (
        echo Visual Studio 2013 - 12.0 %WIN_ARCH% configuring...
    REM Crude Hack for VS2012 Express
    if "%WIN_ARCH%" == "AMD64" ( 
      call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
    ) else (
          call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
    )
        goto :vsconfigend
      ) else (
      echo Visual Studio 2013 - 12.0 could NOT be found Please configure your machine correctly....
      exit /B 1
    )
    )
   
    if "%VC%"=="vc11" (
    if EXIST "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" (
        echo Visual Studio 2012 - 11.0 %WIN_ARCH% configuring...
    REM Crude Hack for VS2012 Express
    if "%WIN_ARCH%" == "AMD64" ( 
      call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
    ) else (
          call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
    )
        goto :vsconfigend
      ) else (
      echo Visual Studio 2012 - 11.0 could NOT be found Please configure your machine correctly....
      exit /B 1
    )
    )

  
  if "%VC%"=="vc10" (
    if EXIST "%VS100COMNTOOLS%\..\VC\vcvarsall.bat" (
      echo Visual Studio 2010 - 10.0  configuring...	  
      call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
      goto :vsconfigend
    ) else (
      echo Visual Studio 2010 - 10.0 could NOT be found Please configure your machine correctly....
      exit  /B 1
    ) 
  )

  if "%VC%"=="vc9" (
    if EXIST "%VS90COMNTOOLS%\vsvars32.bat" (
      echo Visual Studio 2008 - 9.0  configuring....
      call "%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%

      goto :vsconfigend
    ) else ( 
      echo Visual Studio 2008 - 9.0 could NOT be found Please configure your machine correctly....
      exit  /B 1
    ) 
  )


  if "%VC%" == "vc8" (
    if EXIST "%VS80COMNTOOLS%\vsvars32.bat" (
      echo Visual Studio 2005 - 8.0  configuring....
      call "%VS80COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
      set VC=vc8
      goto :vsconfigend
    ) else (
      echo Visual Studio 2005 - 8.0 could NOT be found Please configure your machine correctly....
      exit  /B 1
    )
  )
   
) else (
	echo Detecting the most recent Visual Studio Compiler...
	
	:: Visual Studio 2013 - UNTESTED
    if EXIST "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" (
        echo Visual Studio 2013 - 12.0 %WIN_ARCH% configuring...
		if "%WIN_ARCH%" == "AMD64" ( 
			call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
		) else (
			call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
		)
		set VC=vc12
        goto :vsconfigend      
    )   
	
	if EXIST "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" (
        echo Visual Studio 2012 - 11.0 %WIN_ARCH% configuring...
		REM Crude Hack for VS2012 Express
		if "%WIN_ARCH%" == "AMD64" ( 
			call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" x86_amd64
		) else (
			call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
		)
		
		set VC=vc11
        goto :vsconfigend      
    )   
    
    if EXIST "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" (
      echo Visual Studio 2010 - 10.0  configuring...	  
      call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%	  
	  set VC=vc10
      goto :vsconfigend    
    ) 
  

    if EXIST "%VS90COMNTOOLS%\vsvars32.bat" (
      echo Visual Studio 2008 - 9.0  configuring....
      call "%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
	  set VC=vc9
      goto :vsconfigend
	)
  
    if EXIST "%VS80COMNTOOLS%\vsvars32.bat" (
      echo Visual Studio 2005 - 8.0  configuring....
      call "%VS80COMNTOOLS%\..\..\VC\vcvarsall.bat" %WIN_ARCH%
      set VC=vc8
      goto :vsconfigend  
	)
)

REM if NOT DEFINED VC (
    echo ********************************************************************
	echo ERROR - Unable to detect Visual Studio. Is one installed.
	echo ********************************************************************
	exit /B 2
REM )


:vsconfigend



