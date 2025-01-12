@echo off

set PROJECT_DIR=%cd%
set BUILD_DIR=%PROJECT_DIR%\build

echo update gitproject

git pull

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

echo build project
cmake -G "MinGW Makefiles" .. || exit /b
mingw32-make || exit /b

echo build succeeded
