#include "BossEngine.h"

//--------------------------------------------------------

void BossEngine::setup(int lvl)
{
	attackStartDelay = true;
	attackStartTimer = 0;

	charging = false;
	chargeDirection = 0;
	chargeDistance = 0;

	level = lvl;

	wingmanCount = 0;

	dashing = false;
	dashDirection = 0;
	dashDistance = 0;
	dashDelay = 0;

	for (int i = 0; i < 8; i++)
	{
		wingman[i].removeAlive();

		for (int j = 0; j < 3; j++)
		{
			wingmanBullet[i][j].deactivate();
		}
	}

	// ---------------------------------------------------------
	// POSITION
	// ---------------------------------------------------------

	x = width / 2 - 3;
	y = 1;

	// ---------------------------------------------------------
	// RESET BOSS STATE
	// ---------------------------------------------------------

	phase = 1;

	direction = (rand() % 2 == 0) ? -1 : 1;

	moveCounter = 0;
	shootCounter = 0;
	attackCounter = 0;

	attackPause = 0;

	tripleCounter = 0;
	diagonalCounter = 0;
	volleyCounter = 0;

	teleporting = false;
	teleportTimer = 0;

	cloneActive = false;
	clonesUsed = false;
	cloneTimer = 0;

	cloneX[0] = -100;
	cloneX[1] = -100;

	enraged = false;

	// ---------------------------------------------------------
	// IMPORTANT:
	// EVERY BOSS STARTS WITH A KNOWN ATTACK
	// ---------------------------------------------------------

	currentAttack = TRIPLE_STORM;

	// ---------------------------------------------------------
	// BOSS STATS
	// ---------------------------------------------------------

	switch (level)
	{
	case 10:

		maxHealth = 50;

		moveDelay = 6;
		shootDelay = 10;

		break;


	case 20:

		maxHealth = 75;

		moveDelay = 4;
		shootDelay = 8;

		break;


	case 30:

		maxHealth = 100;

		moveDelay = 3;
		shootDelay = 7;

		break;


	case 40:

		maxHealth = 120;

		moveDelay = 2;
		shootDelay = 6;

		currentAttack = WINGMEN;

		break;


	case 50:

		maxHealth = 150;

		moveDelay = 2;
		shootDelay = 5;

		break;


	default:

		// Safety fallback
		maxHealth = 50;
		moveDelay = 6;
		shootDelay = 10;

		break;
	}

	// ---------------------------------------------------------
	// HEALTH
	// ---------------------------------------------------------

	health = maxHealth;

	// 5 power drops over the boss fight
	powerDropStep = maxHealth / 5;

	nextPowerDrop = maxHealth - powerDropStep;

	// ---------------------------------------------------------
	// CLEAR BOSS BULLETS
	// ---------------------------------------------------------

	for (int i = 0; i < 50; i++)
	{
		bullet[i].deactivate();
	}
}

