#include "PowerUp.h"

void PowerUp::spawn(int px, int py, PowerType power)
{
	x = px;
	y = py;

	type = power;

	active = true;
}

void PowerUp::move()
{
	if (!active)
		return;

	moveCounter++;

	if (moveCounter < moveDelay)
		return;

	moveCounter = 0;

	y++;

	if (y >= height)
	{
		active = false;
	}
}
void PowerUp::deactivate()
{
	active = false;
}

bool PowerUp::isActive() const
{
	return active;
}

int PowerUp::getX() const
{
	return x;
}

int PowerUp::getY() const
{
	return y;
}

PowerType PowerUp::getType() const
{
	return type;
}

void PowerUp::setPosition(int newX, int newY)
{
	x = newX;
	y = newY;
}