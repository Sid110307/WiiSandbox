#include <stdio.h>
#include <stdlib.h>

#include <gccore.h>
#include <wiiuse/wpad.h>

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

void initialize()
{
    VIDEO_Init();
    WPAD_Init();

    rmode = VIDEO_GetPreferredMode(NULL);
    xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
    console_init(xfb, 20, 20, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE) VIDEO_WaitVSync();
}

int main()
{
    initialize();
    printf("\033[2;0HHello World!\n\n\n\n");

    while (1)
    {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_A) printf("HELLO");
        if (pressed & WPAD_BUTTON_B) printf("WORLD");
        if (pressed & WPAD_BUTTON_HOME) break;

        VIDEO_WaitVSync();
    }

    return EXIT_SUCCESS;
}
