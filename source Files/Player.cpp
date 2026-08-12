#include "Player.h"

Player::Player()
{
	x = playerSpawnX;
	y = playerSpawnY;
	health = playerHealth;
	damage = playerDamage;
}

void Player::setSpawn()
{
	x = playerSpawnX;
	y = playerSpawnY;
}


void Player::move(char input)
{
	if ((input == 'a' || input == 'A') && x > 1)
	{
		x--;
	}
	else if ((input == 'd' || input == 'D') && x < width - 2)
	{
		x++;
	}
	else if ((input == 'w' || input == 'W') && y > 0)
	{
		y--;
	}
	else if ((input == 's' || input == 'S') && y < height - 1)
	{
		y++;
	}
}

int Player::getX()
{
	return x;
}

int Player::getY()
{
	return y;
}

float Player::getHealth()
{
	return health;
}

float Player::getDamage()
{
	return damage;
}

void Player::setHealth(float hp)
{
	health = hp;
}