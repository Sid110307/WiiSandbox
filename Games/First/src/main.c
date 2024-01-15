#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <gccore.h>
#include <wiiuse/wpad.h>

static void* framebuffer = NULL;
static GXRModeObj* screenMode = NULL;

void initialize()
{
    srand(time(NULL));

    VIDEO_Init();
    WPAD_Init();

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
    printf("\033[2;0HHello World!\n\nPress A to print HELLO\nPress B to print WORLD\nPress 1 for magic\nPress HOME to exit\n\n");

    while (1)
    {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_A) printf("HELLO");
        if (pressed & WPAD_BUTTON_B) printf("WORLD");
        if (pressed & WPAD_BUTTON_1)
            for (int i = 0; i < screenMode->fbWidth * screenMode->xfbHeight; ++i) ((u32*) framebuffer)[i] = rand();
        if (pressed & WPAD_BUTTON_HOME) break;

        VIDEO_WaitVSync();
    }

    return EXIT_SUCCESS;
}
