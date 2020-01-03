#include <fstream>
#include <cstdio>
#include "main.h"

using namespace std;

PPU ppu;
MemoryController memory;
CPU_6502 cpu(memory);


MemoryController& getCpuRam()
{
    return memory;
}


char pattern_table[8192] = {1};


SDL_Surface* screen = NULL;
Uint32 paleta[64];

uint8_t paleta_nes[64][3] =
{
    {124, 124, 124},
    {0, 0, 252},
    {0, 0, 188},
    {68, 40, 188},
    {148, 0, 132},
    {168, 0, 32},
    {168, 16, 0},
    {136, 20, 0},
    {80, 48, 0},
    {0, 120, 0},
    {0, 104, 0},
    {0, 88, 0},
    {0, 64, 88},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {188, 188, 188},
    {0, 120, 248},
    {0, 88, 248},
    {104, 68, 252},
    {216, 0, 204},
    {228, 0, 88},
    {248, 56, 0},
    {228, 92, 16},
    {172, 124, 0},
    {0, 184, 0},
    {0, 168, 0},
    {0, 168, 68},
    {0, 136, 136},
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
    {248, 248, 248},
    {60, 188, 252},
    {104, 136, 252},
    {152, 120, 248},
    {248, 120, 248},
    {248, 88, 152},
    {248, 120, 88},
    {252, 160, 68},
    {248, 184, 0},
    {184, 248, 24},
    {88, 216, 84},
    {88, 248, 152},
    {0, 232, 216},
    {120, 120, 120},
    {0, 0, 0},
    {0, 0, 0},
    {252, 252, 252},
    {164, 228, 252},
    {184, 184, 248},
    {216, 184, 248},
    {248, 184, 248},
    {248, 164, 192},
    {240, 208, 176},
    {252, 224, 168},
    {248, 216, 120},
    {216, 248, 120},
    {184, 248, 184},
    {184, 248, 216},
    {0, 252, 252},
    {248, 216, 248},
    {0, 0, 0},
    {0, 0, 0},
};
void inicia_sdl()
{

    SDL_Init(SDL_INIT_VIDEO);
    //Set up screen
    screen = SDL_SetVideoMode(256, 240, 32, SDL_SWSURFACE);


    for(int i=0; i < 64; i++)
    {
        paleta[i] = SDL_MapRGB(screen->format, paleta_nes[i][0], paleta_nes[i][1], paleta_nes[i][2]);
    }

}

void desenha_framebuffer()
{
    for(int i=0; i < 240; i++)
        for(int k=0; k < 256; k++)
        {
            SDL_Rect rect;
            rect.w = rect.h = 1;
            rect.x = k;
            rect.y = i;

            int a3 = ppu.framebuffer[i][k];
            ppu.framebuffer[i][k] = 0; //vai zerando logo
            SDL_FillRect(screen, &rect, paleta[a3]);
        }

    //Update screen
    SDL_Flip(screen);
}


void dump_data(uint8_t* data, int n, FILE* saida=stdout)
{
    for(int i=0; i < n; i++)
    {
        printf("%02X ", data[i]);
    }
}

// define todos os registradores que vão controlar a PPU, APU, CHR rom etc
void configura_memoria()
{
    //registradores da ppu
    memory.registerReadCallback(0x2002, ppu_register_read);
    memory.registerReadCallback(0x2004, ppu_register_read);
    memory.registerReadCallback(0x2007, ppu_register_read);

    memory.registerWriteCallback(0x2000, ppu_register_write);
    memory.registerWriteCallback(0x2001, ppu_register_write);
    memory.registerWriteCallback(0x2003, ppu_register_write);
    memory.registerWriteCallback(0x2004, ppu_register_write);
    memory.registerWriteCallback(0x2005, ppu_register_write);
    memory.registerWriteCallback(0x2006, ppu_register_write);
    memory.registerWriteCallback(0x2007, ppu_register_write);
    memory.registerWriteCallback(0x4014, ppu_register_write);

    //registradores do controle
    memory.registerReadCallback(0x4016, gamepad_register_read);
    memory.registerWriteCallback(0x4016, gamepad_register_write);
}


uint32_t ButtonConfig[] = {SDLK_a, SDLK_z, SDLK_BACKSPACE, SDLK_SPACE, SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT};

void emulador_main_loop()
{
    ines_rom_reader teste3("nestest.nes");
    Uint8 *keystates = SDL_GetKeyState( NULL );

    if(!teste3.m_open)
    {
        Logger::out("MAIN") << "Rom nao encontrada.";
        return;
    }
    if(teste3.m_mapper != 0)
    {
        Logger::out("MAIN") << "Mapper não suportado.";
        return;
    }
    configura_memoria();
    Mapper_base nmap(teste3, memory, ppu);


    inicia_sdl();
    cpu.start();
    Logger::silence("PPU");
    Logger::silence("CPU");

    SDL_Event event;

    int k=0;
    while(1)
    {
        while(SDL_PollEvent( &event ))
        {
            switch( event.type )
            {
            case SDL_QUIT:
                return;
            default:
                break;
            }
        }
        for(int i=0; i < 8; i++)
                Gamepad::setButtonState(i, keystates[ButtonConfig[i]]);

        cpu._executar_instrucoes(27007); //instrucoes fora da vblank

        if(cpu.crash)
        {
            printf("\ncrashou\n");

            break;
        }

        if(ppu.VerticalBlank())
        {
            cpu.NMI();
            while(cpu.dentro_da_nmi && !cpu.crash)
                cpu._executar_instrucoes(1);
        }

        ppu.Draw();
        desenha_framebuffer();
        //SDL_Delay(16);

        continue;
    }



    getchar();

}

int main(int argc, char* argv[])
{
    //cpu_test();
    emulador_main_loop();

    //teste_pattern_table();
    return 0;
}
