# ZomboGIG

Isometric tile map prototype built with Qt 6 Widgets.

- 12 x 12 tile map
- Inset 3D wall blocks, thick terrain edges and a gloomy colour palette
- Stormy skybox (procedural clouds, fog and vignette in Qt, no mesh library)
- Day/night cycle with ambient darkness, light rain and a 12-hour blood clock
- Campfire in the compound with a soft glow after dark
- Responsive window that maximises on launch and scales its HUD with the screen; minimum 1120 x 720
- Zombies (Walker, Runner, Brute, Crawler), humans (Survivor, Soldier, Doctor, Engineer) and props (walls, trees, bushes, crates, barrels, rocks, corpses, campfire)
- Blood splatters scattered across the ground
- Hover entities for their stats; props show only their name
- Top bar with title, time, day counter and clock; bottom bar with world statistics on the left and `SOUND` / `SKIP TURN +1 HOUR` on the right
- Rain-only procedural ambience with a `SOUND ON/OFF` toggle

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

## Sound

The looping rain ambience is synthesised into `build/sounds/rain.wav` on first
launch. The game plays it through a system player (`paplay`, `pw-play` or
`aplay`) and falls back to the Windows PowerShell `System.Media.SoundPlayer`
under WSL. Set `ZOMBOGIG_NO_SOUND=1` to launch silently.
