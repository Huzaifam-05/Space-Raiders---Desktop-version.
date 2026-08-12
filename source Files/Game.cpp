#include "raylib.h"
#include "Game.h"
#include "Graphics.h"

Game::Game()
{
	enemyCount = enemyCounts;
	health = playerHealth;

	formationDirection = rand() % 2;

	maxPlayerHealth = playerHealth;
	maxPlayerHeat = maxHeat;

	if (formationDirection == 0)
	{
		formationDirection = -1;
	}

	enemy = nullptr;
	eBullet = nullptr;

	clearGrid();

	std::ifstream saveFile("save.dat", std::ios::binary);
	hasSave = saveFile.good();
	saveFile.close();
}

Game::~Game()
{
	delete[] eBullet;
	delete[] enemy;
}

void Game::start()
{
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);

	InitWindow(
		1600,
		900,
		"Space Raiders"
	);

	SetExitKey(KEY_NULL);

	graphics.loadSounds();

	gameState = GameState::MAIN_MENU;
	paused = false;

	double previousTime = GetTime();
	double accumulator = 0.0;

	const double fixedStep = gameDelay / 1000.0;

	while (!WindowShouldClose())
	{

		if (IsKeyPressed(KEY_F11))
		{
			ToggleFullscreen();
		}

		graphics.loadSounds();

		double currentTime = GetTime();

		double deltaTime = currentTime - previousTime;

		previousTime = currentTime;

		if (deltaTime > 0.25)
		{
			deltaTime = 0.25;
		}

		if (gameState == GameState::PLAYING)
		{
			if (IsKeyPressed(KEY_ESCAPE))
			{
				paused = !paused;
				accumulator = 0.0;
				previousTime = GetTime();
			}
		}

		if (gameState == GameState::PLAYING && !paused)
		{
			accumulator += deltaTime;
		}
		else
		{
			accumulator = 0.0;
		}

		if (gameState == GameState::MAIN_MENU)
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				startGame();
				accumulator = 0.0;
				previousTime = GetTime();
			}
		}
		else if (gameState == GameState::LEVEL_INTRO)
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				gameState = GameState::PLAYING;
				accumulator = 0.0;
				previousTime = GetTime();
			}
			else if (GetTime() - levelIntroStartTime >= levelIntroDuration)
			{
				gameState = GameState::PLAYING;
				accumulator = 0.0;
				previousTime = GetTime();
			}
		}
		else if (gameState == GameState::END_SCREEN)
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				gameState = GameState::MAIN_MENU;

				paused = false;

				accumulator = 0.0;
				previousTime = GetTime();

				BeginDrawing();

				graphics.mainMenu(*this);

				EndDrawing();

				PollInputEvents();
				SwapScreenBuffer();

				continue;
			}
		}

		if (gameState == GameState::PLAYING && !paused)
		{
			while (accumulator >= fixedStep &&
				!isGameOver &&
				!WindowShouldClose())
			{
				if (!boss && enemyCounter == enemyCount)
				{
					if (health < maxPlayerHealth)
					{
						health++;
					}

					levelSetup();

					enemyCounter = 0;

					accumulator = 0.0;
					break;
				}

				clearGrid();

				input();

				update();

				checkCollision();

				setCharacters();

				if (health <= 0)
				{
					graphics.playPlayerDeath();
					isGameOver = true;
					playerWon = false;
					gameState = GameState::END_SCREEN;
					paused = false;
					accumulator = 0.0;
					break;
				}

				accumulator -= fixedStep;
			}

			if (level > 50)
			{
				playerWon = true;
				gameState = GameState::END_SCREEN;
				paused = false;
				accumulator = 0.0;
			}
		}

		BeginDrawing();

		if (gameState == GameState::MAIN_MENU)
		{
			graphics.mainMenu(*this);
		}
		else if (gameState == GameState::LEVEL_INTRO)
		{
			graphics.levelIntro(*this);
		}
		else if (gameState == GameState::PLAYING)
		{
			graphics.render(*this);

			if (paused)
			{
				graphics.pauseMenu(*this);
			}
		}
		else if (gameState == GameState::END_SCREEN)
		{
			graphics.endScreen(*this);
		}

		EndDrawing();

		PollInputEvents();
		SwapScreenBuffer();
	}

	graphics.unloadSounds();

	CloseWindow();
}

void Game::update()
{
	updatePowerUp();
	updatePowerTimers();
	collectPowerUp();

	playerShootCounter++;

	if (overheated)
	{
		coolCounter++;

		if (coolCounter >= coolDelay)
		{
			coolCounter = 0;

			if (heat > 0)
			{
				heat--;
			}

			if (heat == 0)
			{
				overheated = false;
			}
		}
	}

	if (invincible)
	{
		invincibleCounter++;

		if (invincibleCounter >= invincibleDelay)
		{
			invincible = false;
			invincibleCounter = 0;
		}
	}

	for (int i = 0; i < playerBulletCount; i++)
	{
		bullet[i].move();
	}

	if (!overheated)
	{
		if (!IsKeyDown(KEY_SPACE))
		{
			coolCounter++;

			if (coolCounter >= coolDelay)
			{
				coolCounter = 0;

				if (heat > 0)
				{
					heat--;
				}
			}
		}
		else
		{
			coolCounter = 0;
		}
	}

	if (boss)
	{
		bossEngine.update(player.getX());
	}
	else
	{
		enemyUpdate();
	}
}

