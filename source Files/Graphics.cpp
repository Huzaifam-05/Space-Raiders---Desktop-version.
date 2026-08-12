#include "Graphics.h"
#include "Game.h"
#include "raylib.h"

#include <cmath>
#include <algorithm>
#include <cstdlib>

namespace
{
    constexpr const char* BACKGROUND = "assets/bg.jpg";
    constexpr const char* PLAYER = "assets/player.PNG";
    constexpr const char* ENEMY = "assets/enemy.png";
    constexpr const char* ELITE = "assets/elite.png";
    constexpr const char* BOSS = "assets/boss.png";
    constexpr const char* PLAYER_BULLET = "assets/player_bullet.png";
    constexpr const char* ENEMY_BULLET = "assets/enemy_bullet.png";
    constexpr const char* ELITE_BULLET = "assets/elite_bullet.png";
    constexpr const char* ASSET_POWER_HEALTH = "assets/power_health.png";
    constexpr const char* ASSET_POWER_RAPID = "assets/power_rapid.png";
    constexpr const char* ASSET_POWER_SHIELD = "assets/power_shield.png";
    constexpr const char* ASSET_POWER_DAMAGE = "assets/power_damage.png";
    constexpr const char* ASSET_POWER_DIAGONAL = "assets/power_diagonal.png";

    struct Assets
    {
        Texture2D background{};
        Texture2D player{};
        Texture2D enemy{};
        Texture2D elite{};
        Texture2D boss{};
        Texture2D playerBullet{};
        Texture2D enemyBullet{};
        Texture2D eliteBullet{};
        Texture2D health{};
        Texture2D powerHealth{};
        Texture2D rapid{};
        Texture2D shield{};
        Texture2D damage{};
        Texture2D diagonal{};
        bool loaded = false;
    } assets;

    const Color TEXT = { 238, 244, 255, 255 };
    const Color MUTED = { 145, 158, 185, 255 };
    const Color CYAN = { 80, 220, 255, 255 };
    const Color UI_RED = { 255, 76, 92, 255 };
    const Color UI_YELLOW = { 255, 215, 75, 255 };
    const Color UI_GREEN = { 70, 235, 145, 255 };
    const Color PANEL = { 7, 12, 27, 218 };
    const Color UI_LINE = { 80, 150, 210, 90 };
    const Color EMPTY = { 30, 38, 58, 220 };

    bool valid(Texture2D t)
    {
        return t.id != 0 && t.width > 0 && t.height > 0;
    }

    void loadAssets()
    {
        if (assets.loaded)
            return;

        assets.background = LoadTexture(BACKGROUND);
        assets.player = LoadTexture(PLAYER);
        assets.enemy = LoadTexture(ENEMY);
        assets.elite = LoadTexture(ELITE);
        assets.boss = LoadTexture(BOSS);
        assets.playerBullet = LoadTexture(PLAYER_BULLET);
        assets.enemyBullet = LoadTexture(ENEMY_BULLET);
        assets.eliteBullet = LoadTexture(ELITE_BULLET);
        assets.powerHealth = LoadTexture(ASSET_POWER_HEALTH);
        assets.rapid = LoadTexture(ASSET_POWER_RAPID);
        assets.shield = LoadTexture(ASSET_POWER_SHIELD);
        assets.damage = LoadTexture(ASSET_POWER_DAMAGE);
        assets.diagonal = LoadTexture(ASSET_POWER_DIAGONAL);
        assets.loaded = true;
    }

    void drawBackground(int screenW, int screenH)
    {
        ClearBackground(Color{ 3, 6, 16, 255 });

        if (!valid(assets.background))
            return;

        Rectangle src{
            0.0f,
            0.0f,
            static_cast<float>(assets.background.width),
            static_cast<float>(assets.background.height)
        };

        Rectangle dst{
            0.0f,
            0.0f,
            static_cast<float>(screenW),
            static_cast<float>(screenH)
        };

        DrawTexturePro(
            assets.background,
            src,
            dst,
            Vector2{ 0.0f, 0.0f },
            0.0f,
            WHITE
        );
    }

    void drawSprite(Texture2D texture, float cx, float cy, float size, Color tint = WHITE)
    {
        if (!valid(texture))
            return;

        const float tw = static_cast<float>(texture.width);
        const float th = static_cast<float>(texture.height);
        const float scale = size / ((tw > th) ? tw : th);

        Rectangle src{
            0.0f,
            0.0f,
            tw,
            th
        };

        Rectangle dst{
            cx - tw * scale * 0.5f,
            cy - th * scale * 0.5f,
            tw * scale,
            th * scale
        };

        DrawTexturePro(
            texture,
            src,
            dst,
            Vector2{ 0.0f, 0.0f },
            0.0f,
            tint
        );
    }

    void drawBar(float x, float y, float w, float h, float ratio, Color fill)
    {
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;

        DrawRectangleRounded(
            Rectangle{ x, y, w, h },
            0.35f,
            12,
            EMPTY
        );

        if (ratio > 0.0f)
        {
            DrawRectangleRounded(
                Rectangle{ x, y, w * ratio, h },
                0.35f,
                12,
                fill
            );
        }

        DrawRectangleRoundedLinesEx(
            Rectangle{ x, y, w, h },
            0.35f,
            12,
            1.0f,
            UI_LINE
        );
    }

    void drawTextRight(const char* text, int x, int y, int size, Color color)
    {
        DrawText(
            text,
            x - MeasureText(text, size),
            y,
            size,
            color
        );
    }

    void drawCentered(const char* text, int y, int size, Color color, int screenW)
    {
        DrawText(
            text,
            (screenW - MeasureText(text, size)) / 2,
            y,
            size,
            color
        );
    }

    Texture2D powerTexture(PowerType type)
    {
        switch (type)
        {
        case POWER_HEALTH: return assets.powerHealth;
        case POWER_RAPID_FIRE: return assets.rapid;
        case POWER_SHIELD: return assets.shield;
        case POWER_DOUBLE_DAMAGE: return assets.damage;
        case POWER_DIAGONAL_SHOT: return assets.diagonal;
        default: return Texture2D{};
        }
    }
}

