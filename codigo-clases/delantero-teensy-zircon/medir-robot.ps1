# =====================================================================
#  MEDIR-ROBOT - carga en el DELANTERO un programa de MEDICION
#  IITA - Roboliga 2026 - 2026-09-22
# =====================================================================
#
#  Se usa con doble clic en MEDIR-ROBOT.bat (esta al lado).
#
#  EL NOMBRE ES A PROPOSITO. Antes se llamaba CALIBRAR-ROBOT y se
#  confundia con CARGAR-ROBOT en la carpeta: las dos empiezan con "CA".
#  MEDIR vs CARGAR no se parecen ni de reojo. [pedido del equipo 22/09]
#
#  HERMANO DE CARGAR-ROBOT.bat, PERO SEPARADO A PROPOSITO. Aquel carga los
#  programas de PARTIDO y ya gano partidos: no se toca. Este carga los de
#  MEDIR. Ninguno de los de aca juega.
#
#  QUE HACE Y QUE NO HACE:
#    SI   - revisa que haya UN solo robot y que sea el DELANTERO
#    SI   - compila y carga la prueba que elijas
#    SI   - te dice si esa prueba necesita el cable o no
#    SI   - con la opcion L, lee lo que el robot tiene para decir y lo
#           GUARDA en un archivo antes de mostrarlo
#    NO   - no escribe los valores medidos en el programa principal.
#           Eso se hace a mano, mirando el veredicto. [pedido del equipo,
#           22/09: "despues mas adelante vemos lo de que guarde solo"]
#
#  EL FLUJO SIN CABLE (el de grabar-linea, que es el importante):
#    1. aca: elegis la prueba, se carga, y EMPIEZA A GRABAR sola
#    2. prendes la bateria y RECIEN AHI desenchufas el USB
#       (si se corta la alimentacion, el Teensy se reinicia y se pierde todo)
#    3. llevas el robot a la cancha y haces la secuencia
#    4. volves, enchufas el USB, y corres esto de nuevo con la opcion L
# =====================================================================

param([string]$Opcion = "")

$ErrorActionPreference = "Stop"

$SERIE_DELANTERO = "15708680"
$raiz    = $PSScriptRoot
# OJO: NO llamarla $pruebas. PowerShell no distingue mayusculas, asi que
# $pruebas y $PRUEBAS (la lista del menu, mas abajo) serian LA MISMA
# variable, y la lista le pisaria la ruta. Ya paso. [22/09]
$dirPruebas = Join-Path $raiz "pruebas"
$pio     = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\pio.exe"
$loader  = Join-Path $env:USERPROFILE ".platformio\packages\tool-teensy\teensy.exe"

function Linea { Write-Host ("=" * 64) }

function Error-Y-Salir([string]$msg) {
    Write-Host ""
    Write-Host "  XXX  $msg" -ForegroundColor White -BackgroundColor DarkRed
    Write-Host ""
    exit 1
}

# En modo carga (PID 0478) la Teensy informa el numero de serie en HEXADECIMAL
# y dividido por 10. Misma regla que en cargar-robot.ps1.
function Serie-Decimal([string]$s) {
    try { $n = [Convert]::ToUInt64($s, 16) } catch { return $null }
    if ($n -lt 10000000) { $n = $n * 10 }
    return "$n"
}

function Buscar-Teensys {
    $lista = @()
    Get-CimInstance Win32_PnPEntity -ErrorAction SilentlyContinue |
      Where-Object { $_.PNPDeviceID -like "*VID_16C0*" } |
      ForEach-Object {
        if ($_.PNPDeviceID -match "VID_16C0&PID_(\w{4})\\(\w+)$") {
            $lista += [pscustomobject]@{ ProductId = $matches[1]; Serie = $matches[2] }
        }
      }
    return $lista
}

function Buscar-Puerto {
    $d = Get-CimInstance Win32_PnPEntity -ErrorAction SilentlyContinue |
         Where-Object { $_.PNPDeviceID -like "*VID_16C0*" -and $_.Name -match "\(COM\d+\)" }
    if (-not $d) { return $null }
    return "COM" + [regex]::Match($d[0].Name, "COM(\d+)").Groups[1].Value
}

