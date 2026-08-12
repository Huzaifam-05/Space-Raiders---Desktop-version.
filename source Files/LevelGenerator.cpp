#include "LevelGenerator.h"

const int spacingX = 2;
const int spacingY = 2;

bool isPrime(int n)
{
	if (n < 2)
	{
		return false;
	}

	for (int i = 2; i * i <= n; i++)
	{
		if (n % i == 0)
		{
			return false;
		}
	}
	return true;
}

void LevelGenerator::generateFormation(Enemy enemy[], int enemyCount)
{
	formation = static_cast<Formation>(rand() % formationPool + 1);

	if (priorityFormation)
	{
		formation = (Formation)priority;
		priorityFormation = false;
	}

	switch (formation)
	{
	case Formation::RECTANGLE:
		rectangle(enemy, enemyCount);
		break;

	case Formation::TRIANGLE:
		triangle(enemy, enemyCount);
		break;

	case Formation::DIAMOND:
		diamond(enemy, enemyCount);
		break;

	case Formation::CIRCLE:
		circle(enemy, enemyCount);
		break;

	case Formation::RANDOM:
		randomFormation(enemy, enemyCount);
		break;
	}
}

void LevelGenerator::generateEnemyType(Enemy enemy[], int enemyCount)
{
	enemyType = static_cast<EnemyType>(rand() % enemyTypePool + 1);
	EnemyType e;

	if (priorityEnemy)
	{
		enemyType = (EnemyType)priority;
		priorityEnemy = false;
	}

	if (mixedEnemies)
	{
		for (int i = 0; i < enemyCount; i++)
		{
			int rate = rand() % 100;

			if (rate < 35)
			{
				e = NORMAL;
			}
			else if (rate < 60)
			{
				e = SPEEDY;
			}
			else if (rate < 85)
			{
				e = TANK;
			}
			else
			{
				e = DESTROYER;
			}

			enemy[i].setType(e);
		}
	}
	else
	{
		for (int i = 0; i < enemyCount; i++)
		{
			enemy[i].setType(enemyType);
		}
	}
}

void LevelGenerator::generateMovement(Enemy enemy[], int enemyCount)
{
	movement = static_cast<Movement>(rand() % movementPool + 1);

	if (priorityMovement)
	{
		movement = (Movement)priority;
		priorityMovement = false;
	}

	for (int i = 0; i < enemyCount; i++)
	{
		enemy[i].setMovement(movement);
	}
}

void LevelGenerator::generateFireType(Enemy enemy[], int enemyCount)
{
	fireType = static_cast<FireType>(rand() % fireTypePool + 1);
	FireType e;

	if (priorityFire)
	{
		fireType = (FireType)priority;
		priorityFire = false;
	}

	if (mixedEnemies)
	{
		for (int i = 0; i < enemyCount; i++)
		{
			int rate = rand() % 100;

			if (rate < 50)
			{
				e = SINGLE;
			}
			else if (rate < 75)
			{
				e = TRIPLE;
			}
			else
			{
				e = DIAGONAL;
			}

			enemy[i].setFireType(e);
		}
	}
	else
	{
		for (int i = 0; i < enemyCount; i++)
		{
			enemy[i].setFireType(fireType);
		}

	}
}

void LevelGenerator::rectangle(Enemy enemy[], int enemyCount)
{
	int maxCols = (width - 6) / spacingX;

	int cols = (int)ceil(sqrt((double)enemyCount));
	int rows = (enemyCount + cols - 1) / cols;

	while (cols < rows)
	{
		cols++;
		rows = (enemyCount + cols - 1) / cols;
	}

	int formationCount = rows * cols;

	while (formationCount > enemyCount)
	{
		rows--;

		formationCount = rows * cols;
	}

	int rowSizes[100];

	for (int i = 0; i < rows; i++)
	{
		rowSizes[i] = cols;
	}

	placeRows(enemy, rowSizes, rows);

	int extras = enemyCount - formationCount;

	if (extras > 0)
	{
		int formationWidth = (cols - 1) * spacingX;

		int leftMostX = (width - formationWidth) / 2;
		int rightMostX = leftMostX + formationWidth;

		int topY = 2;
		int bottomY = rows + 1;

		spawnExtras(enemy, formationCount, extras, leftMostX, rightMostX, topY, bottomY);
	}
}

