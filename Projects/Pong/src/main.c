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
#define PLAYER_WIDTH 10
#define PLAYER_HEIGHT 60

#define FONT_SIZE 24
#define BALL_SIZE 10
#define WIN_SCORE 10
#define PLAYER_SPEED 4

#define INITIAL_BALL_SPEED_X 3
#define INITIAL_BALL_SPEED_Y 2

static void* framebuffer = NULL;
static GXRModeObj* screenMode = NULL;

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
    const size_t read = fread(buffer, 1, *size, file);
    fclose(file);

    if (read != (size_t)*size)
    {
        printf("Failed to read file \"%s\"!\n", path);
        free(buffer);

        return NULL;
    }

    return buffer;
}

int main()
{
    initialize();

    long musicSize = 0, fontSize = 0;
    void* musicBuffer = readFile("sd:/apps/Pong/music.mp3", &musicSize);
    void* fontBuffer = readFile("sd:/apps/Pong/font.ttf", &fontSize);
    if (!musicBuffer || !fontBuffer)
    {
        GRRLIB_Exit();
        return EXIT_FAILURE;
    }

    GRRLIB_ttfFont* font = GRRLIB_LoadTTF(fontBuffer, fontSize);
    if (!font) printf("Failed to load font!\n");

    int gameStarted = 0, gameOver = 0, winner = 0, player1Score = 0, player2Score = 0;
    f32 player1Y = (SCREEN_HEIGHT - PLAYER_HEIGHT) / 2.0f, player2Y = (SCREEN_HEIGHT - PLAYER_HEIGHT) / 2.0f;
    f32 ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f, ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
    f32 ballSpeedX = 0, ballSpeedY = 0;
    u32 color = 0xFFFFFFFF;

    printf("P1 Controls: Up/Down\n");
    printf("P2 Controls: 1/2\n");
    printf("Randomize color: B\n");
    printf("Start/restart game: A\n");
    printf("Exit: Home\n");

    while (1)
    {
        WPAD_ScanPads();
        if (!MP3Player_IsPlaying()) MP3Player_PlayBuffer(musicBuffer, musicSize, NULL);

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_UP && player1Y > 0) player1Y -= PLAYER_SPEED;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_DOWN && player1Y < SCREEN_HEIGHT - PLAYER_HEIGHT)
            player1Y += PLAYER_SPEED;

        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_1 && player2Y > 0) player2Y -= PLAYER_SPEED;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_2 && player2Y < SCREEN_HEIGHT - PLAYER_HEIGHT)
            player2Y += PLAYER_SPEED;

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B)
        {
            const u32 r = rand() & 0xFF;
            const u32 g = rand() & 0xFF;
            const u32 b = rand() & 0xFF;
            color = r << 24 | g << 16 | b << 8 | 0xFF;
        }

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_A)
        {
            if (gameOver)
            {
                gameOver = 0;
                winner = 0;
                player1Score = 0;
                player2Score = 0;
            }

            if (!gameStarted)
            {
                gameStarted = 1;

                ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
                ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
                ballSpeedX = rand() & 1 ? INITIAL_BALL_SPEED_X : -INITIAL_BALL_SPEED_X;
                ballSpeedY = rand() & 1 ? INITIAL_BALL_SPEED_Y : -INITIAL_BALL_SPEED_Y;
            }
        }

        if (gameStarted && !gameOver)
        {
            ballX += ballSpeedX;
            ballY += ballSpeedY;

            if (ballY <= 0)
            {
                ballY = 0;
                ballSpeedY = -ballSpeedY;
            }
            else if (ballY >= SCREEN_HEIGHT - BALL_SIZE)
            {
                ballY = SCREEN_HEIGHT - BALL_SIZE;
                ballSpeedY = -ballSpeedY;
            }

            const f32 p1X = 0;
            const f32 p2X = SCREEN_WIDTH - PLAYER_WIDTH;
            const int overlapP1Y = ballY + BALL_SIZE >= player1Y && ballY <= player1Y + PLAYER_HEIGHT;
            const int hitP1X = ballX <= p1X + PLAYER_WIDTH && ballX + BALL_SIZE >= p1X;

            if (hitP1X && overlapP1Y && ballSpeedX < 0)
            {
                ballX = p1X + PLAYER_WIDTH;
                ballSpeedX = -ballSpeedX;
            }

            const int overlapP2Y = ballY + BALL_SIZE >= player2Y && ballY <= player2Y + PLAYER_HEIGHT;
            const int hitP2X = ballX + BALL_SIZE >= p2X && ballX <= p2X + PLAYER_WIDTH;

            if (hitP2X && overlapP2Y && ballSpeedX > 0)
            {
                ballX = p2X - BALL_SIZE;
                ballSpeedX = -ballSpeedX;
            }

            if (ballX + BALL_SIZE < 0)
            {
                player2Score++;
                gameStarted = 0;

                ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
                ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
                ballSpeedX = ballSpeedY = 0;
            }
            else if (ballX > SCREEN_WIDTH)
            {
                player1Score++;
                gameStarted = 0;

                ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
                ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
                ballSpeedX = ballSpeedY = 0;
            }

            if (player1Score >= WIN_SCORE || player2Score >= WIN_SCORE)
            {
                gameOver = 1;
                gameStarted = 0;
                winner = player1Score >= WIN_SCORE ? 1 : 2;

                ballSpeedX = ballSpeedY = 0;
                ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
                ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
            }
        }

        GRRLIB_FillScreen(0x000000FF);
        GRRLIB_Rectangle(ballX, ballY, BALL_SIZE, BALL_SIZE, color, 1);
        GRRLIB_Rectangle(0, player1Y, PLAYER_WIDTH, PLAYER_HEIGHT, color, 1);
        GRRLIB_Rectangle(SCREEN_WIDTH - PLAYER_WIDTH, player2Y, PLAYER_WIDTH, PLAYER_HEIGHT, color, 1);

        if (font)
        {
            char p1ScoreStr[16], p2ScoreStr[16];
            sprintf(p1ScoreStr, "P1: %d", player1Score);
            sprintf(p2ScoreStr, "P2: %d", player2Score);

            GRRLIB_PrintfTTF(20, 20, font, p1ScoreStr, FONT_SIZE, color);
            GRRLIB_PrintfTTF(SCREEN_WIDTH - 120, 20, font, p2ScoreStr, FONT_SIZE, color);

            if (gameOver)
            {
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 20, font, "GAME OVER", FONT_SIZE, color);
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 0, font,
                                 winner == 1 ? "P1 WINS!" : "P2 WINS!", FONT_SIZE, color);
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT / 2 + 20, font, "Press A to restart", FONT_SIZE,
                                 color);
            }
            else if (!gameStarted)
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 120, SCREEN_HEIGHT - 40, font, "Press A to start", FONT_SIZE,
                                 color);
        }

        GRRLIB_Render();
    }

    free(musicBuffer);
    MP3Player_Stop();
    GRRLIB_FreeTTF(font);
    GRRLIB_Exit();

    return EXIT_SUCCESS;
}
