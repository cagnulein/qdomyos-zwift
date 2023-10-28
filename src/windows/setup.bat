@echo off
setlocal enabledelayedexpansion

REM Get the current directory of the script
set "script_dir=%~dp0"

REM Check if the directory is already in the PATH
echo !PATH! | find /i "!script_dir!python\x64\Scripts\;!script_dir!python\x64\" > nul
if errorlevel 1 (
  REM Append the directory to the PATH variable
  set "PATH=!PATH!;!script_dir!python\x64\Scripts\;!script_dir!python\x64\"
  echo The script directory has been added to the PATH.
) else (
  echo The script directory is already in the PATH.
)

REM Update the system PATH variable
setx PATH "!PATH!" /M

endlocal
