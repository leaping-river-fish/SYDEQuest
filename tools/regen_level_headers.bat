@echo off
REM Regenerate Pico embedded level headers from CSV. Run from repo root or adjust paths.
set ROOT=%~dp0..
python "%~dp0csv_to_binary_header.py" "%ROOT%\levels\Level1.csv" "%ROOT%\Pico\assets\level1_data.h" level1
if errorlevel 1 exit /b 1
python "%~dp0csv_to_binary_header.py" "%ROOT%\levels\Level2.csv" "%ROOT%\Pico\assets\level2_data.h" level2
if errorlevel 1 exit /b 1
python "%~dp0csv_to_binary_header.py" "%ROOT%\levels\Level3.csv" "%ROOT%\Pico\assets\level3_data.h" level3
if errorlevel 1 exit /b 1
echo Done. Review: git diff Pico/assets/level*_data.h
