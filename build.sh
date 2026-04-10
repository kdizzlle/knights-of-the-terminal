#!/bin/bash
set -e
DIR="$(cd "$(dirname "$0")" && pwd)"
g++ -O2 -std=c++17 -Wall -Wextra -I "$DIR/include" "$DIR/src"/*.cpp -o "$DIR/engine"
echo "Built ./engine"
