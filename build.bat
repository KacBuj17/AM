REM Dopasuj poniższe ścieżki:
set MINGW_PATH=C:\mingw64\bin
set CMAKE_PATH=C:\Program Files\CMake\bin


set PATH=%PATH%;%MINGW_PATH%;%CMAKE_PATH%;

cmake -G "MinGW Makefiles" -B build
mingw32-make.exe -C build