void BossEngine::update(int playerX)
{
	if (attackStartDelay)
	{
		attackStartTimer++;

		if (attackStartTimer >= attackStartDelayTime)
		{
			attackStartDelay = false;
			attackStartTimer = 0;

			currentAttack = TRIPLE_STORM;
			attackCounter = 0;
			tripleCounter = 0;
			diagonalCounter = 0;
			volleyCounter = 0;
		}

		move();

		return;
	}
	if (dashing)
	{
		dashDelay++;

		if (dashDelay >= 2)
		{
			dashDelay = 0;

			x += dashDirection * 3;

			dashDistance += 3;

			if (x < 2)
				x = 2;

			if (x > width - 11)
				x = width - 11;

			if (dashDistance >= 24)
			{
				dashing = false;

				dashDistance = 0;

				shootTriple();

				shootDiagonal();

				if (phase == 2)
					shootTriple();

				nextAttack();
			}
		}
	}

	move();

	if (teleporting)
	{
		teleport();
	}
	else
	{
		if (charging)
		{
			charge();
		}
		else
		{
			attack(playerX);
		}
	}

	cloneShoot();

	if (cloneActive)
	{
		cloneTimer++;

		if (cloneTimer >= 420)
		{
			cloneActive = false;

			cloneTimer = 0;

			swapCounter = 0;

			cloneX[0] = -100;
			cloneX[1] = -100;

			nextAttack();
		}
	}

	if (health <= maxHealth / 2)
	{
		phase = 2;
	}

	if (cloneActive && attackCounter % 30 == 0)
	{
		int oldX = x;

		shootTriple();      

		for (int i = 0; i < 2; i++)
		{
			x = cloneX[i];

			if (rand() % 2)
				shootTriple();
			else
				shootDiagonal();
		}

		x = oldX;
	}

	if (!enraged && health <= maxHealth / 6)
	{
		enraged = true;

		moveDelay = 2;

		shootDelay = 5;
	}

	for (int i = 0; i < 50; i++)
	{
		if (!bullet[i].isActive())
			continue;

		bullet[i].move();
	}

	for (int i = 0; i < wingmanCount; i++)
	{
		if (!wingman[i].getAlive())
			continue;

		wingman[i].updateCooldown();

		if (!wingman[i].getInFormation())
			wingman[i].moveIntoFormation();

		if (wingman[i].getInFormation())
			wingman[i].move(playerX, direction);

		if (wingman[i].canShoot())
		{
			for (int b = 0; b < 3; b++)
			{
				if (!wingmanBullet[i][b].isActive())
				{
					int dx = playerX - wingman[i].getX();

					if (dx > 1) dx = 1;
					if (dx < -1) dx = -1;

					wingmanBullet[i][b].spawn(wingman[i].getX(), wingman[i].getY(), 1, dx);

					wingman[i].setCooldown(wingman[i].getBaseCooldown());

					break;
				}
			}
		}

		for (int b = 0; b < 3; b++)
		{
			if (wingmanBullet[i][b].isActive())
				wingmanBullet[i][b].move();
		}
	}
}

void BossEngine::move()
{
	if (cloneActive)
		return;

	if (dashing)
		return;

	moveCounter++;

	moveDelay = enraged ? moveDelay/2 : (phase == 2 ? moveDelay / 2 : moveDelay);

	if (phase == 2)
		moveDelay = 3;

	if (enraged)
		moveDelay = 2;

	if (moveCounter < moveDelay)
		return;

	moveCounter = 0;

	if (currentAttack == VOLLEY)
		return;

	if (rand() % 120 == 0)
		direction *= -1;

	if (dashing)
	{
		x += direction;

		if (rand() % 8 == 0)
		{
			direction = (direction > 0) ? 1 : -1;

			dashing = false;
		}
	}
	else
	{
		x += direction;
	}
	if (x <= 2)
	{
		x = 2;
		direction = 1;
	}

	if (x >= width - 11)
	{
		x = width - 11;
		direction = -1;
	}
}

void BossEngine::attack(int playerX)
{
	attackCounter++;

	if (attackPause > 0)
	{
		attackPause--;
		return;
	}

	switch (currentAttack)
	{
	case TRIPLE_STORM:

		tripleStorm();

		break;

	case DIAGONAL_WAVE:

		diagonalWave();

		break;

	case VOLLEY:

		precisionVolley();

		break;

	case WINGMEN:

		shouldSpawnWingmen();

		break;

	case BARRAGE:

		barrage();

		break;

	case LASER_RAIN:

		laserRain();

		break;

	case CHARGE:

		if (!charging)
		{
			startCharge(playerX);
		}

		break;

	case DASH:

		if (!dashing)
		{
			dashAttack(playerX);
		}

		break;

	case TELEPORT:

		if (!teleporting)
		{
			teleporting = true;
			teleportTimer = 0;
		}

		break;

	case CLONES:

		if (!cloneActive)
		{
			spawnClones();
		}

		break;

	case CROSS_FIRE:

		crossFire();

		break;
	}

}