void Graphics::render(Game& game)
{
    loadAssets();

    if (soundsLoaded)
    {
        UpdateMusicStream(backgroundMusic);

        if (!IsMusicStreamPlaying(backgroundMusic))
        {
            PlayMusicStream(backgroundMusic);
        }
    }

    // ---------------------------------------------------------
    // VIRTUAL RESOLUTION
    // ---------------------------------------------------------
    // The entire game is laid out at 1600x900. Raylib then
    // uniformly scales that virtual canvas to the actual window.
    constexpr float VIRTUAL_WIDTH = 1600.0f;
    constexpr float VIRTUAL_HEIGHT = 900.0f;

    const int actualScreenW = GetScreenWidth();
    const int actualScreenH = GetScreenHeight();

    const float scaleX =
        static_cast<float>(actualScreenW) / VIRTUAL_WIDTH;

    const float scaleY =
        static_cast<float>(actualScreenH) / VIRTUAL_HEIGHT;

    const float virtualScale =
        std::min(scaleX, scaleY);

    Camera2D camera{};
    camera.target = Vector2{
        VIRTUAL_WIDTH * 0.5f,
        VIRTUAL_HEIGHT * 0.5f
    };

    camera.offset = Vector2{
        static_cast<float>(actualScreenW) * 0.5f,
        static_cast<float>(actualScreenH) * 0.5f
    };

    camera.rotation = 0.0f;
    camera.zoom = virtualScale;

    // Clear the real window first. The game itself is then
    // rendered completely in virtual 1600x900 coordinates.
    ClearBackground(Color{ 3, 6, 16, 255 });

    BeginMode2D(camera);

    const int screenW = static_cast<int>(VIRTUAL_WIDTH);
    const int screenH = static_cast<int>(VIRTUAL_HEIGHT);

    drawBackground(screenW, screenH);

    const float margin = 8.0f;
    const float gap = 12.0f;
    const float uiW = 360.0f;

    const float boardAreaW =
        static_cast<float>(screenW) -
        uiW -
        gap -
        margin * 2.0f;

    const float boardAreaH =
        static_cast<float>(screenH) -
        margin * 2.0f;

    // Keep horizontal scale based on available width
    const float cellW =
        boardAreaW / static_cast<float>(width);

    // Stretch the playable area vertically
    const float cellH =
        boardAreaH / static_cast<float>(height);

    const float cellX = cellW;
    const float cellY = cellH;

    const float boardW =
        static_cast<float>(width) * cellX;

    const float boardH =
        static_cast<float>(height) * cellY;

    const float boardX = margin;

    const float boardY =
        (static_cast<float>(screenH) - boardH) * 0.5f;


    const double time = GetTime();

    DrawRectangleRounded(
        Rectangle{
            boardX - 5.0f,
            boardY - 5.0f,
            boardW + 10.0f,
            boardH + 10.0f
        },
        0.015f,
        8,
        Color{ 0, 4, 15, 55 }
    );

    DrawRectangleRoundedLinesEx(
        Rectangle{
            boardX - 5.0f,
            boardY - 5.0f,
            boardW + 10.0f,
            boardH + 10.0f
        },
        0.015f,
        8,
        2.0f,
        UI_LINE
    );

    if (!game.boss && game.enemy != nullptr)
    {
        for (int i = 0; i < game.enemyCount; ++i)
        {
            if (!game.enemy[i].getAlive())
                continue;

            const float cx =
                boardX + (game.enemy[i].getX() + 0.5f) * cellX;

            const float cy =
                boardY + (game.enemy[i].getY() + 0.5f) * cellY;

            const float size =
                cellX * (game.enemy[i].isElite() ? 2.5f : 2.5f);

            drawSprite(
                game.enemy[i].isElite() ? assets.elite : assets.enemy,
                cx,
                cy,
                size
            );
        }
    }


    if (game.boss && valid(assets.boss))
    {
        for (int gy = 0; gy < height; ++gy)
        {
            for (int gx = 0; gx < width; ++gx)
            {
                const char ch = game.grid[gy][gx];

                if (gx + 8 >= width || gy + 4 >= height)
                    continue;

                if (ch == '<')
                {
                    const float cloneCX =
                        boardX + (gx + 4.5f) * cellX;

                    const float cloneCY =
                        boardY + (gy + 1.0f) * cellY;

                    float cloneSize = cellX * 8.0f;

                    const float maxCloneSize = boardW * 2.20f;
                    const float maxCloneHeight = boardH * 2.26f;

                    if (cloneSize > maxCloneSize)
                        cloneSize = maxCloneSize;

                    if (cloneSize > maxCloneHeight)
                        cloneSize = maxCloneHeight;

                    drawSprite(
                        assets.boss,
                        cloneCX,
                        cloneCY,
                        cloneSize,
                        Color{ 155, 180, 255, 210 }
                    );
                }
            }
        }
    }

    if (!game.boss && game.eBullet != nullptr)
    {
        for (int i = 0; i < game.enemyCount; ++i)
        {
            for (int b = 0; b < 3; ++b)
            {
                Bullet& shot = game.eBullet[i].bullet[b];

                if (!shot.isActive())
                    continue;

                const float cx =
                    boardX + (shot.getX() + 0.5f) * cellX;

                const float cy =
                    boardY + (shot.getY() + 0.5f) * cellY;

                drawSprite(
                    shot.isElite()
                    ? assets.eliteBullet
                    : assets.enemyBullet,
                    cx,
                    cy,
                    cellX * 2.30f
                );
            }
        }
    }

    if (game.boss)
    {
        for (int gy = 0; gy < height; ++gy)
        {
            for (int gx = 0; gx < width; ++gx)
            {
                if (game.grid[gy][gx] != '|')
                    continue;

                const float cx =
                    boardX + (gx + 0.5f) * cellX;

                const float cy =
                    boardY + (gy + 0.5f) * cellY;

                drawSprite(
                    assets.enemyBullet,
                    cx,
                    cy,
                    cellX * 2.30f
                );
            }
        }

        for (int gy = 0; gy < height; ++gy)
        {
            for (int gx = 0; gx < width; ++gx)
            {
                if (game.grid[gy][gx] != 'P')
                    continue;

                const float cx =
                    boardX + (gx + 0.5f) * cellX;

                const float cy =
                    boardY + (gy + 0.5f) * cellY;

                drawSprite(
                    assets.enemy,
                    cx,
                    cy,
                    cellX * 2.70f
                );
            }
        }

    }

    for (int i = 0; i < playerBulletCount; ++i)
    {
        if (!game.bullet[i].isActive())
            continue;

        const float cx =
            boardX + (game.bullet[i].getX() + 0.5f) * cellX;

        const float cy =
            boardY + (game.bullet[i].getY() + 0.5f) * cellY;

        drawSprite(
            assets.playerBullet,
            cx,
            cy,
            cellX * 2.35f
        );
    }

    for (int i = 0; i < game.maxPowerUps; ++i)
    {
        if (!game.powerUp[i].isActive())
            continue;

        Texture2D power =
            powerTexture(game.powerUp[i].getType());

        const float cx =
            boardX + (game.powerUp[i].getX() + 0.5f) * cellX;

        const float cy =
            boardY + (game.powerUp[i].getY() + 0.5f) * cellY;

        const float bob =
            std::sin(static_cast<float>(time * 5.0 + i))
            * cellX * 0.10f;

        drawSprite(
            power,
            cx,
            cy + bob,
            cellX * 1.80f
        );
    }

    const float playerCX =
        boardX + (game.player.getX() + 0.5f) * cellX;

    const float playerCY =
        boardY + (game.player.getY() + 0.5f) * cellY;

    // ---------------------------------------------------------
    // PLAYER
    // ---------------------------------------------------------

    // NEVER blink the spaceship.
    drawSprite(
        assets.player,
        playerCX,
        playerCY,
        cellX * 3.15f
    );

    // ---------------------------------------------------------
    // NORMAL POWER SHIELD
    // ---------------------------------------------------------

    if (game.shield)
    {
        const float r =
            cellX * (0.95f + 0.08f *
                static_cast<float>(
                    (std::sin(time * 7.0) + 1.0) * 0.5
                    ));

        DrawCircleLines(
            playerCX,
            playerCY,
            r,
            Color{ 75, 210, 255, 220 }
        );

        DrawCircleLines(
            playerCX,
            playerCY,
            r + 5.0f,
            Color{ 70, 130, 255, 80 }
        );
    }

    // ---------------------------------------------------------
    // INVINCIBILITY SHIELD
    // ---------------------------------------------------------

    if (game.invincible && !game.shield)
    {
        const float pulse =
            1.0f +
            0.10f * static_cast<float>(
                (std::sin(time * 6.0) + 1.0) * 0.5
                );

        const float r = cellX * 1.15f * pulse;

        // Outer glow
        DrawCircleLines(
            playerCX,
            playerCY,
            r + 7.0f,
            Color{ 255, 215, 75, 65 }
        );

        // Main ring
        DrawCircleLines(
            playerCX,
            playerCY,
            r,
            Color{ 255, 215, 75, 235 }
        );

        // Inner ring
        DrawCircleLines(
            playerCX,
            playerCY,
            r - 5.0f,
            Color{ 255, 165, 40, 120 }
        );
    }
    // ============================================================
// HUD - CLEAN LARGE VERSION
// ============================================================

    const float hudX = boardX + boardW + gap;
    const float hudY = margin;
    const float hudH = boardH;

    const float hudPad = 22.0f;
    const float hudLeft = hudX + hudPad;
    const float hudRight = hudX + uiW - hudPad;
    const float hudWidth = hudRight - hudLeft;

    DrawRectangleRounded(
        Rectangle{ hudX, hudY, uiW, hudH },
        0.035f,
        12,
        PANEL
    );

    DrawRectangleRoundedLinesEx(
        Rectangle{ hudX, hudY, uiW, hudH },
        0.035f,
        12,
        1.5f,
        UI_LINE
    );

    // ============================================================
    // HEADER
    // ============================================================

    float y = hudY + 28.0f;

    y += 10.0f;

    DrawText(
        "      SPACE RAIDERS",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        25,
        TEXT
    );

    y += 65.0f;

    DrawLine(
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        static_cast<int>(hudRight),
        static_cast<int>(y),
        UI_LINE
    );

    // ============================================================
    // MISSION
    // ============================================================

    y += 22.0f;

    DrawText(
        "              MISSION",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        20,
        CYAN
    );

    y += 30.0f;

    DrawText(
        "ACTIVE COMBAT OPERATION",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        20,
        TEXT
    );

    y += 60.0f;

    // ============================================================
    // LEVEL / SCORE
    // ============================================================

    const float readoutGap = 10.0f;
    const float readoutW =
        (hudWidth - readoutGap) * 0.5f;

    const float readoutH = 78.0f;

    DrawRectangle(
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        static_cast<int>(readoutW),
        static_cast<int>(readoutH),
        Color{ 9, 20, 37, 230 }
    );

    DrawRectangleLinesEx(
        Rectangle{
            hudLeft,
            y,
            readoutW,
            readoutH
        },
        1.0f,
        Color{ 80, 220, 255, 100 }
    );

    DrawText(
        "LEVEL",
        static_cast<int>(hudLeft + 12),
        static_cast<int>(y + 10),
        15,
        MUTED
    );

    DrawText(
        TextFormat("%02d", game.level),
        static_cast<int>(hudLeft + 12),
        static_cast<int>(y + 31),
        34,
        CYAN
    );

    // SCORE

    const float scoreX =
        hudLeft + readoutW + readoutGap;

    DrawRectangle(
        static_cast<int>(scoreX),
        static_cast<int>(y),
        static_cast<int>(readoutW),
        static_cast<int>(readoutH),
        Color{ 20, 18, 28, 230 }
    );

    DrawRectangleLinesEx(
        Rectangle{
            scoreX,
            y,
            readoutW,
            readoutH
        },
        1.0f,
        Color{ 255, 215, 75, 100 }
    );

    DrawText(
        "SCORE",
        static_cast<int>(scoreX + 12),
        static_cast<int>(y + 10),
        15,
        MUTED
    );

    DrawText(
        TextFormat("%d", game.score),
        static_cast<int>(scoreX + 12),
        static_cast<int>(y + 32),
        34,
        UI_YELLOW
    );

    y += readoutH + 25.0f;

    // ============================================================
    // PLAYER STATUS
    // ============================================================



    DrawLine(
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        static_cast<int>(hudRight),
        static_cast<int>(y),
        UI_LINE
    );

    y += 34.0f;


    DrawText(
        "         PLAYER STATUS",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        20,
        CYAN
    );

    y += 38.0f;

    DrawText(
        "Health",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        20,
        MUTED
    );

    const int maxHealth =
        std::max(1, game.maxPlayerHealth);

    const int currentHealth =
        std::max(
            0,
            std::min(game.health, maxHealth)
        );

    const float healthRatio =
        static_cast<float>(currentHealth) /
        static_cast<float>(maxHealth);

    Color healthColor = UI_GREEN;

    if (healthRatio <= 0.25f)
        healthColor = UI_RED;
    else if (healthRatio <= 0.55f)
        healthColor = UI_YELLOW;

    drawTextRight(
        TextFormat(
            "%d / %d",
            game.health, game.maxPlayerHealth
        ),
        static_cast<int>(hudRight),
        static_cast<int>(y),
        20,
        healthColor
    );

    y += 30.0f;

    // HEALTH BAR

    const float healthBarH = 20.0f;

    DrawRectangle(
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        static_cast<int>(hudWidth),
        static_cast<int>(healthBarH),
        EMPTY
    );

    if (healthRatio > 0.0f)
    {
        DrawRectangle(
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            static_cast<int>(hudWidth * healthRatio),
            static_cast<int>(healthBarH),
            healthColor
        );
    }

    y += 52.0f;

    // ============================================================
    // WEAPON SYSTEM
    // ============================================================

    DrawLine(
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        static_cast<int>(hudRight),
        static_cast<int>(y),
        UI_LINE
    );

    y += 30.0f;

    DrawText(
        "         WEAPON SYSTEM",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        20,
        CYAN
    );

    y += 35.0f;

    DrawText(
        "PRIMARY WEAPON",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        20,
        MUTED
    );

    y += 30.0f;

    float heatRatio = 0.0f;

    if (game.maxPlayerHeat > 0)
    {
        heatRatio =
            static_cast<float>(game.heat) /
            static_cast<float>(game.maxPlayerHeat);
    }

    heatRatio =
        std::max(
            0.0f,
            std::min(1.0f, heatRatio)
        );

    Color heatColor = CYAN;

    if (game.overheated)
    {
        heatColor = UI_RED;
    }
    else if (heatRatio >= 0.80f)
    {
        heatColor = UI_YELLOW;
    }

    DrawRectangle(
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        static_cast<int>(hudWidth),
        20,
        EMPTY
    );

    if (heatRatio > 0.0f)
    {
        DrawRectangle(
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            static_cast<int>(hudWidth * heatRatio),
            20,
            heatColor
        );
    }

    y += 34.0f;

    if (game.overheated)
    {
        DrawText(
            "!! OVERHEATED !!",
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            20,
            UI_RED
        );
    }
    else
    {
        DrawText(
            "HEAT",
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            20,
            MUTED
        );

        drawTextRight(
            TextFormat(
                "%d / %d",
                game.heat,
                game.maxPlayerHeat
            ),
            static_cast<int>(hudRight),
            static_cast<int>(y),
            20,
            TEXT
        );
    }

    y += 48.0f;

    // ============================================================
    // COMBAT STATUS
    // ============================================================

    DrawLine(
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        static_cast<int>(hudRight),
        static_cast<int>(y),
        UI_LINE
    );

    y += 30.0f;

    DrawText(
        "         COMBAT STATUS",
        static_cast<int>(hudLeft),
        static_cast<int>(y),
        20,
        game.boss ? UI_RED : CYAN
    );

    y += 38.0f;

    // ============================================================
    // BOSS
    // ============================================================

    if (game.boss)
    {
        const int bossHP =
            std::max(0, game.bossEngine.getHealth());

        const int bossMaxHP =
            std::max(1, game.bossEngine.getMaxHealth());

        float bossRatio =
            static_cast<float>(bossHP) /
            static_cast<float>(bossMaxHP);

        bossRatio =
            std::max(
                0.0f,
                std::min(1.0f, bossRatio)
            );

        DrawText(
            "BOSS HEALTH",
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            20,
            MUTED
        );

        y += 30.0f;

        DrawRectangle(
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            static_cast<int>(hudWidth),
            20,
            Color{ 50, 15, 25, 255 }
        );

        DrawRectangle(
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            static_cast<int>(hudWidth * bossRatio),
            20,
            UI_RED
        );

        y += 35.0f;

        DrawText(
            TextFormat(
                "%d / %d HP",
                bossHP,
                bossMaxHP
            ),
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            20,
            TEXT
        );
    }
    else
    {
        const int remaining =
            std::max(
                0,
                game.enemyCount - game.enemyCounter
            );

        DrawText(
            "REMAINING",
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            20,
            MUTED
        );

        drawTextRight(
            TextFormat(
                "%02d",
                remaining,
                game.maxPlayerHeat
            ),
            static_cast<int>(hudRight),
            static_cast<int>(y),
            20,
            TEXT
        );



        y += 30.0f;

        const float enemyRatio =
            game.enemyCount > 0
            ? static_cast<float>(remaining) /
            static_cast<float>(game.enemyCount)
            : 0.0f;

        DrawRectangle(
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            static_cast<int>(hudWidth),
            12,
            EMPTY
        );

        DrawRectangle(
            static_cast<int>(hudLeft),
            static_cast<int>(y),
            static_cast<int>(hudWidth * enemyRatio),
            20,
            CYAN
        );


        y += 30.0f;

    }
    const float controlsY =
        hudY + hudH - 180.0f;


    // ============================================================
    // BOTTOM ACCENT
    // ============================================================

    DrawLine(
        static_cast<int>(hudLeft),
        static_cast<int>(hudY + hudH - 15),
        static_cast<int>(hudRight),
        static_cast<int>(hudY + hudH - 15),
        UI_LINE
    );

    DrawRectangle(
        static_cast<int>(hudLeft),
        static_cast<int>(hudY + hudH - 16),
        18,
        2,
        CYAN
    );

    EndMode2D();
}