void LevelGenerator::triangle(Enemy enemy[], int enemyCount)
{
	int rows = 1;

	while ((rows * (rows + 1)) / 2 <= enemyCount)
	{
		rows++;
	}

	rows--;

	int formationCount = rows * (rows + 1) / 2;

	int rowSizes[100];

	for (int i = 0; i < rows; i++)
	{
		rowSizes[i] = i + 1;
	}

	placeRows(enemy, rowSizes, rows);

	int extras = enemyCount - formationCount;

	if (extras > 0)
	{
		int maxCols = rowSizes[rows - 1];

		int formationWidth = (maxCols - 1) * spacingX;

		int leftMostX = (width - formationWidth) / 2;

		int rightMostX = leftMostX + formationWidth;

		int topY = 2;

		int bottomY = rows + 1;

		spawnExtras(enemy, formationCount, extras, leftMostX, rightMostX, topY, bottomY);
	}
}

void LevelGenerator::diamond(Enemy enemy[], int enemyCount)
{
	int radius = 0;
	int formationCount = 1;

	while (true)
	{
		int nextRadius = radius + 1;
		int nextCount = 1 + 2 * nextRadius * (nextRadius + 1);

		if (nextCount > enemyCount)
			break;

		radius = nextRadius;
		formationCount = nextCount;
	}

	int rows = radius * 2 + 1;
	int rowSizes[100];

	int index = 0;

	for (int i = 0; i <= radius; i++)
	{
		rowSizes[index++] = i * 2 + 1;
	}

	for (int i = radius - 1; i >= 0; i--)
	{
		rowSizes[index++] = i * 2 + 1;
	}

	placeRows(enemy, rowSizes, rows);

	int extras = enemyCount - formationCount;

	if (extras > 0)
	{
		int formationWidth = (rowSizes[radius] - 1) * spacingX;

		int leftMostX = (width - formationWidth) / 2;
		int rightMostX = leftMostX + formationWidth;

		int topY = 2;
		int bottomY = rows + 1;

		spawnExtras(enemy, formationCount, extras, leftMostX, rightMostX, topY, bottomY);
	}
}

struct CircleTemplate
{
	int rows;
	int rowSizes[9];
};

CircleTemplate circleTemplates[] = {
	{1,{3}}, {3,{2,3,2}}, {3,{3,5,3}}, {4,{3,5,5,3}}, {3,{5,7,5}},
	{5,{3,5,7,5,3}}, {5,{5,7,7,5,3}}, {5,{5,7,9,7,5}},
	{7,{3,5,7,9,7,5,3}}, {6,{5,7,9,9,7,5}}, {7,{5,7,9,11,9,7,5}}
};

int circleCapacity[] = { 3, 7, 11, 16, 17, 23, 27, 33, 39, 42, 53 };

void LevelGenerator::circle(Enemy enemy[], int enemyCount)
{
	int templateIndex = 0;

	for (int i = 0; i < 11; i++)
	{
		if (circleCapacity[i] <= enemyCount)
			templateIndex = i;
		else
			break;
	}

	int rows = circleTemplates[templateIndex].rows;
	int rowSizes[9];

	int formationCount = 0;

	for (int i = 0; i < rows; i++)
	{
		rowSizes[i] = circleTemplates[templateIndex].rowSizes[i];
		formationCount += rowSizes[i];
	}

	int remaining = enemyCount - formationCount;

	while (remaining > 0)
	{
		bool changed = false;

		for (int layer = 0; layer <= rows / 2 && remaining > 0; layer++)
		{
			int top = layer;
			int bottom = rows - 1 - layer;

			if (remaining > 0)
			{
				rowSizes[top]++;
				remaining--;
				changed = true;
			}

			if (top != bottom && remaining > 0)
			{
				rowSizes[bottom]++;
				remaining--;
			}
		}

		if (!changed)
			break;
	}

	placeRows(enemy, rowSizes, rows);
}

