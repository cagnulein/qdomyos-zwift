<#
.SYNOPSIS
    Sobe a ponte QZ -> RTSS, espera o QZ conectar e so entao abre o Rouvy.

.DESCRIPTION
    Ordem importa: o RTSS precisa estar de pe' antes do Rouvy para o hook
    entrar, e a ponte precisa estar conectada antes do Rouvy para o OSD ja'
    nascer com dado bom. Quando o Rouvy fecha, a ponte e' desligada de forma
    limpa (stdin fechada), liberando o slot de OSD no RTSS.

.EXAMPLE
    .\Start-QzRouvy.ps1 -QzHost 192.168.1.50:8080

.EXAMPLE
    .\Start-QzRouvy.ps1 -QzHost 192.168.1.50:8080 -Rouvy "C:\caminho\Rouvy.exe"
    O caminho fica salvo em qz-rouvy.config.json e nao precisa ser repetido.
#>

[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string] $QzHost,

    [string] $Rouvy,
    [int]    $MaxGear = 24,
    [int]    $MaxResistance = 32,

    # segundos esperando o QZ conectar antes de desistir
    [int]    $ConnectTimeout = 90,

    # abre o Rouvy mesmo se o QZ nao conectar a tempo
    [switch] $LaunchAnyway,

    # nao tenta subir o RTSS
    [switch] $SkipRtss
)

$ErrorActionPreference = 'Stop'
$scriptDir  = Split-Path -Parent $MyInvocation.MyCommand.Path
$bridgePath = Join-Path $scriptDir 'qz_rtss_bridge.py'
$configPath = Join-Path $scriptDir 'qz-rouvy.config.json'

function Write-Step  ($m) { Write-Host "==> $m" -ForegroundColor Cyan }
function Write-Ok    ($m) { Write-Host "    $m" -ForegroundColor Green }
function Write-Warn2 ($m) { Write-Host "    $m" -ForegroundColor Yellow }

# ---------------------------------------------------------------------------
# Descoberta do Rouvy
# ---------------------------------------------------------------------------

