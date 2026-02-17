#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <algorithm>

extern "C" {
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <asndlib.h>
#include <mp3player.h>
}

constexpr int SCREEN_WIDTH = 640, SCREEN_HEIGHT = 480;
constexpr int PLAYER_SPEED = 5, BULLET_SPEED = 8, NUM_BULLETS = 5;
constexpr int ENEMY_ROWS = 5, ENEMY_COLS = 11, ENEMY_WIDTH = 40, ENEMY_HEIGHT = 30, ENEMY_MARGIN = 10;

static void* framebuffer = nullptr;
static GXRModeObj* screenMode = nullptr;

void initialize()
{
    srand(static_cast<unsigned>(time(nullptr)));

    VIDEO_Init();
    GRRLIB_Init();
    WPAD_Init();
    ASND_Init();
    MP3Player_Init();

    screenMode = VIDEO_GetPreferredMode(nullptr);
    framebuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(screenMode));
    console_init(framebuffer, 20, 20, screenMode->fbWidth, screenMode->xfbHeight,
                 screenMode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(screenMode);
    VIDEO_SetNextFramebuffer(framebuffer);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (screenMode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}

struct FileData
{
    void* data = nullptr;
    long size = 0;

    FileData() = default;
    FileData(const FileData&) = delete;
    FileData& operator=(const FileData&) = delete;

    FileData(FileData&& other) noexcept : data(other.data), size(other.size)
    {
        other.data = nullptr;
        other.size = 0;
    }

    FileData& operator=(FileData&& other) noexcept
    {
        if (this != &other)
        {
            if (data) free(data);
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }

    ~FileData()
    {
        if (data) free(data);
    }

    [[nodiscard]] bool ok() const { return data != nullptr && size > 0; }
};

FileData readFile(const char* path)
{
    FileData out;
    FILE* file = fopen(path, "rb");
    if (!file)
    {
        printf("Failed to open file \"%s\"!\n", path);
        return out;
    }

    fseek(file, 0, SEEK_END);
    out.size = ftell(file);
    fseek(file, 0, SEEK_SET);

    out.data = malloc(static_cast<size_t>(out.size));
    if (!out.data)
    {
        fclose(file);
        out.size = 0;

        return out;
    }

    const size_t read = fread(out.data, 1, static_cast<size_t>(out.size), file);
    fclose(file);

    if (read != static_cast<size_t>(out.size))
    {
        printf("Failed to read file \"%s\"!\n", path);
        free(out.data);
        out.data = nullptr;
        out.size = 0;
    }

    return out;
}

struct Bullet
{
    float x = 0, y = 0, width = 5, height = 10;
    bool active = false;
};

struct Enemy
{
    float x = 0, y = 0, width = ENEMY_WIDTH, height = ENEMY_HEIGHT;
    bool alive = true;
};

struct Player
{
    float x = 0, y = 0, width = 50, height = 30;
};

class Game
{
public:
    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;

    bool gameStarted = false;
    int score = 0, currentLevel = 1, enemyDirection = 1, enemyMoveCount = 0;

    Game()
    {
        bullets.resize(NUM_BULLETS);
        enemies.resize(ENEMY_ROWS * ENEMY_COLS);
        resetLevel();
    }

    void resetLevel()
    {
        player.x = (SCREEN_WIDTH - player.width) / 2.0f;
        player.y = SCREEN_HEIGHT - player.height - 20.0f;

        for (auto& b : bullets) b.active = false;
        int index = 0;

        for (int row = 0; row < ENEMY_ROWS; ++row)
            for (int col = 0; col < ENEMY_COLS; ++col)
            {
                auto& e = enemies[index++];

                e.x = 50.0f + static_cast<float>(col) * (ENEMY_WIDTH + ENEMY_MARGIN);
                e.y = 50.0f + static_cast<float>(row) * (ENEMY_HEIGHT + ENEMY_MARGIN);
                e.alive = true;
            }

        enemyDirection = 1;
        enemyMoveCount = 0;
    }

    void shoot()
    {
        for (auto& b : bullets)
            if (!b.active)
            {
                b.x = player.x + (player.width - b.width) / 2.0f;
                b.y = player.y - b.height;
                b.active = true;

                break;
            }
    }

    void moveBullets()
    {
        for (auto& b : bullets)
            if (b.active)
            {
                b.y -= BULLET_SPEED;
                if (b.y + b.height < 0) b.active = false;
            }
    }

    void moveEnemies()
    {
        bool edgeHit = false;
        for (auto& e : enemies)
            if (e.alive)
            {
                e.x += static_cast<float>(enemyDirection);
                if (e.x < 0 || e.x + e.width > SCREEN_WIDTH) edgeHit = true;
            }

        if (edgeHit)
        {
            enemyDirection *= -1;
            enemyMoveCount++;
        }

        if (enemyMoveCount == 5)
        {
            for (auto& e : enemies) if (e.alive) e.y += 10;
            enemyMoveCount = 0;
        }
    }

    void handleCollisions()
    {
        for (auto& b : bullets)
            if (b.active)
                for (auto& e : enemies)
                    if (e.alive && b.x < e.x + e.width && b.x + b.width > e.x && b.y < e.y + e.height &&
                        b.y + b.height > e.y)
                    {
                        b.active = false;
                        e.alive = false;
                        score += 10;

                        break;
                    }
    }

    [[nodiscard]] bool allEnemiesDefeated() const
    {
        return std::none_of(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.alive; });
    }
};

int main()
{
    initialize();

    const auto musicFile = readFile("sd:/apps/SpaceInvaders/music.mp3");
    const auto playerPng = readFile("sd:/apps/SpaceInvaders/player.png");
    const auto enemyPng = readFile("sd:/apps/SpaceInvaders/enemy.png");

    if (!musicFile.ok() || !playerPng.ok() || !enemyPng.ok())
    {
        GRRLIB_Exit();
        return EXIT_FAILURE;
    }

    GRRLIB_ttfFont* font = GRRLIB_LoadTTFFromFile("sd:/apps/SpaceInvaders/font.ttf");
    if (!font) printf("Failed to load font!\n");

    GRRLIB_texImg* player_img = GRRLIB_LoadTexturePNG(static_cast<u8*>(playerPng.data));
    GRRLIB_texImg* enemy_img = GRRLIB_LoadTexturePNG(static_cast<u8*>(enemyPng.data));
    if (!player_img || !enemy_img)
    {
        GRRLIB_FreeTTF(font);
        GRRLIB_Exit();

        return EXIT_FAILURE;
    }

    Game game;
    printf("Move: Left/Right\n");
    printf("Shoot: A\n");
    printf("Start/restart game: B\n");
    printf("Exit: Home\n");

    while (true)
    {
        WPAD_ScanPads();
        if (!MP3Player_IsPlaying()) MP3Player_PlayBuffer(musicFile.data, musicFile.size, nullptr);

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_LEFT && game.player.x > 0) game.player.x -= PLAYER_SPEED;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_RIGHT && game.player.x < SCREEN_WIDTH - game.player.width)
            game.player.x += PLAYER_SPEED;

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) game.shoot();
        if (!game.gameStarted && WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
        {
            game.gameStarted = true;
            game.score = 0;
            game.currentLevel = 1;
            game.resetLevel();
        }

        if (game.gameStarted)
        {
            game.moveBullets();
            game.moveEnemies();
            game.handleCollisions();

            if (game.allEnemiesDefeated())
            {
                game.currentLevel++;
                game.resetLevel();
            }
        }

        GRRLIB_FillScreen(0x000000FF);
        GRRLIB_DrawImg(game.player.x, game.player.y, player_img, 0, 1, 1, 0xFFFFFFFF);

        for (const auto& b : game.bullets)
            if (b.active) GRRLIB_Rectangle(b.x, b.y, b.width, b.height, 0xFFFFFFFF, true);
        for (const auto& e : game.enemies)
            if (e.alive) GRRLIB_DrawImg(e.x, e.y, enemy_img, 0, 1, 1, 0xFFFFFFFF);

        if (font)
        {
            char scoreText[32];
            sprintf(scoreText, "Score: %d", game.score);
            GRRLIB_PrintfTTF(10, 10, font, scoreText, 24, 0xFFFFFFFF);
            char levelText[32];
            sprintf(levelText, "Level: %d", game.currentLevel);
            GRRLIB_PrintfTTF(SCREEN_WIDTH - 150, 10, font, levelText, 24, 0xFFFFFFFF);
        }

        GRRLIB_Render();
    }

    MP3Player_Stop();
    GRRLIB_FreeTTF(font);
    GRRLIB_FreeTexture(player_img);
    GRRLIB_FreeTexture(enemy_img);
    GRRLIB_Exit();

    return EXIT_SUCCESS;
}
