# =====================================================================
#  CARGAR-ROBOT - carga en el DELANTERO el programa del arco que toque
#  IITA - Roboliga 2026 - 2026-09-21
# =====================================================================
#
#  Se usa con doble clic en CARGAR-ROBOT.bat (esta al lado). Pregunta
#  1 = ARCO AZUL o 2 = ARCO AMARILLO, y hace todo solo:
#
#    1. revisa que haya UN solo robot enchufado y que sea el DELANTERO
#       (numero de serie de la Teensy 15708680). Si es el arquero, NO carga.
#    2. abre la app Teensy Loader si no estaba abierta
#    3. compila y carga funciona/delantero con el arco elegido
#       (pio run -e azul / -e amarillo)
#    4. lee el banner del robot y CONFIRMA que dice el arco elegido.
#       El "SUCCESS" del cargador no prueba nada; lo que dice el robot si.
#
#  Tambien se puede llamar con el arco:  CARGAR-ROBOT.bat azul
#
#  Por que dos programas: al arrancar, los robots rivales tapan el arco y
#  el robot no llegaba a elegirlo mirando. El juez permite tener dos.
# =====================================================================

param([string]$Arco = "")

$ErrorActionPreference = "Stop"

$SERIE_DELANTERO = "15708680"
$proyecto = Join-Path $PSScriptRoot "funciona\delantero"
$pio      = Join-Path $env:USERPROFILE ".platformio\penv\Scripts\pio.exe"
$loader   = Join-Path $env:USERPROFILE ".platformio\packages\tool-teensy\teensy.exe"

function Linea { Write-Host ("=" * 60) }

# Despues de cargar, el Teensy Loader queda en modo "Auto" con el programa del
# DELANTERO adentro: si en esta PC despues se pone el ARQUERO en modo carga, el
# Loader le graba ese programa. Por eso se cierra al terminar. [revision 21/09]
$script:yaCargue = $false
function Cerrar-Loader {
    Get-Process teensy -ErrorAction SilentlyContinue | Stop-Process -Force -ErrorAction SilentlyContinue
}

function Error-Y-Salir([string]$msg) {
    if ($script:yaCargue) { Cerrar-Loader }
    Write-Host ""
    Write-Host "  XXX  $msg" -ForegroundColor White -BackgroundColor DarkRed
    Write-Host ""
    exit 1
}

# En modo carga (PID 0478) la Teensy informa el numero de serie en HEXADECIMAL
# y dividido por 10: 0017F834 -> 1570868 -> x10 = 15708680. Es la regla de
# Teensyduino (los menores a 10.000.000 se multiplican por 10). Verificado con
# el registro de esta PC para el delantero y el arquero. [revision 21/09]
function Serie-Decimal([string]$s) {
    try { $n = [Convert]::ToUInt64($s, 16) } catch { return $null }
    if ($n -lt 10000000) { $n = $n * 10 }
    return "$n"
}

function Mostrar-Arco([string]$nombre, [string]$texto) {
    if ($nombre -eq "AZUL") {
        Write-Host "  $texto  " -ForegroundColor White -BackgroundColor DarkBlue
    } else {
        Write-Host "  $texto  " -ForegroundColor Black -BackgroundColor Yellow
    }
}

# Las Teensy enchufadas. El Id del dispositivo "padre" termina en el numero
# de serie: USB\VID_16C0&PID_0483\15708680. PID 0478 = modo carga (bootloader).
function Buscar-Teensys {
    $patron = '^USB\\VID_16C0&PID_([0-9A-F]{4})\\([^\\]+)$'
    @(Get-CimInstance Win32_PnPEntity |
        Where-Object { $_.PNPDeviceID -match $patron } |
        ForEach-Object {
            $null = $_.PNPDeviceID -match $patron
            [pscustomobject]@{ ProductId = $Matches[1]; Serie = $Matches[2] }
        })
}

# El puerto COM de la Teensy (su interfaz serie es la MI_00).
function Buscar-Puerto {
    $d = Get-CimInstance Win32_PnPEntity |
        Where-Object { $_.PNPDeviceID -like 'USB\VID_16C0&PID_0483&MI_00*' -and $_.Name -match '\(COM\d+\)' } |
        Select-Object -First 1
    if ($d -and ($d.Name -match '\((COM\d+)\)')) { return $Matches[1] }
    return $null
}

# ---------------------------------------------------------------- menu
Clear-Host
Linea
Write-Host "   DELANTERO  -  cargar programa de partido"
Linea
Write-Host "   Antes: BATERIA DEL ROBOT APAGADA (si no, sale andando" -ForegroundColor Yellow
Write-Host "   apenas termina de cargar), USB enchufado, y SOLO este robot." -ForegroundColor Yellow