void Game::enemyUpdate()
{
	ready = true;

	if (!canEnemiesAttack)
	{
		for (int i = 0; i < enemyCount; i++)
		{
			if (!enemy[i].getInFormation())
			{
				enemy[i].moveIntoFormation();

				ready = false;
			}
		}
	}

	if (ready && !canEnemiesAttack)
	{
		levelStartCounter++;

		if (levelStartCounter >= levelStartDelay)
		{
			canEnemiesAttack = true;

			for (int i = 0; i < enemyCount; i++)
			{
				if (!enemy[i].getAlive())
					continue;

				if (rand() % 2 == 0)
				{
					enemyShoot(i);

					enemy[i].setCooldown(enemy[i].getBaseCooldown());
				}
			}

			if (generator.movement == DIVE || generator.movement == STRAIGHT)
			{
				for (int i = 0; i < enemyCount; i++)
				{
					enemy[i].setAttacking(false);
				}
			}
		}
	}

	bulletDelayCounter++;
	shootTimer++;

	if (level >= 40 && level % 10 != 0 && !reinforcementSpawned)
	{
		if (enemyCount - enemyCounter <= reinforcementCount + 5)
		{
			spawnReinforcements();
			reinforcementSpawned = true;
		}
	}

	if (level >= 40 && level % 10 != 0 &&
		!elitePromoted)
	{
		int alive = enemyCount - enemyCounter;

		if (alive <= (3 + (level % 10)))
		{
			promoteRemainingEnemies();
			elitePromoted = true;
		}
	}

	for (int i = 0; i < enemyCount; i++)
	{
		if (enemy[i].getAlive())
		{
			enemy[i].updateCooldown();
		}
	}

	if (canEnemiesAttack)
	{
		if (generator.movement == DIVE || generator.movement == STRAIGHT)
		{
			attackTimerCounter++;

			if (attackTimerCounter >= attackDelay * 6)
			{
				chooseAttackers();
				attackTimerCounter = 0;
			}
		}
	}

	bool reverseFormation = false;

	if (canEnemiesAttack && generator.movement == LEFT_RIGHT)
	{
		for (int i = 0; i < enemyCount; i++)
		{
			if (enemy[i].getAlive())
			{
				int nextX = enemy[i].getX() + formationDirection;

				if (nextX < 1 || nextX > width - 2)
				{
					reverseFormation = true;
					break;
				}
			}
		}

		if (reverseFormation)
		{
			formationDirection *= -1;
		}
	}

	for (int i = 0; i < enemyCount; i++)
	{
		if (enemy[i].getAlive() && canEnemiesAttack)
		{
			enemy[i].move(player.getX(), formationDirection);

			int aliveEnemies = enemyCount - enemyCounter;

			if (aliveEnemies <= 3)
			{
				shootDelay = 10;
			}
			else
			{
				shootDelay = 30 - shootDelayScaling;

				if (shootDelay < 10)
				{
					shootDelay = 10;
				}
			}

			if (shootTimer >= shootDelay)
			{
				chooseShooters();
				shootTimer = 0;
			}
		}

		for (int j = 0; j < 3; j++)
		{
			if (eBullet[i].bullet[j].isActive())
			{
				int s = enemy[i].isElite() ? 1 : 0;

				if (bulletDelayCounter >= enemyBulletDelay - bulletSpeedScaling - s)
				{
					eBullet[i].bullet[j].move();
				}
			}
		}
	}

	if (bulletDelayCounter == enemyBulletDelay - bulletSpeedScaling)
	{
		bulletDelayCounter = 0;
	}
}

void Game::input()
{
	playerMoveCounter++;

	if (playerMoveCounter >= playerMoveDelay)
	{
		if (IsKeyDown(KEY_A))
		{
			player.move('a');
			playerMoveCounter = 0;
		}
		if (IsKeyDown(KEY_D))
		{
			player.move('d');
			playerMoveCounter = 0;
		}
		if (IsKeyDown(KEY_W))
		{
			player.move('w');
			playerMoveCounter = 0;
		}
		if (IsKeyDown(KEY_S))
		{
			player.move('s');
			playerMoveCounter = 0;
		}
	}

	int currentShootDelay = playerShootDelay;

	if (rapidFire)
	{
		currentShootDelay /= 2;

		if (currentShootDelay < 1)
		{
			currentShootDelay = 1;
		}
	}

	if (IsKeyDown(KEY_SPACE) &&
		!overheated &&
		((!boss && ready) || boss) &&
		playerShootCounter >= currentShootDelay)
	{
		if ((!boss && ready) || boss)
		{
			int shots = diagonalShot ? 3 : 1;
			int spawned = 0;

			for (int i = 0; i < playerBulletCount && spawned < shots; i++)
			{
				if (!bullet[i].isActive())
				{
					int xDirection = 0;
					int yDirection = playerBulletDirection;

					if (diagonalShot)
					{
						if (spawned == 0)
						{
							xDirection = -1;
						}
						else if (spawned == 1)
						{
							xDirection = 0;
						}
						else
						{
							xDirection = 1;
						}
					}

					bullet[i].spawn(
						player.getX(),
						player.getY(),
						yDirection,
						xDirection
					);

					if (diagonalShot)
					{
						bullet[i].setTightDiagonal(true);
					}
					else
					{
						bullet[i].setTightDiagonal(false);
					}

					spawned++;
				}
			}
		}

		playerShootCounter = 0;

		heat++;

		if (heat >= maxPlayerHeat)
		{
			heat = maxPlayerHeat;
			overheated = true;
			overheatCounter = 0;
		}
	}
}

void Game::clearGrid()
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			grid[i][j] = '.';
		}
	}
}

void Game::setCharacters()
{
	//Player
	int px = player.getX();
	int py = player.getY();

	bool drawPlayer = true;

	if (py >= 0 && py < height)
	{
		if (px >= 0 && px < width)
		{
			grid[py][px] = 'A';
		}
	}


	for (int i = 0; i < playerBulletCount; i++)
	{
		if (!bullet[i].isActive())
		{
			continue;
		}

		int x = bullet[i].getX();
		int y = bullet[i].getY();

		if (x >= 0 && x < width &&
			y >= 0 && y + 1 < height)
		{
			grid[y + 1][x] = '^';
		}
	}

	// Power-ups
	for (int i = 0; i < maxPowerUps; i++)
	{
		if (!powerUp[i].isActive())
			continue;

		int px = powerUp[i].getX();
		int py = powerUp[i].getY();

		if (px < 0 || px >= width ||
			py < 0 || py >= height)
			continue;

		switch (powerUp[i].getType())
		{
		case POWER_HEALTH:
			grid[py][px] = 'H';
			break;

		case POWER_RAPID_FIRE:
			grid[py][px] = 'R';
			break;

		case POWER_SHIELD:
			grid[py][px] = 'S';
			break;

		case POWER_DOUBLE_DAMAGE:
			grid[py][px] = 'D';
			break;

		case POWER_DIAGONAL_SHOT:
			grid[py][px] = 'X';
			break;
		}
	}


	//Boss
	if (boss)
	{
		bossEngine.setCharacters(grid);
	}

	//Enemy
	else
	{

		for (int i = 0; i < enemyCount; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				if (enemy[i].getAlive())
				{
					int x = enemy[i].getX();
					int y = enemy[i].getY();

					if (x >= 0 && x < width &&
						y >= 0 && y < height)
					{
						grid[y][x] = enemy[i].isElite() ? 'M' : 'V';
					}
				}

				if (eBullet[i].bullet[j].isActive())
				{
					int x = eBullet[i].bullet[j].getX();
					int y = eBullet[i].bullet[j].getY();

					eBullet[i].bullet[j].setElite(enemy[i].isElite());

					if (x >= 0 && x < width &&
						y >= 0 && y < height)
					{
						grid[y][x] = '|';
					}
				}
			}
		}
	}

}

