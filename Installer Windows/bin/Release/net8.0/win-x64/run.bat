@echo off

dotnet new install Avalonia.Templates
QEngineInstallator.exe
setx PATH "%PATH%;%LOCALAPPDATA%\qengine"
pause

