#include "MemoryController.h"
#include <cassert>
#include <cstring>

MemoryController::MemoryController()
{
    memset(m_callback_onread, 0, sizeof(m_callback_onread));
    memset(m_callback_onwrite, 0, sizeof(m_callback_onwrite));
}

MemoryController::~MemoryController()
{
    //dtor
}

uint8_t MemoryController::read(uint16_t addr)
{
    if(m_callback_onread[addr] != nullptr)
        return m_callback_onread[addr](addr);

    if(addr < 2048)
        return m_ram[addr];

    return 0;
}

void MemoryController::write(uint16_t addr, uint8_t val)
{
    if(m_callback_onwrite[addr] != nullptr)
    {
        m_callback_onwrite[addr](addr, val);
        return;
    }

    if(addr < 2048)
        m_ram[addr] = val;
}

void MemoryController::registerReadCallback(uint16_t addr, read_callback f)
{
    m_callback_onread[addr] = f;
}


void MemoryController::registerWriteCallback(uint16_t addr, write_callback f)
{
    m_callback_onwrite[addr] = f;
}
