#include "ppu.h"
#include <cstring>
#include <cstdio>
#include "Logger.h"
#include <cassert>

#include "main.h"

PPU* __ppu_atual = nullptr;

PPU::PPU()
{
    assert(__ppu_atual == nullptr);
    __ppu_atual = this;
    m_ppuctrl = 0;
    proximo_byte_no_ptr = 0;
    ponteiro = 0;
    vertical_blank = false;
    sprite_0_hit = true;
    sprite_overflow = false;
    proximo_scroll = 0;

}

PPU::~PPU()
{
    //dtor
}




void PPU::NovaPatternTable(uint8_t* dados, int local)
{
    memcpy(&memoria[0x1000*local], dados, 4096);
}


void PPU::ppucrtl(const uint8_t new_flags)
{
    Logger::out("PPU") << "ppuctrl (write): " << std::hex << (int)new_flags;

    m_ppuctrl = new_flags;
}


void PPU::ppumask(const uint8_t mask)
{
    m_ppumask = mask;
}

uint8_t PPU::ppustatus()
{
    uint8_t retorno=0;

    if(vertical_blank) retorno |= 0x80;
    if(sprite_0_hit) retorno |= 0x40;
    if(sprite_overflow) retorno |= 0x20;

    vertical_blank = false;
    return retorno;
}

void PPU::SetOamAddr(const uint8_t addr)
{
    oam_addr = addr;
}

uint8_t PPU::ReadOamData()
{
    return OAM[oam_addr];
}

void PPU::WriteOamData(const uint8_t byte)
{
    OAM[oam_addr++] = byte;
}

void PPU::ppuscroll(const uint8_t byte)
{
    Logger::out("PPU") << "PPUSCROLL: " << std::hex << (int) byte;
    if(proximo_scroll == 1)
    {
        scroll_y = byte;
        proximo_scroll = 0;
    }
    else
    {
        scroll_x = byte;
        proximo_scroll = 1;
    }
}

void PPU::ppuaddr(const uint8_t byte)
{
    Logger::out("PPU") << "PPUADDR: " << std::hex << (int)byte;


    uint16_t tmp = byte;
    if(proximo_byte_no_ptr == 1)
    {
        ponteiro &= ~0xff;
        ponteiro |= byte;
        proximo_byte_no_ptr = 0;
    }
    else
    {
        ponteiro &= ~0xff00;
        ponteiro |= (tmp<<8);

        proximo_byte_no_ptr = 1;
    }
    //printf("ptr_atual = %04X\n", ponteiro);
}

void PPU::ppudata_write(const uint8_t byte)
{
    //Logger::out("PPU") << "ppudata_write: " << std::hex << (int) byte;

    memoria[ponteiro] = byte;
    if(m_ppuctrl & 4) ponteiro += 32;
    else ponteiro += 1;
}

uint8_t PPU::ppudata_read()
{
    uint8_t retorno = memoria[ponteiro];
    if(m_ppuctrl & 4) ponteiro += 32;
    else ponteiro += 1;
    return retorno;
}

void PPU::oamdma_write(const uint8_t* v)
{
    memcpy(OAM, v, 256);
}
#include <SDL/SDL.h>


void PPU::drawTile(uint8_t x, uint8_t y, uint8_t paleta, uint8_t pattern_t, uint8_t tile, uint8_t flipx, uint8_t flipy)
{
    for(int i=0; i < 8; i++)
    {
        uint8_t byte1 = memoria[i+16*tile + 0x1000*pattern_t];
        uint8_t byte2 = memoria[i+8+16*tile + 0x1000*pattern_t];

        uint8_t mask = 0x80;
        for(int a=0; a < 8; a++)
        {
            int atual=0;
            if(byte1 & mask) atual |= 1;
            if(byte2 & mask) atual |= 2;

            int pos_x = (flipx) ? (x + 8 - a) : (x+a);
            int pos_y = (flipy) ? (y + 8 - i) : (y+i);

            if(pos_y < 240 && pos_y >= 0 && pos_x < 256 && pos_x >= 0)
               framebuffer[pos_y][pos_x] = memoria[4*paleta + atual + 0x3f00];

            mask >>= 1;
        }
    }
}

