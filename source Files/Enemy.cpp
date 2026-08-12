#include "Enemy.h"

void Enemy::setPosition(int ex, int ey)
{
	x = ex;
	y = ey;
}

int Enemy::getX()
{
	return x;
}

int Enemy::getY()
{
	return y;
}

int Enemy::getHealth()
{
	return health;
}

int Enemy::getDamage()
{
	return damage;
}

void Enemy::setHealth(float hp)
{
	health = hp;
}

EnemyType Enemy::getType() const
{
	return type;
}

void Enemy::setType(EnemyType enemyType)
{
	type = enemyType;

	switch (type)
	{
	case EnemyType::NORMAL:
		health = 2 + healthScaling;
		damage = 1;
		speed = 6 - speedScaling;
		shootCooldown = 70 - cooldownScaling;
		break;

	case EnemyType::SPEEDY:
		health = 2 + healthScaling;
		damage = 1;
		speed = 2 - speedScaling;
		shootCooldown = 40 - cooldownScaling;
		break;

	case EnemyType::TANK:
		health = 4 + healthScaling;
		damage = 1;
		speed = 10 - speedScaling;
		shootCooldown = 100 - cooldownScaling;

		break;
	case EnemyType::DESTROYER:
		health = 1 + healthScaling;
		damage = 3;
		speed = 8 - speedScaling;
		shootCooldown = 50 - cooldownScaling;

		break;
	}

	if (elite)
	{
		health = 3;
		damage = 3;
		shootCooldown = 30 - cooldownScaling;

	}

	baseCooldown = shootCooldown;

}

void Enemy::setMovement(Movement move)
{
	movement = move;
}

Movement Enemy::getMovement() const
{
	return movement;
}

void Enemy::setAlive()
{
	alive = true;
}

void Enemy::removeAlive()
{
	alive = false;
}

bool Enemy::getAlive()
{
	return alive;
}

void Enemy::setAttacking(bool attack)
{
	attacking = attack;
}

bool Enemy::isAttacking() const
{
	return attacking;
}

void Enemy::move(int playerX, int& dir)
{
	if (returningToFormation)
	{
		moveIntoFormation();
		return;
	}

	if (!attacking &&movement != LEFT_RIGHT &&movement != ZIGZAG)
	{
		return;
	}

	delay++;

	switch (movement)
	{
	case Movement::STRAIGHT:

		if (delay >= speed - 4)
		{
			moveStraight();
			delay = 0;
		}
		break;

	case Movement::LEFT_RIGHT:

		if (delay >= speed +5)
		{
			moveLeftRight(dir);
			delay = 0;
		}
		break;

	case Movement::DIVE:
		if (delay >= speed - 1)
		{
			moveDive(playerX);
			delay = 0;
		}

		break;

	case Movement::ZIGZAG:
		if (delay >= enemyMoveDelay * 2 / speed)
		{
			moveZigzag();
			delay = 0;
		}
		break;
	}


	if (y >= height)
	{
		y = 0;
		returningToFormation = true;

		if (movement != Movement::LEFT_RIGHT && movement != Movement::ZIGZAG)
		{
			attacking = false;
		}
	}
}

void Enemy::setFireType(FireType fire)
{
	fireType = fire;
}

FireType Enemy::getFireType() const
{
	return fireType;
}

void Enemy::moveStraight()
{
	y++;
}

void Enemy::moveDive(int playerX)
{
	if (playerX > x)
	{
		x++;
	}
	else if (playerX < x)
	{
		x--;
	}

	y++;

}

void Enemy::moveLeftRight(int& dir)
{
	x += dir;
	steps++;

	if (x <= 0 || x >= width - 1)
	{
		x = (x <= 0) ? 0 : width - 1;
	}

	if (steps >= enemyDownSteps)
	{
		y++;
		steps = 0;
		if (y >= height)
		{
			y = 0;
		}
	}
}

void Enemy::moveZigzag()
{
	switch (zigzagState)
	{
	case 0:
		if (x > 0 && x < width - 1 && y>0)
		{
			x++;
			y++;
		}
		break;

	case 1:
		if (x > 0 && x <= width - 1 && y > 0)
		{
			x--;
			y--;
		}
		break;

	case 2:
		if (x > 0 && x <= width - 1 && y >= 0)
		{
			x--;
			y++;
		}
		break;

	case 3:
		if (x >= 0 && x < width - 1 && y>0)
		{
			x++;
			y--;
		}
		break;
	}

	zigzagCounter++;

	if (zigzagCounter == 3)
	{
		zigzagCounter = 0;
		zigzagState++;

		if (zigzagState > 3)
		{
			zigzagState = 0;
			y++;
		}
	}
}

void Enemy::setFormationPosition(int a, int b)
{
	xfor = a;
	yfor = b;
}

bool Enemy::getInFormation() { return inFormation; }

void Enemy::setInFormation(bool b) { inFormation = b; }

void Enemy::moveIntoFormation()
{
	if (x < xfor)
	{
		x++;
	}
	else if (x > xfor)
	{
		x--;
	}

	if (y < yfor)
	{
		y++;
	}

	if (x == xfor && y == yfor)
	{
		inFormation = true;
		returningToFormation = false;
	}
}

bool Enemy::isReturningToFormation()
{
	return returningToFormation;
}

bool Enemy::canShoot() const
{
	return shootCooldown == 0;
}

void Enemy::updateCooldown()
{
	if (shootCooldown > 0)
	{
		shootCooldown--;
	}
}

void Enemy::setCooldown(int a)
{
	shootCooldown = a;
}

int Enemy::getFormationX()
{
	return xfor;
}

int Enemy::getFormationY()
{
	return yfor;
}

void Enemy::setElite(bool value)
{
	elite = value;
}

bool Enemy::isElite() const
{
	return elite;
}

int Enemy::getBaseCooldown() { return baseCooldown; }

void Enemy::setReturningToFormation(bool b)
{
	returningToFormation = b;
}