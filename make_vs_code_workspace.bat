@echo off
setlocal EnableDelayedExpansion

REM ============================================================
REM  make_vs_code_workspace.bat
REM  Sets up the MSVC + Qt environment and launches VS Code.
REM
REM  Usage:
REM    make_vs_code_workspace.bat [options] [project_folder]
REM
REM  Options:
REM    -vs <version>   Visual Studio version to use.
REM                    Accepts a calendar year ("2022", "2019"),
REM                    a major version ("17", "16"), a version
REM                    prefix ("17.8"), or a full installation path.
REM                    If omitted and multiple installations are
REM                    found, an interactive menu is shown.
REM    -h, --help      Show this help.
REM
REM  Examples:
REM    make_vs_code_workspace.bat
REM    make_vs_code_workspace.bat D:\my\proj
REM    make_vs_code_workspace.bat -vs 2022 D:\my\proj
REM    make_vs_code_workspace.bat -vs 17
REM    make_vs_code_workspace.bat -vs "C:\Program Files\Microsoft Visual Studio\2022\Community"
REM ============================================================

set "PROJECT_DIR="
set "VS_VERSION="

REM ---------- Parse arguments ----------
:parse_args
if "%~1"=="" goto args_done
if /i "%~1"=="-h"     goto show_help
if /i "%~1"=="--help" goto show_help
if /i "%~1"=="-vs" (
    if "%~2"=="" (
        echo ERROR: -vs requires an argument.
        exit /b 1
    )
    set "VS_VERSION=%~2"
    shift
    shift
    goto parse_args
)
if /i "%~1"=="--vs" (
    if "%~2"=="" (
        echo ERROR: --vs requires an argument.
        exit /b 1
    )
    set "VS_VERSION=%~2"
    shift
    shift
    goto parse_args
)
if not defined PROJECT_DIR (
    set "PROJECT_DIR=%~1"
) else (
    echo ERROR: unexpected argument: %~1
    exit /b 1
)
shift
goto parse_args

:args_done

if not defined PROJECT_DIR set "PROJECT_DIR=%~dp0"
if "%PROJECT_DIR:~-1%"=="\" set "PROJECT_DIR=%PROJECT_DIR:~0,-1%"

if not exist "%PROJECT_DIR%\" (
    echo ERROR: project folder not found: %PROJECT_DIR%
    exit /b 1
)

echo.
echo ==========================================
echo  Project: %PROJECT_DIR%
if defined VS_VERSION echo  VS request: %VS_VERSION%
echo ==========================================
echo.

REM ---------- 1. Locate vswhere ----------
echo [1/6] Locating vswhere...

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found.
    echo Is Visual Studio 2017 or newer installed?
    exit /b 1
)

REM ---------- 2. Enumerate VS installations ----------
echo [2/6] Enumerating Visual Studio installations...

set /a VS_COUNT=0
for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -all -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set /a VS_COUNT+=1
    set "VS_PATH_!VS_COUNT!=%%i"
    for /f "usebackq tokens=*" %%v in (`"%VSWHERE%" -path "%%i" -property installationVersion`) do (
        set "VS_VER_!VS_COUNT!=%%v"
    )
)

if %VS_COUNT%==0 (
    echo ERROR: no Visual Studio installations with the C++ workload were found.
    echo Please install the "Desktop development with C++" component.
    exit /b 1
)

echo     Found %VS_COUNT% installation(s).

REM ---------- 3. Choose VS installation ----------
set "VS_PATH="

