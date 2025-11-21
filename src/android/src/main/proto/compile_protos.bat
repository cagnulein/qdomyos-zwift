@echo off
setlocal enabledelayedexpansion

REM Percorso al tuo protoc specifico
set PROTOC_EXE=C:\Users\violarob\Downloads\protoc-3.25.8-windows-x86_64.exe

REM Verifica che protoc esista
if not exist "%PROTOC_EXE%" (
    echo ERRORE: protoc non trovato in: %PROTOC_EXE%
    echo Verifica che il file esista e il percorso sia corretto.
    pause
    exit /b 1
)

REM Directory di output
set OUTPUT_DIR=..\java

REM Crea directory di output
if not exist "%OUTPUT_DIR%" mkdir "%OUTPUT_DIR%"

echo ===============================
echo COMPILAZIONE PROTOBUF
echo ===============================
echo Protoc: %PROTOC_EXE%
echo Directory corrente: %CD%
echo Output in: %OUTPUT_DIR%
echo.

REM Verifica versione protoc
echo Versione protoc:
"%PROTOC_EXE%" --version
echo.

REM Contatori
set /a success_count=0
set /a error_count=0

REM Compila file .proto nella directory corrente
for %%f in (*.proto) do (
    echo [INFO] Compilando: %%f
    "%PROTOC_EXE%" --java_out=lite:"%OUTPUT_DIR%" --proto_path=. "%%f"
    if errorlevel 1 (
        echo [ERRORE] Fallito: %%f
        set /a error_count+=1
    ) else (
        echo [OK] Successo: %%f
        set /a success_count+=1
    )
    echo.
)

REM Compila file .proto nelle sottocartelle
for /d %%d in (*) do (
    if exist "%%d\*.proto" (
        echo [INFO] Sottocartella trovata: %%d
        for %%f in (%%d\*.proto) do (
            echo [INFO] Compilando: %%f
            "%PROTOC_EXE%" --java_out=lite:"%OUTPUT_DIR%" --proto_path=. "%%f"
            if errorlevel 1 (
                echo [ERRORE] Fallito: %%f
                set /a error_count+=1
            ) else (
                echo [OK] Successo: %%f
                set /a success_count+=1
            )
        )
        echo.
    )
)

REM Riepilogo finale
echo ===============================
echo RIEPILOGO COMPILAZIONE:
echo File compilati con successo: %success_count%
echo File con errori: %error_count%
echo Directory output: %OUTPUT_DIR%
echo ===============================

if %error_count% gtr 0 (
    echo ATTENZIONE: Compilazione completata con %error_count% errori!
    pause
    exit /b 1
) else (
    echo SUCCESSO: Tutti i file compilati correttamente!
    pause
    exit /b 0
)