void Game::checkCollision()
{
	if (boss)
	{
		bossCollision();
	}
	else
	{
		enemyCollision();
	}
}

void Game::enemyCollision()
{
	for (int i = 0; i < enemyCount; i++)
	{
		if (enemy[i].getAlive())
		{

			for (int b = 0; b < playerBulletCount; b++)
			{
				if (!bullet[b].isActive())
				{
					continue;
				}

				if (bullet[b].getX() == enemy[i].getX() &&
					bullet[b].getY() == enemy[i].getY())
				{
					int damage = doubleDamage ? 2 : 1;

					enemy[i].setHealth(enemy[i].getHealth() - damage);

					if (enemy[i].getHealth() <= 0)
					{
						enemy[i].removeAlive();

						if (enemy[i].isElite())
						{
							score += 2;
						}
						else
						{
							score++;
						}
						enemyCounter++;

						graphics.playEnemyDeath();

						int dropChance = enemyDropChance;

						if (enemy[i].isElite())
						{
							dropChance = eliteDropChance;
						}

						if (rand() % 100 < dropChance)
						{
							spawnPowerUp(enemy[i].getX(), enemy[i].getY());
						}
					}

					bullet[b].deactivate();
				}
			}


			if (player.getX() == enemy[i].getX() && player.getY() == enemy[i].getY())
			{
				if (!invincible && !shield)
				{
					health -= enemy[i].getDamage();

					if (!shield)
						invincible = true;

					invincibleCounter = 0;

					int damage = doubleDamage ? 2 : 1;

					enemy[i].setHealth(enemy[i].getHealth() - damage);
				}

				if (enemy[i].getHealth() <= 0)
				{
					enemy[i].removeAlive();
					score++;
					enemyCounter++;
				}
			}
		}
		for (int j = 0; j < 3; j++)
		{
			if (eBullet[i].bullet[j].isActive())
			{
				if (eBullet[i].bullet[j].getX() == player.getX() && eBullet[i].bullet[j].getY() == player.getY())
				{
					if (!invincible && !shield)
					{
						health -= enemy[i].getDamage();

						if (!shield)
							invincible = true;

						invincibleCounter = 0;
					}

					eBullet[i].bullet[j].deactivate();
				}
			}
		}
	}
}

void Game::bossCollision()
{
	bool bossHit = bossEngine.checkCollision(bullet, playerBulletCount, player.getX(), player.getY(), health, invincible, invincibleCounter, doubleDamage);

	if (bossHit && bossEngine.shouldDropPower())
	{
		spawnPowerUp(bossEngine.getX() + 4, bossEngine.getY() + 5);
	}

	if (bossEngine.isDead())
	{
		graphics.playEnemyDeath();

		score += 10;

		if (health < maxPlayerHealth)
		{
			health++;
		}

		boss = false;

		levelSetup();
	}
}

void Game::levelSetup()
{
	rapidFire = false;
	shield = false;
	doubleDamage = false;
	diagonalShot = false;

	rapidFireTimer = 0;
	shieldTimer = 0;
	doubleDamageTimer = 0;
	diagonalShotTimer = 0;

	reinforcementSpawned = false;
	elitePromoted = false;

	formationDirection = (rand() % 2) ? 1 : -1;
	levelStartCounter = 0;
	canEnemiesAttack = false;

	heat = 0;
	level++;

	updatePlayerScaling();

	player.setSpawn();

	saveCheckpoint();

	if (level % 10 == 0)
	{
		boss = true;
	}

	attackTimerCounter = 0;
	bulletDelayCounter = 0;

	for (int i = 0; i < playerBulletCount; i++)
	{
		bullet[i].deactivate();
	}

	if (boss)
	{
		bossEngine.setup(level);
	}
	else
	{
		normalLevel();
	}

	levelIntroStartTime = GetTime();

	gameState = GameState::LEVEL_INTRO;
}

void Game::normalLevel()
{
	if (!resumingSavedGame)
		generator.levelSetup(enemyCount, level, attackerScaling, shootDelayScaling, bulletSpeedScaling, speedScaling, healthScaling, cooldownScaling);

	if (level >= 40 && level % 10 != 0)
	{
		reinforcementCount = enemyCount / 3;

		activeEnemyCount = enemyCount - reinforcementCount;
	}
	else
	{
		activeEnemyCount = enemyCount;
	}

	delete[] enemy;
	delete[] eBullet;

	enemy = new Enemy[enemyCount];
	eBullet = new EnemyBullets[enemyCount];

	for (int i = 0; i < enemyCount; i++)
	{
		enemy[i].speedScaling = speedScaling;
		enemy[i].healthScaling = healthScaling;
		enemy[i].cooldownScaling = cooldownScaling;
	}

	for (int i = 0;i < enemyCount;i++)
	{
		for (int j = 0;j < 3;j++)
		{
			eBullet[i].bullet[j].deactivate();
		}
	}

	for (int i = 0; i < enemyCount; i++)
	{
		enemy[i].setElite(false);

		enemy[i].setInFormation(false);

		enemy[i].setAttacking(false);

		if (i < activeEnemyCount)
		{
			enemy[i].setAlive();

			enemy[i].setReturningToFormation(true);
		}
		else
		{
			enemy[i].removeAlive();

			enemy[i].setReturningToFormation(false);
		}
	}

	generator.generateFormation(enemy, enemyCount);
	generator.generateEnemyType(enemy, enemyCount);
	generator.generateMovement(enemy, enemyCount);
	generator.generateFireType(enemy, enemyCount);

	if (generator.eliteCount > 0)
	{
		int center = generator.getCenterEnemy(enemy, enemyCount);
		generator.makeElite(enemy, enemyCount, center);
	}

}

void Game::chooseAttackers()
{

	int attackers = rand() % 3 + 2 + attackerScaling;
	int attempts = 0;

	while (attackers > 0 && attempts < enemyCount * 2)
	{
		int index = rand() % enemyCount;

		if (enemy[index].getAlive() && !enemy[index].isAttacking() && enemy[index].getInFormation())
		{
			enemy[index].setAttacking(true);
			enemy[index].setInFormation(false);
			enemy[index].setPosition(enemy[index].getX(), enemy[index].getY() + 1);
			attackers--;
		}
		attempts++;
	}
}