$Arco = $Arco.Trim().ToLower()
if ($Arco -eq "1") { $Arco = "azul" }
if ($Arco -eq "2") { $Arco = "amarillo" }

while ($Arco -ne "azul" -and $Arco -ne "amarillo") {
    Write-Host ""
    Write-Host "   1) " -NoNewline; Mostrar-Arco "AZUL" "atacar el ARCO AZUL"
    Write-Host "   2) " -NoNewline; Mostrar-Arco "AMARILLO" "atacar el ARCO AMARILLO"
    Write-Host ""
    $op = (Read-Host "   Elegi 1 o 2 y apreta Enter").Trim()
    if ($op -eq "1") { $Arco = "azul" }
    elseif ($op -eq "2") { $Arco = "amarillo" }
    else { Write-Host "   '$op' no es 1 ni 2. De nuevo." -ForegroundColor Red }
}
$NOMBRE = $Arco.ToUpper()

Write-Host ""
Write-Host "   Voy a cargar: " -NoNewline; Mostrar-Arco $NOMBRE "ARCO $NOMBRE"
Write-Host ""

# ---------------------------------------------------------------- herramientas
if (-not (Test-Path $pio))      { Error-Y-Salir "No encuentro PlatformIO en $pio" }
if (-not (Test-Path $loader))   { Error-Y-Salir "No encuentro el Teensy Loader en $loader" }
if (-not (Test-Path (Join-Path $proyecto "platformio.ini"))) { Error-Y-Salir "No encuentro el proyecto en $proyecto" }

# ---------------------------------------------------------------- 1. el robot
Write-Host "[1/4] Buscando el robot..."
# El @() va ACA y no solo adentro de la funcion: PowerShell desarma los
# arreglos al devolverlos, y con un solo robot .Count daria vacio.
$teensys = @(Buscar-Teensys)
if ($teensys.Count -eq 0) {
    Error-Y-Salir "No hay ningun robot enchufado por USB."
}
if ($teensys.Count -gt 1) {
    Error-Y-Salir "Hay $($teensys.Count) robots enchufados. Deja SOLO el delantero: el cargador no elige y podria cargar el arquero."
}
$t = $teensys[0]
if ($t.ProductId -eq "0478") {
    $serieBoot = Serie-Decimal $t.Serie
    if ($serieBoot -eq $SERIE_DELANTERO) {
        Write-Host "      OK: es el DELANTERO (serie $SERIE_DELANTERO), en modo carga" -ForegroundColor Green
    } elseif ($serieBoot) {
        Error-Y-Salir "El robot enchufado (en modo carga) tiene numero de serie $serieBoot y el delantero es $SERIE_DELANTERO. Es el ARQUERO? No cargo nada."
    } else {
        Write-Host "      El robot esta en MODO CARGA y no pude leer su numero de serie ('$($t.Serie)')." -ForegroundColor Yellow
        $ok = (Read-Host "      Es el DELANTERO y es el unico enchufado? (s/n)").Trim().ToLower()
        if ($ok -ne "s") { Error-Y-Salir "Cancelado." }
    }
} elseif ($t.Serie -ne $SERIE_DELANTERO) {
    Error-Y-Salir "El robot enchufado tiene numero de serie $($t.Serie) y el delantero es $SERIE_DELANTERO. Es el ARQUERO? No cargo nada."
} else {
    Write-Host "      OK: es el DELANTERO (serie $SERIE_DELANTERO)" -ForegroundColor Green
}

# ---------------------------------------------------------------- 2. loader
Write-Host "[2/4] Teensy Loader..."
if (-not (Get-Process teensy -ErrorAction SilentlyContinue)) {
    Start-Process $loader
    Start-Sleep -Seconds 2
}
Write-Host "      OK: abierto" -ForegroundColor Green

# ---------------------------------------------------------------- 3. cargar
Write-Host "[3/4] Compilando y cargando ARCO $NOMBRE (la primera vez tarda ~1 min, despues ~10 s)..."
$script:yaCargue = $true   # desde aca, cualquier salida cierra el Loader
$logOut = Join-Path $env:TEMP "cargar-robot-out.txt"
$logErr = Join-Path $env:TEMP "cargar-robot-err.txt"
# -WorkingDirectory en vez de pasarle la ruta a pio: la ruta tiene espacios y
# Start-Process no los comilla solo.
$p = Start-Process -FilePath $pio -ArgumentList @("run", "-e", $Arco, "-t", "upload") `
        -WorkingDirectory $proyecto -NoNewWindow -PassThru `
        -RedirectStandardOutput $logOut -RedirectStandardError $logErr
