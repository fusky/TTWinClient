@echo off

@rem set envs
call %~dp0set_envs.bat

:x86
call "%ENVIRONMENT%\vcvars32.bat"

echo build teamtalk solution
DEVENV /build %teamtalk_sln_config% %speex_sln_path%
DEVENV /build %teamtalk_sln_config% %teamtalk_sln_path%

pause