void Game::enemyShoot(int index)
{
	switch (enemy[index].getFireType())
	{
	case SINGLE:

		if (!eBullet[index].bullet[0].isActive())
		{
			eBullet[index].bullet[0].spawn(enemy[index].getX(), enemy[index].getY(), 1, 0);
		}
		break;

	case DIAGONAL:

		if (!eBullet[index].bullet[0].isActive() && !eBullet[index].bullet[1].isActive() && !eBullet[index].bullet[2].isActive())
		{
			eBullet[index].bullet[0].spawn(enemy[index].getX(), enemy[index].getY(), 1, 1);

			eBullet[index].bullet[1].spawn(enemy[index].getX(), enemy[index].getY(), 1, -1);

			eBullet[index].bullet[2].spawn(enemy[index].getX(), enemy[index].getY(), 1, 0);
		}
		break;

	case TRIPLE:

		if (!eBullet[index].bullet[0].isActive() && !eBullet[index].bullet[1].isActive() && !eBullet[index].bullet[2].isActive())
		{
			eBullet[index].bullet[0].spawn(enemy[index].getX(), enemy[index].getY(), 1, 0);

			eBullet[index].bullet[1].spawn(enemy[index].getX(), enemy[index].getY() + 1, 1, 0);

			eBullet[index].bullet[2].spawn(enemy[index].getX(), enemy[index].getY() + 2, 1, 0);
		}
		break;
	}
}

void Game::chooseShooters()
{
	int shooters;

	int alive = enemyCount - enemyCounter;

	if (alive <= 5)
	{
		if (rand() % 3 == 0)
		{
			shooters = 8;
		}
		else
		{
			shooters = rand() % 5 + 2;
		}
	}
	else
	{
		if (rand() % 50 == 0)
		{
			shooters = 8;
		}
		else
		{
			shooters = rand() % 4 + 2;
		}
	}

	int attempts = 0;

	for (int a = 0; a < enemyCount;a++)
	{
		if (enemy[a].isElite())
		{
			if (!enemy[a].getAlive())
			{
				break;
			}

			if (!enemy[a].canShoot())
			{
				break;
			}

			enemyShoot(a);
			enemy[a].setCooldown(enemy[a].getBaseCooldown());

			shooters--;
			break;
		}
	}

	while (shooters > 0 && attempts < enemyCount * 3)
	{
		int index = rand() % enemyCount;
		attempts++;

		if (!enemy[index].getAlive())
		{
			continue;
		}

		if (!enemy[index].canShoot())
		{
			continue;
		}

		if (enemy[index].isAttacking() &&
			generator.movement != LEFT_RIGHT &&
			generator.movement != ZIGZAG)
		{
			continue;
		}

		enemyShoot(index);
		enemy[index].setCooldown(enemy[index].getBaseCooldown());
		shooters--;
	}
}

void Game::spawnReinforcements()
{
	for (int i = activeEnemyCount; i < enemyCount; i++)
	{
		enemy[i].setAlive();

		enemy[i].setPosition(enemy[i].getFormationX(), -6);

		enemy[i].setReturningToFormation(true);

		enemy[i].setInFormation(false);

		enemy[i].setAttacking(false);

		eBullet[i].bullet[0].deactivate();
		eBullet[i].bullet[1].deactivate();
		eBullet[i].bullet[2].deactivate();
	}
}

void Game::promoteRemainingEnemies()
{
	for (int i = 0; i < enemyCount; i++)
	{
		if (!enemy[i].getAlive())
		{
			continue;
		}

		if (enemy[i].isElite())
		{
			continue;
		}

		enemy[i].setElite(true);

		enemy[i].setType(enemy[i].getType());

		enemy[i].setFireType(DIAGONAL);
	}
}

void Game::spawnPowerUp(int x, int y)
{
	int slot = -1;

	for (int i = 0; i < maxPowerUps; i++)
	{
		if (!powerUp[i].isActive())
		{
			slot = i;
			break;
		}
	}

	if (slot == -1)
		return;

	int randomType = rand() % 5;

	PowerType type = static_cast<PowerType>(randomType);

	powerUp[slot].spawn(x, y, type);
}

void Game::activatePowerUp(PowerType type)
{
	switch (type)
	{
	case POWER_HEALTH:

		if (health < maxPlayerHealth)
		{
			health++;
		}

		break;


	case POWER_RAPID_FIRE:

		rapidFire = true;
		rapidFireTimer = rapidFireDuration;

		break;


	case POWER_SHIELD:

		shield = true;
		shieldTimer = shieldDuration;

		break;


	case POWER_DOUBLE_DAMAGE:

		doubleDamage = true;
		doubleDamageTimer = doubleDamageDuration;

		break;


	case POWER_DIAGONAL_SHOT:

		diagonalShot = true;
		diagonalShotTimer = diagonalShotDuration;

		break;
	}
}

void Game::updatePowerUp()
{
	for (int i = 0; i < maxPowerUps; i++)
	{
		if (powerUp[i].isActive())
		{
			powerUp[i].move();
		}
	}
}

void Game::collectPowerUp()
{
	for (int i = 0; i < maxPowerUps; i++)
	{
		if (!powerUp[i].isActive())
			continue;

		int powerX = powerUp[i].getX();
		int powerY = powerUp[i].getY();

		int playerX = player.getX();
		int playerY = player.getY();

		int dx = playerX - powerX;
		int dy = playerY - powerY;

		// -------------------------------------------------
		// MAGNET
		// -------------------------------------------------

		if (abs(dx) <= powerMagnetRange &&
			abs(dy) <= powerMagnetRange)
		{
			// Move horizontally toward player
			if (powerX < playerX)
			{
				powerX++;
			}
			else if (powerX > playerX)
			{
				powerX--;
			}

			// Move vertically toward player
			if (powerY < playerY)
			{
				powerY++;
			}
			else if (powerY > playerY)
			{
				powerY--;
			}

			powerUp[i].setPosition(powerX, powerY);
		}

		// -------------------------------------------------
		// COLLECT
		// -------------------------------------------------

		if (powerUp[i].getX() == playerX &&
			powerUp[i].getY() == playerY)
		{
			activatePowerUp(powerUp[i].getType());

			powerUp[i].deactivate();
		}
	}
}

void Game::updatePowerTimers()
{
	if (rapidFire)
	{
		rapidFireTimer--;

		if (rapidFireTimer <= 0)
		{
			rapidFire = false;
			rapidFireTimer = 0;
		}
	}

	if (shield)
	{
		shieldTimer--;

		if (shieldTimer <= 0)
		{
			shield = false;
			shieldTimer = 0;
		}
	}

	if (doubleDamage)
	{
		doubleDamageTimer--;

		if (doubleDamageTimer <= 0)
		{
			doubleDamage = false;
			doubleDamageTimer = 0;
		}
	}

	if (diagonalShot)
	{
		diagonalShotTimer--;

		if (diagonalShotTimer <= 0)
		{
			diagonalShot = false;
			diagonalShotTimer = 0;
		}
	}
}

