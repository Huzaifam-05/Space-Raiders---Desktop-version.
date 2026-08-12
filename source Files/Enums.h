#pragma once
#include "Settings.h"


enum EnemyType
{
    NORMAL=1,
    SPEEDY,
    TANK,
    DESTROYER,
    BOSS
};

enum Formation
{
    RECTANGLE=1,
    TRIANGLE,
    DIAMOND,
    CIRCLE,
    RANDOM
};

enum Movement
{
    LEFT_RIGHT=1,
    STRAIGHT,
    DIVE,
    ZIGZAG
};

enum FireType
{
    SINGLE=1,
    TRIPLE,
    DIAGONAL
};

enum Boosts
{
    SPEED=1,
    DAMAGE,
    HEALTH,
    EXTRA_BULLETS,
    FIRE_RATE
};