void BossEngine::nextAttack()
{
	attackCounter = 0;

	tripleCounter = 0;
	diagonalCounter = 0;
	volleyCounter = 0;

	attackPause = 25;

	if (level == 10)
	{
		switch (currentAttack)
		{
		case TRIPLE_STORM:

			currentAttack = DIAGONAL_WAVE;
			break;


		case DIAGONAL_WAVE:

			currentAttack = VOLLEY;
			break;


		case VOLLEY:

			currentAttack = WINGMEN;
			break;


		case WINGMEN:

			currentAttack = TRIPLE_STORM;
			break;


		default:

			currentAttack = TRIPLE_STORM;
			break;
		}

		return;
	}


	if (level == 20)
	{
		switch (currentAttack)
		{
		case TRIPLE_STORM:

			currentAttack = BARRAGE;

			break;

		case BARRAGE:

			currentAttack = CHARGE;

			break;

		case CHARGE:

			currentAttack = LASER_RAIN;

			break;

		case VOLLEY:

			currentAttack = BARRAGE;

			break;

		case LASER_RAIN:

			currentAttack = VOLLEY;

			break;

		default:

			currentAttack = TRIPLE_STORM;
		}

		return;
	}

	if (level == 30)
	{
		switch (currentAttack)
		{
		case TRIPLE_STORM:

			currentAttack = DASH;

			break;

		case DASH:

			currentAttack = CLONES;

			break;

		case TELEPORT:

			currentAttack = VOLLEY;

			break;

		case CLONES:

			currentAttack = DIAGONAL_WAVE;

			break;

		case DIAGONAL_WAVE:

			currentAttack = TELEPORT;

			break;

		case VOLLEY:

			currentAttack = DASH;
		}

		return;
	}

	if (level == 40)
	{
		switch (currentAttack)
		{
		case WINGMEN:

			currentAttack = CROSS_FIRE;

			break;

		case CROSS_FIRE:

			currentAttack = LASER_RAIN;

			break;

		case LASER_RAIN:

			currentAttack = VOLLEY;

			break;

		case VOLLEY:

			currentAttack = DIAGONAL_WAVE;

			break;

		case DIAGONAL_WAVE:

			currentAttack = BARRAGE;

			break;

		case BARRAGE:

			currentAttack = WINGMEN;
		default:

			currentAttack = WINGMEN;
		}

		return;
	}

	if (level == 50)
	{
		switch (currentAttack)
		{
		case TRIPLE_STORM:

			currentAttack = WINGMEN;

			break;

		case WINGMEN:

			currentAttack = DASH;

			break;

		case DASH:

			currentAttack = BARRAGE;

			break;

		case BARRAGE:

			currentAttack = CROSS_FIRE;

			break;

		case CROSS_FIRE:

			currentAttack = TELEPORT;

			break;

		case TELEPORT:

			currentAttack = LASER_RAIN;

			break;

		case LASER_RAIN:

			currentAttack = CLONES;

			break;

		case CLONES:

			currentAttack = VOLLEY;

			break;

		case VOLLEY:

			currentAttack = DIAGONAL_WAVE;

			break;

		case DIAGONAL_WAVE:

			currentAttack = CHARGE;

			break;

		case CHARGE:

			currentAttack = TRIPLE_STORM;

			break;

		default:

			currentAttack = TRIPLE_STORM;

			break;
		}

		return;
	}
}

