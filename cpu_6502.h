#ifndef CPU_6502_H
#define CPU_6502_H
#include <cstdint>
#include "MemoryController.h"
#include <string>

enum FLAGS_t
{
    CARRY_FLAG = 1<<0,
    ZERO_FLAG = 1<<1,
    INTERRUPT_FLAG = 1<<2,
    DECIMAL_FLAG = 1<<3,
    BREAK_FLAG = 1<<4,
    NOT_USED_FLAG = 1<<5,
    OVERFLOW_FLAG = 1<<6,
    SIGN_FLAG = 1<<7,
};

//a classe da cpu vai usar dois ponteiros para funcoes para conseguir acessar a memoria
//a memoria eh configurada em outro local do programa e pra cpu sao passados apenas os ponteiros para leitura ou escrita
typedef uint8_t (*memory_reader_func)(uint16_t);
typedef void (*memory_writer_func)(uint16_t, uint8_t);

class CPU_6502
{
    public:
        explicit CPU_6502(MemoryController& m);
        virtual ~CPU_6502();

        uint8_t m_accumulator;
        uint8_t m_registrador_x;
        uint8_t m_registrador_y;
        uint8_t m_flags;

        uint16_t m_program_counter;

        uint8_t m_stack_pointer;

        void start();
        void _executar_instrucoes(int n=0); //executa n instrucoes

        std::string PrintState();

        inline void STACK_PUSH(uint8_t v)
        {
            _write(0x0100 + m_stack_pointer, v);
            m_stack_pointer--;
        }
        inline uint8_t STACK_POP()
        {
            m_stack_pointer++;
            uint8_t ret = _read(0x0100 + m_stack_pointer);

            return ret;
        }
        bool crash;
        bool ComparaEstado(const char* str);

        void NMI(); //causa uma NMI
        bool dentro_da_nmi;

        bool NMI_enabled();
    protected:
        MemoryController& m_memory;

        int m_ciclos;

        inline uint8_t _read(uint16_t val)
        {
            return m_memory.read(val);
        }
        inline void _write(uint16_t addr, uint8_t val)
        {
            m_memory.write(addr, val);
        }


    private:
};

#endif // CPU_6502_H
