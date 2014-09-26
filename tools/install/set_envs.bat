@echo off

@rem envs.conf
FOR /F "eol=; tokens=1,* delims==" %%a IN (%~dp0envs.conf) DO @SET %%a=%%b