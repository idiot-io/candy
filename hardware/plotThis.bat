:: this (windows) batch script will turn your svg2shnzn .kicad_pcb file to a gerber
:: using kicad python and gerbv

:: SETUP:
:: you will now need to adapt the path to your kicad bin folder and gerbv.exe location. 

:: USAGE: drag the xxx.kicad_pcb file genearted by the export command of svg2shnzn pnto this script. 
:: i usally export my kicad_pcb file to a kicad folder under the hardware/board directory. 
:: but it can live anywhere
@echo off

set Path=C:/Program Files/KiCad/bin/
set gerbv_path=C:\bin\GerbVPortable\App\gerbv\bin

pushd %~dp0

echo first we delete the previous plot folder
rmdir /s %~dp1\plot
python.exe kicad_gerber_gen.py %1 %gerbv_path%

::uncomment pause to debug
::pause