if defined VS_VERSION (
    call :match_vs "%VS_VERSION%"
    if not defined VS_PATH (
        echo ERROR: no installation matches "%VS_VERSION%".
        echo.
        call :list_vs
        exit /b 1
    )
) else if %VS_COUNT%==1 (
    set "VS_PATH=!VS_PATH_1!"
    set "VS_VER=!VS_VER_1!"
) else (
    echo.
    echo Multiple Visual Studio installations detected:
    call :list_vs
    echo.
    :prompt_vs
    set /p "VS_CHOICE=Select installation [1-%VS_COUNT%] (default 1): "
    if "!VS_CHOICE!"=="" set "VS_CHOICE=1"
    set "CHOICE_OK="
    for /l %%n in (1,1,%VS_COUNT%) do if "!VS_CHOICE!"=="%%n" set "CHOICE_OK=1"
    if not defined CHOICE_OK (
        echo Invalid input: "!VS_CHOICE!". Please enter a number between 1 and %VS_COUNT%.
        goto prompt_vs
    )
    set "VS_PATH=!VS_PATH_%VS_CHOICE%!"
    set "VS_VER=!VS_VER_%VS_CHOICE%!"
)

if not defined VS_PATH (
    echo ERROR: failed to select a Visual Studio installation.
    exit /b 1
)

echo     Selected: !VS_PATH! (version !VS_VER!)

set "VCVARS=!VS_PATH!\VC\Auxiliary\Build\vcvars64.bat"
if not exist "!VCVARS!" (
    echo ERROR: vcvars64.bat not found: !VCVARS!
    exit /b 1
)

REM ---------- 4. Activate MSVC ----------
echo [4/6] Activating the MSVC environment...
call "!VCVARS!" >nul
if errorlevel 1 (
    echo ERROR: failed to run vcvars64.bat
    exit /b 1
)

REM ---------- 5. Locate Qt and configure environment ----------
echo [5/6] Locating Qt and configuring environment...

set "QT_ROOT="

if defined QTDIR if exist "%QTDIR%\bin\qmake.exe" set "QT_ROOT=%QTDIR%"
if not defined QT_ROOT if defined QT_DIR if exist "%QT_DIR%\bin\qmake.exe" set "QT_ROOT=%QT_DIR%"
if not defined QT_ROOT if defined Qt6_DIR (
    for %%i in ("%Qt6_DIR%\..\..\..") do set "QT_ROOT=%%~fi"
    if not exist "!QT_ROOT!\bin\qmake.exe" set "QT_ROOT="
)
if not defined QT_ROOT if defined QT_ROOT if exist "%QT_ROOT%\bin\qmake.exe" set "QT_ROOT=%QT_ROOT%"

if not defined QT_ROOT (
    for %%d in (C D E F) do (
        if not defined QT_ROOT if exist "%%d:\Qt" (
            for /f "delims=" %%v in ('dir /b /ad /o-n "%%d:\Qt" 2^>nul ^| findstr /r "^6\."') do (
                if not defined QT_ROOT (
                    for %%c in (msvc2022_64 msvc2019_64 msvc2017_64) do (
                        if not defined QT_ROOT if exist "%%d:\Qt\%%v\%%c\bin\qmake.exe" (
                            set "QT_ROOT=%%d:\Qt\%%v\%%c"
                        )
                    )
                )
            )
        )
    )
)

if not defined QT_ROOT (
    echo ERROR: could not find a Qt build for MSVC.
    echo Set an environment variable, for example:
    echo     setx QTDIR "D:\Qt\6.11.2\msvc2022_64"
    exit /b 1
)

echo     Qt: !QT_ROOT!

set "PATH=!QT_ROOT!\bin;!PATH!"
set "QT_PLUGIN_PATH=!QT_ROOT!\plugins"
set "CMAKE_PREFIX_PATH=!QT_ROOT!"

if exist "!QT_ROOT!\..\..\Tools\Ninja\ninja.exe" (
    for %%i in ("!QT_ROOT!\..\..\Tools\Ninja") do set "QT_NINJA=%%~fi"
    set "PATH=!QT_NINJA!;!PATH!"
    echo     Ninja: !QT_NINJA!
)

where ninja >nul 2>nul
if errorlevel 1 (
    echo     WARNING: ninja was not found in PATH.
    echo     Install Ninja or use the "Visual Studio 17 2022" generator.
)