void Game::updatePlayerScaling()
{
	maxPlayerHealth = playerHealth;
	maxPlayerHeat = maxHeat;

	if (level >= 6)
	{
		maxPlayerHeat = maxHeat + 3;
		maxPlayerHealth = playerHealth + 1;
	}
	if (level >= 11)
	{
		maxPlayerHeat = maxHeat + 5;
		maxPlayerHealth = playerHealth + 2;
	}

	if (level >= 16)
	{
		maxPlayerHeat = maxHeat + 7;
	}

	if (level >= 21)
	{
		maxPlayerHeat = maxHeat + 10;
		maxPlayerHealth = playerHealth + 3;

	}

	if (level >= 26)
	{
		maxPlayerHeat = maxHeat + 13;
	}

	if (level >= 31)
	{
		maxPlayerHeat = maxHeat + 15;
		maxPlayerHealth = playerHealth + 4;

	}

	if (level >= 36)
	{
		maxPlayerHeat = maxHeat + 17;
	}

	if (level >= 41)
	{
		maxPlayerHeat = maxHeat + 20;
		maxPlayerHealth = playerHealth + 5;
	}
	if (level >= 46)
	{
		maxPlayerHeat = maxHeat + 22;
	}

	if (health > maxPlayerHealth)
	{
		health = maxPlayerHealth;
	}

	if (heat > maxPlayerHeat)
	{
		heat = maxPlayerHeat;
	}
}

void Game::startGame()
{
	score = 0;
	health = playerHealth;

	maxPlayerHealth = playerHealth;
	maxPlayerHeat = maxHeat;

	heat = 0;

	isGameOver = false;
	playerWon = false;
	paused = false;

	boss = false;

	enemyCounter = 0;

	level = 0;

	attackerScaling = 0;
	shootDelayScaling = 0;
	bulletSpeedScaling = 0;
	speedScaling = 0;
	healthScaling = 0;
	cooldownScaling = 0;

	rapidFire = false;
	shield = false;
	doubleDamage = false;
	diagonalShot = false;

	rapidFireTimer = 0;
	shieldTimer = 0;
	doubleDamageTimer = 0;
	diagonalShotTimer = 0;

	reinforcementSpawned = false;
	elitePromoted = false;

	reinforcementCount = 0;
	activeEnemyCount = 0;

	playerShootCounter = 0;
	playerMoveCounter = 0;

	heat = 0;
	overheated = false;
	overheatCounter = 0;
	coolCounter = 0;

	invincible = false;
	invincibleCounter = 0;

	canEnemiesAttack = false;
	ready = false;

	attackTimerCounter = 0;
	bulletDelayCounter = 0;
	shootTimer = 0;
	shootDelay = 30;

	formationDirection = (rand() % 2) ? 1 : -1;

	checkpointLevel = 1;

	generator.reset();

	deleteSave();
	deleteCheckpoint();

	levelSetup();

	gameState = GameState::LEVEL_INTRO;
	levelIntroStartTime = GetTime();

}

bool Game::isMainMenu() const
{
	return gameState == GameState::MAIN_MENU;
}

bool Game::isLevelIntro() const
{
	return gameState == GameState::LEVEL_INTRO;
}

bool Game::isPlaying() const
{
	return gameState == GameState::PLAYING;
}

bool Game::isEndScreen() const
{
	return gameState == GameState::END_SCREEN;
}

bool Game::hasWon() const
{
	return playerWon;
}

void Game::saveGame()
{
	std::ofstream file("save.dat", std::ios::binary | std::ios::trunc);

	if (!file)
		return;

	file.write(reinterpret_cast<char*>(&level), sizeof(level));
	file.write(reinterpret_cast<char*>(&score), sizeof(score));

	file.write(reinterpret_cast<char*>(&health), sizeof(health));
	file.write(reinterpret_cast<char*>(&maxPlayerHealth), sizeof(maxPlayerHealth));

	file.write(reinterpret_cast<char*>(&heat), sizeof(heat));
	file.write(reinterpret_cast<char*>(&maxPlayerHeat), sizeof(maxPlayerHeat));

	file.write(reinterpret_cast<char*>(&enemyCount), sizeof(enemyCount));
	file.write(reinterpret_cast<char*>(&enemyCounter), sizeof(enemyCounter));

	file.write(reinterpret_cast<char*>(&attackerScaling), sizeof(attackerScaling));
	file.write(reinterpret_cast<char*>(&shootDelayScaling), sizeof(shootDelayScaling));
	file.write(reinterpret_cast<char*>(&bulletSpeedScaling), sizeof(bulletSpeedScaling));
	file.write(reinterpret_cast<char*>(&speedScaling), sizeof(speedScaling));
	file.write(reinterpret_cast<char*>(&healthScaling), sizeof(healthScaling));
	file.write(reinterpret_cast<char*>(&cooldownScaling), sizeof(cooldownScaling));

	file.write(reinterpret_cast<char*>(&formationDirection), sizeof(formationDirection));

	file.write(reinterpret_cast<char*>(&rapidFire), sizeof(rapidFire));
	file.write(reinterpret_cast<char*>(&shield), sizeof(shield));
	file.write(reinterpret_cast<char*>(&doubleDamage), sizeof(doubleDamage));
	file.write(reinterpret_cast<char*>(&diagonalShot), sizeof(diagonalShot));

	file.write(reinterpret_cast<char*>(&rapidFireTimer), sizeof(rapidFireTimer));
	file.write(reinterpret_cast<char*>(&shieldTimer), sizeof(shieldTimer));
	file.write(reinterpret_cast<char*>(&doubleDamageTimer), sizeof(doubleDamageTimer));
	file.write(reinterpret_cast<char*>(&diagonalShotTimer), sizeof(diagonalShotTimer));

	file.write(reinterpret_cast<char*>(&generator.formationPool), sizeof(generator.formationPool));
	file.write(reinterpret_cast<char*>(&generator.enemyTypePool), sizeof(generator.enemyTypePool));
	file.write(reinterpret_cast<char*>(&generator.movementPool), sizeof(generator.movementPool));
	file.write(reinterpret_cast<char*>(&generator.fireTypePool), sizeof(generator.fireTypePool));

	file.write(reinterpret_cast<char*>(&generator.formation), sizeof(generator.formation));
	file.write(reinterpret_cast<char*>(&generator.enemyType), sizeof(generator.enemyType));
	file.write(reinterpret_cast<char*>(&generator.movement), sizeof(generator.movement));
	file.write(reinterpret_cast<char*>(&generator.fireType), sizeof(generator.fireType));

	file.write(reinterpret_cast<char*>(&generator.priority), sizeof(generator.priority));

	file.write(reinterpret_cast<char*>(&generator.priorityFormation), sizeof(generator.priorityFormation));
	file.write(reinterpret_cast<char*>(&generator.priorityEnemy), sizeof(generator.priorityEnemy));
	file.write(reinterpret_cast<char*>(&generator.priorityMovement), sizeof(generator.priorityMovement));
	file.write(reinterpret_cast<char*>(&generator.priorityFire), sizeof(generator.priorityFire));

	file.write(reinterpret_cast<char*>(&generator.mixedEnemies), sizeof(generator.mixedEnemies));
	file.write(reinterpret_cast<char*>(&generator.insaneLevel), sizeof(generator.insaneLevel));

	file.write(reinterpret_cast<char*>(&generator.eliteCount), sizeof(generator.eliteCount));

	file.write(reinterpret_cast<char*>(&generator.savedEnemy), sizeof(generator.savedEnemy));
	file.write(reinterpret_cast<char*>(&generator.savedAttacker), sizeof(generator.savedAttacker));
	file.write(reinterpret_cast<char*>(&generator.savedShootDelay), sizeof(generator.savedShootDelay));
	file.write(reinterpret_cast<char*>(&generator.savedBulletSpeed), sizeof(generator.savedBulletSpeed));

	file.write(reinterpret_cast<char*>(&checkpointLevel), sizeof(checkpointLevel));

	file.close();

	hasSave = true;
}

