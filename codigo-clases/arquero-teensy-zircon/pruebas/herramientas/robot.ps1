# Compilar y cargar sketches al Teensy 4.1 sin abrir el Arduino IDE.
#   .\robot.ps1 compilar pruebas\motores-a-mano
#   .\robot.ps1 cargar    pruebas\motores-a-mano
param(
    [Parameter(Mandatory = $true)][ValidateSet("compilar", "cargar")][string]$accion,
    [Parameter(Mandatory = $true)][string]$sketch
)

$cli  = "C:\Users\alumnos\Documents\OTTO\arduino\Arduino IDE\resources\app\lib\backend\resources\arduino-cli.exe"
$cfg  = "C:\Users\alumnos\.arduinoIDE\arduino-cli.yaml"
$base = "C:\Users\alumnos\IITA Martes Vibe Robotics\iita-vibe-robotics-martes-SL\codigo-clases\arquero-teensy-zircon"
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
