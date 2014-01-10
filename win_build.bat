@echo off
set VisualStudioVersion=12.0
IF "%1" == "Rebuild" (
set args=/t:Rebuild
) ELSE (
set args=""
)

MSBuild.exe rasp.sln %args% /m /p:Platform=Win32 /p:TargetFrameworkVersion=v4.5.1 /p:PlatformToolset=v120 /toolsversion:12.0
