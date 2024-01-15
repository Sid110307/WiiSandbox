#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>
#include <asndlib.h>
#include <mp3player.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define PLAYER_SPEED 5
#define BULLET_SPEED 8
#define NUM_BULLETS 5
#define ENEMY_ROWS 5
#define ENEMY_COLS 11
#define ENEMY_WIDTH 40
#define ENEMY_HEIGHT 30
#define ENEMY_MARGIN 10

static void* framebuffer = NULL;
static GXRModeObj* screenMode = NULL;
static int currentLevel = 1, numEnemies = ENEMY_ROWS * ENEMY_COLS;

void initialize()
{
    srand(time(NULL));

    VIDEO_Init();
    GRRLIB_Init();
    WPAD_Init();
    ASND_Init();
    MP3Player_Init();

    screenMode = VIDEO_GetPreferredMode(NULL);
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

void* readFile(const char* path, long* size)
{
    FILE* file = fopen(path, "rb");
    if (file == NULL)
    {
        printf("Failed to open file \"%s\"!\n", path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void* buffer = malloc(*size);
    fread(buffer, 1, *size, file);
    fclose(file);

    return buffer;
}

typedef struct
{
    f32 x, y, width, height;
    int isActive;
} Bullet;

typedef struct
{
    f32 x, y, width, height;
    int isAlive;
} Enemy;

typedef struct
{
    f32 x, y, width, height;
} Player;

void initializeEntities(Player* player, Bullet* bullets, Enemy* enemies)
{
    player->width = 50;
    player->height = 30;
    player->x = (SCREEN_WIDTH - player->width) / 2;
    player->y = SCREEN_HEIGHT - player->height - 20;

    for (int i = 0; i < NUM_BULLETS; ++i)
    {
        bullets[i].width = 5;
        bullets[i].height = 10;
        bullets[i].isActive = 0;
    }

    int startX = 50, startY = 50;
    for (int row = 0; row < ENEMY_ROWS; ++row)
        for (int col = 0; col < ENEMY_COLS; ++col)
        {
            int index = row * ENEMY_COLS + col;
            enemies[index].width = ENEMY_WIDTH;
            enemies[index].height = ENEMY_HEIGHT;
            enemies[index].x = (f32) (startX + col * (ENEMY_WIDTH + ENEMY_MARGIN));
            enemies[index].y = (f32) (startY + row * (ENEMY_HEIGHT + ENEMY_MARGIN));
            enemies[index].isAlive = 1;
        }

    int extraEnemies = currentLevel - 1;
    numEnemies += extraEnemies;

    for (int i = 0; i < extraEnemies; ++i)
    {
        int randomRow = rand() % ENEMY_ROWS;
        int randomCol = rand() % ENEMY_COLS;
        int index = randomRow * ENEMY_COLS + randomCol;

        enemies[index].x = (f32) (startX + randomCol * (ENEMY_WIDTH + ENEMY_MARGIN));
        enemies[index].y = (f32) (startY + randomRow * (ENEMY_HEIGHT + ENEMY_MARGIN));
        enemies[index].isAlive = 1;
    }
}

void shootBullet(Bullet* bullets, Player* player)
{
    for (int i = 0; i < NUM_BULLETS; ++i)
        if (!bullets[i].isActive)
        {
            bullets[i].x = player->x + (player->width - bullets[i].width) / 2;
            bullets[i].y = player->y - bullets[i].height;
            bullets[i].isActive = 1;

            break;
        }
}

void moveBullets(Bullet* bullets)
{
    for (int i = 0; i < NUM_BULLETS; ++i)
        if (bullets[i].isActive)
        {
            bullets[i].y -= BULLET_SPEED;
            if (bullets[i].y < 0) bullets[i].isActive = 0;
        }
}

void moveEnemies(Enemy* enemies)
{
    static int direction = 1, moveCount = 0;

    for (int i = 0; i < ENEMY_ROWS * ENEMY_COLS; ++i)
        if (enemies[i].isAlive)
        {
            enemies[i].x += (f32) direction;
            if (enemies[i].x < 0 || enemies[i].x + enemies[i].width > SCREEN_WIDTH)
            {
                direction *= -1;
                moveCount++;
            }
        }

    if (moveCount == 5)
    {
        for (int i = 0; i < ENEMY_ROWS * ENEMY_COLS; ++i) if (enemies[i].isAlive) enemies[i].y += 10;
        moveCount = 0;
    }
}

void handleCollisions(Bullet* bullets, Enemy* enemies, int* score)
{
    for (int i = 0; i < NUM_BULLETS; ++i)
        if (bullets[i].isActive)
            for (int j = 0; j < ENEMY_ROWS * ENEMY_COLS; ++j)
                if (enemies[j].isAlive && bullets[i].x < enemies[j].x + enemies[j].width &&
                    bullets[i].x + bullets[i].width > enemies[j].x && bullets[i].y < enemies[j].y + enemies[j].height &&
                    bullets[i].y + bullets[i].height > enemies[j].y)
                {
                    bullets[i].isActive = 0;
                    enemies[j].isAlive = 0;
                    *score += 10;
                }
}

void drawEntities(Player* player, Bullet* bullets, Enemy* enemies, int score, GRRLIB_ttfFont* font,
                  GRRLIB_texImg* player_img, GRRLIB_texImg* enemy_img)
{
    GRRLIB_DrawImg(player->x, player->y, player_img, 0, 1, 1, 0xFFFFFFFF);

    for (int i = 0; i < NUM_BULLETS; ++i)
        if (bullets[i].isActive)
            GRRLIB_Rectangle(bullets[i].x, bullets[i].y, bullets[i].width, bullets[i].height, 0xFFFFFFFF, 1);

    for (int i = 0; i < ENEMY_ROWS * ENEMY_COLS; ++i)
        if (enemies[i].isAlive) GRRLIB_DrawImg(enemies[i].x, enemies[i].y, enemy_img, 0, 1, 1, 0xFFFFFFFF);

    char scoreText[20];
    sprintf(scoreText, "Score: %d", score);
    GRRLIB_PrintfTTF(10, 10, font, scoreText, 24, 0xFFFFFFFF);

    char levelText[20];
    sprintf(levelText, "Level: %d", currentLevel);
    GRRLIB_PrintfTTF(SCREEN_WIDTH - 150, 10, font, levelText, 24, 0xFFFFFFFF);

    char enemiesText[20];
    sprintf(enemiesText, "Enemies left: %d", numEnemies);
    GRRLIB_PrintfTTF(SCREEN_WIDTH - 150, 40, font, enemiesText, 24, 0xFFFFFFFF);
}

int main()
{
    initialize();

    long size;
    void* fontBuffer = readFile("sd:/apps/Pong/font.ttf", &size);
    void* musicBuffer = readFile("sd:/apps/Pong/music.mp3", &size);

    GRRLIB_ttfFont* font = GRRLIB_LoadTTF(fontBuffer, size);

    void* player_png = readFile("sd:/apps/SpaceInvaders/player.png", &size);
    void* enemy_png = readFile("sd:/apps/SpaceInvaders/enemy.png", &size);

    GRRLIB_texImg* player_img = GRRLIB_LoadTexture(player_png);
    GRRLIB_texImg* enemy_img = GRRLIB_LoadTexture(enemy_png);

    Player player;
    Bullet bullets[NUM_BULLETS];
    Enemy enemies[ENEMY_ROWS * ENEMY_COLS];

    printf("Move: Left/Right\n");
    printf("Shoot: A\n");
    printf("Start/restart game: B\n");
    printf("Exit: Home\n");

    int score = 0, gameStarted = 0;
    while (1)
    {
        WPAD_ScanPads();
        if (!MP3Player_IsPlaying()) MP3Player_PlayBuffer(musicBuffer, size, NULL);

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_LEFT && player.x > 0) player.x -= PLAYER_SPEED;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_RIGHT && player.x < SCREEN_WIDTH - player.width) player.x += PLAYER_SPEED;
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A) shootBullet(bullets, &player);
        if (!gameStarted && (WPAD_ButtonsDown(0) & WPAD_BUTTON_B))
        {
            gameStarted = 1;
            currentLevel = 1;
            score = 0;
            numEnemies = ENEMY_ROWS * ENEMY_COLS;

            initializeEntities(&player, bullets, enemies);
        }

        if (gameStarted)
        {
            moveBullets(bullets);
            moveEnemies(enemies);
            handleCollisions(bullets, enemies, &score);
            drawEntities(&player, bullets, enemies, score, font, player_img, enemy_img);

            int allEnemiesDefeated = 1;
            for (int i = 0; i < ENEMY_ROWS * ENEMY_COLS; ++i)
                if (enemies[i].isAlive)
                {
                    allEnemiesDefeated = 0;
                    break;
                }

            if (allEnemiesDefeated)
            {
                currentLevel++;
                initializeEntities(&player, bullets, enemies);
            }
        }

        GRRLIB_Render();
    }

    free(fontBuffer);
    free(musicBuffer);
    free(player_png);
    free(enemy_png);

    MP3Player_Stop();
    GRRLIB_FreeTTF(font);
    GRRLIB_FreeTexture(player_img);
    GRRLIB_FreeTexture(enemy_img);
    GRRLIB_Exit();

    return EXIT_SUCCESS;
}
