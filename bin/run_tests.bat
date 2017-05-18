
@setlocal
@echo off

set SCRIPT_DIR=%~dp0

perl %SCRIPT_DIR%\auto_run_tests.pl -r %DAF_ROOT% -l %SCRIPT_DIR%\daf_tests.lst

@echo on
@endlocal