bool Game::loadGame()
{
	std::ifstream file("save.dat", std::ios::binary);

	if (!file)
	{
		hasSave = false;
		return false;
	}

	file.read(reinterpret_cast<char*>(&level), sizeof(level));
	file.read(reinterpret_cast<char*>(&score), sizeof(score));

	file.read(reinterpret_cast<char*>(&health), sizeof(health));
	file.read(reinterpret_cast<char*>(&maxPlayerHealth), sizeof(maxPlayerHealth));

	file.read(reinterpret_cast<char*>(&heat), sizeof(heat));
	file.read(reinterpret_cast<char*>(&maxPlayerHeat), sizeof(maxPlayerHeat));

	file.read(reinterpret_cast<char*>(&enemyCount), sizeof(enemyCount));
	file.read(reinterpret_cast<char*>(&enemyCounter), sizeof(enemyCounter));

	file.read(reinterpret_cast<char*>(&attackerScaling), sizeof(attackerScaling));
	file.read(reinterpret_cast<char*>(&shootDelayScaling), sizeof(shootDelayScaling));
	file.read(reinterpret_cast<char*>(&bulletSpeedScaling), sizeof(bulletSpeedScaling));
	file.read(reinterpret_cast<char*>(&speedScaling), sizeof(speedScaling));
	file.read(reinterpret_cast<char*>(&healthScaling), sizeof(healthScaling));
	file.read(reinterpret_cast<char*>(&cooldownScaling), sizeof(cooldownScaling));

	file.read(reinterpret_cast<char*>(&formationDirection), sizeof(formationDirection));

	file.read(reinterpret_cast<char*>(&rapidFire), sizeof(rapidFire));
	file.read(reinterpret_cast<char*>(&shield), sizeof(shield));
	file.read(reinterpret_cast<char*>(&doubleDamage), sizeof(doubleDamage));
	file.read(reinterpret_cast<char*>(&diagonalShot), sizeof(diagonalShot));

	file.read(reinterpret_cast<char*>(&rapidFireTimer), sizeof(rapidFireTimer));
	file.read(reinterpret_cast<char*>(&shieldTimer), sizeof(shieldTimer));
	file.read(reinterpret_cast<char*>(&doubleDamageTimer), sizeof(doubleDamageTimer));
	file.read(reinterpret_cast<char*>(&diagonalShotTimer), sizeof(diagonalShotTimer));

	file.read(reinterpret_cast<char*>(&generator.formationPool),
		sizeof(generator.formationPool));

	file.read(reinterpret_cast<char*>(&generator.enemyTypePool),
		sizeof(generator.enemyTypePool));

	file.read(reinterpret_cast<char*>(&generator.movementPool),
		sizeof(generator.movementPool));

	file.read(reinterpret_cast<char*>(&generator.fireTypePool),
		sizeof(generator.fireTypePool));

	file.read(reinterpret_cast<char*>(&generator.formation),
		sizeof(generator.formation));

	file.read(reinterpret_cast<char*>(&generator.enemyType),
		sizeof(generator.enemyType));

	file.read(reinterpret_cast<char*>(&generator.movement),
		sizeof(generator.movement));

	file.read(reinterpret_cast<char*>(&generator.fireType),
		sizeof(generator.fireType));

	file.read(reinterpret_cast<char*>(&generator.priority),
		sizeof(generator.priority));

	file.read(reinterpret_cast<char*>(&generator.priorityFormation),
		sizeof(generator.priorityFormation));

	file.read(reinterpret_cast<char*>(&generator.priorityEnemy),
		sizeof(generator.priorityEnemy));

	file.read(reinterpret_cast<char*>(&generator.priorityMovement),
		sizeof(generator.priorityMovement));

	file.read(reinterpret_cast<char*>(&generator.priorityFire),
		sizeof(generator.priorityFire));

	file.read(reinterpret_cast<char*>(&generator.mixedEnemies),
		sizeof(generator.mixedEnemies));

	file.read(reinterpret_cast<char*>(&generator.insaneLevel),
		sizeof(generator.insaneLevel));

	file.read(reinterpret_cast<char*>(&generator.eliteCount),
		sizeof(generator.eliteCount));

	file.read(reinterpret_cast<char*>(&generator.savedEnemy),
		sizeof(generator.savedEnemy));

	file.read(reinterpret_cast<char*>(&generator.savedAttacker),
		sizeof(generator.savedAttacker));

	file.read(reinterpret_cast<char*>(&generator.savedShootDelay),
		sizeof(generator.savedShootDelay));

	file.read(reinterpret_cast<char*>(&generator.savedBulletSpeed),
		sizeof(generator.savedBulletSpeed));

	file.read(reinterpret_cast<char*>(&checkpointLevel),
		sizeof(checkpointLevel));

	// ---------------------------------------------
	// CHECK FILE INTEGRITY
	// ---------------------------------------------

	if (!file)
	{
		file.close();

		hasSave = false;

		return false;
	}

	file.close();

	// ---------------------------------------------
	// RESET RUNTIME STATE
	// ---------------------------------------------

	resumingSavedGame = true;

	hasSave = true;

	boss = false;
	isGameOver = false;
	playerWon = false;
	paused = false;

	enemyCounter = 0;

	rapidFire = false;
	shield = false;
	doubleDamage = false;
	diagonalShot = false;

	rapidFireTimer = 0;
	shieldTimer = 0;
	doubleDamageTimer = 0;
	diagonalShotTimer = 0;

	levelStartCounter = 0;
	canEnemiesAttack = false;
	ready = false;

	attackTimerCounter = 0;
	bulletDelayCounter = 0;
	shootTimer = 0;

	playerShootCounter = 0;
	playerMoveCounter = 0;

	heat = 0;
	overheated = false;
	overheatCounter = 0;
	coolCounter = 0;

	invincible = false;
	invincibleCounter = 0;

	reinforcementSpawned = false;
	elitePromoted = false;
	reinforcementCount = 0;
	activeEnemyCount = 0;

	// ---------------------------------------------
	// CLEAR OLD BULLETS
	// ---------------------------------------------

	for (int i = 0; i < playerBulletCount; i++)
	{
		bullet[i].deactivate();
	}

	// ---------------------------------------------
	// REBUILD CURRENT LEVEL
	// ---------------------------------------------

	if (level % 10 == 0)
	{
		boss = true;
		bossEngine.setup(level);
	}
	else
	{
		normalLevel();
	}

	clearGrid();
	setCharacters();
	resumingSavedGame = false;

	levelIntroStartTime = GetTime();
	gameState = GameState::LEVEL_INTRO;



	return true;
}

