#pragma once
#include "Settings.h"

class Player
{
	int x;
	int y;
	float health;
	float damage;


public:

	Player();

	void setSpawn();

	void move(char input);

	int getX();

	int getY();

	float getHealth();

	float getDamage();

	void setHealth(float hp);
};
