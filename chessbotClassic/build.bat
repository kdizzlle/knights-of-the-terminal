@echo off
g++ -O2 -std=c++17 -Wall -Wextra -static -I . *.cpp -o engine.exe
pause
