#ifndef INES_ROM_READER_H
#define INES_ROM_READER_H

#include <cstdint>
#include <vector>


class ines_rom_reader
{
public:
    explicit ines_rom_reader(const char* filename);
    ~ines_rom_reader();

    bool m_open;
    uint8_t m_mapper;

    std::vector< std::vector<char> > m_prg_rom;
    std::vector< std::vector<char> > m_chr_rom;

};

#endif // INES_ROM_READER_H
