#pragma once

#include "raylib.h"

class Game;

class Graphics
{
public:
	Music backgroundMusic;
	Sound enemyDeathSound;
	Sound playerDeathSound;

	bool soundsLoaded = false;


	void loadSounds();
	void unloadSounds();

	void playEnemyDeath();
	void playPlayerDeath();

	void render(Game& game);

	void mainMenu(Game& game);

	void levelIntro(Game& game);

	void endScreen(Game& game);

	void pauseMenu(Game& game);

	void gameover(bool isGameOver, int score);
};