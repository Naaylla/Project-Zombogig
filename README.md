# ZomboGIG

Isometric tile map prototype built with Qt 6 Widgets.

- 12 x 12 tile map
- Thin isometric wall panels around the compound (no chunky blocks)
- Stormy skybox (procedural clouds, fog and vignette in Qt, no mesh library)
- Day/night cycle with ambient darkness, light rain and a 12-hour blood clock
- Campfire in the compound with a soft glow after dark
- Responsive window that maximises on launch and scales its HUD with the screen; minimum 1120 x 720
- Zombies (Tank, Runner, Exploder), humans (Farmer, Doctor, Military, Builder) and props (thin walls, dead trees, berry bushes, food plants, corpses, campfire)
- Blood only appears when a zombie attacks a human: press `SKIP TURN` while a zombie is next to a human to see the wound
- Food plants grow every turn and add to the `FOOD` total
- Hover entities for their stats; props show their name (plants show growth and food)
- Top bar with title, time, day counter and clock; bottom bar with world statistics on the left, spawn counts and `SKIP TURN +1 HOUR` on the right
- Gloomy Project-Zomboid-style HUD with procedural grime, scratches and blood splatters
- No audio

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

- g++ with C++17
- make
- qt6-base-dev, qt6-base-dev-tools (Qt 6.10.2)
- WSLg for the window (WSL 2 on Windows 11)
