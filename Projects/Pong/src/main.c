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
    fread(buffer, 1, *size, file);
    fclose(file);

    return buffer;
}

int main()
{
    initialize();

    long size;
    void* fontBuffer = readFile("sd:/apps/Pong/font.ttf", &size);
    void* musicBuffer = readFile("sd:/apps/Pong/music.mp3", &size);

    f32 player1Y = (SCREEN_HEIGHT - PLAYER_HEIGHT) / 2.0f, player2Y = (SCREEN_HEIGHT - PLAYER_HEIGHT) / 2.0f;
    f32 ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f, ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
    f32 ballSpeedX = 0, ballSpeedY = 0;
    u32 color = 0xFFFFFFFF;
    int gameStarted = 0, player1Score = 0, player2Score = 0;
    GRRLIB_ttfFont* font = GRRLIB_LoadTTF(fontBuffer, size);

    printf("P1 Controls: Up/Down\n");
    printf("P2 Controls: 1/2\n");
    printf("Randomize color: B\n");
    printf("Start/restart game: A\n");
    printf("Exit: Home\n");

    while (1)
    {
        WPAD_ScanPads();
        if (!MP3Player_IsPlaying()) MP3Player_PlayBuffer(musicBuffer, size, NULL);

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) break;
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

            ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
            ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
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

                ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
                ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
                ballSpeedX = INITIAL_BALL_SPEED_X;
                ballSpeedY = INITIAL_BALL_SPEED_Y;
            }
            if (ballX >= SCREEN_WIDTH - BALL_SIZE)
            {
                player1Score++;

                ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
                ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
                ballSpeedX = INITIAL_BALL_SPEED_X;
                ballSpeedY = INITIAL_BALL_SPEED_Y;
            }

            if (player1Score == WIN_SCORE || player2Score == WIN_SCORE)
            {
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 8, SCREEN_HEIGHT / 2 - 8, font, "GAME OVER", 1, color);
                GRRLIB_PrintfTTF(SCREEN_WIDTH / 2 - 8, SCREEN_HEIGHT / 2 + 8, font,
                                 player1Score == WIN_SCORE ? "P1 WINS!" : "P2 WINS!", 1, color);

                gameStarted = 0;
                player1Score = 0;
                player2Score = 0;

                ballX = (SCREEN_WIDTH - BALL_SIZE) / 2.0f;
                ballY = (SCREEN_HEIGHT - BALL_SIZE) / 2.0f;
                ballSpeedX = 0;
                ballSpeedY = 0;
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

    free(fontBuffer);
    free(musicBuffer);

    MP3Player_Stop();
    GRRLIB_FreeTTF(font);
    GRRLIB_Exit();

    return EXIT_SUCCESS;
}
