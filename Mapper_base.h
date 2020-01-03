#ifndef MAPPER_BASE_H
#define MAPPER_BASE_H

#include "ines_rom_reader.h"
#include <cstdint>
#include "MemoryController.h"
#include "ppu.h"

class Mapper_base
{
public:
    explicit Mapper_base(const ines_rom_reader& dados, MemoryController& mc, PPU& _ppu);

    ~Mapper_base();

    std::vector<char> m_prg_rom[2];
    std::vector<char> m_chr_rom[2];
private:
    MemoryController& m_mem_control;
    PPU& m_ppu;




    void init_registers();
};

#endif // MAPPER_BASE_H