void Game::deleteSave()
{
	std::remove("save.dat");
	hasSave = false;
}

void Game::saveCheckpoint()
{
	if (level != 1 &&
		level != 11 &&
		level != 21 &&
		level != 31 &&
		level != 41)
	{
		return;
	}

	checkpointLevel = level;

	std::ofstream file("checkpoint.dat", std::ios::binary | std::ios::trunc);

	if (!file)
		return;

	file.write(reinterpret_cast<char*>(&level), sizeof(level));
	file.write(reinterpret_cast<char*>(&score), sizeof(score));

	file.write(reinterpret_cast<char*>(&health), sizeof(health));
	file.write(reinterpret_cast<char*>(&maxPlayerHealth), sizeof(maxPlayerHealth));

	file.write(reinterpret_cast<char*>(&heat), sizeof(heat));
	file.write(reinterpret_cast<char*>(&maxPlayerHeat), sizeof(maxPlayerHeat));

	file.write(reinterpret_cast<char*>(&enemyCount), sizeof(enemyCount));
	file.write(reinterpret_cast<char*>(&enemyCounter), sizeof(enemyCounter));

	file.write(reinterpret_cast<char*>(&attackerScaling), sizeof(attackerScaling));
	file.write(reinterpret_cast<char*>(&shootDelayScaling), sizeof(shootDelayScaling));
	file.write(reinterpret_cast<char*>(&bulletSpeedScaling), sizeof(bulletSpeedScaling));
	file.write(reinterpret_cast<char*>(&speedScaling), sizeof(speedScaling));
	file.write(reinterpret_cast<char*>(&healthScaling), sizeof(healthScaling));
	file.write(reinterpret_cast<char*>(&cooldownScaling), sizeof(cooldownScaling));

	file.write(reinterpret_cast<char*>(&formationDirection), sizeof(formationDirection));

	file.write(reinterpret_cast<char*>(&rapidFire), sizeof(rapidFire));
	file.write(reinterpret_cast<char*>(&shield), sizeof(shield));
	file.write(reinterpret_cast<char*>(&doubleDamage), sizeof(doubleDamage));
	file.write(reinterpret_cast<char*>(&diagonalShot), sizeof(diagonalShot));

	file.write(reinterpret_cast<char*>(&rapidFireTimer), sizeof(rapidFireTimer));
	file.write(reinterpret_cast<char*>(&shieldTimer), sizeof(shieldTimer));
	file.write(reinterpret_cast<char*>(&doubleDamageTimer), sizeof(doubleDamageTimer));
	file.write(reinterpret_cast<char*>(&diagonalShotTimer), sizeof(diagonalShotTimer));

	file.write(reinterpret_cast<char*>(&generator.formationPool), sizeof(generator.formationPool));
	file.write(reinterpret_cast<char*>(&generator.enemyTypePool), sizeof(generator.enemyTypePool));
	file.write(reinterpret_cast<char*>(&generator.movementPool), sizeof(generator.movementPool));
	file.write(reinterpret_cast<char*>(&generator.fireTypePool), sizeof(generator.fireTypePool));

	file.write(reinterpret_cast<char*>(&generator.formation), sizeof(generator.formation));
	file.write(reinterpret_cast<char*>(&generator.enemyType), sizeof(generator.enemyType));
	file.write(reinterpret_cast<char*>(&generator.movement), sizeof(generator.movement));
	file.write(reinterpret_cast<char*>(&generator.fireType), sizeof(generator.fireType));

	file.write(reinterpret_cast<char*>(&generator.priority), sizeof(generator.priority));

	file.write(reinterpret_cast<char*>(&generator.priorityFormation), sizeof(generator.priorityFormation));
	file.write(reinterpret_cast<char*>(&generator.priorityMovement), sizeof(generator.priorityMovement));
	file.write(reinterpret_cast<char*>(&generator.priorityFire), sizeof(generator.priorityFire));
	file.write(reinterpret_cast<char*>(&generator.priorityEnemy), sizeof(generator.priorityEnemy));

	file.write(reinterpret_cast<char*>(&generator.insaneLevel), sizeof(generator.insaneLevel));

	file.write(reinterpret_cast<char*>(&generator.savedEnemy), sizeof(generator.savedEnemy));
	file.write(reinterpret_cast<char*>(&generator.savedAttacker), sizeof(generator.savedAttacker));
	file.write(reinterpret_cast<char*>(&generator.savedShootDelay), sizeof(generator.savedShootDelay));
	file.write(reinterpret_cast<char*>(&generator.savedBulletSpeed), sizeof(generator.savedBulletSpeed));

	file.write(reinterpret_cast<char*>(&generator.eliteCount), sizeof(generator.eliteCount));
	file.write(reinterpret_cast<char*>(&generator.mixedEnemies), sizeof(generator.mixedEnemies));

	file.write(reinterpret_cast<char*>(&checkpointLevel), sizeof(checkpointLevel));

	file.close();
}

