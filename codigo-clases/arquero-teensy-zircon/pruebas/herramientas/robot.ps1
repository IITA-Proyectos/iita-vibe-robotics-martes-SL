# Compilar y cargar sketches al Teensy 4.1 sin abrir el Arduino IDE.
#   .\robot.ps1 compilar pruebas\motores-a-mano
#   .\robot.ps1 cargar    pruebas\motores-a-mano
param(
    [Parameter(Mandatory = $true)][ValidateSet("compilar", "cargar")][string]$accion,
    [Parameter(Mandatory = $true)][string]$sketch
)

# El arduino-cli viene adentro del Arduino IDE 2.x. Se busca primero donde esta en la
# compu del taller (portable) y despues donde lo deja el instalador comun.
$cli  = @(
    "C:\Users\alumnos\Documents\OTTO\arduino\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe",
    "$env:ProgramFiles\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe",
    "$env:LOCALAPPDATA\Programs\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $cli) { Write-Output "No encuentro el arduino-cli del Arduino IDE 2.x"; exit 1 }
$cfg  = "$env:USERPROFILE\.arduinoIDE\arduino-cli.yaml"
# La carpeta del arquero: dos niveles arriba de este script (pruebas\herramientas).
$base = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$fqbn = "teensy:avr:teensy41"

$ruta = Join-Path $base $sketch
if (-not (Test-Path $ruta)) { Write-Output "No existe el sketch: $ruta"; exit 1 }

Write-Output "== compilando $sketch =="
& $cli compile --config-file $cfg --fqbn $fqbn $ruta
if ($LASTEXITCODE -ne 0) { Write-Output "== FALLO LA COMPILACION - no se carga nada =="; exit 1 }

if ($accion -eq "cargar") {
    $puerto = (& $cli board list --config-file $cfg |
               Select-String -Pattern $fqbn |
               ForEach-Object { ($_.Line -split '\s+')[0] } |
               Select-Object -First 1)
    if (-not $puerto) { Write-Output "== NO SE DETECTA NINGUN TEENSY CONECTADO =="; exit 1 }
    Write-Output "== cargando a $puerto =="
    & $cli upload --config-file $cfg --fqbn $fqbn -p $puerto $ruta
    if ($LASTEXITCODE -ne 0) { Write-Output "== FALLO LA CARGA =="; exit 1 }
    Write-Output "== CARGADO OK =="
}
