@echo off
setlocal enabledelayedexpansion

:: Define the source directory
set SRC_DIR=%~dp0source
set COMPILED_DIR=%~dp0compiled
set SHADERCROSS_DIR=%~dp0

:: Ensure the output directories exist
mkdir "%COMPILED_DIR%\SPIRV" 2>nul
mkdir "%COMPILED_DIR%\MSL" 2>nul
mkdir "%COMPILED_DIR%\DXIL" 2>nul

:: Process .vert.hlsl files
for %%f in (%SRC_DIR%\*.vert.hlsl) do (
    if exist "%%f" (
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\SPIRV\%%~nf.spv"
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\MSL\%%~nf.msl"
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\DXIL\%%~nf.dxil"
    )
)

:: Process .frag.hlsl files
for %%f in (%SRC_DIR%\*.frag.hlsl) do (
    if exist "%%f" (
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\SPIRV\%%~nf.spv"
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\MSL\%%~nf.msl"
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\DXIL\%%~nf.dxil"
    )
)

:: Process .comp.hlsl files
for %%f in (%SRC_DIR%\*.comp.hlsl) do (
    if exist "%%f" (
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\SPIRV\%%~nf.spv"
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\MSL\%%~nf.msl"
        %SHADERCROSS_DIR%shadercross.exe "%%f" -o "%COMPILED_DIR%\DXIL\%%~nf.dxil"
    )
)

echo Done compiling shaders

endlocal
