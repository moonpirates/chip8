@echo off
setlocal enabledelayedexpansion

:: Define the source directory
set "SRC_DIR=Source"

:: Ensure the output directories exist
mkdir "Compiled\SPIRV" 2>nul
mkdir "Compiled\MSL" 2>nul
mkdir "Compiled\DXIL" 2>nul

:: Process .vert.hlsl files
for %%f in (%SRC_DIR%\*.vert.hlsl) do (
    if exist "%%f" (
        shadercross.exe "%%f" -o "Compiled\SPIRV\%%~nf.spv"
        shadercross.exe "%%f" -o "Compiled\MSL\%%~nf.msl"
        shadercross.exe "%%f" -o "Compiled\DXIL\%%~nf.dxil"
    )
)

:: Process .frag.hlsl files
for %%f in (%SRC_DIR%\*.frag.hlsl) do (
    if exist "%%f" (
        shadercross.exe "%%f" -o "Compiled\SPIRV\%%~nf.spv"
        shadercross.exe "%%f" -o "Compiled\MSL\%%~nf.msl"
        shadercross.exe "%%f" -o "Compiled\DXIL\%%~nf.dxil"
    )
)

:: Process .comp.hlsl files
for %%f in (%SRC_DIR%\*.comp.hlsl) do (
    if exist "%%f" (
        shadercross.exe "%%f" -o "Compiled\SPIRV\%%~nf.spv"
        shadercross.exe "%%f" -o "Compiled\MSL\%%~nf.msl"
        shadercross.exe "%%f" -o "Compiled\DXIL\%%~nf.dxil"
    )
)

endlocal