void LevelGenerator::randomFormation(Enemy enemy[], int enemyCount)
{
	bool used[100][100] = {};

	int index = 0;

	while (index < enemyCount)
	{
		int x, y;

		if (index == 0 || rand() % 100 < 20)
		{
			x = rand() % (width / spacingX - 2) + 1;
			y = rand() % enemySpawnRows + 2;
		}
		else
		{
			x = enemy[index - 1].getFormationX() / spacingX + (rand() % 5 - 2);
			y = enemy[index - 1].getFormationY() + (rand() % 3 - 1);

			if (x < 1)
			{
				x = 1;
			}
			if (x > width / spacingX - 2)
			{
				x = width / spacingX - 2;
			}

			if (y < 2)
			{
				y = 2;
			}
			if (y > enemySpawnRows + 1)
			{
				y = enemySpawnRows + 1;
			}
		}

		if (used[y][x])
		{
			continue;
		}

		used[y][x] = true;

		enemy[index].setFormationPosition(x * spacingX, y);
		enemy[index].setPosition(x * spacingX, -5 - index * 2);

		index++;
	}
}

void LevelGenerator::placeRows(Enemy enemy[], int rowSizes[], int rows)
{
	int maxCols = 0;

	for (int i = 0; i < rows; i++)
	{
		if (rowSizes[i] > maxCols)
		{
			maxCols = rowSizes[i];
		}
	}

	int formationWidth = (maxCols - 1) * spacingX;
	int startX = (width - formationWidth) / 2;
	int startY = 2;

	int index = 0;

	for (int r = 0; r < rows; r++)
	{
		int rowWidth = (rowSizes[r] - 1) * spacingX;

		int rowStartX = startX + (formationWidth - rowWidth) / 2;

		for (int c = 0; c < rowSizes[r]; c++)
		{
			int x = rowStartX + c * spacingX;
			int y = startY + r * spacingY;

			enemy[index].setFormationPosition(x, y);
			enemy[index].setPosition(x, -5 - index * 2);

			index++;
		}
	}
}

void LevelGenerator::spawnExtras(Enemy enemy[], int startIndex, int extraCount,
	int leftMostX, int rightMostX, int topY, int bottomY)
{
	int leftX = leftMostX - spacingX * 2;
	int rightX = rightMostX + spacingX * 2;

	int midY = (topY + bottomY) / 2;

	int x[15];
	int y[15];

	switch (extraCount)
	{
	case 1:
		x[0] = leftX;  y[0] = midY;
		break;

	case 2:
		x[0] = leftX;  y[0] = topY;
		x[1] = rightX; y[1] = topY;
		break;

	case 3:
		x[0] = leftX;  y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = (leftMostX + rightMostX) / 2;
		y[2] = bottomY + spacingY + 2;
		break;

	case 4:
		x[0] = leftX;  y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = leftX;  y[2] = bottomY;
		x[3] = rightX; y[3] = bottomY;
		break;

	case 5:
		x[0] = leftX;  y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = leftX;  y[2] = bottomY;
		x[3] = rightX; y[3] = bottomY;
		x[4] = (leftMostX + rightMostX) / 2;
		y[4] = bottomY + spacingY + 2;
		break;

	case 6:
		x[0] = leftX;  y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = leftX;  y[2] = midY;
		x[3] = rightX; y[3] = midY;
		x[4] = leftX;  y[4] = bottomY;
		x[5] = rightX; y[5] = bottomY;
		break;

	case 7:
		x[0] = leftX; y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = leftX; y[2] = midY;
		x[3] = rightX; y[3] = midY;
		x[4] = leftX; y[4] = bottomY;
		x[5] = rightX; y[5] = bottomY;
		x[6] = (leftMostX + rightMostX) / 2; 	
		y[6] = bottomY + spacingY + 2;

		break;

	case 8:
		x[0] = leftX; y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = leftX; y[2] = midY;
		x[3] = rightX; y[3] = midY;
		x[4] = leftX; y[4] = bottomY;
		x[5] = rightX; y[5] = bottomY;
		x[6] = leftX - spacingX * 2; y[6] = midY;
		x[7] = rightX + spacingX * 2; y[7] = midY;
		break;

	case 9:
		x[0] = leftX; y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = leftX; y[2] = midY;
		x[3] = rightX; y[3] = midY;
		x[4] = leftX; y[4] = bottomY;
		x[5] = rightX; y[5] = bottomY;
		x[6] = (leftMostX + rightMostX) / 2; y[6] = topY - 2;
		x[7] = (leftMostX + rightMostX) / 2; y[7] = bottomY + 2;
		x[8] = (leftMostX + rightMostX) / 2;
		y[8] = bottomY + spacingY + 2;
		break;

	case 10:
		x[0] = leftX; y[0] = topY;
		x[1] = rightX; y[1] = topY;
		x[2] = leftX; y[2] = midY;
		x[3] = rightX; y[3] = midY;
		x[4] = leftX; y[4] = bottomY;
		x[5] = rightX; y[5] = bottomY;
		x[6] = leftX - spacingX * 2; y[6] = topY;
		x[7] = rightX + spacingX * 2; y[7] = topY;
		x[8] = leftX - spacingX * 2; y[8] = bottomY;
		x[9] = rightX + spacingX * 2; y[9] = bottomY;
		break;

	case 11:
	case 12:
	case 13:
	case 14:
	case 15:
	{
		int index = 0;

		for (int row = -2; row <= 2 && index < extraCount; row++)
		{
			for (int col = -1; col <= 1 && index < extraCount; col++)
			{
				x[index] = (leftMostX + rightMostX) / 2 + col * spacingX * 4;
				y[index] = midY + row * 2;
				index++;
			}
		}
	}
	break;
	}

	int count = (extraCount > 15) ? 15 : extraCount;

	for (int i = 0; i < count; i++)
	{
		enemy[startIndex + i].setFormationPosition(x[i], y[i]);
		enemy[startIndex + i].setPosition(x[i], -5 - (startIndex + i) * 2);
	}
}