void Graphics::gameover(bool isGameOver, int score)
{
    loadAssets();

    while (!WindowShouldClose())
    {
        constexpr float VIRTUAL_WIDTH = 1600.0f;
        constexpr float VIRTUAL_HEIGHT = 900.0f;

        const int actualScreenW = GetScreenWidth();
        const int actualScreenH = GetScreenHeight();

        const float scaleX =
            static_cast<float>(actualScreenW) / VIRTUAL_WIDTH;

        const float scaleY =
            static_cast<float>(actualScreenH) / VIRTUAL_HEIGHT;

        const float virtualScale =
            std::min(scaleX, scaleY);

        Camera2D camera{};
        camera.target = Vector2{
            VIRTUAL_WIDTH * 0.5f,
            VIRTUAL_HEIGHT * 0.5f
        };

        camera.offset = Vector2{
            static_cast<float>(actualScreenW) * 0.5f,
            static_cast<float>(actualScreenH) * 0.5f
        };

        camera.rotation = 0.0f;
        camera.zoom = virtualScale;

        const int screenW = static_cast<int>(VIRTUAL_WIDTH);
        const int screenH = static_cast<int>(VIRTUAL_HEIGHT);
        const double time = GetTime();

        BeginDrawing();

        ClearBackground(Color{ 3, 6, 16, 255 });

        BeginMode2D(camera);

        drawBackground(screenW, screenH);

        DrawRectangle(
            0,
            0,
            screenW,
            screenH,
            Color{ 0, 3, 12, 150 }
        );

        const float cardW =
            std::min(720.0f, screenW * 0.72f);

        const float cardH =
            std::min(390.0f, screenH * 0.58f);

        const float cardX =
            (screenW - cardW) * 0.5f;

        const float cardY =
            (screenH - cardH) * 0.5f;

        DrawRectangleRounded(
            Rectangle{ cardX, cardY, cardW, cardH },
            0.06f,
            16,
            Color{ 7, 12, 28, 235 }
        );

        DrawRectangleRoundedLinesEx(
            Rectangle{ cardX, cardY, cardW, cardH },
            0.06f,
            16,
            2.0f,
            isGameOver
            ? Color{ 255, 76, 92, 150 }
            : Color{ 70, 235, 145, 150 }
        );

        const char* title =
            isGameOver ? "GAME OVER" : "YOU WIN";

        const Color titleColor =
            isGameOver
            ? UI_RED
            : UI_GREEN;

        drawCentered(
            title,
            static_cast<int>(cardY + 75),
            64,
            titleColor,
            screenW
        );

        drawCentered(
            TextFormat("FINAL SCORE  %d", score),
            static_cast<int>(cardY + 175),
            28,
            TEXT,
            screenW
        );

        const float pulse =
            150.0f +
            105.0f *
            static_cast<float>(
                (std::sin(time * 4.0) + 1.0) * 0.5
                );

        drawCentered(
            "PRESS ENTER TO CLOSE",
            static_cast<int>(cardY + cardH - 80),
            18,
            Color{
                180,
                210,
                255,
                static_cast<unsigned char>(pulse)
            },
            screenW
        );

        EndMode2D();

        EndDrawing();

        PollInputEvents();
        SwapScreenBuffer();

        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE))
            break;
    }
}

