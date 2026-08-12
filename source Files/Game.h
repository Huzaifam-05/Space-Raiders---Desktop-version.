#pragma once

#include <iostream>
#include <conio.h>
#include <ctime>
#include <fstream>

#include "Settings.h"
#include "Bullet.h"
#include "Player.h"
#include "Enemy.h"
#include "LevelGenerator.h"
#include "BossEngine.h"
#include "PowerUp.h"
#include "Graphics.h"

using namespace std;

struct EnemyBullets
{
	Bullet bullet[3];
};

enum class GameState
{
	MAIN_MENU,
	LEVEL_INTRO,
	PLAYING,
	END_SCREEN
};

class Game
{
	friend class Graphics;

	Bullet bullet[playerBulletCount];
	EnemyBullets* eBullet;
	Player player;
	Enemy* enemy;
	LevelGenerator generator;
	BossEngine bossEngine;
	Graphics graphics;

	static const int maxPowerUps = 3;

	PowerUp powerUp[maxPowerUps];

	char grid[height][width];

	int score = 0;
	int level = 0;
	int enemyCounter = 0;
	int levelStartCounter = 0;

	int enemyCount;
	int health;
	int formationDirection;
	int maxPlayerHealth ;
	int maxPlayerHeat;

	int bulletDelayCounter = 0;
	int attackTimerCounter = 0;
	int barrageCounter = 0;
	int shootTimer = 0;
	int shootDelay = 30;

	int attackerScaling = 0;
	int shootDelayScaling = 0;
	int bulletSpeedScaling = 0;
	int speedScaling = 0;
	int healthScaling = 0;
	int cooldownScaling = 0;

	int playerShootCounter = 0;
	int playerShootDelay = 6;
	int playerMoveCounter = 0;

	int heat = 0;
	bool overheated = false;
	int overheatCounter = 0;
	const int overheatCooldown = 45;
	int coolCounter = 0;
	const int coolDelay = 3;

	int invincibleCounter = 0;

	bool isGameOver = false;
	bool canEnemiesAttack = false;
	bool invincible = false;
	bool ready = false;
	bool boss = false;

	int activeEnemyCount = 0;
	bool reinforcementSpawned = false;
	int reinforcementCount = 0;

	bool elitePromoted = false;

	bool rapidFire = false;
	bool shield = false;
	bool doubleDamage = false;
	bool diagonalShot = false;

	int rapidFireTimer = 0;
	int shieldTimer = 0;
	int doubleDamageTimer = 0;
	int diagonalShotTimer = 0;

	GameState gameState = GameState::MAIN_MENU;

	double levelIntroStartTime = 0.0;
	double levelIntroDuration = 3.5;

	bool playerWon = false;
	bool paused = false;

	bool hasSave = false;
	int checkpointLevel = 1;

	bool resumingSavedGame = false;

	void saveGame();
	bool loadGame();
	void deleteSave();
	void saveCheckpoint();


public:
	Game();

	~Game();

	void start();

	void update();

	void enemyUpdate();

	void input();

	void clearGrid();

	void setCharacters();

	void checkCollision();

	void enemyCollision();

	void bossCollision();

	void levelSetup();

	void normalLevel();

	void chooseAttackers();

	void enemyShoot(int index);

	void chooseShooters();

	void spawnReinforcements();

	void promoteRemainingEnemies();

	void updatePowerUp();

	void spawnPowerUp(int x, int y);

	void collectPowerUp();

	void activatePowerUp(PowerType type);

	void updatePowerTimers();

	void updatePlayerScaling();

	void startGame();

	void deleteCheckpoint();


	bool isMainMenu() const;
	bool isLevelIntro() const;
	bool isPlaying() const;
	bool isEndScreen() const;
	bool hasWon() const;
	bool loadCheckpoint();

	void restartCheckpoint();
};