void LevelGenerator::levelSetup(int& enemyCount, int level, int& attackerScaling, int& shootDelayScaling, int& bulletSpeedScaling, int& speedScaling, int& healthScaling, int& cooldownScaling)
{

	if (insaneLevel)
	{
		enemyCount = savedEnemy;
		attackerScaling = savedAttacker;
		shootDelayScaling = savedShootDelay;
		bulletSpeedScaling = savedBulletSpeed;

		insaneLevel = false;
	}
	eliteCount = 0;

	savedEnemy = 0;
	savedAttacker = 0;
	savedShootDelay = 0;
	savedBulletSpeed = 0;

	if (level <= 40)
	{
		if (level % 10 == 1)
		{
			if (level < 30 && level != 1)
			{
				fireTypePool++;
				priority = fireTypePool;
				priorityFire = true;
			}
			else
			{
				attackerScaling += 2;
			}
		}
		else if (level % 10 == 2)
		{
			if (level == 22)
			{
				enemyTypePool++;
				priority = enemyTypePool;
				priorityEnemy = true;
			}
			enemyCount += 2;
		}
		else if (level % 10 == 3)
		{
			movementPool++;
			priority = movementPool;
			priorityMovement = true;

			if (level == 33)

				healthScaling = 1;
		}

		else if (level % 10 == 4)
		{
			if (level < 30)
			{
				formationPool++;
				priority = formationPool;
				priorityFormation = true;

			}
			else
			{
				shootDelayScaling -= 5;
			}
		}
		else if (level % 10 == 5)
		{
			eliteCount = 2 + ((level - 5) / 7);

			if (level < 30)
			{
				speedScaling++;
			}
			else
			{
				bulletSpeedScaling = 1;
			}
		}
		else if (level % 10 == 6)
		{
			if (level != 26 && level < 30)
			{
				enemyTypePool++;
				priority = enemyTypePool;
				priorityEnemy = true;;
			}
			else
			{
				enemyCount++;
			}
		}
		else if (level % 10 == 7)
		{

			cooldownScaling += 4;

		}
		else if (level % 10 == 8)
		{
			if (level == 8)
			{
				formationPool++;
				priority = formationPool;
				priorityFormation = true;
			}
			enemyCount += 3;
		}
		else if (level % 10 == 9)
		{
			insaneLevel = true;

			eliteCount = 3 + ((level - 9) / 10) * 2;

			savedEnemy = enemyCount;
			savedAttacker = attackerScaling;
			savedShootDelay = shootDelayScaling;
			savedBulletSpeed = bulletSpeedScaling;

			enemyCount += 5 + ((level - 9) / 10) * 2;
			attackerScaling += 3 + ((level - 9) / 10);
			shootDelayScaling += 8;
			bulletSpeedScaling += 1;
		}
	}
	else
	{
		eliteCount = (level % 10) + 1;
	}

	if (level > 30)
	{
		mixedEnemies = true;
	}
	else
	{
		mixedEnemies = false;
	}

}

