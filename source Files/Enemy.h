#pragma once

#include <cstdlib>

#include "Settings.h"
#include "Enums.h"

class Enemy
{
	int x = 0;
	int y = 0;

	int xfor = 0;
	int yfor = 0;

	int health = 1;
	int damage = 1;
	int speed = 1;

	int steps = 0;
	int delay = 0;
	int direction = 1;
	int zigzagState = 0;
	int zigzagCounter = 0;

	int shootCooldown = 0;
	int baseCooldown = 0;

	bool elite = false;

	bool alive = false;
	bool attacking = true;
	bool inFormation = false;
	bool returningToFormation = false;

	EnemyType type;
	Movement movement;
	FireType fireType;

public:
	int healthScaling = 0;
	int speedScaling = 0;
	int cooldownScaling = 0;

	void setPosition(int ex, int ey);

	int getX();
	int getY();

	int getHealth();
	void setHealth(float hp);

	int getDamage();

	void setAlive();
	void removeAlive();
	bool getAlive();

	void setType(EnemyType enemyType);
	EnemyType getType() const;

	void setMovement(Movement move);
	Movement getMovement() const;

	void setFireType(FireType fire);
	FireType getFireType() const;

	void setFormationPosition(int, int);
	int getFormationX();
	int getFormationY();

	bool getInFormation();
	void setInFormation(bool);
	void moveIntoFormation();
	bool isReturningToFormation();

	void setAttacking(bool attack);
	bool isAttacking() const;

	void move();
	void move(int, int&);
	void moveStraight();
	void moveDive(int playerX);
	void moveLeftRight(int&);
	void moveZigzag();

	void updateCooldown();
	bool canShoot() const;
	void setCooldown(int);
	int getBaseCooldown();

	void setElite(bool value);
	bool isElite() const;

	void setReturningToFormation(bool b);
};