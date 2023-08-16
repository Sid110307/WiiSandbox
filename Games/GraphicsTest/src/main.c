#include <stdio.h>
#include <stdlib.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>

enum Colors
{
    BLACK = 0x000000FF,
    MAROON = 0x800000FF,
    GREEN = 0x008000FF,
    OLIVE = 0x808000FF,
    NAVY = 0x000080FF,
    PURPLE = 0x800080FF,
    TEAL = 0x008080FF,
    GRAY = 0x808080FF,
    SILVER = 0xC0C0C0FF,
    RED = 0xFF0000FF,
    LIME = 0x00FF00FF,
    YELLOW = 0xFFFF00FF,
    BLUE = 0x0000FFFF,
    FUCHSIA = 0xFF00FFFF,
    AQUA = 0x00FFFFFF,
    WHITE = 0xFFFFFFFF
};

void initialize()
{
    GRRLIB_Init();
    WPAD_Init();

    GRRLIB_Camera3dSettings(0, 0, 0, 0, 1, 0, 0, 0, -1);
}

int main()
{
    const u32 colors[3] = {RED, GREEN, BLUE};
    int z = 0;

    initialize();

    while (1)
    {
        WPAD_ScanPads();

        if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) exit(0);
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_A) z += 1;
        if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_B) z -= 1;

        GRRLIB_3dMode(0.1, 1000, 45, 0, 0);
        GRRLIB_ObjectView(0, 0, z, 0, 0, 0, 1, 1, 1);

        GX_Begin(GX_QUADS, GX_VTXFMT0, 24);
        GX_Position3f32(-1.0f, 1.0f, -1.0f);
        GX_Color1u32(colors[0]);
        GX_Position3f32(-1.0f, -1.0f, -1.0f);
        GX_Color1u32(colors[0]);
        GX_Position3f32(1.0f, -1.0f, -1.0f);
        GX_Color1u32(colors[0]);
        GX_Position3f32(1.0f, 1.0f, -1.0f);
        GX_Color1u32(colors[0]);

        GX_Position3f32(-1.0f, 1.0f, 1.0f);
        GX_Color1u32(colors[0]);
        GX_Position3f32(-1.0f, -1.0f, 1.0f);
        GX_Color1u32(colors[0]);
        GX_Position3f32(1.0f, -1.0f, 1.0f);
        GX_Color1u32(colors[0]);
        GX_Position3f32(1.0f, 1.0f, 1.0f);
        GX_Color1u32(colors[0]);

        GX_Position3f32(-1.0f, 1.0f, 1.0f);
        GX_Color1u32(colors[1]);
        GX_Position3f32(1.0f, 1.0f, 1.0f);
        GX_Color1u32(colors[1]);
        GX_Position3f32(1.0f, 1.0f, -1.0f);
        GX_Color1u32(colors[1]);
        GX_Position3f32(-1.0f, 1.0f, -1.0f);
        GX_Color1u32(colors[1]);

        GX_Position3f32(-1.0f, -1.0f, 1.0f);
        GX_Color1u32(colors[1]);
        GX_Position3f32(1.0f, -1.0f, 1.0f);
        GX_Color1u32(colors[1]);
        GX_Position3f32(1.0f, -1.0f, -1.0f);
        GX_Color1u32(colors[1]);
        GX_Position3f32(-1.0f, -1.0f, -1.0f);
        GX_Color1u32(colors[1]);

        GX_Position3f32(-1.0f, 1.0f, 1.0f);
        GX_Color1u32(colors[2]);
        GX_Position3f32(-1.0f, 1.0f, -1.0f);
        GX_Color1u32(colors[2]);
        GX_Position3f32(-1.0f, -1.0f, -1.0f);
        GX_Color1u32(colors[2]);
        GX_Position3f32(-1.0f, -1.0f, 1.0f);
        GX_Color1u32(colors[2]);

        GX_Position3f32(1.0f, 1.0f, 1.0f);
        GX_Color1u32(colors[2]);
        GX_Position3f32(1.0f, 1.0f, -1.0f);
        GX_Color1u32(colors[2]);
        GX_Position3f32(1.0f, -1.0f, -1.0f);
        GX_Color1u32(colors[2]);
        GX_Position3f32(1.0f, -1.0f, 1.0f);
        GX_Color1u32(colors[2]);
        GX_End();

        GRRLIB_Render();
    }

    GRRLIB_Exit();
    return EXIT_SUCCESS;
}