echo     CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!
echo     QT_PLUGIN_PATH=!QT_PLUGIN_PATH!

REM ---------- 6. Launch VS Code ----------
echo [6/6] Launching VS Code...
where code >nul 2>nul
if errorlevel 1 (
    echo ERROR: the 'code' command was not found.
    echo In VS Code: Ctrl+Shift+P -^> "Shell Command: Install 'code' command in PATH".
    exit /b 1
)

pushd "%PROJECT_DIR%"
code .
popd

echo.
echo Done. The VS Code window should have opened in "%PROJECT_DIR%".
echo.
echo IMPORTANT: if VS Code was already running BEFORE this script, it will
echo NOT inherit the new environment variables. Fully close VS Code and run
echo the script again.
echo.

endlocal
exit /b 0

REM ============================================================
REM  Subroutines
REM ============================================================

:list_vs
for /l %%n in (1,1,%VS_COUNT%) do (
    echo     [%%n] !VS_PATH_%%n!
    echo          version !VS_VER_%%n!
)
exit /b 0

REM Match user-supplied selector against the enumerated installs.
REM Accepts: exact installation path, calendar year ("2022"),
REM major version ("17"), or version prefix ("17.8").
:match_vs
set "MATCH_ARG=%~1"

REM 1) Exact installation path
for /l %%n in (1,1,%VS_COUNT%) do (
    if not defined VS_PATH (
        if /i "!VS_PATH_%%n!"=="!MATCH_ARG!" (
            set "VS_PATH=!VS_PATH_%%n!"
            set "VS_VER=!VS_VER_%%n!"
            exit /b 0
        )
    )
)

REM 2) Translate calendar year to major version
if /i "!MATCH_ARG!"=="2026" set "MATCH_ARG=18"
if /i "!MATCH_ARG!"=="2022" set "MATCH_ARG=17"
if /i "!MATCH_ARG!"=="2019" set "MATCH_ARG=16"
if /i "!MATCH_ARG!"=="2017" set "MATCH_ARG=15"

REM 3) Prefix match (supports "17", "17.8", "17.8.3", ...)
for /l %%n in (1,1,%VS_COUNT%) do (
    if not defined VS_PATH (
        set "V=!VS_VER_%%n!"
        if "!V:~0,2!"=="!MATCH_ARG!" (
            set "VS_PATH=!VS_PATH_%%n!"
            set "VS_VER=!VS_VER_%%n!"
        )
        if "!V:~0,3!"=="!MATCH_ARG!" (
            set "VS_PATH=!VS_PATH_%%n!"
            set "VS_VER=!VS_VER_%%n!"
        )
        if "!V:~0,4!"=="!MATCH_ARG!" (
            set "VS_PATH=!VS_PATH_%%n!"
            set "VS_VER=!VS_VER_%%n!"
        )
        if "!V:~0,5!"=="!MATCH_ARG!" (
            set "VS_PATH=!VS_PATH_%%n!"
            set "VS_VER=!VS_VER_%%n!"
        )
    )
)
exit /b 0

:show_help
echo Usage:
echo     make_vs_code_workspace.bat [options] [project_folder]
echo.
echo Options:
echo     -vs ^<version^> Visual Studio version to use.
echo                     Accepts a calendar year ("2022", "2019"),
echo                     a major version ("17", "16"), a version
echo                     prefix ("17.8"), or a full installation path.
echo                     If omitted and multiple installations are
echo                     found, an interactive menu is shown.
echo     -h, --help      Show this help.
echo.
echo Examples:
echo     make_vs_code_workspace.bat
echo     make_vs_code_workspace.bat D:\my\proj
echo     make_vs_code_workspace.bat -vs 2022 D:\my\proj
echo     make_vs_code_workspace.bat -vs 17
echo     make_vs_code_workspace.bat -vs "C:\Program Files\Microsoft Visual Studio\2022\Community"
exit /b 0