#include "raylib.h"
#include <cmath>

#ifdef PLATFORM_WEB
#include <emscripten.h>
#endif

void Graphics::mainMenu(Game& game)
{
    loadAssets();

    if (soundsLoaded)
    {
        UpdateMusicStream(backgroundMusic);

        if (!IsMusicStreamPlaying(backgroundMusic))
        {
            PlayMusicStream(backgroundMusic);
        }
    }

    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    const double time = GetTime();

    drawBackground(screenW, screenH);

    DrawRectangle(
        0,
        0,
        screenW,
        screenH,
        Color{ 2, 5, 14, 175 }
    );

    const int margin =
        static_cast<int>(screenW * 0.045f);

    // =========================================================
    // TOP HUD
    // =========================================================

    DrawText(
        "SR // COMMAND TERMINAL",
        margin,
        24,
        14,
        Color{ 100, 210, 240, 190 }
    );

    DrawText(
        "BUILD 01",
        margin,
        46,
        10,
        MUTED
    );

    DrawCircle(
        screenW - margin - 76,
        32,
        4,
        Color{ 70, 235, 145, 230 }
    );

    DrawText(
        "ONLINE",
        screenW - margin - 62,
        24,
        14,
        Color{ 70, 235, 145, 220 }
    );

    DrawLine(
        margin,
        72,
        screenW - margin,
        72,
        Color{ 80, 210, 240, 65 }
    );

    // =========================================================
    // LEFT TITLE
    // =========================================================

    const int brandX = margin;
    const int brandY =
        static_cast<int>(screenH * 0.19f);

    const int titleSize = 108;

    DrawText(
        "SPACE",
        brandX,
        brandY,
        titleSize,
        WHITE
    );

    DrawText(
        "RAIDERS",
        brandX,
        brandY + 100,
        titleSize,
        CYAN
    );

    DrawRectangle(
        brandX,
        brandY + 220,
        260,
        4,
        CYAN
    );

    DrawText(
        "GALACTIC DEFENSE INITIATIVE",
        brandX,
        brandY + 242,
        15,
        Color{ 150, 180, 195, 210 }
    );

    DrawText(
        "CREATED BY MUHAMMAD HUZAIFA",
        brandX,
        brandY + 270,
        15,
        MUTED
    );

    // =========================================================
    // CENTER RADAR
    // =========================================================

    const float radarX =
        screenW * 0.49f;

    const float radarY =
        screenH * 0.48f;

    const float radarRadius =
        std::min(
            145.0f,
            screenH * 0.18f
        );

    DrawCircleLines(
        static_cast<int>(radarX),
        static_cast<int>(radarY),
        radarRadius,
        Color{ 60, 190, 230, 35 }
    );

    DrawCircleLines(
        static_cast<int>(radarX),
        static_cast<int>(radarY),
        radarRadius * 0.68f,
        Color{ 60, 190, 230, 45 }
    );

    DrawCircleLines(
        static_cast<int>(radarX),
        static_cast<int>(radarY),
        radarRadius * 0.30f,
        Color{ 60, 190, 230, 65 }
    );

    DrawLine(
        static_cast<int>(radarX - radarRadius - 20),
        static_cast<int>(radarY),
        static_cast<int>(radarX + radarRadius + 20),
        static_cast<int>(radarY),
        Color{ 60, 190, 230, 30 }
    );

    DrawLine(
        static_cast<int>(radarX),
        static_cast<int>(radarY - radarRadius - 20),
        static_cast<int>(radarX),
        static_cast<int>(radarY + radarRadius + 20),
        Color{ 60, 190, 230, 30 }
    );

    const float radarAngle =
        static_cast<float>(time * 0.45);

    const float scanX =
        radarX +
        std::cos(radarAngle) * radarRadius;

    const float scanY =
        radarY +
        std::sin(radarAngle) * radarRadius;

    DrawLine(
        static_cast<int>(radarX),
        static_cast<int>(radarY),
        static_cast<int>(scanX),
        static_cast<int>(scanY),
        Color{ 80, 225, 255, 150 }
    );

    DrawCircle(
        static_cast<int>(radarX),
        static_cast<int>(radarY),
        6,
        Color{ 80, 230, 255, 230 }
    );

    DrawText(
        "SECTOR 07",
        static_cast<int>(radarX - 43),
        static_cast<int>(radarY - 13),
        12,
        Color{ 170, 210, 225, 180 }
    );

    DrawText(
        "NO HOSTILES",
        static_cast<int>(radarX - 47),
        static_cast<int>(radarY + 9),
        11,
        Color{ 70, 235, 145, 180 }
    );

    // =========================================================
// FLIGHT CONTROLS
// =========================================================

    const float controlsX = margin;
    const float controlsY = screenH * 0.64f;
    const float controlsW = 440.0f;
    const float controlsH = 145.0f;

    Rectangle controlsPanel{
        controlsX,
        controlsY,
        controlsW,
        controlsH
    };

    // Panel background
    DrawRectangle(
        static_cast<int>(controlsPanel.x),
        static_cast<int>(controlsPanel.y),
        static_cast<int>(controlsPanel.width),
        static_cast<int>(controlsPanel.height),
        Color{ 6, 15, 28, 225 }
    );

    // Panel border
    DrawRectangleLinesEx(
        controlsPanel,
        1.0f,
        Color{ 70, 190, 225, 75 }
    );

    // Small cyan accent
    DrawRectangle(
        static_cast<int>(controlsX),
        static_cast<int>(controlsY),
        4,
        static_cast<int>(controlsH),
        Color{ 80, 225, 255, 190 }
    );

    // Header
    DrawText(
        "FLIGHT CONTROLS",
        static_cast<int>(controlsX + 20),
        static_cast<int>(controlsY + 15),
        17,
        WHITE
    );

    // Header line
    DrawLine(
        static_cast<int>(controlsX + 20),
        static_cast<int>(controlsY + 40),
        static_cast<int>(controlsX + controlsW - 20),
        static_cast<int>(controlsY + 40),
        Color{ 80, 210, 240, 55 }
    );

    // Controls
    DrawText(
        "W A S D",
        static_cast<int>(controlsX + 22),
        static_cast<int>(controlsY + 55),
        15,
        Color{ 80, 225, 255, 210 }
    );

    DrawText(
        "MOVE",
        static_cast<int>(controlsX + 110),
        static_cast<int>(controlsY + 55),
        13,
        MUTED
    );

    DrawText(
        "ARROW KEYS",
        static_cast<int>(controlsX + 185),
        static_cast<int>(controlsY + 55),
        15,
        Color{ 80, 225, 255, 210 }
    );

    DrawText(
        "ALTERNATIVE",
        static_cast<int>(controlsX + 305),
        static_cast<int>(controlsY + 55),
        11,
        MUTED
    );

    DrawText(
        "SPACE",
        static_cast<int>(controlsX + 22),
        static_cast<int>(controlsY + 88),
        15,
        Color{ 80, 225, 255, 210 }
    );

    DrawText(
        "FIRE",
        static_cast<int>(controlsX + 110),
        static_cast<int>(controlsY + 88),
        13,
        MUTED
    );

    DrawText(
        "ESC",
        static_cast<int>(controlsX + 185),
        static_cast<int>(controlsY + 88),
        15,
        Color{ 80, 225, 255, 210 }
    );

    DrawText(
        "PAUSE / MENU",
        static_cast<int>(controlsX + 240),
        static_cast<int>(controlsY + 88),
        13,
        MUTED
    );


    // =========================================================
    // RIGHT MENU
    // =========================================================

    const float navW = 400.0f;

    const float navX =
        screenW - margin - navW;

    const float navY =
        screenH * 0.18f;

    DrawText(
        "MISSION CONTROL",
        static_cast<int>(navX),
        static_cast<int>(navY),
        18,
        WHITE
    );

    DrawText(
        "SELECT OPERATION",
        static_cast<int>(navX),
        static_cast<int>(navY + 22),
        18,
        MUTED
    );

    DrawLine(
        static_cast<int>(navX),
        static_cast<int>(navY + 42),
        static_cast<int>(navX + navW),
        static_cast<int>(navY + 42),
        Color{ 80, 210, 240, 70 }
    );

    const float itemH = 78.0f;
    const float itemGap = 10.0f;

    const float firstItemY =
        navY + 62.0f;

    Rectangle newGameButton{
        navX,
        firstItemY,
        navW,
        itemH
    };

    Rectangle resumeButton{
        navX,
        firstItemY + (itemH + itemGap),
        navW,
        itemH
    };

    Rectangle featuresButton{
        navX,
        firstItemY + (itemH + itemGap) * 2.0f,
        navW,
        itemH
    };

    Rectangle exitButton{
        navX,
        firstItemY + (itemH + itemGap) * 3.0f,
        navW,
        itemH
    };

    Vector2 mouse =
        GetMousePosition();

    const bool newGameHover =
        CheckCollisionPointRec(
            mouse,
            newGameButton
        );

    const bool resumeHover =
        CheckCollisionPointRec(
            mouse,
            resumeButton
        );

    const bool featuresHover =
        CheckCollisionPointRec(
            mouse,
            featuresButton
        );

    const bool exitHover =
        CheckCollisionPointRec(
            mouse,
            exitButton
        );

    // =========================================================
    // BUTTON DRAWING
    // =========================================================

    auto drawNavigation =
        [&](Rectangle rect,
            const char* number,
            const char* label,
            bool hover,
            bool enabled,
            bool danger)
        {
            Color background =
                enabled
                ? Color{ 6, 15, 28, 235 }
            : Color{ 8, 11, 18, 210 };

            Color border =
                enabled
                ? Color{ 70, 190, 225, 75 }
            : Color{ 90, 95, 105, 35 };

            if (hover && enabled)
            {
                background =
                    danger
                    ? Color{ 55, 15, 25, 245 }
                : Color{ 12, 48, 68, 245 };

                border =
                    danger
                    ? Color{ 255, 75, 90, 235 }
                : Color{ 80, 225, 255, 235 };

                DrawRectangle(
                    static_cast<int>(rect.x - 6),
                    static_cast<int>(rect.y),
                    4,
                    static_cast<int>(rect.height),
                    border
                );
            }

            DrawRectangle(
                static_cast<int>(rect.x),
                static_cast<int>(rect.y),
                static_cast<int>(rect.width),
                static_cast<int>(rect.height),
                background
            );

            DrawRectangleLinesEx(
                rect,
                1.0f,
                border
            );

            DrawText(
                number,
                static_cast<int>(rect.x + 22),
                static_cast<int>(rect.y + 14),
                13,
                enabled
                ? Color{ 80, 210, 240, 170 }
                : Color{ 90, 95, 105, 100 }
            );

            DrawText(
                label,
                static_cast<int>(rect.x + 22),
                static_cast<int>(rect.y + 35),
                24,
                enabled
                ? WHITE
                : Color{ 100, 105, 115, 180 }
            );

            if (hover && enabled)
            {
                DrawText(
                    ">>",
                    static_cast<int>(
                        rect.x +
                        rect.width -
                        45
                        ),
                    static_cast<int>(
                        rect.y + 30
                        ),
                    18,
                    border
                );
            }
        };

    drawNavigation(
        newGameButton,
        "01",
        "NEW GAME",
        newGameHover,
        true,
        false
    );

    drawNavigation(
        resumeButton,
        "02",
        "RESUME",
        resumeHover,
        game.hasSave,
        false
    );

    drawNavigation(
        featuresButton,
        "03",
        "FEATURES",
        featuresHover,
        true,
        false
    );

    drawNavigation(
        exitButton,
        "04",
        "EXIT",
        exitHover,
        true,
        true
    );

    // =========================================================
    // CHECKPOINT STATUS
    // =========================================================

    const int statusY =
        static_cast<int>(
            firstItemY +
            (itemH + itemGap) * 4.0f +
            16.0f
            );

    DrawText(
        "CHECKPOINT",
        static_cast<int>(navX),
        statusY,
        15,
        MUTED
    );

    DrawText(
        game.hasSave
        ? "   AVAILABLE"
        : "   EMPTY",
        static_cast<int>(navX + 105),
        statusY,
        15,
        game.hasSave
        ? Color{ 70, 235, 145, 190 }
        : MUTED
    );

    // =========================================================
    // BOTTOM HUD
    // =========================================================

    DrawLine(
        margin,
        screenH - 72,
        screenW - margin,
        screenH - 72,
        Color{ 80, 210, 240, 50 }
    );

    DrawText(
        "ENTER  SELECT",
        margin,
        screenH - 48,
        11,
        MUTED
    );

    DrawText(
        "MOUSE  NAVIGATE",
        margin + 125,
        screenH - 48,
        11,
        MUTED
    );

    const char* readyText =
        "SYSTEM READY";

    DrawText(
        readyText,
        screenW -
        MeasureText(readyText, 11) -
        margin,
        screenH - 48,
        11,
        Color{ 70, 235, 145, 180 }
    );

    // =========================================================
    // MOUSE INPUT
    // =========================================================

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (newGameHover)
        {
            loadSounds();
            game.startGame();
        }
        else if (resumeHover && game.hasSave)
        {
            loadSounds();

            if (game.loadGame())
            {
                game.gameState = GameState::LEVEL_INTRO;
                game.levelIntroStartTime = GetTime();
            }
        }
        else if (featuresHover)
        {
#ifdef PLATFORM_WEB
            EM_ASM({
                window.location.href = "features.html";
                });
#else
            OpenURL("features.html");
#endif
        }
        else if (exitHover)
        {
            CloseWindow();
            exit(0);
        }
    }

    // =========================================================
    // KEYBOARD
    // =========================================================
}


