# knights-of-the-terminal

## Overview

This project is a chess engine developed by a team of 6 members. The engine analyzes a given chess position, generates all possible legal moves using Algebraic Notation, evaluates each move, and returns the best move for the current board state.

## Features

- Full chess board representation
- Move generation for all pieces
- Basic evaluation function for board positions
- Input/Output using Algebraic Notation
- Best move selection based on evaluation

## Build Instructions

## Prerequisites

C Make (version 3.10 or higher)

- A C++ compiler:
- Linux: g++
- macOS: clang++ or Xcode Command Line Tools
- Windows: Visual Studio

## Linux

```bash
# Install dependencies (Ubuntu/Debian example)
sudo apt update
sudo apt install build-essential cmake

# Clone the repository
git clone https://github.com/your-repo/chess-engine.git
cd chess-engine

# Build
./build.sh

# Run
./run.sh
```

## MacOS

```bash
# Install dependencies (if not already installed)
brew install cmake

# Clone the repository
git clone https://github.com/your-repo/chess-engine.git
cd chess-engine

# Build
./build.sh

# Run
./run.sh
```

## Windows (Visual Studio)

```bash
# Clone the repository
git clone https://github.com/your-repo/chess-engine.git
cd chess-engine

# Generate Visual Studio solution
cmake -S . -B build

# Open the project
# Open build/chess-engine.sln in Visual Studio

# Build and run inside Visual Studio
```

## Example: UCI Handshake

**Input:**

```bash
uci
```

**Output**

```text
Knights of the Terminal
Sebastian, Kelly, Mia, Andrew, Tran, Ella
uciok
```

### Authors

- Sebastian Graciano
- Kelly Williams
- Mia Diliberti-Brandenburg
- Andrew Ravadan Castillo
- Tran Vo
- Ella Shepherd