# ---------------------------------------------------------------------
# EL MENU. Cada prueba dice para que sirve y si necesita el cable.
# 'tecla' es lo que se manda por serie para que vuelque (los que graban).
# ---------------------------------------------------------------------
$PRUEBAS = @(
  [pscustomobject]@{ n="1";  carpeta="grabar-linea";        cable=$false; tecla=$true;
                     que="LINEA: verde, blanco y negro, y propone los 3 umbrales";
                     como="EL IMPORTANTE. Bateria puesta, desenchufa, 5 puntos de verde + blanco por sensor + negro, volve y usa la opcion L." }
  [pscustomobject]@{ n="2";  carpeta="identificar-sensores"; cable=$true;  tecla=$false;
                     que="LINEA: que numero de sensor es cada posicion fisica";
                     como="En la mesa. Hacen falta una muestra de blanco y una de negro." }
  [pscustomobject]@{ n="3";  carpeta="giroscopo-crudo";      cable=$true;  tecla=$false;
                     que="GIROSCOPO: que chip es y si la fusion esta corriendo";
                     como="En la mesa. El que descubrio que nunca estuvo roto." }
  [pscustomobject]@{ n="4";  carpeta="rumbo-vivo";           cable=$true;  tecla=$false;
                     que="GIROSCOPO: el rumbo en vivo, sin mover motores";
                     como="En la mesa. Giralo a mano y mira que el rumbo acompane y vuelva." }
  [pscustomobject]@{ n="5";  carpeta="giroscopo-recupera";   cable=$true;  tecla=$false;
                     que="GIROSCOPO: si el lazo recupera de un empujon";
                     como="EN EL PISO: mueve los motores. Empujalo a proposito." }
  [pscustomobject]@{ n="6";  carpeta="signos";               cable=$true;  tecla=$false;
                     que="GIROSCOPO: los tres signos (giro, orbita, camara)";
                     como="EN EL PISO: mueve los motores." }
  [pscustomobject]@{ n="7";  carpeta="piso-de-pwm";          cable=$true;  tecla=$false;
                     que="MOTORES: con cuanta potencia arranca cada rueda";
                     como="EN EL PISO. OJO: tiene los pines del ARQUERO, hay que corregirlo antes." }
  [pscustomobject]@{ n="8";  carpeta="motores-a-mano";       cable=$true;  tecla=$false;
                     que="MOTORES: manejar cada uno a mano, sin secuencias";
                     como="EN EL PISO. Vos mandas por el monitor." }
  [pscustomobject]@{ n="9";  carpeta="diagnostico-motores";  cable=$true;  tecla=$false;
                     que="MOTORES: los 3, en los dos sentidos";
                     como="EN EL PISO." }
  [pscustomobject]@{ n="10"; carpeta="quien-es-quien";       cable=$true;  tecla=$false;
                     que="MOTORES: que rueda cuelga de que par de pines";
                     como="EN EL PISO. Mueve dos y apaga una." }
  [pscustomobject]@{ n="11"; carpeta="tabla-camara";         cable=$true;  tecla=$false;
                     que="CAMARA: cuantos cm es un Xp (cierra XP_ORBITA)";
                     como="En la mesa, SIN motores. Pendiente desde el 25/08." }
  [pscustomobject]@{ n="12"; carpeta="patada-derecha";       cable=$true;  tecla=$false;
                     que="PATADA: cuantos grados se tuerce al patear";
                     como="EN EL PISO. OJO: mide 240x1000 ms y el firmware patea 215x420." }
  [pscustomobject]@{ n="13"; carpeta="identificar-robot";    cable=$true;  tecla=$false;
                     que="ES EL ARQUERO O EL DELANTERO?";
                     como="En la mesa." }
)