void Graphics::levelIntro(Game& game)
{
    loadAssets();

    if (soundsLoaded)
    {
        UpdateMusicStream(backgroundMusic);

        if (!IsMusicStreamPlaying(backgroundMusic))
        {
            PlayMusicStream(backgroundMusic);
        }
    }


    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    drawBackground(screenW, screenH);

    DrawRectangle(
        0, 0, screenW, screenH,
        Color{ 0, 3, 15, 190 }
    );

    const float cardW = std::min(700.0f, screenW * 0.84f);
    const float cardH = std::min(580.0f, screenH * 0.84f);

    const float cardX = (screenW - cardW) * 0.5f;
    const float cardY = (screenH - cardH) * 0.5f;

    DrawRectangleRounded(
        Rectangle{ cardX, cardY, cardW, cardH },
        0.045f,
        16,
        Color{ 7, 12, 28, 245 }
    );

    DrawRectangleRoundedLinesEx(
        Rectangle{ cardX, cardY, cardW, cardH },
        0.045f,
        16,
        2.0f,
        CYAN
    );

    // =========================================================
    // LEVEL NUMBER
    // =========================================================

    const char* levelText =
        TextFormat("LEVEL %02d", game.level);

    DrawText(
        levelText,
        static_cast<int>(
            cardX +
            (cardW - MeasureText(levelText, 52)) * 0.5f
            ),
        static_cast<int>(cardY + 28),
        52,
        CYAN
    );

    // =========================================================
    // ACT INTRO LEVELS
    // =========================================================

    if (game.level == 1 ||
        game.level == 11 ||
        game.level == 21 ||
        game.level == 31 ||
        game.level == 41)
    {
        const char* act = "";
        const char* title = "";
        const char* subtitle = "";

        switch (game.level)
        {
        case 1:
            act = "ACT I";
            title = "THE BEGINNING";
            subtitle = "YOUR JOURNEY BEGINS";
            break;

        case 11:
            act = "ACT II";
            title = "ESCALATION";
            subtitle = "THE ENEMY ADAPTS";
            break;

        case 21:
            act = "ACT III";
            title = "ADAPTATION";
            subtitle = "THE BATTLE INTENSIFIES";
            break;

        case 31:
            act = "ACT IV";
            title = "WARZONE";
            subtitle = "NOTHING FIGHTS ALONE";
            break;

        case 41:
            act = "ACT V";
            title = "ENDGAME";
            subtitle = "THE FINAL ASSAULT";
            break;
        }

        DrawText(
            act,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(act, 38)) * 0.5f
                ),
            static_cast<int>(cardY + 105),
            38,
            YELLOW
        );

        DrawText(
            title,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(title, 34)) * 0.5f
                ),
            static_cast<int>(cardY + 150),
            34,
            WHITE
        );

        DrawText(
            subtitle,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(subtitle, 23)) * 0.5f
                ),
            static_cast<int>(cardY + 195),
            23,
            MUTED
        );

        float y = cardY + 255.0f;

        const char* section = "NEW IN THIS ACT";

        DrawText(
            section,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(section, 27)) * 0.5f
                ),
            static_cast<int>(y),
            27,
            CYAN
        );

        y += 55.0f;

        if (game.level == 1)
        {
            const char* a = "STARTING COMBAT";
            const char* b =
                "Basic enemies, formations and combat patterns begin here.";

            DrawText(
                a,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a, 25)) * 0.5f
                    ),
                static_cast<int>(y),
                25,
                UI_GREEN
            );

            y += 40.0f;

            DrawText(
                b,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(b, 19)) * 0.5f
                    ),
                static_cast<int>(y),
                19,
                MUTED
            );

            y += 70.0f;

            const char* c = "PROGRESSION AHEAD";
            const char* d =
                "New attributes unlock as you advance.";

            DrawText(
                c,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(c, 25)) * 0.5f
                    ),
                static_cast<int>(y),
                25,
                UI_YELLOW
            );

            y += 40.0f;

            DrawText(
                d,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(d, 18)) * 0.5f
                    ),
                static_cast<int>(y),
                18,
                MUTED
            );
        }
        else if (game.level == 11)
        {
            const char* a = "NEW FIRE MODE";
            const char* b =
                "A new enemy firing pattern enters the battlefield.";

            DrawText(
                a,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a, 25)) * 0.5f
                    ),
                static_cast<int>(y),
                25,
                UI_GREEN
            );

            y += 40.0f;

            DrawText(
                b,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(b, 19)) * 0.5f
                    ),
                static_cast<int>(y),
                19,
                MUTED
            );

            y += 65.0f;

            const char* h = "PLAYER MAX HEALTH     +1";
            const char* heat = "PLAYER MAX HEAT       +5";

            DrawText(
                h,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(h, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );

            y += 38.0f;

            DrawText(
                heat,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(heat, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );
        }
        else if (game.level == 21)
        {
            const char* a = "NEW FIRE MODE";
            const char* b =
                "Another enemy firing pattern enters the pool.";

            DrawText(
                a,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a, 25)) * 0.5f
                    ),
                static_cast<int>(y),
                25,
                UI_GREEN
            );

            y += 40.0f;

            DrawText(
                b,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(b, 19)) * 0.5f
                    ),
                static_cast<int>(y),
                19,
                MUTED
            );

            y += 65.0f;

            const char* h = "PLAYER MAX HEALTH     +1";
            const char* heat = "PLAYER MAX HEAT       +3";

            DrawText(
                h,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(h, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );

            y += 38.0f;

            DrawText(
                heat,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(heat, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );
        }
        else if (game.level == 31)
        {
            const char* a = "MIXED ENEMIES";
            const char* b =
                "Different enemy types can now appear together.";

            DrawText(
                a,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a, 25)) * 0.5f
                    ),
                static_cast<int>(y),
                25,
                UI_GREEN
            );

            y += 40.0f;

            DrawText(
                b,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(b, 19)) * 0.5f
                    ),
                static_cast<int>(y),
                19,
                MUTED
            );

            y += 65.0f;

            const char* a1 = "ATTACKER PRESSURE     +2";
            const char* a2 = "PLAYER MAX HEALTH     +1";
            const char* a3 = "PLAYER MAX HEAT       +3";

            DrawText(
                a1,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a1, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                UI_YELLOW
            );

            y += 38.0f;

            DrawText(
                a2,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a2, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );

            y += 38.0f;

            DrawText(
                a3,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a3, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );
        }
        else if (game.level == 41)
        {
            const char* a = "REINFORCEMENTS";
            const char* b =
                "Additional enemies can enter during the wave.";

            DrawText(
                a,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(a, 25)) * 0.5f
                    ),
                static_cast<int>(y),
                25,
                UI_RED
            );

            y += 40.0f;

            DrawText(
                b,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(b, 19)) * 0.5f
                    ),
                static_cast<int>(y),
                19,
                MUTED
            );

            y += 60.0f;

            const char* c = "ELITE PROMOTION";
            const char* d =
                "Remaining enemies can be promoted to ELITE.";

            DrawText(
                c,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(c, 25)) * 0.5f
                    ),
                static_cast<int>(y),
                25,
                UI_RED
            );

            y += 40.0f;

            DrawText(
                d,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(d, 19)) * 0.5f
                    ),
                static_cast<int>(y),
                19,
                MUTED
            );

            y += 65.0f;

            const char* h = "PLAYER MAX HEALTH     +1";
            const char* heat = "PLAYER MAX HEAT       +3";

            DrawText(
                h,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(h, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );

            y += 38.0f;

            DrawText(
                heat,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(heat, 24)) * 0.5f
                    ),
                static_cast<int>(y),
                24,
                CYAN
            );
        }

        const char* deploy = "PRESS ENTER TO DEPLOY";

        DrawText(
            deploy,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(deploy, 21)) * 0.5f
                ),
            static_cast<int>(cardY + cardH - 45),
            21,
            TEXT
        );

        return;
    }

    // =========================================================
    // BOSS LEVELS
    // =========================================================

    if (game.boss)
    {
        const char* bossText = "BOSS INCOMING";

        DrawText(
            bossText,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(bossText, 36)) * 0.5f
                ),
            static_cast<int>(cardY + 125),
            36,
            UI_RED
        );

        const char* objective =
            "DEFEAT THE BOSS TO CONTINUE";

        DrawText(
            objective,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(objective, 23)) * 0.5f
                ),
            static_cast<int>(cardY + 185),
            23,
            MUTED
        );

        const char* deploy =
            "PRESS ENTER TO DEPLOY";

        DrawText(
            deploy,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(deploy, 21)) * 0.5f
                ),
            static_cast<int>(cardY + cardH - 55),
            21,
            TEXT
        );

        return;
    }

    // =========================================================
    // NORMAL LEVELS
    // =========================================================

    float y = cardY + 125.0f;
    bool hasChanges = false;

    const char* heading = "CHANGES/NEW POOL";

    DrawText(
        heading,
        static_cast<int>(
            cardX +
            (cardW - MeasureText(heading, 30)) * 0.5f
            ),
        static_cast<int>(y),
        30,
        CYAN
    );

    y += 70.0f;

    auto addLine =
        [&](const char* text, Color color = WHITE)
        {
            const int fontSize = 27;

            DrawText(
                text,
                static_cast<int>(
                    cardX +
                    (cardW - MeasureText(text, fontSize)) * 0.5f
                    ),
                static_cast<int>(y),
                fontSize,
                color
            );

            y += 48.0f;
            hasChanges = true;
        };

    switch (game.level)
    {
    case 2:
        addLine("ENEMIES          +2");
        break;

    case 3:
        addLine("MOVEMENT         STRAIGHT", UI_GREEN);
        break;

    case 4:
        addLine("FORMATION        TRIANGLE", UI_GREEN);
        break;

    case 5:
        addLine("ENEMY SPEED      +1", UI_YELLOW);
        addLine("ELITES           +2", UI_RED);
        break;

    case 6:
        addLine("ENEMY TYPE       SPEEDY", UI_GREEN);
        addLine("MAX HEALTH       +1", CYAN);
        addLine("MAX WEAPON HEAT  +3", CYAN);
        break;

    case 7:
        addLine("ENEMY COOLDOWN   +4", UI_YELLOW);
        break;

    case 8:
        addLine("FORMATION        DIAMOND", UI_GREEN);
        addLine("ENEMIES          +3");
        break;

    case 9:
        addLine("INSANE LEVEL", UI_RED);
        addLine("ENEMIES          +5");
        addLine("ATTACKERS        +3");
        addLine("FIRE PRESSURE    +8");
        addLine("BULLET SPEED     +1");
        addLine("ELITES           +3", UI_RED);
        break;

    case 12:
        addLine("ENEMIES          +2");
        break;

    case 13:
        addLine("MOVEMENT         DIVE", UI_GREEN);
        break;

    case 14:
        addLine("FORMATION        CIRCLE", UI_GREEN);
        break;

    case 15:
        addLine("ENEMY SPEED      +1", UI_YELLOW);
        addLine("ELITES           +4", UI_RED);
        break;

    case 16:
        addLine("ENEMY TYPE       TANK", UI_GREEN);
        addLine("MAX WEAPON HEAT  +3", CYAN);

        break;

    case 17:
        addLine("ENEMY COOLDOWN   +4", UI_YELLOW);
        break;

    case 18:
        addLine("ENEMIES          +3");
        break;

    case 19:
        addLine("INSANE LEVEL", UI_RED);
        addLine("ENEMIES          +7");
        addLine("ATTACKERS        +4");
        addLine("FIRE PRESSURE    +8");
        addLine("BULLET SPEED     +1");
        addLine("ELITES           +5", UI_RED);
        break;

    case 22:
        addLine("ENEMY TYPE       DESTROYER", UI_GREEN);
        addLine("ENEMIES          +2");
        break;

    case 23:
        addLine("MOVEMENT         ZIGZAG ", UI_GREEN);
        break;

    case 24:
        addLine("FORMATION        RANDOM", UI_GREEN);
        break;

    case 25:
        addLine("ENEMY SPEED      +1", UI_YELLOW);
        addLine("ELITES           +6", UI_RED);
        break;

    case 26:
        addLine("ENEMIES          +1");
        addLine("MAX WEAPON HEAT  +3", CYAN);

        break;

    case 27:
        addLine("ENEMY COOLDOWN   +4", UI_YELLOW);
        break;

    case 28:
        addLine("ENEMIES          +3");
        break;

    case 29:
        addLine("INSANE LEVEL", UI_RED);
        addLine("ENEMIES          +9");
        addLine("ATTACKERS        +5");
        addLine("FIRE PRESSURE    +8");
        addLine("BULLET SPEED     +1");
        addLine("ELITES           +7", UI_RED);
        break;

    case 32:
        addLine("ENEMIES          +2");
        break;

    case 33:
        addLine("ENEMY HEALTH     +SCALING", UI_RED);
        break;

    case 34:
        addLine("FIRE DELAY       -5", UI_YELLOW);
        break;

    case 35:
        addLine("BULLET SPEED     +1", UI_YELLOW);
        addLine("ELITES           +6", UI_RED);
        break;

    case 36:
        addLine("ENEMIES          +1");
        addLine("MAX WEAPON HEAT  +3", CYAN);

        break;

    case 37:
        addLine("ENEMY COOLDOWN   +4", UI_YELLOW);
        break;

    case 38:
        addLine("ENEMIES          +3");
        break;

    case 39:
        addLine("INSANE LEVEL", UI_RED);
        addLine("ENEMIES          +11");
        addLine("ATTACKERS        +6");
        addLine("FIRE PRESSURE    +8");
        addLine("BULLET SPEED     +1");
        addLine("ELITES           +9", UI_RED);
        break;

    case 42:
        addLine("ELITES           2", UI_RED);
        break;

    case 43:
        addLine("ELITES           3", UI_RED);
        break;

    case 44:
        addLine("ELITES           4", UI_RED);
        break;

    case 45:
        addLine("ELITES           5", UI_RED);
        break;

    case 46:
        addLine("ELITES           6", UI_RED);
        addLine("MAX WEAPON HEAT  +5", CYAN);

        break;

    case 47:
        addLine("ELITES           7", UI_RED);
        break;

    case 48:
        addLine("ELITES           8", UI_RED);
        break;

    case 49:
        addLine("ELITES           9", UI_RED);
        break;

    default:
        break;
    }

    if (!hasChanges)
    {
        const char* text =
            "ENEMY DIFFICULTY CONTINUES TO RISE";

        DrawText(
            text,
            static_cast<int>(
                cardX +
                (cardW - MeasureText(text, 23)) * 0.5f
                ),
            static_cast<int>(cardY + 300),
            23,
            MUTED
        );
    }

    const char* deploy =
        "PRESS ENTER TO DEPLOY";

    DrawText(
        deploy,
        static_cast<int>(
            cardX +
            (cardW - MeasureText(deploy, 21)) * 0.5f
            ),
        static_cast<int>(cardY + cardH - 45),
        21,
        TEXT
    );
}