void BossEngine::setCharacters(char grid[][width])
{
	if (teleporting)
	{
		for (int i = 0; i < 50; i++)
		{
			if (!bullet[i].isActive())
				continue;

			int bx = bullet[i].getX();
			int by = bullet[i].getY();

			if (bx >= 0 && bx < width &&
				by >= 0 && by < height)
			{
				grid[by][bx] = '|';
			}
		}

		return;
	}

	char sprite[5][9] =
	{
		{' ','/','^','^','^','^','^','\\',' '},
		{'<','#','#','#','#','#','#','#','>'},
		{' ','#','O',' ',' ',' ','O','#','>'},
		{' ','#','#','#','#','#','#','#','>'},
		{' ',' ',' ',' ' ,' ',' ',' ',' ',' '}
	};

	drawClones(grid);

	for (int r = 0;r < 5;r++)
	{
		for (int c = 0;c < 9;c++)
		{
			if (sprite[r][c] == ' ')
				continue;

			int gx = x + c;
			int gy = y + r;

			if (gx >= 0 && gx < width && gy >= 0 && gy < height)
			{
				grid[gy][gx] = sprite[r][c];
			}
		}
	}

	for (int i = 0;i < 50;i++)
	{
		if (!bullet[i].isActive())
			continue;

		int bx = bullet[i].getX();
		int by = bullet[i].getY();

		if (bx >= 0 && bx < width &&
			by >= 0 && by < height)
		{
			grid[by][bx] = '|';
		}
	}

	for (int i = 0; i < wingmanCount; i++)
	{
		if (!wingman[i].getAlive())
			continue;

		int wx = wingman[i].getX();
		int wy = wingman[i].getY();

		if (wx >= 0 && wx < width &&wy >= 0 && wy < height)
		{
			grid[wy][wx] = 'P';
		}
		for (int j = 0; j < 3; j++)
		{
			if (!wingmanBullet[i][j].isActive())
				continue;

			int bx = wingmanBullet[i][j].getX();
			int by = wingmanBullet[i][j].getY();

			if (bx >= 0 && bx < width &&
				by >= 0 && by < height)
			{
				grid[by][bx] = '|';
			}
		}
	}
}

bool BossEngine::checkCollision(Bullet playerBullet[], int playerBulletCount, int playerX, int playerY, int& playerHealth, bool& invincible, int& invincibleCounter, bool doubleDamage)
{
	if (teleporting)
	{
		return false;
	}

	for (int i = 0; i < playerBulletCount; i++)
	{
		if (!playerBullet[i].isActive())
		{
			continue;
		}

		int bx = playerBullet[i].getX();
		int by = playerBullet[i].getY();

		if (bx >= x && bx < x + 9 &&
			by >= y && by < y + 5)
		{
			int damage = doubleDamage ? 2 : 1;

			health -= damage;

			playerBullet[i].deactivate();

			if (health <= 0)
			{
				health = 0;
			}

			return true;
		}
	}

	if (playerX >= x && playerX < x + 9 &&
		playerY >= y && playerY < y + 5)
	{
		if (!invincible)
		{
			playerHealth--;
			invincible = true;
			invincibleCounter = 0;

			health--;

			if (health <= 0)
			{
				health = 0;
			}

			return false;
		}
	}

	for (int i = 0; i < 50; i++)
	{
		if (!bullet[i].isActive())
		{
			continue;
		}

		if (bullet[i].getX() == playerX &&
			bullet[i].getY() == playerY)
		{
			if (!invincible)
			{
				playerHealth--;
				invincible = true;
				invincibleCounter = 0;
			}

			bullet[i].deactivate();
		}
	}

	for (int i = 0; i < wingmanCount; i++)
	{
		if (!wingman[i].getAlive())
			continue;

		for (int b = 0; b < playerBulletCount; b++)
		{
			if (!playerBullet[b].isActive())
				continue;

			if (playerBullet[b].getX() == wingman[i].getX() &&
				playerBullet[b].getY() == wingman[i].getY())
			{
				wingman[i].setHealth(wingman[i].getHealth() - 1);

				playerBullet[b].deactivate();

				if (wingman[i].getHealth() <= 0)
				{
					wingman[i].removeAlive();
				}
				break;
			}
		}

		if (playerX == wingman[i].getX() &&
			playerY == wingman[i].getY())
		{
			if (!invincible)
			{
				playerHealth -= wingman[i].getDamage();

				wingman[i].setHealth(wingman[i].getHealth() - 1);

				
			}

			if (wingman[i].getHealth() <= 0)
			{
				wingman[i].removeAlive();
			}
		}

		for (int b = 0; b < 3; b++)
		{
			if (!wingmanBullet[i][b].isActive())
				continue;

			if (wingmanBullet[i][b].getX() == playerX &&
				wingmanBullet[i][b].getY() == playerY)
			{
				if (!invincible)
				{
					playerHealth -= wingman[i].getDamage();

					invincible = true;

					invincibleCounter = 0;
				}

				wingmanBullet[i][b].deactivate();
			}
		}
	}

	return false;
}

