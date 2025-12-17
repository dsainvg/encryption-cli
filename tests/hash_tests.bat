@echo off
setlocal
pushd "%~dp0.."
if not exist build mkdir build
if not exist build\obj mkdir build\obj
echo Building test_hash.exe...
g++ -Wall -Wextra -O2 -std=c++17 -Iinclude -c tests\test_crypto.cpp -o build\obj\test_crypto.o 2>nul
if %ERRORLEVEL%==0 (
  g++ -Wall -Wextra -O2 -std=c++17 build\obj\test_crypto.o build\obj\crypto.o -ladvapi32 -o build\test_hash.exe 2>nul
  if %ERRORLEVEL%==0 (
    echo Build succeeded with g++.
    build\test_hash.exe
    popd
    exit /b 0
  )
)
echo Build failed.
popd
exit /b 1