void Graphics::endScreen(Game& game)
{
    loadAssets();

    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    const double time = GetTime();

    drawBackground(screenW, screenH);

    DrawRectangle(
        0,
        0,
        screenW,
        screenH,
        Color{ 1, 4, 12, 205 }
    );

    const bool won = game.hasWon();

    const Color accent =
        won
        ? Color{ 70, 235, 145, 255 }
    : Color{ 255, 75, 90, 255 };

    const Color accentSoft =
        won
        ? Color{ 70, 235, 145, 90 }
    : Color{ 255, 75, 90, 90 };

    // =========================================================
    // TOP HUD
    // =========================================================

    const int margin =
        static_cast<int>(screenW * 0.055f);

    DrawText(
        "SR // MISSION REPORT",
        margin,
        28,
        18,
        Color{ 100, 210, 240, 190 }
    );

    DrawText(
        won
        ? "MISSION COMPLETE"
        : "MISSION FAILED",
        screenW -
        margin -
        MeasureText(
            won
            ? "MISSION COMPLETE"
            : "MISSION FAILED",
            13
        ),
        30,
        16,
        accent
    );

    DrawLine(
        margin,
        62,
        screenW - margin,
        62,
        Color{ 80, 210, 240, 60 }
    );

    // =========================================================
    // MAIN PANEL
    // =========================================================

    const float panelW =
        std::min(820.0f, screenW * 0.72f);

    const float panelH =
        std::min(570.0f, screenH * 0.70f);

    const float panelX =
        (screenW - panelW) * 0.5f;

    const float panelY =
        (screenH - panelH) * 0.5f;

    Rectangle panel{
        panelX,
        panelY,
        panelW,
        panelH
    };

    DrawRectangleRounded(
        Rectangle{
            panelX - 4,
            panelY - 4,
            panelW + 8,
            panelH + 8
        },
        0.045f,
        16,
        Color{
            accent.r,
            accent.g,
            accent.b,
            15
        }
    );

    DrawRectangleRounded(
        panel,
        0.045f,
        16,
        Color{ 5, 12, 27, 248 }
    );

    DrawRectangleRoundedLinesEx(
        panel,
        0.045f,
        16,
        1.5f,
        accentSoft
    );

    DrawRectangle(
        static_cast<int>(panelX),
        static_cast<int>(panelY),
        4,
        static_cast<int>(panelH),
        accent
    );

    // =========================================================
    // STATUS
    // =========================================================

    const char* status =
        won
        ? "MISSION ACCOMPLISHED"
        : "SHIP LOST";

    DrawText(
        status,
        static_cast<int>(panelX + 35),
        static_cast<int>(panelY + 30),
        16,
        accent
    );

    DrawText(
        won
        ? "THE GALAXY SURVIVES"
        : "THE RAIDERS HAVE FALLEN",
        static_cast<int>(panelX + 35),
        static_cast<int>(panelY + 52),
        16,
        MUTED
    );

    DrawCircle(
        static_cast<int>(panelX + panelW - 48),
        static_cast<int>(panelY + 40),
        5,
        accent
    );

    // =========================================================
    // LARGE RESULT
    // =========================================================

    const char* title =
        won
        ? "YOU WIN"
        : "GAME OVER";

    const int titleSize = 72;

    DrawText(
        title,
        static_cast<int>(
            panelX +
            (panelW -
                MeasureText(title, titleSize)) *
            0.5f
            ),
        static_cast<int>(panelY + 95),
        titleSize,
        accent
    );

    // =========================================================
    // DIVIDER
    // =========================================================

    const float dividerW = 270.0f;

    DrawRectangle(
        static_cast<int>(
            panelX +
            (panelW - dividerW) * 0.5f
            ),
        static_cast<int>(panelY + 185),
        static_cast<int>(dividerW),
        2,
        accentSoft
    );

    // =========================================================
    // STAT CARDS
    // =========================================================

    const float statW = 300.0f;
    const float statH = 82.0f;
    const float statGap = 18.0f;

    const float statsX =
        panelX +
        (panelW -
            statW * 2.0f -
            statGap) *
        0.5f;

    const float statsY =
        panelY + 215.0f;

    Rectangle scoreCard{
        statsX,
        statsY,
        statW,
        statH
    };

    Rectangle levelCard{
        statsX + statW + statGap,
        statsY,
        statW,
        statH
    };

    DrawRectangleRounded(
        scoreCard,
        0.06f,
        10,
        Color{ 8, 20, 36, 240 }
    );

    DrawRectangleRoundedLinesEx(
        scoreCard,
        0.06f,
        10,
        1.0f,
        Color{ 70, 190, 225, 70 }
    );

    DrawRectangleRounded(
        levelCard,
        0.06f,
        10,
        Color{ 8, 20, 36, 240 }
    );

    DrawRectangleRoundedLinesEx(
        levelCard,
        0.06f,
        10,
        1.0f,
        Color{ 70, 190, 225, 70 }
    );

    // =========================================================
    // SCORE
    // =========================================================

    DrawText(
        "FINAL SCORE",
        static_cast<int>(scoreCard.x + 18),
        static_cast<int>(scoreCard.y + 14),
        16,
        MUTED
    );

    const char* scoreText =
        TextFormat("%d", game.score);

    DrawText(
        scoreText,
        static_cast<int>(scoreCard.x + 18),
        static_cast<int>(scoreCard.y + 34),
        30,
        WHITE
    );

    // =========================================================
    // LEVEL
    // =========================================================

    DrawText(
        "LEVEL REACHED",
        static_cast<int>(levelCard.x + 18),
        static_cast<int>(levelCard.y + 14),
        16,
        MUTED
    );

    const char* levelText =
        TextFormat("%d", game.level);

    DrawText(
        levelText,
        static_cast<int>(levelCard.x + 18),
        static_cast<int>(levelCard.y + 34),
        30,
        WHITE
    );

    // =========================================================
    // RESULT MESSAGE
    // =========================================================

    const char* message =
        won
        ? "THE GALAXY IS SAFE"
        : "THE MISSION ENDS HERE";

    DrawText(
        message,
        static_cast<int>(
            panelX +
            (panelW -
                MeasureText(message, 16)) *
            0.5f
            ),
        static_cast<int>(panelY + 330),
        18,
        Color{ 180, 200, 215, 210 }
    );

    // =========================================================
    // ENTER PROMPT
    // =========================================================

    const float pulse =
        150.0f +
        105.0f *
        static_cast<float>(
            (std::sin(time * 4.0) + 1.0) * 0.5
            );

    const char* prompt =
        "PRESS ENTER  //  RETURN TO MAIN MENU";

    DrawText(
        prompt,
        static_cast<int>(
            panelX +
            (panelW -
                MeasureText(prompt, 15)) *
            0.5f
            ),
        static_cast<int>(panelY + panelH - 60),
        15,
        Color{
            180,
            215,
            230,
            static_cast<unsigned char>(pulse)
        }
    );

    // =========================================================
    // BOTTOM HUD
    // =========================================================

    DrawLine(
        margin,
        screenH - 55,
        screenW - margin,
        screenH - 55,
        Color{ 80, 210, 240, 45 }
    );

    DrawText(
        "ENTER  RETURN",
        margin,
        screenH - 35,
        16,
        MUTED
    );

    const char* footer =
        won
        ? "STATUS // COMPLETE"
        : "STATUS // FAILED";

    DrawText(
        footer,
        screenW -
        margin -
        MeasureText(footer, 11),
        screenH - 35,
        16,
        accent
    );


}

