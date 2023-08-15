#include <iostream>
#include <random>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <grrlib.h>

enum Colors : u32
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
}

int main()
{
    initialize();
    u32 color = Colors::WHITE;

    while (true)
    {
        WPAD_ScanPads();
        u32 pressed = WPAD_ButtonsDown(0);

        if (pressed & WPAD_BUTTON_HOME) break;
        if (pressed & WPAD_BUTTON_A) color = Colors::WHITE;
        if (pressed & WPAD_BUTTON_B)
        {
            std::random_device rd;
            std::mt19937 gen(rd());

            color = std::uniform_int_distribution<u32>(0, 0xFFFFFFFF)(gen);
        }

        GRRLIB_3dMode(0.01f, 1000.0f, 45.0f, true, true);
        GRRLIB_Camera3dSettings(0.0f, 0.0f, 5.0f, 0.0f, 1.0f, 0.0f, 0, 0, 0);

        GRRLIB_ObjectViewBegin();
        GRRLIB_DrawTorus(0.5f, 1.0f, 10, 10, true, color);
        GRRLIB_ObjectViewTrans(1.5f, 0.0f, 0.0f);
        GRRLIB_DrawSphere(1.0f, 10, 10, true, color);
        GRRLIB_ObjectViewTrans(0.0f, 1.5f, 0.0f);
        GRRLIB_DrawCube(1.0f, true, color);
        GRRLIB_ObjectViewTrans(-1.5f, 0.0f, 0.0f);
        GRRLIB_DrawCylinder(1.0f, 1.0f, 10, true, color);
        GRRLIB_ObjectViewTrans(0.75f, -0.75f, 0.0f);
        GRRLIB_DrawCone(1.0f, 1.0f, 10, true, color);
        GRRLIB_ObjectViewEnd();

        GRRLIB_Render();
    }

    GRRLIB_Exit();
    return EXIT_SUCCESS;
}
