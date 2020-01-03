#include "ines_rom_reader.h"

#include <fstream>

#include "Logger.h"

const char header_magic_code[4] = {0x4e, 0x45, 0x53, 0x1A};

struct file_header
{
    char constant[4];
    uint8_t PRG_ROM_SIZE;
    uint8_t CHR_ROM_SIZE;
    uint8_t flags_6;
    uint8_t flags_7;
    uint8_t PRG_RAM_SIZE;
    uint8_t flags_9;
    uint8_t unused[6];
};

bool teste_header(file_header* h)
{
    for(int i=0; i < 4; i++)
    {
        if(h->constant[i] != header_magic_code[i])
            return false;
    }
    return true;
}

ines_rom_reader::ines_rom_reader(const char* filename)
{
    m_open = false;
    std::fstream file(filename, std::fstream::in | std::fstream::binary);

    if(file.is_open())
    {
        m_open = true;

        file_header header;
        file.read((char*)&header, sizeof(struct file_header));

        //valida o cabecalho
        if(!teste_header(&header))
        {
            m_open = false;
            goto ERRO;
        }
        Logger::out("rom_reader") << "Rom valida!";

        Logger::out("rom_reader") << "PRG_ROM_SIZE: " << (int)header.PRG_ROM_SIZE;
        Logger::out("rom_reader") << "CHR_ROM_SIZE: " << (int)header.CHR_ROM_SIZE;
        m_mapper = (header.flags_7 & 0xF0) | (header.flags_6 >> 4);
        Logger::out("rom_reader") << "Mapper: " << (int)m_mapper;

        m_prg_rom.resize(header.PRG_ROM_SIZE);
        m_chr_rom.resize(header.CHR_ROM_SIZE);

        //ler as PGR_ROM
        for(int i=0; i < header.PRG_ROM_SIZE; i++)
        {
            m_prg_rom[i].resize(16384);
            file.read(&m_prg_rom[i][0], 16384);

            if(file.gcount() != 16384)
                goto ERRO;
        }

        //ler as CHR_ROM
        for(int i=0; i < header.CHR_ROM_SIZE; i++)
        {
            m_chr_rom[i].resize(8192);
            file.read(&m_chr_rom[i][0], 8192);

            if(file.gcount() != 8192)
            {
                Logger::out("rom_reader") << file.gcount();
                goto ERRO;
            }

        }


    }

    return;
ERRO:
    Logger::out("rom_reader") << "Erro na leitura do arquivo.";

}

ines_rom_reader::~ines_rom_reader()
{
    //dtor
}
