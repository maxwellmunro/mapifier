# Mapifier

## Description

Mind map making software using Directed Acyclic Graph data structures.

## Requirements

- SDL2
- SDL2_ttf
- SDL2_gfx
- boost-serialization

# Build

- Ensure you have g++ and build-essential / make installed (for make command)

```bash
cd ~
mkdir .mind
git clone https://github.com/maxwellmunro/mapifier.git
cd mapifier
make
```

# Keybinds

- Ctrl-O -> open file
- Ctrl-W -> write/save file
- Ctrl-N -> create new node
- Ctrl-S -> set selected nodes parent
- Ctrl-Backspace -> clear node
- Ctrl-C -> clear selected nodes connections
- Ctrl-A -> copy selected nodes colour
- Ctrl-Z -> paste selected nodes colour
- Ctrl-R -> give selected node random colour
- Return -> change text for selected node
- Escape -> exit typing/setting parent
