@echo off
setLocal enableDelayedExpansion

set package=package.nea
if exist !package! del !package!
echo.Creating package '!package!' ...
echo.

for /f "tokens=1 delims=" %%a in ('dir /b /ad internal\newEngineProject 2^>nul') do (
    echo.Packaging '\%%a\' ...
    for /f "tokens=1 delims=" %%b in ('dir /b /a-d internal\newEngineProject\%%a 2^>nul') do (
        echo.Packaging '\%%a\%%b' ...
        echo.$$FILE:newEngineProject\%%a\%%b >>package.nea
        for /f "tokens=1 delims=" %%c in (internal\newEngineProject\%%a\%%b) do (
            echo.%%c >>package.nea
        )
        echo.$$ENDFILE >>package.nea
    )
)

echo.
echo.Packaging finished.

:halt
pause >nul
goto :halt
