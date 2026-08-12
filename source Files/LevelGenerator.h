#pragma once

#include "Settings.h"

#include "Enemy.h"
#include "Enums.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

class LevelGenerator
{
public:
	Formation formation;
	EnemyType enemyType;
	Movement movement;
	FireType fireType;

	int formationPool = 1;
	int enemyTypePool = 1;
	int movementPool = 1;
	int fireTypePool = 1;

	int priority = 0;
	bool priorityFormation = false;
	bool priorityMovement = false;
	bool priorityFire = false;
	bool priorityEnemy = false;

	bool insaneLevel = false;

	int savedEnemy = 0;
	int savedAttacker = 0;
	int savedShootDelay = 0;
	int savedBulletSpeed = 0;

	int eliteCount = 0;
	bool mixedEnemies = false;

public:
	void generateFormation(Enemy enemy[], int enemyCount);
	void generateEnemyType(Enemy enemy[], int enemyCount);
	void generateMovement(Enemy enemy[], int enemyCount);
	void generateFireType(Enemy enemy[], int enemyCount);

	int getCenterEnemy(Enemy enemy[], int enemyCount);

	void levelSetup(int& enemyCount, int level, int& attackerScaling, int& shootDelayScaling, int& bulletSpeedScaling, int& speedScaling, int& healthScaling, int& cooldownScaling);

	void makeElite(Enemy enemy[], int enemyCount, int center);

	void reset();

private:
	void circle(Enemy enemy[], int enemyCount);
	void rectangle(Enemy enemy[], int enemyCount);
	void triangle(Enemy enemy[], int enemyCount);
	void diamond(Enemy enemy[], int enemyCount);
	void randomFormation(Enemy enemy[], int enemyCount);
	void placeRows(Enemy enemy[], int rowSizes[], int rows);
	void spawnExtras(Enemy enemy[], int startIndex, int extraCount, int leftMostX, int rightMostX, int topY, int bottomY);

};