//--------------------------------------------------------

void BossEngine::shootSingle()
{
	for (int i = 0; i < 50; i++)
	{
		if (!bullet[i].isActive())
		{
			bullet[i].spawn(x + 3, y + 2, 1, 0);
			break;
		}
	}
}

void BossEngine::shootTriple()
{
	for (int i = 0; i < 48; i += 3)
	{
		if (!bullet[i].isActive() && !bullet[i + 1].isActive() && !bullet[i + 2].isActive())
		{
			bullet[i].spawn(x + 2, y + 2, 1, 0);

			bullet[i + 1].spawn(x + 3, y + 2, 1, 0);

			bullet[i + 2].spawn(x + 4, y + 2, 1, 0);

			bullet[i + 3].spawn(x + 6, y + 2, 1, 0);

			bullet[i + 4].spawn(x + 5, y + 2, 1, 0);

			break;
		}
	}
}

void BossEngine::shootDiagonal()
{
	for (int i = 0; i < 48; i += 3)
	{
		if (!bullet[i].isActive() &&
			!bullet[i + 1].isActive() &&
			!bullet[i + 2].isActive())
		{
			bullet[i].spawn(x + 3, y + 2, 1, -1);

			bullet[i + 1].spawn(x + 3, y + 2, 1, 0);

			bullet[i + 2].spawn(x + 3, y + 2, 1, 1);

			break;
		}
	}
}

//--------------------------------------------------------

void BossEngine::tripleStorm()
{
	int delay = (phase == 1) ? 8 : 6;

	if (enraged)
		delay = 4;

	if (attackCounter % delay == 0)
	{
		shootTriple();

		if (phase == 2)
			shootSingle();

		if (enraged)
			shootDiagonal();

		tripleCounter++;
	}

	if (tripleCounter >= (phase == 1 ? 25 : 45))
	{
		nextAttack();
	}
}

void BossEngine::diagonalWave()
{
	int delay = enraged ? 10 : 18;

	if (attackCounter % delay == 0)
	{
		shootDiagonal();

		if (phase == 2)
			shootDiagonal();

		if (enraged)
			shootTriple();
		
		diagonalCounter++;
	}

	if (diagonalCounter >= (phase == 1 ? 3 : 5)) {
		nextAttack();
	}
}

void BossEngine::precisionVolley()
{
	int delay = (phase == 1) ? 14 : 10;

	if (attackCounter % delay != 0)
		return;

	shootVolley();

	if (phase == 2)
	{
		shootTriple();
		shootVolley();
	}

	if (enraged)
	{
		shootDiagonal();
		shootTriple();
	}

	volleyCounter++;

	if (volleyCounter >= (phase == 1 ? 3 : 5))
	{
		nextAttack();
	}
}

void BossEngine::shootVolley()
{
	int dx[] = { -4,-3,-2,-1,0,1,2,3,4 };

	int bulletIndex = 0;

	for (int i = 0; i < 9; i++)
	{
		while (bulletIndex < 50 && bullet[bulletIndex].isActive())
		{
			bulletIndex++;
		}

		if (bulletIndex >= 50)
		{
			return;
		}

		bullet[bulletIndex].spawn(x + 4, y + 4, 1, dx[i]);

		bulletIndex++;
	}
}

