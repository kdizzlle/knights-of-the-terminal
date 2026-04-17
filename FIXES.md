# Windows Build & Arena Fix Log

## Problem
The chess arena returned a fatal timeout error when trying to connect to the engine:
```
Fatal error: java.util.concurrent.TimeoutException: Timeout waiting for: uciok
=== Stopped ===
```

## Fixes Applied

### 1. Missing `build.bat` and `run.bat` for Windows
The repo only had `build.sh` and `run.sh` (Linux/Mac scripts). Windows batch equivalents were created:

**`build.bat`** (repo root)
```bat
@echo off
g++ -O2 -std=c++17 -Wall -Wextra -I include src\*.cpp -o engine.exe
pause
```

**`run.bat`** (repo root)
```bat
@echo off
chessbotClassic\chessbot.exe
```

**`chessbotClassic/build.bat`**
```bat
@echo off
g++ -O2 -std=c++17 -Wall -Wextra -static -I . *.cpp -o engine.exe
pause
```

**`chessbotClassic/run.bat`**
```bat
@echo off
engine.exe
```

### 2. Missing g++ Compiler
`g++` was not recognized on the system. Fix: install MSYS2 and add it to PATH.

1. Download and install [MSYS2](https://www.msys2.org)
2. Open the MSYS2 terminal and run:
   ```
   pacman -S mingw-w64-ucrt-x86_64-gcc
   ```
3. Add `C:\msys64\ucrt64\bin` to your system PATH via Environment Variables

### 3. Engine Crashing Due to Missing MSYS2 DLLs (Root Cause of Timeout)
By default, MSYS2's g++ dynamically links the C++ runtime. When the arena launched `engine.exe`, the MSYS2 DLLs were not in its PATH, causing the engine to crash before outputting `uciok`.

**Fix:** Added `-static` flag to the build command so all runtime dependencies are bundled into the executable:
```bat
g++ -O2 -std=c++17 -Wall -Wextra -static -I . *.cpp -o engine.exe
```
