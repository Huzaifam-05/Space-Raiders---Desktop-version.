#pragma once

#include <iostream>

#include "Settings.h"
#include "Bullet.h"
#include "Enemy.h"

enum BossAttack
{
	TRIPLE_STORM,
	DIAGONAL_WAVE,
	VOLLEY,
	WINGMEN,

	BARRAGE,
	LASER_RAIN,
	CHARGE,

	DASH,
	TELEPORT,
	CLONES,

	CROSS_FIRE
};

class BossEngine
{
private:
	int x;
	int y;

	int health;
	int maxHealth;

	int level;
	int phase;

	BossAttack currentAttack = TRIPLE_STORM;

	Bullet bullet[50];

	Enemy wingman[8];
	Bullet wingmanBullet[8][3];

	int wingmanCount = 0;
	bool wingmenSpawned = false;

	int moveCounter = 0;
	int moveDelay = 5;
	int direction = 1;

	int shootCounter = 0;
	int shootDelay = 30;

	int attackCounter = 0;
	int attackIndex = 0;
	int attackDelay = 0;
	int attackPause = 0;

	int tripleCounter = 0;
	int diagonalCounter = 0;
	int volleyCounter = 0;

	bool enraged = false;

	bool charging = false;
	int chargeTimer = 0;
	int chargeDirection = 0;
	int chargeDistance = 0;

	bool dashing = false;
	int dashDirection = 0;
	int dashDistance = 0;
	int dashDelay = 0;

	bool teleporting = false;
	int teleportTimer = 0;
	int teleportDelay = 20;

	bool cloneActive = false;
	bool clonesUsed = false;

	int cloneTimer = 0;
	int cloneX[2];

	bool realBossLeft = true;

	int swapCounter = 0;

	int nextPowerDrop = 80;
	int powerDropStep = 20;

	bool attackStartDelay = true;
	int attackStartTimer = 0;

	static const int attackStartDelayTime = 20;

public:
	void setup(int level);

	void update(int);

	void move();

	void attack(int);

	void nextAttack();

	bool checkCollision(Bullet playerBullet[], int playerBulletCount, int playerX, int playerY, int& health, bool& invincible, int& invincibleCounter, bool doubleDamage);

	void setCharacters(char grid[][width]);

	void tripleStorm();
	void diagonalWave();
	void precisionVolley();

	void barrage();
	void laserRain();
	void crossFire();

	void startCharge(int playerX);
	void charge();

	void dashAttack(int);
	void teleport();

	void spawnClones();
	void cloneShoot();
	void drawClones(char grid[][width]);

	bool shouldSpawnWingmen();

	void shootSingle();
	void shootTriple();
	void shootDiagonal();
	void shootVolley();
	void shootCrossFire();
	void shootRain();

	bool isDead() const;
	bool isEnraged() const;
	int getHealth() const;
	int getMaxHealth() const;

	bool shouldDropPower();

	int getX() const;
	int getY() const;

};