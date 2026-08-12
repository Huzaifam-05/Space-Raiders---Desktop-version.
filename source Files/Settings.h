#pragma once

//Game Grid
const int width = 40;
const int height = 20;
const int enemySpawnRows = 5; //For Random

const int rapidFireDuration = 400;
const int shieldDuration = 300;
const int doubleDamageDuration = 400;
const int diagonalShotDuration = 500;
const int enemyDropChance = 5;
const int eliteDropChance = 20;
const int powerMagnetRange = 2;

//Players
const int playerSpawnX = width / 2;
const int playerSpawnY = height - 2;
const int playerHealth = 5;
const int playerDamage = 1;
const int invincibleDelay = 25;
const int playerBulletCount = 50;
const int playerMoveDelay = 3;
const int maxHeat = 10;


//Enemy
const int enemyCounts = 10;

//Ticks settings
const int enemyMoveDelay = 3;
const int enemyDownSteps = 50;
const int enemyBulletDelay = 4;
const int attackDelay = 20;
const int gameDelay = 35;
const int levelStartDelay = 30;

//Bullet diection
const int playerBulletDirection = -1;
const int enemyBulletDirection = 1;

