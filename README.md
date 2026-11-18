# PokéArena

**A turn-based Pokémon battle simulator written in C.**

Made by **Team Tasty_crousty** for **the Computer Programming class, Inha University**.

PokéArena is a terminal-based Pokémon battle simulator developed as a team project. The goal is to recreate the core mechanics of classic Pokémon battles, allowing players to build teams, choose moves, and compete against AI-controlled opponents or other players.

The project focuses on modular programming, data structures, algorithms, memory management, and file handling in C.

## Features

The project aims to provide the following features:

- **Pokémon Management:** Browse Pokémon and view their statistics, types, and available moves.
- **Team Building:** Create and customize teams containing up to six Pokémon.
- **Turn-Based Combat:** Fight using attacks, Pokémon switches, and strategic decisions.
- **Damage Calculation:** Calculate damage based on attack power, Pokémon statistics, and type effectiveness.
- **Type Effectiveness:** Implement elemental advantages, resistances, and immunities.
- **Special Effects:** Support effects such as poisoning, burning, healing, and critical hits.
- **AI Opponent:** Battle against a computer-controlled opponent capable of evaluating available moves.
- **Save & Load:** Save teams and battle statistics for future sessions.

## Tech Stack

| Technology | Purpose |
|---|---|
| C | Main programming language |
| Clang / GCC | Compilation |
| Make | Build automation |
| Git & GitHub | Version control and collaboration |
| clang-format | Code formatting |
| Cppcheck | Static code analysis |

## Project Structure

```text
pokeArena/
│
├── data/               # Game data and resources
│
├── include/            # Header files (.h)
│
├── src/                # Source files (.c)
│
├── .vscode/            # Editor configuration
├── .clang-format       # Formatting configuration
├── .cppcheck           # Static analysis configuration
├── .gitignore
├── Makefile
└── README.md
```

The codebase is organized into independent modules to keep responsibilities separated and make development easier to maintain.

## Getting Started

### Requirements

Make sure you have the following installed:

- Git
- Clang or GCC
- GNU Make

### Clone the repository

```bash
git clone https://github.com/konogannn/pokeArena.git
cd pokeArena
```

### Compile the project

Build the application using:

```bash
make
```

### Run the application

```bash
./pokeArena
```

### Clean generated files

Remove generated object files:

```bash
make clean
```

Remove all generated build files:

```bash
make fclean
```

Rebuild the project:

```bash
make re
```

## Development Tools

The Makefile also provides several development utilities.

### Code formatting

Format the source and header files:

```bash
make format
```

Check formatting without modifying files:

```bash
make format-check
```

### Static analysis

Run Cppcheck:

```bash
make cppcheck
```

Run all static checks:

```bash
make lint
```

### Runtime sanitizers

Build a version with AddressSanitizer and UndefinedBehaviorSanitizer:

```bash
make sanitize
```

## Architecture

The application is divided into several logical modules:

| Module | Responsibility |
|---|---|
| Pokémon | Pokémon attributes, statistics, and types |
| Moves | Attacks, power, accuracy, and special effects |
| Team | Team creation and Pokémon management |
| Battle | Turn execution, damage calculation, and victory conditions |
| AI | Computer-controlled opponent decisions |
| UI | Terminal menus and battle interface |
| Save | File management and player data persistence |

## Game Mechanics

### Pokémon Statistics

Each Pokémon is defined by individual attributes:

- Health Points (HP)
- Attack
- Defense
- Speed
- Type
- Available moves

Each Pokémon can have up to four different moves.

### Battle System

Battles follow a turn-based system in which players can:

1. Select an attack.
2. Switch their active Pokémon.
3. Make strategic decisions based on the opponent's condition.
4. Continue until one team has no remaining Pokémon able to battle.

The speed statistic determines the action order, while attack power, defensive statistics, and type effectiveness influence damage calculation.