bool Game::loadCheckpoint()
{
	std::ifstream file("checkpoint.dat", std::ios::binary);

	if (!file)
		return false;

	file.read(reinterpret_cast<char*>(&level), sizeof(level));
	file.read(reinterpret_cast<char*>(&score), sizeof(score));

	file.read(reinterpret_cast<char*>(&health), sizeof(health));
	file.read(reinterpret_cast<char*>(&maxPlayerHealth), sizeof(maxPlayerHealth));

	file.read(reinterpret_cast<char*>(&heat), sizeof(heat));
	file.read(reinterpret_cast<char*>(&maxPlayerHeat), sizeof(maxPlayerHeat));

	file.read(reinterpret_cast<char*>(&enemyCount), sizeof(enemyCount));
	file.read(reinterpret_cast<char*>(&enemyCounter), sizeof(enemyCounter));

	file.read(reinterpret_cast<char*>(&attackerScaling), sizeof(attackerScaling));
	file.read(reinterpret_cast<char*>(&shootDelayScaling), sizeof(shootDelayScaling));
	file.read(reinterpret_cast<char*>(&bulletSpeedScaling), sizeof(bulletSpeedScaling));
	file.read(reinterpret_cast<char*>(&speedScaling), sizeof(speedScaling));
	file.read(reinterpret_cast<char*>(&healthScaling), sizeof(healthScaling));
	file.read(reinterpret_cast<char*>(&cooldownScaling), sizeof(cooldownScaling));

	file.read(reinterpret_cast<char*>(&formationDirection), sizeof(formationDirection));

	file.read(reinterpret_cast<char*>(&rapidFire), sizeof(rapidFire));
	file.read(reinterpret_cast<char*>(&shield), sizeof(shield));
	file.read(reinterpret_cast<char*>(&doubleDamage), sizeof(doubleDamage));
	file.read(reinterpret_cast<char*>(&diagonalShot), sizeof(diagonalShot));

	file.read(reinterpret_cast<char*>(&rapidFireTimer), sizeof(rapidFireTimer));
	file.read(reinterpret_cast<char*>(&shieldTimer), sizeof(shieldTimer));
	file.read(reinterpret_cast<char*>(&doubleDamageTimer), sizeof(doubleDamageTimer));
	file.read(reinterpret_cast<char*>(&diagonalShotTimer), sizeof(diagonalShotTimer));

	file.read(reinterpret_cast<char*>(&generator.formationPool), sizeof(generator.formationPool));
	file.read(reinterpret_cast<char*>(&generator.enemyTypePool), sizeof(generator.enemyTypePool));
	file.read(reinterpret_cast<char*>(&generator.movementPool), sizeof(generator.movementPool));
	file.read(reinterpret_cast<char*>(&generator.fireTypePool), sizeof(generator.fireTypePool));

	file.read(reinterpret_cast<char*>(&generator.formation), sizeof(generator.formation));
	file.read(reinterpret_cast<char*>(&generator.enemyType), sizeof(generator.enemyType));
	file.read(reinterpret_cast<char*>(&generator.movement), sizeof(generator.movement));
	file.read(reinterpret_cast<char*>(&generator.fireType), sizeof(generator.fireType));

	file.read(reinterpret_cast<char*>(&generator.priority), sizeof(generator.priority));

	file.read(reinterpret_cast<char*>(&generator.priorityFormation), sizeof(generator.priorityFormation));
	file.read(reinterpret_cast<char*>(&generator.priorityMovement), sizeof(generator.priorityMovement));
	file.read(reinterpret_cast<char*>(&generator.priorityFire), sizeof(generator.priorityFire));
	file.read(reinterpret_cast<char*>(&generator.priorityEnemy), sizeof(generator.priorityEnemy));

	file.read(reinterpret_cast<char*>(&generator.insaneLevel), sizeof(generator.insaneLevel));

	file.read(reinterpret_cast<char*>(&generator.savedEnemy), sizeof(generator.savedEnemy));
	file.read(reinterpret_cast<char*>(&generator.savedAttacker), sizeof(generator.savedAttacker));
	file.read(reinterpret_cast<char*>(&generator.savedShootDelay), sizeof(generator.savedShootDelay));
	file.read(reinterpret_cast<char*>(&generator.savedBulletSpeed), sizeof(generator.savedBulletSpeed));

	file.read(reinterpret_cast<char*>(&generator.eliteCount), sizeof(generator.eliteCount));
	file.read(reinterpret_cast<char*>(&generator.mixedEnemies), sizeof(generator.mixedEnemies));

	file.read(reinterpret_cast<char*>(&checkpointLevel), sizeof(checkpointLevel));

	if (!file)
	{
		file.close();
		return false;
	}

	file.close();

	isGameOver = false;
	playerWon = false;
	paused = false;
	boss = false;

	enemyCounter = 0;

	rapidFire = false;
	shield = false;
	doubleDamage = false;
	diagonalShot = false;

	rapidFireTimer = 0;
	shieldTimer = 0;
	doubleDamageTimer = 0;
	diagonalShotTimer = 0;

	levelStartCounter = 0;
	canEnemiesAttack = false;
	ready = false;

	attackTimerCounter = 0;
	bulletDelayCounter = 0;
	shootTimer = 0;

	playerShootCounter = 0;
	playerMoveCounter = 0;

	heat = 0;
	overheated = false;
	overheatCounter = 0;
	coolCounter = 0;

	invincible = false;
	invincibleCounter = 0;

	reinforcementSpawned = false;
	elitePromoted = false;
	reinforcementCount = 0;
	activeEnemyCount = 0;

	for (int i = 0; i < playerBulletCount; i++)
	{
		bullet[i].deactivate();
	}

	if (level % 10 == 0)
	{
		boss = true;
		bossEngine.setup(level);
	}
	else
	{
		normalLevel();
	}

	clearGrid();
	setCharacters();

	levelIntroStartTime = GetTime();
	gameState = GameState::LEVEL_INTRO;

	return true;
}

void Game::restartCheckpoint()
{
	if (!loadCheckpoint())
	{
		return;
	}

	paused = false;

	gameState = GameState::LEVEL_INTRO;

	levelIntroStartTime = GetTime();
}

void Game::deleteCheckpoint()
{
	std::remove("checkpoint.dat");
	checkpointLevel = 1;
}
