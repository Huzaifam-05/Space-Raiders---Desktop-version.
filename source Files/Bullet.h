#pragma once
#include "Settings.h"

class Bullet
{
	int x;
	int y;
	int xDirection;
	int yDirection;
	bool active;
	bool elite = false;
	bool tightDiagonal = false;
	int diagonalCounter = 0;


public:

	Bullet();

	void spawn(int, int, int, int);

	void move();

	int getX();

	int getY();

	bool isActive();

	void deactivate();

	void setElite(bool);

	bool isElite();

	void setTightDiagonal(bool);
};