void BossEngine::crossFire()
{
	int delay = (phase == 1) ? 12 : 8;

	if (attackCounter % delay != 0)
		return;

	shootCrossFire();

	if (level == 50)
	{
		shootTriple();
		shootDiagonal();
	}

	if (phase == 2)
	{
		shootTriple();
	}

	if (enraged)
	{
		shootDiagonal();
	}

	volleyCounter++;

	if (volleyCounter >= 5)
	{
		nextAttack();
	}
}

void BossEngine::shootCrossFire()
{
	static int rotation = 0;

	int bulletIndex = 0;

	const int dx[4][4] =
	{
		{-1, 1, -1, 1},
		{ 0, 1, -1, 0},
		{ 1,-1,  1,-1},
		{ 1, 0,  0,-1}
	};

	for (int i = 0; i < 4; i++)
	{
		while (bulletIndex < 50 && bullet[bulletIndex].isActive())
			bulletIndex++;

		if (bulletIndex >= 50)
			return;

		int sx = (i < 2) ? x + 2 : x + 6;

		bullet[bulletIndex].spawn(
			sx,
			y + 4,
			1,
			dx[rotation][i]);

		bulletIndex++;
	}

	rotation++;

	if (rotation >= 4)
		rotation = 0;
}

bool BossEngine::shouldSpawnWingmen()
{
	if (currentAttack != WINGMEN)

		return false;

	for (int i = 0; i < wingmanCount; i++)
	{
		if (wingman[i].getAlive())
		{
			nextAttack();

			return false;
		}
	}

	if (level == 40)
	{
		wingmanCount = (phase == 1) ? 6 : 8;
	}
	else if (level == 50)
	{
		wingmanCount = 8;
	}
	else
	{
		wingmanCount = (phase == 1) ? 2 : 4;
	}

	const int spawnX[8] =
	{
		6,width-7,9,width - 10,12,width - 13,width - 16,15
	};

	for (int i = 0; i < wingmanCount; i++)
	{
		wingman[i].setAlive();

		wingman[i].setElite(false);

		wingman[i].setType(SPEEDY);

		wingman[i].setMovement(STRAIGHT);

		wingman[i].setFireType(SINGLE);

		wingman[i].setAttacking(true);

		wingman[i].setInFormation(false);

		wingman[i].setFormationPosition(spawnX[i], y + 5);

		wingman[i].setPosition(spawnX[i], -2 - i * 2);

		for (int b = 0; b < 3; b++)
		{
			wingmanBullet[i][b].deactivate();
		}
	}

	nextAttack();

	return true;
}

void BossEngine::barrage()
{
	int delay = (phase == 1) ? 8 : 6;

	if (attackCounter % delay == 0)
	{
		shootTriple();

		shootDiagonal();

		if (phase == 2)
			shootTriple();

		if (enraged)
			shootDiagonal();

		tripleCounter++;
	}

	if (tripleCounter >= (phase == 1 ? 25 : 40))
	{
		nextAttack();
	}
}

void BossEngine::laserRain()
{
	int delay = (phase == 1) ? 12 : 8;

	if (attackCounter % delay == 0)
	{
		shootRain();

		if (phase == 2 && attackCounter % (delay * 2) == 0)
			shootRain();

		volleyCounter++;
	}

	if (volleyCounter >= (phase == 1 ? 16 : 28))
	{
		nextAttack();
	}
}

void BossEngine::shootRain()
{
	for (int i = 0; i < 10; i++)
	{
		int bulletIndex = -1;

		for (int j = 0; j < 50; j++)
		{
			if (!bullet[j].isActive())
			{
				bulletIndex = j;
				break;
			}
		}

		if (bulletIndex == -1)
			return;

		int rx = rand() % (width - 2) + 1;

		bullet[bulletIndex].spawn(rx, 0, 1, 0);
	}
}

void BossEngine::startCharge(int playerX)
{
	if (charging)
		return;

	charging = true;

	chargeDistance = 0;

	if (playerX > x + 4)
		chargeDirection = 2;
	else
		chargeDirection = -2;
}

