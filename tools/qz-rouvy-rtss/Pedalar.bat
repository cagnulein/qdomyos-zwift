@echo off
setlocal
rem ---------------------------------------------------------------------------
rem  Clique duplo aqui para: subir o RTSS, conectar no QZ e abrir o Rouvy.
rem
rem  Coloque abaixo o IP:porta do QZ no tablet (o QZ mostra os enderecos em
rem  Settings > Template Settings). Deixe em branco para ele perguntar.
rem ---------------------------------------------------------------------------
set "QZHOST="

set "AQUI=%~dp0"
set "MEMORIA=%AQUI%qz-host.txt"

if not defined QZHOST if exist "%MEMORIA%" set /p QZHOST=<"%MEMORIA%"

if not defined QZHOST (
    echo.
    set /p "QZHOST=IP:porta do QZ no tablet (ex 192.168.1.50:8080): "
)

if not defined QZHOST (
    echo Nenhum endereco informado. Saindo.
    pause
    exit /b 1
)

rem lembra para a proxima vez
>"%MEMORIA%" echo %QZHOST%

rem powershell.exe (5.1) de proposito: tem o modulo Appx nativo, usado para
rem achar o Rouvy quando ele vem da Microsoft Store.
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%AQUI%Start-QzRouvy.ps1" -QzHost %QZHOST% %*

if errorlevel 1 (
    echo.
    echo Algo deu errado - a mensagem acima explica o que.
    pause
)
endlocal