function Find-RouvyExe {
    # 1. registro de desinstalacao (Win32 clssico)
    $roots = @(
        'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*',
        'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*',
        'HKCU:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*'
    )
    foreach ($root in $roots) {
        $hits = Get-ItemProperty $root -ErrorAction SilentlyContinue |
                Where-Object { $_.DisplayName -like '*Rouvy*' }
        foreach ($h in $hits) {
            if ($h.DisplayIcon) {
                $exe = ($h.DisplayIcon -split ',')[0].Trim('"')
                if ($exe -like '*.exe' -and (Test-Path $exe)) { return $exe }
            }
            if ($h.InstallLocation -and (Test-Path $h.InstallLocation)) {
                $exe = Get-ChildItem $h.InstallLocation -Filter '*ouvy*.exe' `
                        -Recurse -Depth 2 -ErrorAction SilentlyContinue |
                       Select-Object -First 1
                if ($exe) { return $exe.FullName }
            }
        }
    }

    # 2. caminhos usuais
    $globs = @(
        (Join-Path $env:LOCALAPPDATA 'Programs\*ouvy*'),
        (Join-Path $env:LOCALAPPDATA '*ouvy*'),
        (Join-Path $env:ProgramFiles '*ouvy*'),
        (Join-Path ${env:ProgramFiles(x86)} '*ouvy*')
    ) | Where-Object { $_ }
    foreach ($g in $globs) {
        $dir = Get-Item $g -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($dir) {
            $exe = Get-ChildItem $dir.FullName -Filter '*ouvy*.exe' `
                    -Recurse -Depth 2 -ErrorAction SilentlyContinue |
                   Select-Object -First 1
            if ($exe) { return $exe.FullName }
        }
    }

    # 3. atalho do menu iniciar
    $menus = @(
        (Join-Path $env:APPDATA   'Microsoft\Windows\Start Menu\Programs'),
        (Join-Path $env:ProgramData 'Microsoft\Windows\Start Menu\Programs')
    )
    foreach ($m in $menus) {
        if (-not (Test-Path $m)) { continue }
        $lnk = Get-ChildItem $m -Filter '*ouvy*.lnk' -Recurse -ErrorAction SilentlyContinue |
               Select-Object -First 1
        if ($lnk) {
            $target = (New-Object -ComObject WScript.Shell).CreateShortcut($lnk.FullName).TargetPath
            if ($target -and (Test-Path $target)) { return $target }
        }
    }
    return $null
}

function Find-RouvyAppx {
    # o Rouvy tambem e' distribuido como app da Microsoft Store.
    # Get-AppxPackage nao existe no PowerShell 7 sem o shim de compatibilidade,
    # entao checamos antes de chamar em vez de deixar estourar.
    if (-not (Get-Command Get-AppxPackage -ErrorAction SilentlyContinue)) { return $null }
    try {
        $pkg = Get-AppxPackage -Name '*ouvy*' -ErrorAction SilentlyContinue |
               Select-Object -First 1
        if (-not $pkg) { return $null }
        $manifest = Get-AppxPackageManifest $pkg -ErrorAction Stop
        $appId = $manifest.Package.Applications.Application.Id
        if ($appId -is [array]) { $appId = $appId[0] }
        return "shell:AppsFolder\$($pkg.PackageFamilyName)!$appId"
    } catch {
        return $null
    }
}

function Resolve-Rouvy {
    param([string] $Explicit)

    if ($Explicit) {
        if ($Explicit -like 'shell:AppsFolder\*' -or (Test-Path $Explicit)) { return $Explicit }
        throw "nao achei o Rouvy em '$Explicit'"
    }
    if (Test-Path $configPath) {
        $saved = (Get-Content $configPath -Raw | ConvertFrom-Json).Rouvy
        if ($saved -and ($saved -like 'shell:AppsFolder\*' -or (Test-Path $saved))) { return $saved }
    }
    $found = Find-RouvyAppx
    if (-not $found) { $found = Find-RouvyExe }
    if (-not $found) {
        throw ("nao encontrei o Rouvy instalado. Rode uma vez com o caminho " +
               "explicito: .\Start-QzRouvy.ps1 -QzHost $QzHost -Rouvy 'C:\...\Rouvy.exe'")
    }
    return $found
}

# ---------------------------------------------------------------------------

function Start-RtssIfNeeded {
    if ($SkipRtss) { return }
    if (Get-Process -Name 'RTSS' -ErrorAction SilentlyContinue) {
        Write-Ok 'RTSS ja esta rodando'
        return
    }
    $candidates = @(
        (Join-Path ${env:ProgramFiles(x86)} 'RivaTuner Statistics Server\RTSS.exe'),
        (Join-Path $env:ProgramFiles        'RivaTuner Statistics Server\RTSS.exe')
    ) | Where-Object { $_ -and (Test-Path $_) }

    if (-not $candidates) {
        Write-Warn2 'RTSS nao encontrado - a ponte vai cair no modo console'
        return
    }
    Write-Step 'Subindo o RTSS'
    Start-Process -FilePath $candidates[0] | Out-Null
    for ($i = 0; $i -lt 20; $i++) {
        Start-Sleep -Milliseconds 500
        if (Get-Process -Name 'RTSS' -ErrorAction SilentlyContinue) {
            Write-Ok 'RTSS no ar'
            return
        }
    }
    Write-Warn2 'RTSS demorou a subir - seguindo mesmo assim'
}

function Get-PythonCommand {
    foreach ($c in @('python', 'py')) {
        $cmd = Get-Command $c -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
    }
    throw 'python nao esta no PATH - instale o Python 3 e marque "Add to PATH"'
}

# ---------------------------------------------------------------------------
# Execucao
# ---------------------------------------------------------------------------

if (-not (Test-Path $bridgePath)) {
    throw "qz_rtss_bridge.py nao esta na mesma pasta do script ($scriptDir)"
}

$rouvyTarget = Resolve-Rouvy -Explicit $Rouvy
Write-Ok "Rouvy: $rouvyTarget"
@{ Rouvy = $rouvyTarget } | ConvertTo-Json | Set-Content $configPath -Encoding UTF8

Start-RtssIfNeeded

$python = Get-PythonCommand
Write-Step "Subindo a ponte QZ ($QzHost)"

# -u para a saida nao ficar presa no buffer do pipe
$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName               = $python
$psi.Arguments              = ('-u "{0}" --host {1} --max-gear {2} --max-resistance {3} --stop-on-stdin-eof' -f
                               $bridgePath, $QzHost, $MaxGear, $MaxResistance)
$psi.UseShellExecute        = $false
$psi.RedirectStandardOutput = $true
$psi.RedirectStandardInput  = $true
# stderr fica sem redirecionar de proposito: aparece direto no console e nao
# corre risco de encher um pipe que ninguem esta lendo.
$psi.RedirectStandardError  = $false

$bridge = [System.Diagnostics.Process]::Start($psi)

# Leitura assincrona no proprio thread: um runspace separado teria o Write-Host
# indo para outro host, e a saida da ponte nunca apareceria para o usuario.
$io = @{
    Reader    = $bridge.StandardOutput
    Pending   = $null
    Connected = $false
    Eof       = $false
}
$io.Pending = $io.Reader.ReadLineAsync()

function Invoke-Pump {
    param($io, [int] $TimeoutMs = 200)
    while ($true) {
        if ($io.Eof) { Start-Sleep -Milliseconds $TimeoutMs; return }
        if (-not $io.Pending.Wait($TimeoutMs)) { return }
        $line = $io.Pending.Result
        if ($null -eq $line) { $io.Eof = $true; return }
        Write-Host "    [ponte] $line" -ForegroundColor DarkGray
        if ($line -match 'conectado em') { $io.Connected = $true }
        $io.Pending = $io.Reader.ReadLineAsync()
    }
}

try {
    Write-Step "Esperando o QZ responder (ate $ConnectTimeout s)"
    $deadline = (Get-Date).AddSeconds($ConnectTimeout)
    while (-not $io.Connected -and (Get-Date) -lt $deadline) {
        if ($bridge.HasExited) {
            Invoke-Pump $io 100
            throw "a ponte morreu (exit $($bridge.ExitCode)) - confira o --host e se o user_QZWS esta habilitado no QZ"
        }
        Invoke-Pump $io 300
    }

    if (-not $io.Connected) {
        if (-not $LaunchAnyway) {
            throw ("o QZ nao respondeu em $ConnectTimeout s. Confira: QZ aberto, " +
                   "Template Settings > user_QZWS habilitado, mesmo Wi-Fi, IP certo. " +
                   "Use -LaunchAnyway para abrir o Rouvy assim mesmo.")
        }
        Write-Warn2 'sem conexao com o QZ, abrindo o Rouvy assim mesmo (-LaunchAnyway)'
    } else {
        Write-Ok 'QZ conectado'
    }

    Write-Step 'Abrindo o Rouvy'
    if ($rouvyTarget -like 'shell:AppsFolder\*') {
        Start-Process 'explorer.exe' -ArgumentList $rouvyTarget
    } else {
        Start-Process -FilePath $rouvyTarget
    }

    # apps de Store nao devolvem handle, entao localizamos pelo nome
    $proc = $null
    for ($i = 0; $i -lt 60; $i++) {
        Invoke-Pump $io 1000
        $proc = Get-Process -Name '*ouvy*' -ErrorAction SilentlyContinue |
                Select-Object -First 1
        if ($proc) { break }
    }

    if ($proc) {
        Write-Ok "Rouvy rodando (PID $($proc.Id)). Ctrl+C aqui encerra a ponte."
        Write-Host ''
        # segue drenando a saida da ponte enquanto o Rouvy roda
        while (-not $proc.HasExited) { Invoke-Pump $io 500 }
        Write-Step 'Rouvy fechou'
    } else {
        Write-Warn2 'nao consegui achar o processo do Rouvy; a ponte segue ate voce dar Ctrl+C'
        while (-not $bridge.HasExited) { Invoke-Pump $io 500 }
    }
}
finally {
    Write-Step 'Encerrando a ponte'
    if (-not $bridge.HasExited) {
        # fechar a stdin faz a ponte sair pelo caminho limpo e liberar o slot
        try { $bridge.StandardInput.Close() } catch {}
        if (-not $bridge.WaitForExit(5000)) {
            Write-Warn2 'a ponte nao saiu sozinha, encerrando a forca'
            try { $bridge.Kill() } catch {}
        }
    }
    try { Invoke-Pump $io 200 } catch {}   # ultimas linhas, inclusive "slot liberado"
    Write-Ok 'pronto'
}
