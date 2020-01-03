#ifndef MEMORYCONTROLLER_H
#define MEMORYCONTROLLER_H

#include <cstdint>

typedef uint8_t (*read_callback)(uint16_t addr);
typedef void (*write_callback)(uint16_t addr, uint8_t val);

class MemoryController
{
public:
    MemoryController();
    ~MemoryController();

    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t val);


    void registerReadCallback(uint16_t addr, read_callback f);
    void registerWriteCallback(uint16_t addr, write_callback f);


private:
    uint8_t m_ram[2048];

    read_callback m_callback_onread[65536];
    write_callback m_callback_onwrite[65536];
};

#endif // MEMORYCONTROLLER_H
