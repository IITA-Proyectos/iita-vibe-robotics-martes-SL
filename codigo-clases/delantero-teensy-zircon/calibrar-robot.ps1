# =====================================================================
#  CALIBRAR-ROBOT - el delantero, en una cancha que no conocemos
# =====================================================================
#
#  PARA QUE ES. Llegas a una competencia o a una cancha nueva y hay que
#  recalibrar antes de jugar. Esto te dice QUE medir y EN QUE ORDEN, y lo
#  carga. Es la puerta de entrada "de competencia".
#
#  NO DUPLICA NADA. Toda la plomeria (buscar el robot, comprobar que sea el
#  DELANTERO y no el arquero, compilar, cargar, leer el puerto y guardar la
#  medicion en mediciones/) ya esta resuelta en medir-robot.ps1, que esta al
#  lado. Esto es un menu corto que lo llama con la opcion que corresponde.
#  Un solo motor, dos puertas de entrada: si se arregla un bug, se arregla
#  para los dos.
#
#  LOS TRES .BAT, PARA NO CONFUNDIRSE:
#    CARGAR-ROBOT.bat     el PARTIDO. Arco fijo azul o amarillo.
#    MEDIR-ROBOT.bat      las 14 pruebas, de a una, para investigar.
#    CALIBRAR-ROBOT.bat   ESTE. Cancha nueva: que medir y en que orden.
#
#  EL FLUJO SIN CABLE, que es el que importa en la cancha:
#    1. aca elegis la medicion y se carga
#    2. con la BATERIA PUESTA, desenchufas el USB
#       (si se corta la energia el Teensy se reinicia y se pierde todo)
#    3. llevas el robot a la cancha y segui lo que te pide
#    4. volves, enchufas el USB SIN APAGAR LA BATERIA
#    5. opcion L: lee lo que el robot guardo y lo deja en mediciones/
# =====================================================================

$ErrorActionPreference = "Stop"

$raiz  = $PSScriptRoot
$medir = Join-Path $raiz "medir-robot.ps1"
$cargar = Join-Path $raiz "CARGAR-ROBOT.bat"

function Linea { Write-Host ("=" * 68) }

if (-not (Test-Path $medir)) {
    Write-Host ""
    Write-Host "  XXX  No encuentro medir-robot.ps1 al lado de este script." -ForegroundColor White -BackgroundColor DarkRed
    Write-Host "       Tiene que estar en: $raiz"
    Write-Host ""
    Read-Host "  Enter para salir" | Out-Null
    exit 1
}

# El menu de competencia. "op" es lo que se teclea aca; "destino" es la opcion
# que entiende medir-robot.ps1. OJO: no llamar a esta lista $pasos y a otra
# cosa $PASOS -- PowerShell no distingue mayusculas y serian la misma variable.
# Ya nos paso con $pruebas/$PRUEBAS el 22/09.
$listaPasos = @(
  [pscustomobject]@{ op="1"; destino="14"; sinCable=$true;
                     que="TODO EN QUIETO: linea + giroscopio + camara (una sola corrida)";
                     por="EMPEZA ACA. Es lo que cambia al cambiar de cancha o de luz." }
  [pscustomobject]@{ op="2"; destino="1";  sinCable=$true;
                     que="Solo la LINEA, mas prolijo (5 puntos de verde, blanco por sensor, negro)";
                     por="Si la de arriba dio umbrales fragiles o imposibles." }
  [pscustomobject]@{ op="3"; destino="15"; sinCable=$true;
                     que="TODO MOVIENDO: piso de PWM, avanzar, girar, centrar, orbitar, escape";
                     por="EN EL PISO, el robot se mueve. Lo que cambia cuando cambia el PISO." }
  [pscustomobject]@{ op="4"; destino="11"; sinCable=$false;
                     que="CAMARA contra la cinta metrica: cuantos cm es un Xp";
                     por="Confirma que las distancias del programa son centimetros de verdad." }
  [pscustomobject]@{ op="5"; destino="12"; sinCable=$false;
                     que="PATADA: cuantos grados se tuerce al patear";
                     por="Si la pelota sale torcida. Decide entre el trim y el corrector." }
)

while ($true) {
    Write-Host ""
    Linea
    Write-Host "   DELANTERO  -  CALIBRAR para una cancha nueva" -ForegroundColor Cyan
    Linea
    Write-Host "   Ninguna de estas opciones juega. Para el partido es CARGAR-ROBOT.bat." -ForegroundColor Yellow
    Write-Host ""

    foreach ($p in $listaPasos) {
        $marca = "  "
        if ($p.sinCable) { $marca = "*" }
        Write-Host ("  {0,3}) {1} {2}" -f $p.op, $marca, $p.que)
        Write-Host ("        {0}" -f $p.por) -ForegroundColor DarkGray
    }

    Write-Host ""
    Write-Host "    L)   LEER lo que el robot tiene para decir (volviste de la cancha)" -ForegroundColor Cyan
    Write-Host "    P)   PASAR AL PARTIDO: abre CARGAR-ROBOT.bat (arco fijo)" -ForegroundColor Green
    Write-Host "    S)   salir"
    Write-Host ""
    Write-Host "   * = no necesita el cable: graba con la bateria y se lee despues" -ForegroundColor DarkGray
    Write-Host ""
    Write-Host "   ANTES DE JUGAR, acordate de:" -ForegroundColor Yellow
    Write-Host "     - copiar los umbrales nuevos al PANEL DE CONTROL del firmware"
    Write-Host "     - y cargar el programa de PARTIDO con CARGAR-ROBOT.bat (opcion P)"
    Write-Host ""

    $resp = Read-Host "   Que hacemos"
    if ($null -eq $resp) { Write-Host "   (sin consola interactiva) Salgo."; exit 0 }
    $op = $resp.Trim().ToUpper()

    if ($op -eq "S") { exit 0 }

    if ($op -eq "P") {
        if (Test-Path $cargar) {
            Write-Host ""
            Write-Host "   Abriendo CARGAR-ROBOT.bat ..." -ForegroundColor Green
            & $cargar
            exit $LASTEXITCODE
        }
        Write-Host "   No encuentro CARGAR-ROBOT.bat en $raiz" -ForegroundColor Red
        continue
    }

    if ($op -eq "L") {
        & powershell -NoProfile -ExecutionPolicy Bypass -File $medir -Opcion "L"
        continue
    }

    $elegido = $listaPasos | Where-Object { $_.op -eq $op }
    if (-not $elegido) {
        Write-Host "   '$op' no esta en la lista. De nuevo." -ForegroundColor Red
        continue
    }

    Write-Host ""
    Write-Host "   -> " -NoNewline
    Write-Host "  $($elegido.que)  " -ForegroundColor Black -BackgroundColor Yellow
    if ($elegido.sinCable) {
        Write-Host ""
        Write-Host "   Esta graba SIN CABLE. Cuando termine de cargar:" -ForegroundColor Cyan
        Write-Host "     1. deja la BATERIA PUESTA y recien ahi desenchufa el USB"
        Write-Host "     2. llevalo a la cancha y segui lo que pide el robot"
        Write-Host "     3. volve, enchufa el USB SIN APAGAR LA BATERIA, y usa la opcion L"
    }
    Write-Host ""

    & powershell -NoProfile -ExecutionPolicy Bypass -File $medir -Opcion $elegido.destino

    Write-Host ""
    Read-Host "   Enter para volver al menu de calibracion" | Out-Null
}