# ---------------------------------------------------------------------
# LEER EL ROBOT. Guarda SIEMPRE en archivo antes de mostrar: ya se perdio
# una medicion entera por mostrarla recortada sin guardarla. [08/09]
# ---------------------------------------------------------------------
function Leer-Robot([bool]$mandarTecla, [int]$segundos) {
    $puerto = Buscar-Puerto
    if (-not $puerto) { Error-Y-Salir "El robot no aparece por USB. Enchufa el cable." }
    Write-Host "      leyendo $puerto durante $segundos s..." -ForegroundColor DarkGray
    $sp = New-Object System.IO.Ports.SerialPort $puerto, 19200, None, 8, one
    $sp.ReadTimeout = 100
    $sp.DtrEnable = $true
    $sp.ReadBufferSize = 262144
    try { $sp.Open() } catch { Error-Y-Salir "No pude abrir $puerto : $_" }
    Start-Sleep -Milliseconds 250
    if ($mandarTecla) { $sp.Write("`r`n") }
    $sb = New-Object System.Text.StringBuilder
    $fin = [DateTime]::UtcNow.AddSeconds($segundos)
    while ([DateTime]::UtcNow -lt $fin) {
        try { $t = $sp.ReadExisting(); if ($t) { [void]$sb.Append($t) } } catch { }
        Start-Sleep -Milliseconds 10
    }
    try { $sp.Close() } catch { }
    $texto = $sb.ToString()

    $sello   = (Get-Date).ToString("yyyy-MM-dd_HH-mm-ss")
    $carpeta = Join-Path $raiz "mediciones"
    if (-not (Test-Path $carpeta)) { New-Item -ItemType Directory -Path $carpeta | Out-Null }
    $archivo = Join-Path $carpeta "medicion-$sello.txt"
    $texto | Out-File -FilePath $archivo -Encoding utf8

    Write-Host ""
    Linea
    if ($texto.Trim().Length -eq 0) {
        Write-Host "  El robot no dijo nada." -ForegroundColor Yellow
        Write-Host "  Si la prueba esperaba una tecla, proba de nuevo con la opcion L."
        Write-Host "  Si se corto la bateria al desenchufar, la grabacion se perdio."
    } else {
        Write-Host $texto
    }
    Linea
    Write-Host "  Guardado en: $archivo" -ForegroundColor Green
    Write-Host "  (mandaselo a quien te ayude con el codigo, o abrilo con el Bloc de notas)"
}

# ---------------------------------------------------------------------
Clear-Host
Linea
Write-Host "   DELANTERO  -  MEDIR (no juega)"
Linea
Write-Host "   Ninguno de estos programas juega: solo miden." -ForegroundColor Yellow
Write-Host "   Para el partido es CARGAR-ROBOT.bat, no este." -ForegroundColor Yellow
Linea

if (-not (Test-Path $pio))    { Error-Y-Salir "No encuentro PlatformIO en $pio" }
if (-not (Test-Path $loader)) { Error-Y-Salir "No encuentro el Teensy Loader en $loader" }

# ---- menu ----
while ($true) {
    if ($Opcion -ne "") { $op = $Opcion.Trim().ToUpper(); $Opcion = "" }
    else {
        Write-Host ""
        foreach ($p in $PRUEBAS) {
            $marca = "  "
            if (-not $p.cable) { $marca = "*" }
            Write-Host ("  {0,3}) {1} {2}" -f $p.n, $marca, $p.que)
        }
        Write-Host ""
        Write-Host "    L)   LEER lo que el robot tiene para decir (volviste de la cancha)" -ForegroundColor Cyan
        Write-Host "    S)   salir"
        Write-Host ""
        Write-Host "   * = no necesita el cable: graba con la bateria" -ForegroundColor DarkGray
        Write-Host ""
        $resp = Read-Host "   Que hacemos"
        if ($null -eq $resp) { Write-Host "   (sin consola interactiva) Salgo."; exit 0 }
        $op = $resp.Trim().ToUpper()
    }

    if ($op -eq "S") { exit 0 }
    if ($op -eq "L") {
        Write-Host ""
        Write-Host "[1/1] Leyendo el robot..."
        Leer-Robot $true 25
        Write-Host ""
        Read-Host "   Enter para volver al menu" | Out-Null
        continue
    }
    $elegida = $PRUEBAS | Where-Object { $_.n -eq $op }
    if ($elegida) { break }
    Write-Host "   '$op' no esta en la lista. De nuevo." -ForegroundColor Red
}

$proyecto = Join-Path $dirPruebas $elegida.carpeta
if (-not (Test-Path (Join-Path $proyecto "platformio.ini"))) {
    Error-Y-Salir "No encuentro el proyecto en $proyecto"
}

Write-Host ""
Linea
Write-Host "   Voy a cargar: " -NoNewline
Write-Host "  $($elegida.que)  " -ForegroundColor Black -BackgroundColor Yellow
Write-Host ""
Write-Host "   $($elegida.como)"
Linea
Write-Host ""
Write-Host "   Antes: BATERIA APAGADA si la prueba NO mueve motores." -ForegroundColor Yellow
Write-Host "   Y SOLO este robot enchufado: el cargador no elige." -ForegroundColor Yellow
$resp = Read-Host "   Sigo? (s/n)"
if ($null -eq $resp) { exit 0 }
$ok = $resp.Trim().ToLower()
if ($ok -ne "s") { Write-Host "   Cancelado."; exit 0 }

