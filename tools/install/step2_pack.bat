@echo off 
color 0A 

set TEAMTALKDIR=%~dp0..\..\
set TEAMTALK_SRC=%TEAMTALKDIR%\bin\teamtalk
set TEAMTALK_INSTALL=%TEAMTALKDIR%\tools\install
set TEAMTALK_DEPEND=%TEAMTALKDIR%\tools\depends
set TEAMTALK_DEST=%TEAMTALKDIR%\tools\install\history\%1
set TEAMTALK_BIN=%TEAMTALK_DEST%\bin
set TEAMTALK_PDB=%TEAMTALK_DEST%\pdb
set TEAMTALK_VER=%TEAMTALK_DEST%\version

if exist %TEAMTALK_DEST% goto error_FILE

if "%1"=="" goto error_VERSION
if "%2"=="" goto error_VERSION

@echo 删除上个版本的EXE
if exist %TEAMTALK_INSTALL%\teamtalk-1.0.exe del %TEAMTALK_INSTALL%\teamtalk-1.0.exe

@echo 创建文件夹...
md %TEAMTALK_BIN%
md %TEAMTALK_BIN%\bin
md %TEAMTALK_BIN%\gui
md %TEAMTALK_PDB%
md %TEAMTALK_VER%

@echo 写版本数据
set BUILDVER=build_version:%1
@echo %BUILDVER% >> %TEAMTALK_VER%\version.dat
set SVNVER=svn_version:%2
@echo %SVNVER% >> %TEAMTALK_VER%\version.dat

@echo 开始拷贝release...
@rem app
copy %TEAMTALK_SRC%\release\DuiLib.dll 				        %TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\GifSmiley.dll 					%TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\httpclient.dll 					%TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\libogg.dll					    %TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\Modules.dll					    %TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\speexdec.exe					%TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\sqlite3.dll					    %TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\teamtalk.exe					%TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\TTLogic.dll					    %TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\release\utility.dll					    %TEAMTALK_BIN%\bin\
copy %TEAMTALK_SRC%\stringtable.ini                   		%TEAMTALK_BIN%\

@echo 开始拷贝gui...
xcopy %TEAMTALK_SRC%\gui /e /q /i              				%TEAMTALK_BIN%\gui

@echo 开始拷贝data...
xcopy %TEAMTALK_SRC%\data\module /e /q /i              		%TEAMTALK_BIN%\data\module
rmdir /S /Q %TEAMTALK_BIN%\data\module\js\src
xcopy %TEAMTALK_SRC%\data\Emotion /e /q /i              	%TEAMTALK_BIN%\data\Emotion
xcopy %TEAMTALK_SRC%\data\icons /e /q /i              		%TEAMTALK_BIN%\data\icons
xcopy %TEAMTALK_SRC%\data\avatar /e /q /i              		%TEAMTALK_BIN%\data\avatar

@echo 开始拷贝依赖库...
copy %TEAMTALK_DEPEND%\mfc120u.dll             			%TEAMTALK_BIN%\bin\
copy %TEAMTALK_DEPEND%\msvcp120.dll                    	%TEAMTALK_BIN%\bin\
copy %TEAMTALK_DEPEND%\msvcr120.dll                    	%TEAMTALK_BIN%\bin\
copy %TEAMTALK_DEPEND%\gdiplus.dll                    	%TEAMTALK_BIN%\bin\

@echo 开始拷贝PDB等
copy %TEAMTALK_SRC%\release\httpclient.pdb				%TEAMTALK_PDB%\
copy %TEAMTALK_SRC%\release\Modules.pdb 				%TEAMTALK_PDB%\
copy %TEAMTALK_SRC%\release\speexdec.pdb 				%TEAMTALK_PDB%\
copy %TEAMTALK_SRC%\release\teamtalk.pdb 				%TEAMTALK_PDB%\
copy %TEAMTALK_SRC%\release\TTLogic.pdb 				%TEAMTALK_PDB%\
copy %TEAMTALK_SRC%\release\utility.pdb 				%TEAMTALK_PDB%\

@echo 拷贝说明文档
copy %TEAMTALK_INSTALL%\Teamtalk新特性.txt       	    %TEAMTALK_BIN%\

@rem 设置打包版本号
Setlocal ENABLEDELAYEDEXPANSION
set SRCTXT=%TEAMTALK_INSTALL%\version.dat
set DESTTXT=%TEAMTALK_BIN%\bin\version.dat

set var=
for /f %%i in (%SRCTXT%) do (
	set var=%%i
	set /a var+=1
	echo 当前打包版本号：!var:~0!
	echo !var:~0! > %SRCTXT%
	echo !var:~0! > %DESTTXT%
)
set var=pack_version:%var%
@echo %var% >> %TEAMTALK_VER%\version.dat
@echo 设置打包版本号结束

set TEAMTALK_NEWEST=%TEAMTALK_INSTALL%\bin
if exist %TEAMTALK_NEWEST% rd %TEAMTALK_NEWEST% /S /Q
xcopy %TEAMTALK_DEST%\bin /e /q /i %TEAMTALK_NEWEST%
@echo 打包结束

@goto end

:error_FILE
@echo 文件夹已存在！！！
@goto end

:error_VERSION
@echo 版本号非法！！！
@goto end

:end