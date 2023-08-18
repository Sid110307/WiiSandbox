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
#define BALL_SIZE 10
#define WIN_SCORE 10
#define PLAYER_SPEED 4
#define INITIAL_BALL_SPEED_X 3
#define INITIAL_BALL_SPEED_Y 2

static void *framebuffer = NULL;
static GXRModeObj *screenMode = NULL;

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

int main()
{
    initialize();

    FILE *file = fopen("sd:/apps/GraphicsTest/music.mp3", "rb");
    if (file == NULL)
    {
        printf("Failed to open music file!\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *buffer = malloc(size);
    fread(buffer, 1, size, file);
    fclose(file);

    file = fopen("sd:/apps/GraphicsTest/font.ttf", "rb");
    if (file == NULL)
    {
        printf("Failed to open font file!\n");
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void *fontBuffer = malloc(size);
    fread(fontBuffer, 1, size, file);
    fclose(file);

    f32 player1Y = SCREEN_HEIGHT / 2 - PLAYER_HEIGHT / 2, player2Y = SCREEN_HEIGHT / 2 - PLAYER_HEIGHT / 2;
    f32 ballX = SCREEN_WIDTH / 2 - BALL_SIZE / 2, ballY = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
    f32 ballSpeedX = 0, ballSpeedY = 0;
    u32 color = 0xFFFFFFFF;
    int gameStarted = 0, player1Score = 0, player2Score = 0;
    GRRLIB_ttfFont *font = GRRLIB_LoadTTF(fontBuffer, size);

    printf("Press A to start the game!\n");
    while (1)
    {
        WPAD_ScanPads();
        if (!MP3Player_IsPlaying()) MP3Player_PlayBuffer(buffer, size, NULL);

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) exit(0);
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_UP && player1Y > 0) player1Y -= PLAYER_SPEED;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_DOWN && player1Y < SCREEN_HEIGHT - PLAYER_HEIGHT)
            player1Y += PLAYER_SPEED;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_1 && player2Y > 0) player2Y -= PLAYER_SPEED;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_2 && player2Y < SCREEN_HEIGHT - PLAYER_HEIGHT) player2Y += PLAYER_SPEED;
        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B) color = rand() % 0xFFFFFFFF;
        if (!gameStarted && (WPAD_ButtonsDown(0) & WPAD_BUTTON_A))
        {
            gameStarted = 1;
            player1Score = 0;
            player2Score = 0;
            ballX = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
            ballY = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
            ballSpeedX = INITIAL_BALL_SPEED_X;
            ballSpeedY = INITIAL_BALL_SPEED_Y;
        }

        if (gameStarted)
        {
            if (ballY <= BALL_SIZE || ballY >= SCREEN_HEIGHT - BALL_SIZE) ballSpeedY = -ballSpeedY;
            if ((ballX <= PLAYER_WIDTH && ballY >= player1Y && ballY <= player1Y + PLAYER_HEIGHT) ||
                (ballX >= SCREEN_WIDTH - PLAYER_WIDTH - BALL_SIZE && ballY >= player2Y &&
                 ballY <= player2Y + PLAYER_HEIGHT))
                ballSpeedX = -ballSpeedX;

            if (ballX <= 0)
            {
                player2Score++;
                ballX = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
                ballY = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
                ballSpeedX = INITIAL_BALL_SPEED_X;
                ballSpeedY = INITIAL_BALL_SPEED_Y;
            }
            if (ballX >= SCREEN_WIDTH - BALL_SIZE)
            {
                player1Score++;
                ballX = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
                ballY = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
                ballSpeedX = INITIAL_BALL_SPEED_X;
                ballSpeedY = INITIAL_BALL_SPEED_Y;
            }

            if (player1Score == WIN_SCORE || player2Score == WIN_SCORE)
            {
                gameStarted = 0;
                player1Score = 0;
                player2Score = 0;
                ballX = SCREEN_WIDTH / 2 - BALL_SIZE / 2;
                ballY = SCREEN_HEIGHT / 2 - BALL_SIZE / 2;
                ballSpeedX = 0;
                ballSpeedY = 0;

                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 8, SCREEN_HEIGHT / 2 - 8, font, "GAME OVER", 1, color);
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 8, SCREEN_HEIGHT / 2 + 8, font,
                                 player1Score == WIN_SCORE ? "P1 WINS!" : "P2 WINS!", 1, color);
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 8, SCREEN_HEIGHT / 2 + 24, font, "Press A to play again", 1, color);
            }

            ballX += ballSpeedX;
            ballY += ballSpeedY;
        }

        GRRLIB_Circle(ballX, ballY, BALL_SIZE, color, 1);
        GRRLIB_Rectangle(0, player1Y, PLAYER_WIDTH, PLAYER_HEIGHT, color, 1);
        GRRLIB_Rectangle(SCREEN_WIDTH - PLAYER_WIDTH, player2Y, PLAYER_WIDTH, PLAYER_HEIGHT, color, 1);

        char p1Score[16], p2Score[16];
        sprintf(p1Score, "P1: %d", player1Score);
        sprintf(p2Score, "P2: %d", player2Score);

        GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 8, 0, font, p1Score, 1, color);
        GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 + 8, 0, font, p2Score, 1, color);

        GRRLIB_Render();
    }

    free(buffer);
    free(fontBuffer);
    GRRLIB_Exit();

    return EXIT_SUCCESS;
}