$null = $p.Handle   # sin esto, en PowerShell 5.1 ExitCode puede quedar vacio
if (-not $p.WaitForExit(240000)) {
    try { $p.Kill() } catch {}
    Error-Y-Salir "La carga se colgo (4 min). Apreta el boton blanco de la Teensy y volve a correr esto."
}
$p.WaitForExit()
if ($p.ExitCode -ne 0) {
    Write-Host ""
    Write-Host "---- lo ultimo que dijo el cargador ----" -ForegroundColor Yellow
    Get-Content $logOut -Tail 20 -ErrorAction SilentlyContinue
    Get-Content $logErr -Tail 20 -ErrorAction SilentlyContinue
    Error-Y-Salir "La carga FALLO (codigo $($p.ExitCode)). El robot sigue con el programa que tenia."
}
Write-Host "      OK: el cargador dice SUCCESS (falta que lo confirme el robot)" -ForegroundColor Green

# ---------------------------------------------------------------- 4. confirmar
Write-Host "[4/4] Leyendo lo que dice el robot..."
$puerto = $null
for ($i = 0; $i -lt 30 -and -not $puerto; $i++) {
    $puerto = Buscar-Puerto
    if (-not $puerto) { Start-Sleep -Milliseconds 500 }
}
if (-not $puerto) {
    Error-Y-Salir "El robot no volvio a aparecer por USB despues de cargar. Desenchufa y enchufa, y mira el monitor serie."
}

# Segunda barrera: el que volvio despues de cargar tiene que ser el delantero.
$despues = @(Buscar-Teensys)
if ($despues.Count -ne 1 -or $despues[0].Serie -ne $SERIE_DELANTERO) {
    Error-Y-Salir "Despues de cargar, el robot enchufado NO es el delantero ($SERIE_DELANTERO). Revisar que robot quedo cargado."
}

# Abrir con reintentos: Windows a veces muestra el COM un instante antes de
# dejarlo abrir, y eso no es "un monitor serie abierto". [revision 21/09]
$texto = ""
$sp = New-Object System.IO.Ports.SerialPort $puerto, 19200
$sp.DtrEnable = $true
$abierto = $false
$ultimoError = ""
for ($i = 0; $i -lt 6 -and -not $abierto; $i++) {
    try { $sp.Open(); $abierto = $true }
    catch { $ultimoError = $_.Exception.Message; Start-Sleep -Milliseconds 600 }
}
if (-not $abierto) {
    Error-Y-Salir "No pude abrir $puerto para leer el robot ($ultimoError). Si hay un monitor serie abierto (Arduino IDE, PlatformIO), cerralo y proba de nuevo."
}
$reloj = [Diagnostics.Stopwatch]::StartNew()
while ($reloj.ElapsedMilliseconds -lt 12000) {
    try { $texto += $sp.ReadExisting() } catch { break }   # si se corto el USB, uso lo que llego
    if ($texto -match "ARCO: \w+[^\r\n]*[\r\n]" -and $texto -match "Arranca en") { break }
    Start-Sleep -Milliseconds 100
}
try { $sp.Close() } catch {}   # cerrar un puerto USB que desaparecio tira error: no importa

$lineas = $texto -split "`r?`n"
$lineaArco = $lineas | Where-Object { $_ -match "^ARCO: " } | Select-Object -First 1
Write-Host ""
Write-Host "---- el robot dice ----"
$lineas | Where-Object { $_ -match "^ARCO: |^patada: |RETROCEDE YA|^Arranca en|ATACO EL ARCO|^Giroscopo" } | ForEach-Object { Write-Host "   $_" }
Write-Host "-----------------------"
Write-Host ""

Cerrar-Loader
if (-not $lineaArco) {
    Write-Host "  !!  El robot no dijo que arco tiene cargado. La carga dio SUCCESS, pero NO esta confirmado.  " -ForegroundColor Black -BackgroundColor Yellow
    Write-Host "      Abri el monitor serie (19200) y busca la linea 'ARCO:'."
    exit 2
}
if ($lineaArco -notmatch "^ARCO: $NOMBRE\s+\(fijo por programa\)") {
    Error-Y-Salir "EL ROBOT DICE '$($lineaArco.Trim())' Y ELEGISTE ARCO $NOMBRE. NO JUEGUES ASI: volve a cargar."
}

Linea
Mostrar-Arco $NOMBRE "LISTO: EL ROBOT ATACA EL ARCO $NOMBRE  (confirmado por el robot)"
Linea
Write-Host ""
Write-Host "  Ahora:"
Write-Host "   1. Desenchufa el USB."
Write-Host "   2. En la cancha, apoya el robot MIRANDO AL ARCO RIVAL."
Write-Host "   3. Recien ahi prende la bateria. Si ya estaba prendida: apagala y prendela."
Write-Host "      (al prender guarda hacia donde mira: es su plan B para patear)"
Write-Host ""
Write-Host "  Si cambian de lado en el entretiempo: correr esto de nuevo con el otro arco."
Write-Host ""
exit 0
