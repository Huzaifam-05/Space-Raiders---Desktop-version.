#include "Bullet.h"

Bullet::Bullet()
{
	x = 0;
	y = 0;
	active = false;
	yDirection = 0;
	xDirection = 0;

}

void Bullet::spawn(int px, int py, int ydir,int xdir)
{
	if (!active)
	{
		x = px;
		y = py + ydir;
		yDirection = ydir;
		xDirection = xdir;
		active = true;

		tightDiagonal = false;
		diagonalCounter = 0;

	}
}

void Bullet::move()
{
	if (active)
	{
		if (tightDiagonal)
		{
			y += yDirection;

			diagonalCounter++;

			if (diagonalCounter >= 3)
			{
				x += xDirection;
				diagonalCounter = 0;
			}
		}
		else
		{
			x += xDirection;
			y += yDirection;
		}

		if (x < 0 || x >= width || y < 0 || y >= height)
		{
			active = false;
		}
	}
}
int Bullet::getX()
{
	return x;
}

int Bullet::getY()
{
	return y;
}

bool Bullet::isActive()
{
	return active;
}

void Bullet::deactivate()
{
	active = false;
}

void Bullet::setElite(bool b) { elite = b; }

bool Bullet::isElite() { return elite; }

void Bullet::setTightDiagonal(bool value)
{
	tightDiagonal = value;
	diagonalCounter = 0;
}