int LevelGenerator::getCenterEnemy(Enemy enemy[], int enemyCount)
{
	int sumX = 0;
	int sumY = 0;

	for (int i = 0; i < enemyCount; i++)
	{
		sumX += enemy[i].getFormationX();
		sumY += enemy[i].getFormationY();
	}

	int centerX = sumX / enemyCount;
	int centerY = sumY / enemyCount;

	int best = 0;
	int bestDist = INT_MAX;

	for (int i = 0; i < enemyCount; i++)
	{
		int dx = enemy[i].getFormationX() - centerX;
		int dy = enemy[i].getFormationY() - centerY;

		int dist = dx * dx + dy * dy;

		if (dist < bestDist)
		{
			bestDist = dist;
			best = i;
		}
	}

	return best;
}

void LevelGenerator::makeElite(Enemy enemy[], int enemyCount, int center)
{
	bool selected[100] = {};

	for (int placed = 0; placed < eliteCount; placed++)
	{
		int best = -1;
		int bestDist = INT_MAX;

		for (int i = 0; i < enemyCount; i++)
		{
			if (selected[i])
				continue;

			bool adjacent = false;

			for (int j = 0; j < enemyCount; j++)
			{
				if (!selected[j])
					continue;

				int dx = abs(enemy[i].getFormationX() - enemy[j].getFormationX());
				int dy = abs(enemy[i].getFormationY() - enemy[j].getFormationY());

				if (dx <= spacingX && dy <= spacingY)
				{
					adjacent = true;
					break;
				}
			}

			if (adjacent)
				continue;

			int dx = enemy[i].getFormationX() - enemy[center].getFormationX();
			int dy = enemy[i].getFormationY() - enemy[center].getFormationY();

			int dist = dx * dx + dy * dy;

			if (dist < bestDist)
			{
				bestDist = dist;
				best = i;
			}
		}

		if (best == -1)
		{
			for (int i = 0; i < enemyCount; i++)
			{
				if (selected[i])
					continue;

				int dx = enemy[i].getFormationX() - enemy[center].getFormationX();
				int dy = enemy[i].getFormationY() - enemy[center].getFormationY();

				int dist = dx * dx + dy * dy;

				if (dist < bestDist)
				{
					bestDist = dist;
					best = i;
				}
			}
		}

		if (best == -1)
		{
			break;
		}

		selected[best] = true;

		enemy[best].setElite(true);
		enemy[best].setType(enemy[best].getType());

		if (enemy[best].getFireType() == DIAGONAL)
		{
			enemy[best].setFireType(TRIPLE);
		}
		else
		{
			enemy[best].setFireType(DIAGONAL);
		}

	}
}

void LevelGenerator::reset()
{
	formation = RECTANGLE;
	enemyType = NORMAL;
	movement = STRAIGHT;
	fireType = SINGLE;

	formationPool = 1;
	enemyTypePool = 1;
	movementPool = 1;
	fireTypePool = 1;

	priority = 0;

	priorityFormation = false;
	priorityMovement = false;
	priorityFire = false;
	priorityEnemy = false;

	insaneLevel = false;

	savedEnemy = 0;
	savedAttacker = 0;
	savedShootDelay = 0;
	savedBulletSpeed = 0;

	eliteCount = 0;
	mixedEnemies = false;
}