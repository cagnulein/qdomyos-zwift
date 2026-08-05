@echo off
rem Launch xbox-mywhoosh-gears.ahk without relying on the .ahk file association.
rem Finds AutoHotkey v2 in the two places its installer uses (user / machine scope).

setlocal
set "SCRIPT=%~dp0xbox-mywhoosh-gears.ahk"

for %%E in (
    "%LOCALAPPDATA%\Programs\AutoHotkey\v2\AutoHotkey64.exe"
    "%LOCALAPPDATA%\Programs\AutoHotkey\v2\AutoHotkey32.exe"
    "%ProgramFiles%\AutoHotkey\v2\AutoHotkey64.exe"
    "%ProgramFiles%\AutoHotkey\v2\AutoHotkey32.exe"
) do (
    if exist %%E (
        start "" %%E "%SCRIPT%"
        exit /b 0
    )
)

echo AutoHotkey v2 not found. Install it with:
echo     winget install --id AutoHotkey.AutoHotkey --exact
exit /b 1