void BossEngine::charge()
{
	x += chargeDirection;

	chargeDistance += abs(chargeDirection);

	if (x < 2)
		x = 2;

	if (x > width - 11)
		x = width - 11;

	if (chargeDistance >= 24)
	{
		charging = false;

		shootTriple();

		shootDiagonal();

		if (phase == 2)
		{
			shootTriple();
		}

		nextAttack();
	}
}

void BossEngine::dashAttack(int playerX)
{
	if (dashing)
		return;

	dashing = true;

	dashDistance = 0;

	dashDelay = 0;

	if (playerX > x + 4)
		dashDirection = 1;
	else
		dashDirection = -1;
}

void BossEngine::teleport()
{
	teleportTimer++;

	if (teleportTimer < teleportDelay)
		return;

	teleporting = false;

	teleportTimer = 0;

	x = rand() % (width - 12) + 2;

	shootDiagonal();

	if (phase == 2)
	{
		shootTriple();
	}

	nextAttack();
}

//--------------------------------------------------------

void BossEngine::spawnClones()
{
	if (cloneActive)
		return;

	cloneActive = true;

	cloneTimer = 0;

	const int bossWidth = 9;
	const int margin = 2;

	int pos[3];

	pos[0] = margin;
	pos[1] = (width - bossWidth) / 2;
	pos[2] = width - bossWidth - margin;

	// Shuffle positions
	for (int i = 0; i < 3; i++)
	{
		int r = rand() % 3;

		int temp = pos[i];
		pos[i] = pos[r];
		pos[r] = temp;
	}

	// First position = real boss
	x = pos[0];

	// Remaining = clones
	cloneX[0] = pos[1];
	cloneX[1] = pos[2];
}

void BossEngine::drawClones(char grid[][width])
{
	if (!cloneActive)
		return;

	char sprite[5][9] =
	{
		{' ','/','^','^','^','^','^','\\',' '},
		{'<','#','#','#','#','#','#','#','>'},
		{' ','#','O',' ',' ',' ','O','#','>'},
		{' ','#','#','#','#','#','#','#','>'},
		{' ',' ',' ',' ' ,' ',' ',' ',' ',' '}
	};

	for (int c = 0; c < 2; c++)
	{
		for (int r = 0; r < 5; r++)
		{
			for (int k = 0; k < 9; k++)
			{
				if (sprite[r][k] == ' ')
					continue;

				int gx = cloneX[c] + k;
				int gy = y + r;

				if (gx >= 0 && gx < width &&
					gy >= 0 && gy < height)
				{
					grid[gy][gx] = sprite[r][k];
				}
			}
		}
	}
}

void BossEngine::cloneShoot()
{
	if (!cloneActive)
		return;

	if (attackCounter % 25 != 0)
		return;

	int oldX = x;

	for (int i = 0; i < 2; i++)
	{
		x = cloneX[i];

		switch (currentAttack)
		{
		case TRIPLE_STORM:

			shootTriple();

			break;

		case DIAGONAL_WAVE:

			shootDiagonal();

			break;

		case VOLLEY:

			shootVolley();

			break;

		default:

			shootSingle();
		}
	}
	x = oldX;
}

//--------------------------------------------------------

bool BossEngine::isEnraged() const
{
	return enraged;
}

bool BossEngine::isDead() const
{
	return health <= 0;
}

int BossEngine::getHealth() const
{
	return health;
}

int BossEngine::getMaxHealth() const
{
	return maxHealth;
}

//---------------------------------------------------------

bool BossEngine::shouldDropPower()
{
	if (health <= nextPowerDrop)
	{
		nextPowerDrop -= powerDropStep;

		if (nextPowerDrop < 1)
			nextPowerDrop = 1;

		return true;
	}

	return false;
}

int BossEngine::getX() const
{
	return x;
}

int BossEngine::getY() const
{
	return y;
}