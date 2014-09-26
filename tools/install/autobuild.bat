@echo off 

set TEAMTALKDIR=%~dp0..\..\

@echo building...
cd %~dp0
call %~dp0step1_build.bat
@echo packing...
call %~dp0step2_pack.bat %1-%2 %3
@echo installing...
call %~dp0step3_install.bat

pause