# ---- 1. el robot ----
Write-Host ""
Write-Host "[1/3] Buscando el robot..."
$teensys = @(Buscar-Teensys)
if ($teensys.Count -eq 0) { Error-Y-Salir "No hay ningun robot enchufado por USB." }
if ($teensys.Count -gt 1) {
    Error-Y-Salir "Hay $($teensys.Count) robots enchufados. Deja SOLO el delantero: el cargador no elige y podria cargar el arquero."
}
$t = $teensys[0]
if ($t.ProductId -eq "0478") {
    $serieBoot = Serie-Decimal $t.Serie
    if ($serieBoot -eq $SERIE_DELANTERO) {
        Write-Host "      OK: es el DELANTERO (serie $SERIE_DELANTERO), en modo carga" -ForegroundColor Green
    } elseif ($serieBoot) {
        Error-Y-Salir "El robot en modo carga tiene serie $serieBoot y el delantero es $SERIE_DELANTERO. Es el ARQUERO? No cargo nada."
    } else {
        $ok = (Read-Host "      No pude leer la serie. Es el DELANTERO y esta solo? (s/n)").Trim().ToLower()
        if ($ok -ne "s") { Error-Y-Salir "Cancelado." }
    }
} elseif ($t.Serie -ne $SERIE_DELANTERO) {
    Error-Y-Salir "El robot enchufado tiene serie $($t.Serie) y el delantero es $SERIE_DELANTERO. Es el ARQUERO? No cargo nada."
} else {
    Write-Host "      OK: es el DELANTERO (serie $SERIE_DELANTERO)" -ForegroundColor Green
}

# ---- 2. loader ----
Write-Host "[2/3] Teensy Loader..."
if (-not (Get-Process teensy -ErrorAction SilentlyContinue)) {
    Start-Process $loader
    Start-Sleep -Seconds 2
}
Write-Host "      OK: abierto" -ForegroundColor Green

# ---- 3. cargar ----
Write-Host "[3/3] Compilando y cargando (la primera vez tarda ~1 min)..."
$logOut = Join-Path $env:TEMP "medir-robot-out.txt"
$logErr = Join-Path $env:TEMP "medir-robot-err.txt"
$p = Start-Process -FilePath $pio -ArgumentList @("run", "-t", "upload") `
        -WorkingDirectory $proyecto -NoNewWindow -PassThru `
        -RedirectStandardOutput $logOut -RedirectStandardError $logErr
$null = $p.Handle
if (-not $p.WaitForExit(240000)) {
    try { $p.Kill() } catch {}
    Error-Y-Salir "La carga se colgo (4 min). Apreta el boton blanco de la Teensy y proba de nuevo."
}
$p.WaitForExit()
if ($p.ExitCode -ne 0) {
    Write-Host ""
    Write-Host "---- lo ultimo que dijo el cargador ----" -ForegroundColor Yellow
    Get-Content $logOut -Tail 20 -ErrorAction SilentlyContinue
    Get-Content $logErr -Tail 20 -ErrorAction SilentlyContinue
    Error-Y-Salir "La carga FALLO (codigo $($p.ExitCode)). El robot sigue con el programa que tenia."
}
Write-Host "      OK: cargado" -ForegroundColor Green

# El Loader queda en modo Auto con este programa adentro: si despues se pone
# el ARQUERO en modo carga, se lo graba. Mismo cuidado que en cargar-robot.
Get-Process teensy -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue

Write-Host ""
Linea
if ($elegida.cable) {
    Write-Host "   LISTO. Esta prueba NECESITA EL CABLE." -ForegroundColor Green
    Write-Host ""
    Write-Host "   $($elegida.como)"
    Write-Host ""
    Write-Host "   Cuando la hayas hecho, corre esto de nuevo y elegi L para leerla."
} else {
    Write-Host "   LISTO. Esta prueba GRABA SOLA, sin cable." -ForegroundColor Green
    Write-Host ""
    Write-Host "   1. PRENDE LA BATERIA" -ForegroundColor Yellow
    Write-Host "   2. recien ahi DESENCHUFA el USB" -ForegroundColor Yellow
    Write-Host "      (si se corta la alimentacion se reinicia y se pierde todo)"
    Write-Host "   3. el LED de la placa parpadea: mientras parpadea, graba"
    Write-Host "   4. $($elegida.como)"
    Write-Host "   5. volve, enchufa el USB, corre esto de nuevo y elegi L"
}
Linea
Write-Host ""

$resp = Read-Host "   Queres leer el robot ahora? (s/n)"
$ver = ""
if ($null -ne $resp) { $ver = $resp.Trim().ToLower() }
if ($ver -eq "s") { Leer-Robot $elegida.tecla 20 }

Write-Host ""
exit 0
