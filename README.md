# ZomboGIG

Isometric zombie-survival game built with Qt 6 Widgets, rendered entirely in Qt
(no external assets or mesh library).

## Gameplay

- 12 x 12 isometric map with a walled compound, campfire, food plants and props.
- Survive 48 in-game hours (1 turn = 1 hour, the run starts at 07:00 in light rain).
- Pick the number of survivors and zombies in the main menu, then start or quit.
- Humans (Farmer, Doctor, Military, Builder) and zombies (Tank, Runner, Exploder)
  act automatically every turn; humans can eat food, harvest plants, fight,
  fortify walls and cure infection, while zombies chase, attack and explode.
- Hordes spawn every 8 hours and grow stronger over time.
- Food plants regrow each turn (rain doubles growth). Hungry survivors eat from
  the shared food stock.
- Infected survivors turn into zombies after a few turns unless a Doctor cures
  them.
- Win by surviving all 48 hours; lose when every human is gone.

## Controls

- `Space` - start/pause auto-play
- `N` or `Right` - advance one hour
- `1` `2` `3` `4` - set auto-play speed
- `Esc` - return to the menu
- `F11` - toggle fullscreen
- Hover the map to inspect a cell or entity

## Interface

- Animated rain while the weather is rainy, clearing after the first few hours.
- Day/night lighting, campfire glow, drifting fog and blood splatters.
- Project-Zomboid-style grunge HUD and menu with a short intro story.
- Responsive layout: the window starts maximised and the HUD scales with height.
- Optional zombie-style font: drop a `.ttf`/`.otf` file into a `fonts/` folder
  next to the executable (or in the project root) and it is loaded automatically.

## Project layout

The code is split so each class has one job:

- `src/logic/` - `Game` (turns, waves, weather, win/loss) plus `Grid`, `Cell`,
  `Wall`, `Plant` and the shared enums in `types.h`.
- `src/entities/` - one file per entity: `Entity` (abstract), `Human`
  (`Farmer`, `Doctor`, `Military`, `Builder`) and `Zombie` (`Tank`, `Runner`,
  `Exploder`). Each class keeps its own behaviour.
- `src/ui/` - every widget, the map renderer, the menu and the HUD.
- `src/main.cpp` - application entry point.

Model classes: `Game`, `GameState`, `Grid`, `Cell`, `Entity` (abstract),
`Human` (`Farmer`, `Doctor`, `Military`, `Builder`), `Zombie` (`Tank`, `Runner`,
`Exploder`), `Plant`, `Wall`, `Status`.

## Build (WSL / Ubuntu)

```sh
make
```

The Makefile runs a shadow build in `build/` through `qmake6`.

## Run

```sh
make run
```

or:

```sh
./build/ZomboGIG
```

## Clean

```sh
make clean
```

## Requirements

- g++ with C++20
- make
- qt6-base-dev, qt6-base-dev-tools (Qt 6.10.2)
- WSLg for the window (WSL 2 on Windows 11)