void PPU::Draw()
{
    //calcula qual a nametable ativa no momento
    int nametable = (m_ppuctrl & 0x3);
    int sprite_size = (m_ppuctrl & (1<<5));
    uint8_t* attrib_table = &memoria[nametable*0x400 + 0x2000 + 0x3c0];
    uint8_t paletas[16][16];

    //interpreta a attrib_table
    for(int i=0; i < 64; i++)
    {
        uint8_t x = (i%8)*2;
        uint8_t y = (i/8)*2;
        uint8_t atual = attrib_table[i];

        paletas[y][x] = (atual) & 3;
        paletas[y][x+1] = (atual >> 2) & 3;
        paletas[y+1][x] = (atual >> 4) & 3;
        paletas[y+1][x+1] = (atual >> 6) & 3;
    }

    //desenha as pattern tables
    for(int a=0; a < 960; a++) //a tela comporta 960 tiles 8x8
    {
        uint8_t pos_x = (a%32)*8;
        uint8_t pos_y = (a/32)*8;
        drawTile(pos_x, pos_y, paletas[pos_y/16][pos_x/16], (m_ppuctrl & 0x10) >> 4, memoria[nametable*0x0400 + 0x2000 + a]);
    }

    //desenha os sprites
    for(int l=0; l < 64; l++)
    {
        uint8_t pos_y = OAM[l*4];
        uint8_t tile  = OAM[l*4+1];
        uint8_t byte3 = OAM[l*4+2];
        uint8_t pos_x = OAM[l*4+3];
        if(sprite_size == 0)
            drawTile(pos_x, pos_y, (byte3 & 0x3) + 4, (m_ppuctrl & 0x8) >> 3, tile, byte3&0x40, byte3&0x80);
        else
        {
            Logger::out("TODO") << "sprites 8x16 nao implementado.";
        }
    }



}

bool PPU::NMI_ativa()
{
    if(m_ppuctrl & 0x80) return true;
    return false;
}


bool PPU::VerticalBlank()
{
    Logger::out("PPU") << "Vblank time.";
    vertical_blank = true;
    return NMI_ativa();
}


void ppu_register_write(uint16_t addr, uint8_t v)
{
        switch(addr)
        {
        case 0x2000:
            __ppu_atual->ppucrtl(v);
            return;
        case 0x2001:
            __ppu_atual->ppumask(v);
            return;

        case 0x2003:
            __ppu_atual->SetOamAddr(v);
            return;

        case 0x2004:
            __ppu_atual->WriteOamData(v);
            return;

        case 0x2005:
            __ppu_atual->ppuscroll(v);
            return;

        case 0x2006:
            __ppu_atual->ppuaddr(v);
            return;

        case 0x2007:
            __ppu_atual->ppudata_write(v);
            return;

        case 0x4014:
            //TODO: contar os ciclos realizados pela cópia
            for(int i=0; i < 256; i++)
            {
                __ppu_atual->write_oam_byte(i, getCpuRam().read( (((int)v) << 8) | i ));
            }
            return;
        default:
            Logger::out("PPU") << "Endereco " << std::hex << addr << " nao eh um registrador da ppu.";
            return;
        }

}

uint8_t ppu_register_read(uint16_t addr)
{

    switch(addr)
    {
    case 0x2002:
        return __ppu_atual->ppustatus();

    case 0x2004:
        return __ppu_atual->ReadOamData();

    case 0x2007:
        return __ppu_atual->ppudata_read();

    default:
        Logger::out("PPU") << "Endereco " << std::hex << addr << " nao eh um registrador da ppu.";
        return 0;
    }
}
