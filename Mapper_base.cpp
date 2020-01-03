#include "Mapper_base.h"
#include <algorithm>

Mapper_base* __atual = nullptr;

Mapper_base::Mapper_base(const ines_rom_reader& dados, MemoryController& mc, PPU& _ppu) :
    m_mem_control(mc), m_ppu(_ppu)
{
    __atual = this;
    for(size_t i=0; i < 2; i++)
    {
        m_prg_rom[i] = dados.m_prg_rom[std::min(i, dados.m_prg_rom.size()-1)];
    }

    m_chr_rom[0].resize(4096);
    for(int i=0; i < 4096; i++)
    {
        m_chr_rom[0][i] = dados.m_chr_rom[0][i];
    }

    m_chr_rom[1].resize(4096);
    for(int i=0; i < 4096; i++)
    {
        m_chr_rom[1][i] = dados.m_chr_rom[0][i+4096];
    }

    init_registers();

}

uint8_t read_prg_rom_0(uint16_t ptr)
{
    return __atual->m_prg_rom[0][ptr-0x8000];
}

uint8_t read_prg_rom_1(uint16_t ptr)
{
    return __atual->m_prg_rom[1][ptr-0xc000];
}


void Mapper_base::init_registers()
{
    for(uint16_t i = 0x8000; i < 0xc000; i++)
    {
        m_mem_control.registerReadCallback(i, read_prg_rom_0);
    }

    for(int i = 0xc000; i <= 0xffff; i++)
    {
        m_mem_control.registerReadCallback(i, read_prg_rom_1);
    }

    //manda as pattern table pra ppu
    m_ppu.NovaPatternTable((uint8_t*)&m_chr_rom[0][0], 0);
    m_ppu.NovaPatternTable((uint8_t*)&m_chr_rom[1][0], 1);

}

Mapper_base::~Mapper_base()
{
    //dtor
}
