#pragma once

enum class GameState {
    NotStarted,
    Ongoing,
    GameOver
};

enum class Weather {
    Clear,
    LightRain,
    Rain,
    Clearing
};

enum class HumanType {
    Farmer,
    Doctor,
    Military,
    Builder
};

enum class ZombieType {
    Tank,
    Runner,
    Exploder
};

enum class Sprite {
    Farmer,
    Doctor,
    Military,
    Builder,
    Tank,
    Runner,
    Exploder
};

enum class Decoration {
    None,
    Tree,
    Bush,
    Corpse,
    Campfire
};