void Graphics::pauseMenu(Game& game)
{
    const int screenW = GetScreenWidth();
    const int screenH = GetScreenHeight();

    // =========================================================
    // DARK OVERLAY
    // =========================================================

    DrawRectangle(
        0,
        0,
        screenW,
        screenH,
        Color{ 1, 4, 12, 205 }
    );

    // =========================================================
    // PANEL
    // =========================================================

    const float panelW =
        std::min(720.0f, screenW * 0.72f);

    const float panelH =
        std::min(620.0f, screenH * 0.78f);

    const float panelX =
        (screenW - panelW) * 0.5f;

    const float panelY =
        (screenH - panelH) * 0.5f;

    Rectangle panel{
        panelX,
        panelY,
        panelW,
        panelH
    };

    // Outer glow
    DrawRectangleRounded(
        Rectangle{
            panelX - 5.0f,
            panelY - 5.0f,
            panelW + 10.0f,
            panelH + 10.0f
        },
        0.045f,
        16,
        Color{ 0, 170, 220, 18 }
    );

    // Main panel
    DrawRectangleRounded(
        panel,
        0.045f,
        16,
        Color{ 5, 12, 27, 248 }
    );

    // Panel border
    DrawRectangleRoundedLinesEx(
        panel,
        0.045f,
        16,
        1.5f,
        Color{ 80, 210, 240, 110 }
    );

    // =========================================================
    // TOP HUD
    // =========================================================

    const float margin = 34.0f;

    DrawText(
        "SR // COMMAND TERMINAL",
        static_cast<int>(panelX + margin),
        static_cast<int>(panelY + 28),
        13,
        Color{ 100, 210, 240, 190 }
    );

    DrawText(
        "SESSION INTERRUPTED",
        static_cast<int>(panelX + margin),
        static_cast<int>(panelY + 50),
        10,
        MUTED
    );

    // Online indicator


    DrawText(
        "SYSTEM PAUSED",
        static_cast<int>(panelX + panelW - margin - 130),
        static_cast<int>(panelY + 28),
        11,
        Color{ 70, 235, 145, 210 }
    );

    DrawLine(
        static_cast<int>(panelX + margin),
        static_cast<int>(panelY + 76),
        static_cast<int>(panelX + panelW - margin),
        static_cast<int>(panelY + 76),
        Color{ 80, 210, 240, 55 }
    );

    // =========================================================
    // TITLE
    // =========================================================

    const char* title = "PAUSED";

    const int titleSize = 64;

    DrawText(
        title,
        static_cast<int>(
            panelX +
            (panelW - MeasureText(title, titleSize)) * 0.5f
            ),
        static_cast<int>(panelY + 105),
        titleSize,
        WHITE
    );

    const char* subtitle =
        "MISSION CONTROL // AWAITING COMMAND";

    DrawText(
        subtitle,
        static_cast<int>(
            panelX +
            (panelW - MeasureText(subtitle, 12)) * 0.5f
            ),
        static_cast<int>(panelY + 175),
        12,
        Color{ 100, 210, 240, 150 }
    );

    // Accent line underneath title
    const float accentW = 180.0f;

    DrawRectangle(
        static_cast<int>(
            panelX + (panelW - accentW) * 0.5f
            ),
        static_cast<int>(panelY + 200),
        static_cast<int>(accentW),
        2,
        Color{ 80, 210, 240, 150 }
    );

    // =========================================================
    // BUTTONS
    // =========================================================

    const float buttonW =
        std::min(500.0f, panelW - 100.0f);

    const float buttonH = 68.0f;

    const float buttonX =
        panelX + (panelW - buttonW) * 0.5f;

    const float buttonGap = 16.0f;

    const float resumeY =
        panelY + 235.0f;

    const float restartY =
        resumeY + buttonH + buttonGap;

    const float saveExitY =
        restartY + buttonH + buttonGap;

    Rectangle resumeButton{
        buttonX,
        resumeY,
        buttonW,
        buttonH
    };

    Rectangle restartButton{
        buttonX,
        restartY,
        buttonW,
        buttonH
    };

    Rectangle saveExitButton{
        buttonX,
        saveExitY,
        buttonW,
        buttonH
    };

    Vector2 mouse = GetMousePosition();

    const bool resumeHover =
        CheckCollisionPointRec(mouse, resumeButton);

    const bool restartHover =
        CheckCollisionPointRec(mouse, restartButton);

    const bool saveExitHover =
        CheckCollisionPointRec(mouse, saveExitButton);

    // ---------------------------------------------------------
    // BUTTON DRAW HELPER
    // ---------------------------------------------------------

    auto drawButton =
        [](Rectangle rect,
            const char* number,
            const char* label,
            bool hover,
            Color accent)
        {
            Color background =
                hover
                ? Color{
                    static_cast<unsigned char>(
                        accent.r * 0.22f
                    ),
                    static_cast<unsigned char>(
                        accent.g * 0.22f
                    ),
                    static_cast<unsigned char>(
                        accent.b * 0.22f
                    ),
                    245
            }
            : Color{ 8, 20, 36, 245 };

            Color border =
                hover
                ? Color{
                    accent.r,
                    accent.g,
                    accent.b,
                    235
            }
                : Color{
                    accent.r,
                    accent.g,
                    accent.b,
                    65
            };

            // Background
            DrawRectangleRounded(
                rect,
                0.08f,
                12,
                background
            );

            // Border
            DrawRectangleRoundedLinesEx(
                rect,
                0.08f,
                12,
                1.0f,
                border
            );

            // Left accent bar
            DrawRectangle(
                static_cast<int>(rect.x),
                static_cast<int>(rect.y + 12),
                4,
                static_cast<int>(rect.height - 24),
                border
            );

            // Number
            DrawText(
                number,
                static_cast<int>(rect.x + 24),
                static_cast<int>(rect.y + 12),
                12,
                border
            );

            // Label
            DrawText(
                label,
                static_cast<int>(rect.x + 24),
                static_cast<int>(rect.y + 34),
                23,
                hover ? WHITE : Color{ 220, 230, 238, 230 }
            );

            // Arrow
            if (hover)
            {
                DrawText(
                    ">>",
                    static_cast<int>(
                        rect.x + rect.width - 52
                        ),
                    static_cast<int>(rect.y + 25),
                    18,
                    accent
                );
            }
        };

    drawButton(
        resumeButton,
        "01",
        "RESUME MISSION",
        resumeHover,
        CYAN
    );

    drawButton(
        restartButton,
        "02",
        "RESTART CHECKPOINT",
        restartHover,
        Color{ 255, 190, 70, 255 }
    );

    drawButton(
        saveExitButton,
        "03",
        "SAVE & EXIT",
        saveExitHover,
        Color{ 70, 235, 145, 255 }
    );

    // =========================================================
    // BOTTOM STATUS
    // =========================================================

    DrawLine(
        static_cast<int>(panelX + margin),
        static_cast<int>(panelY + panelH - 65),
        static_cast<int>(panelX + panelW - margin),
        static_cast<int>(panelY + panelH - 65),
        Color{ 80, 210, 240, 45 }
    );

    DrawText(
        "ENTER",
        static_cast<int>(panelX + margin),
        static_cast<int>(panelY + panelH - 43),
        11,
        Color{ 100, 210, 240, 170 }
    );

    DrawText(
        "SELECT",
        static_cast<int>(panelX + margin + 48),
        static_cast<int>(panelY + panelH - 43),
        11,
        MUTED
    );

    DrawText(
        "ESC",
        static_cast<int>(panelX + margin + 115),
        static_cast<int>(panelY + panelH - 43),
        11,
        Color{ 100, 210, 240, 170 }
    );

    DrawText(
        "RETURN",
        static_cast<int>(panelX + margin + 150),
        static_cast<int>(panelY + panelH - 43),
        11,
        MUTED
    );

    const char* statusText = "CHECKPOINT SECURE";

    DrawText(
        statusText,
        static_cast<int>(
            panelX +
            panelW -
            margin -
            MeasureText(statusText, 11)
            ),
        static_cast<int>(panelY + panelH - 43),
        11,
        Color{ 70, 235, 145, 180 }
    );

    // =========================================================
    // INPUT
    // =========================================================

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (resumeHover)
        {
            game.paused = false;
        }
        else if (restartHover)
        {
            game.restartCheckpoint();
        }
        else if (saveExitHover)
        {
            game.saveGame();

            game.paused = false;
            game.gameState = GameState::MAIN_MENU;
        }
    }
}

void Graphics::loadSounds()
{
    if (soundsLoaded)
        return;

    InitAudioDevice();

    backgroundMusic =
        LoadMusicStream("assets/background.ogg");

    SetMusicVolume(backgroundMusic, 0.25f);

    PlayMusicStream(backgroundMusic);

    enemyDeathSound =
        LoadSound("assets/enemy_death.wav");

    playerDeathSound =
        LoadSound("assets/player_death.wav");

    SetSoundVolume(enemyDeathSound, 0.8f);
    SetSoundVolume(playerDeathSound, 0.8f);

    soundsLoaded = true;
}

void Graphics::playEnemyDeath()
{
    if (soundsLoaded)
    {
        PlaySound(enemyDeathSound);
    }
}

void Graphics::playPlayerDeath()
{
    if (soundsLoaded)
    {
        PlaySound(playerDeathSound);
    }
}

void Graphics::unloadSounds()
{
    if (!soundsLoaded)
        return;

    StopMusicStream(
        backgroundMusic
    );

    UnloadMusicStream(
        backgroundMusic
    );

    UnloadSound(
        enemyDeathSound
    );

    UnloadSound(
        playerDeathSound
    );

    CloseAudioDevice();

    soundsLoaded = false;
}