#pragma once

#include "Settings.h"

enum PowerType
{
	POWER_HEALTH,
	POWER_RAPID_FIRE,
	POWER_SHIELD,
	POWER_DOUBLE_DAMAGE,
	POWER_DIAGONAL_SHOT
};

class PowerUp
{
private:
	int x = 0;
	int y = 0;

	bool active = false;
	int moveCounter = 0;
	int moveDelay = 4;

	PowerType type = POWER_HEALTH;

public:

	void spawn(int px, int py, PowerType power);

	void move();

	void deactivate();

	bool isActive() const;

	int getX() const;

	int getY() const;

	PowerType getType() const;

	void setPosition(int newX, int newY);
};