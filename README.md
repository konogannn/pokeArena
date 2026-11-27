# PokéArena

**A turn-based Pokémon battle simulator written in C.**

Made by **Team Tasty_crousty** for **the Computer Programming class, Inha University**.

PokéArena is a terminal-based Pokémon battle simulator developed as a team project. Players can browse the full Gen 1 Pokédex, build custom teams, challenge Kanto Gym Leaders and the Indigo League, battle a 3-tier tactical AI, or play locally in 2-player turn-by-turn mode.

The project focuses on modular programming, data structures, algorithms, memory management, and TSV file handling in C.

## Features

- **Complete Gen 1 Pokédex (151 Pokémon & 76 Moves):** Loaded from TSV files with validation (primary-key uniqueness, stat bounds, move references).
- **Custom Data Loading:** Load custom moves and Pokémon TSV files outside of battle (`loadmoves`, `loadpokemon`).
- **Team Building:** Build, reorder, autofill, and heal teams of up to six Pokémon.
- **Turn-Based Combat:** Fight with attacks, switches, forfeits, STAB (`1.5x`), speed-based turn order, and PP tracking (`Struggle` fallback when out of PP).
- **Gen 4 Type Effectiveness:** Full 17×17 type chart covering weaknesses, resistances, and immunities.
- **Status & Special Effects:** Support for `Poison`, `Burn`, `Paralysis`, `Crit`, and `Heal` (including pure status/healing moves with `power = 0`).
- **3-Tier Tactical AI:** `Easy`, `Medium`, and `Hard` difficulty modes evaluating damage, type matchups, KO opportunities, and proactive switching.
- **Kanto Gyms & Indigo League:** Challenge all 8 Gen 1 Gym Leaders to earn badges and unlock the 5-match Indigo League gauntlet.
- **Local 2-Player PvP:** Build two teams (`player <1|2>`, `p2`) and battle turn-by-turn (`pvp`) without altering the single-player flow.
- **Save & Load:** Persist teams (HP, status, remaining PP) and earned gym badges to TSV files.
- **Session Battle Report:** Automatically exports per-battle and cumulative statistics to `battle_report.txt` on exit.
- **Terminal UI/UX:** TTY-aware ANSI colors, HP bars, type badges, and typewriter/HP animations.

## Tech Stack

| Technology | Purpose |
|---|---|
| C (C99) | Main programming language |
| Clang / GCC | Compilation |
| Make | Build automation |
| Git & GitHub | Version control and collaboration |
| clang-format | Code formatting |
| Cppcheck | Static code analysis |

## Project Structure

```text
pokeArena/
│
├── data/               # TSV game data (pokemon.tsv, moves.tsv)
├── include/            # Header files (.h)
├── src/                # Source files (.c)
├── tests/              # Unit test suite (test_main.c)
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

### Compile & Run

```bash
make
./pokeArena
```

### Run Unit Tests

```bash
make tests_run
```

### Clean & Rebuild

```bash
make clean	# removes object files
make fclean # clean + removes binary files
make re		# Full rebuild
```

## Commands
| Command | Description |
|---|---|
| pokedex (p) | List all available Pokémon |
| moves (m) |List all available moves |
| info <name\|index> | Show detailed stats and moves of a Pokémon |
| team (t) | Show the active player's team
| add <name\|index> | Add a Pokémon to the active team
| remove <1-6> | Remove a Pokémon from the active team
| swap <1-6> <1-6> | Swap two Pokémon in the active team
| autofill | Fill remaining team slots randomly
| battle [easy\|medium\|hard] (b) | Battle a random AI team
| gyms | List all 8 Kanto Gym Leaders and earned badges
| gym <1-8\|name> | Challenge a Kanto Gym Leader
| league | Challenge the Indigo League (requires 8 badges)
| player <1\|2> / p2 | Switch active player for 2-player mode
| pvp | Start a local 2-player turn-by-turn battle
| heal | Restore HP, status, and PP for the active team
| save [path] / load [path]| Save or load team and badge progress (default: data/team.tsv)
| loadmoves <path> / loadpokemon <path> | Load custom moves or Pokémon TSV files |
| clear | Clear the active team |
| help (h) / quit (q) | Show help or exit (writes battle_report.txt) |

## Development Tools

### Code formatting

```bash
make format			# Format source and header files
make format-check	# Format in dry-run
make cppcheck		# Run Cppcheck static analysis
make lint			# Run format-check and cppcheck
make sanitize		# Build with AddressSanitizer and UndefinedBehaviorSanitizer
```

## Architecture

The application is divided into several logical modules:

| Module | Responsibility |
|---|---|
| pokemon | Pokémon attributes, statuses, and TSV loading |
| moves | Move attributes, PP, effects, and TSV loading |
| type | Gen 4 17x17 chart and string conversion |
| team | Team creation, swapping, healing, and validation |
| battle | Turn-by-turn engine, STAB/damage calculation, and PvP/AI resolution |
| ai | 3-tier opponent decision-making and team generation |
| gym | 8 Kanto Gym Leaders, badges, and Indigo League progression |
| save | TSV persistence for teams (HP, status, PP) and badges |
| report | Session battle statistics and battle_report.txt export |
| display / color | Terminal rendering, HP bars, ANSI colors, and animations |
| ui | Interactive command loop and player/team routing |
| tsv/utils | Generic TSV parser and string/integer helpers |
