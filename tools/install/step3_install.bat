@echo off

signtool.exe sign /f mogujie.pfx /p "shark@)$^king" %~dp0bin/bin/teamtalk.exe

makensis.exe %~dp0teamtalk_setup.nsi

signtool.exe sign /f mogujie.pfx /p "shark@)$^king" /t http://timestamp.wosign.com/timestamp %~dp0teamtalk-1.0.exe