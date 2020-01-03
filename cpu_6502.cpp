#include "cpu_6502.h"

#include "Logger.h"
#include <cstdio>

#define MACRO_IMMEDIATE() \
    src_ptr = m_program_counter+1;

#define MACRO_ZERO_PAGE() \
    src_ptr = _read(m_program_counter+1);

#define MACRO_ZERO_PAGE_X() \
    src_ptr = (_read(m_program_counter+1) + m_registrador_x)&0xff;

#define MACRO_ZERO_PAGE_Y() \
    src_ptr = (_read(m_program_counter+1) + m_registrador_y)&0xff;

#define MACRO_ABSOLUTE() \
    src_ptr = (_read(m_program_counter+2)<<8) | _read(m_program_counter+1);

#define MACRO_ABSOLUTE_X() \
    src_ptr = ((_read(m_program_counter+2)<<8) | _read(m_program_counter+1))+m_registrador_x;

#define MACRO_ABSOLUTE_Y() \
    src_ptr = ((_read(m_program_counter+2)<<8) | _read(m_program_counter+1))+m_registrador_y;

#define MACRO_INDIRECT_X() \
    aux = (_read(m_program_counter+1) + m_registrador_x)&0xff; \
    src_ptr = (_read((aux+1)&0xff)<<8) | _read(aux);

#define MACRO_INDIRECT_Y() \
    aux = _read(m_program_counter+1); \
    src_ptr = ((_read((aux+1)&0xff)<<8) | _read(aux))+m_registrador_y;


//macros de setar as flag
#define SET_SIGN(x) \
    if((x) & SIGN_FLAG) m_flags |= SIGN_FLAG; \
    else m_flags &= ~SIGN_FLAG;

#define SET_ZERO(x) \
    if((x)==0) m_flags |= ZERO_FLAG; \
    else m_flags &= ~ZERO_FLAG;

#define SET_CARRY(x) \
    if((x) != 0 ) m_flags |= CARRY_FLAG; \
    else m_flags &= ~CARRY_FLAG;

#define SET_OVERFLOW(x) \
    if((x) != 0 ) m_flags |= OVERFLOW_FLAG; \
    else m_flags &= ~OVERFLOW_FLAG;

#define SET_INTERRUPT(x) \
    if((x) != 0 ) m_flags |= INTERRUPT_FLAG; \
    else m_flags &= ~INTERRUPT_FLAG;

#define SET_BREAK(x) \
    if((x) != 0 ) m_flags |= BREAK_FLAG; \
    else m_flags &= ~BREAK_FLAG;

//macro de testar as flags
#define IF_CARRY() \
    (m_flags & CARRY_FLAG)

#define IF_ZERO() \
    (m_flags & ZERO_FLAG)

#define IF_SIGN() \
    (m_flags & SIGN_FLAG)

#define IF_OVERFLOW() \
    (m_flags & OVERFLOW_FLAG)




const int tamanho_instrucao[0xff] = {1,
                                     2, 1, 1, 1, 2, 2, 1, 1, 2, 1,
                                     1, 1, 3, 3, 1, 2, 2, 1, 1, 1,
                                     2, 2, 1, 1, 3, 1, 1, 1, 3, 3,
                                     1, 0, 2, 1, 1, 2, 2, 2, 1, 1,
                                     2, 1, 1, 3, 3, 3, 1, 2, 2, 1,
                                     1, 1, 2, 2, 1, 1, 3, 1, 1, 1,
                                     3, 3, 1, 1, 2, 1, 1, 1, 2, 2,
                                     1, 1, 2, 1, 1, 0, 3, 3, 1, 2,
                                     2, 1, 1, 1, 2, 2, 1, 1, 3, 1,
                                     1, 1, 3, 3, 1, 0, 2, 1, 1, 1,
                                     2, 2, 1, 1, 2, 1, 1, 0, 3, 3,
                                     1, 2, 2, 1, 1, 1, 2, 2, 1, 1,
                                     3, 1, 1, 1, 3, 3, 1, 1, 2, 1,
                                     1, 2, 2, 2, 1, 1, 1, 1, 1, 3,
                                     3, 3, 1, 2, 2, 1, 1, 2, 2, 2,
                                     1, 1, 3, 1, 1, 1, 3, 1, 1, 2,
                                     2, 2, 1, 2, 2, 2, 1, 1, 2, 1,
                                     1, 3, 3, 3, 1, 2, 2, 1, 1, 2,
                                     2, 2, 1, 1, 3, 1, 1, 3, 3, 3,
                                     1, 2, 2, 1, 1, 2, 2, 2, 1, 1,
                                     2, 1, 1, 3, 3, 3, 1, 2, 2, 1,
                                     1, 1, 2, 2, 1, 1, 3, 1, 1, 1,
                                     3, 3, 1, 2, 2, 1, 1, 2, 2, 2,
                                     1, 1, 2, 1, 1, 3, 3, 3, 1, 2,
                                     2, 1, 1, 1, 2, 2, 1, 1, 3, 1,
                                     1, 1, 3, 3
                                    };
const int ciclo_instrucao[0xff] = {7,
                                   6, 1, 1, 1, 3, 5, 1, 3, 2, 2,
                                   1, 1, 4, 6, 1, 2, 5, 1, 1, 1,
                                   4, 6, 1, 2, 4, 1, 1, 1, 4, 7,
                                   1, 6, 6, 1, 1, 3, 3, 5, 1, 4,
                                   2, 2, 1, 4, 4, 6, 1, 2, 5, 1,
                                   1, 1, 4, 6, 1, 2, 4, 1, 1, 1,
                                   4, 7, 1, 6, 6, 1, 1, 1, 3, 5,
                                   1, 3, 2, 2, 1, 3, 4, 6, 1, 2,
                                   5, 1, 1, 1, 4, 6, 1, 2, 4, 1,
                                   1, 1, 4, 7, 1, 6, 6, 1, 1, 1,
                                   3, 5, 1, 4, 2, 2, 1, 5, 4, 6,
                                   1, 2, 5, 1, 1, 1, 4, 6, 1, 2,
                                   4, 1, 1, 1, 4, 7, 1, 1, 6, 1,
                                   1, 3, 3, 3, 1, 2, 1, 2, 1, 4,
                                   4, 4, 1, 2, 6, 1, 1, 4, 4, 4,
                                   1, 2, 5, 2, 1, 1, 5, 1, 1, 2,
                                   6, 2, 1, 3, 3, 3, 1, 2, 2, 2,
                                   1, 4, 4, 4, 1, 2, 5, 1, 1, 4,
                                   4, 4, 1, 2, 4, 2, 1, 4, 4, 4,
                                   1, 2, 6, 1, 1, 3, 3, 5, 1, 2,
                                   2, 2, 1, 4, 4, 6, 1, 2, 5, 1,
                                   1, 1, 4, 6, 1, 2, 4, 1, 1, 1,
                                   4, 7, 1, 2, 6, 1, 1, 3, 3, 5,
                                   1, 2, 2, 2, 1, 4, 4, 6, 1, 2,
                                   5, 1, 1, 1, 4, 6, 1, 2, 4, 1,
                                   1, 1, 4, 7
                                  };


template<class T>
T ror(T x)
{
    return (x >> 1) | (x << sizeof(T)*8 - 1);
}

template<class T>
T rol(T x)
{
    return (x << 1) | (x >> sizeof(T)*8 - 1);
}

enum addr_mode
{
    IMMEDIATE,
    ABSOLUTE,
    ZERO_PAGE_ABSOLUTE,
    IMPLIED,
    ACCUMULATOR,
    INDEXED,
    ZERO_PAGE_INDEXED,
    INDIRECT,
    PRE_INDEXED_INDIRECT
};



CPU_6502::CPU_6502(MemoryController& mem) :
    m_memory(mem)
{
    //ctor
    m_program_counter = 0;
    m_registrador_x = 0x60;

    m_flags = 0;
    m_flags |= NOT_USED_FLAG;
    m_flags |= INTERRUPT_FLAG;
    m_accumulator = 0;
    m_registrador_x = 0;
    m_registrador_y = 0;
    m_stack_pointer = 0xfd ;

    m_ciclos = 0;

    crash = false;
    dentro_da_nmi = false;

}

CPU_6502::~CPU_6502()
{
    //dtor
}

void CPU_6502::start()
{
    m_program_counter = (_read(0xfffd)  << 8) | _read(0xfffc);

    Logger::out("CPU") << "Entry point: " << std::hex << (int)m_program_counter;
}

void CPU_6502::_executar_instrucoes(int n)
{
    for(m_ciclos=0; m_ciclos < n; m_ciclos++)
    {
        uint8_t opcode = _read(m_program_counter);


        uint16_t aux; //utilizado pelas macro indirect
        uint8_t src;
        uint16_t src_ptr;

        unsigned int temp;
        switch(opcode)
        {
        case 0x69:
            MACRO_IMMEDIATE();
            goto ADC;
        case 0x65:
            MACRO_ZERO_PAGE();
            goto ADC;
        case 0x75:
            MACRO_ZERO_PAGE_X();
            goto ADC;
        case 0x6d:
            MACRO_ABSOLUTE();
            goto ADC;
        case 0x7d:
            MACRO_ABSOLUTE_X();
            goto ADC;
        case 0x79:
            MACRO_ABSOLUTE_Y();
            goto ADC;
        case 0x61:
            MACRO_INDIRECT_X();
            goto ADC;
        case 0x71:
            MACRO_INDIRECT_Y();
            goto ADC;

ADC:
            src = _read(src_ptr);
            temp = src + m_accumulator + (IF_CARRY() ? 1 : 0);
            SET_ZERO(temp & 0xff);

            SET_SIGN(temp);
            SET_OVERFLOW(!((m_accumulator ^ src) & 0x80) && ((m_accumulator ^ temp) & 0x80));
            SET_CARRY(temp > 0xff);

            m_accumulator = ((uint8_t) temp);
            break;

        case 0x29:
            MACRO_IMMEDIATE();
            goto AND;
        case 0x25:
            MACRO_ZERO_PAGE();
            goto AND;
        case 0x35:
            MACRO_ZERO_PAGE_X();
            goto AND;
        case 0x2d:
            MACRO_ABSOLUTE();
            goto AND;
        case 0x3d:
            MACRO_ABSOLUTE_X();
            goto AND;
        case 0x39:
            MACRO_ABSOLUTE_Y();
            goto AND;
        case 0x21:
            MACRO_INDIRECT_X();
            goto AND;
        case 0x31:
            MACRO_INDIRECT_Y();
            goto AND;

AND:
            src = _read(src_ptr);
            src &= m_accumulator;
            SET_SIGN(src);
            SET_ZERO(src);
            m_accumulator = src;
            break;

        case 0x0a:
            //ENDERECAMENTO ACCUMULATOR
            SET_CARRY(m_accumulator & 0x80);
            m_accumulator <<= 1;
            m_accumulator &= 0xff;
            SET_SIGN(m_accumulator);
            SET_ZERO(m_accumulator);
            break;
        case 0x06:
            MACRO_ZERO_PAGE();
            goto ASL;
        case 0x16:
            MACRO_ZERO_PAGE_X();
            goto ASL;
        case 0x0e:
            MACRO_ABSOLUTE();
            goto ASL;
        case 0x1e:
            MACRO_ABSOLUTE_X();
            goto ASL;
ASL:
            src = _read(src_ptr);
            SET_CARRY(src & 0x80);
            src <<= 1;
            src &= 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            _write(src_ptr, src);
            break;

        case 0x90: //BCC
            MACRO_IMMEDIATE();
            if (!IF_CARRY())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);
                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0xb0: //BCS
            MACRO_IMMEDIATE();
            if (IF_CARRY())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);
                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0xf0: //BEQ
            MACRO_IMMEDIATE();
            if (IF_ZERO())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);
                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0x24: //BIT
            MACRO_ZERO_PAGE();
            goto BIT;
        case 0x2c:
            MACRO_ABSOLUTE();
            goto BIT;
BIT:
            src = _read(src_ptr);
            SET_SIGN(src);
            SET_OVERFLOW(0x40 & src);	/* Copy bit 6 to OVERFLOW flag. */
            SET_ZERO(src & m_accumulator);
            break;

        case 0x30: //BMI
            MACRO_IMMEDIATE();
            if (IF_SIGN())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);
                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0xd0: //BNE
            MACRO_IMMEDIATE();
            if (!IF_ZERO())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);
                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0x10: //BPL
            MACRO_IMMEDIATE();
            if (!IF_SIGN())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);
                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0x50: //BVC
            MACRO_IMMEDIATE();
            if (!IF_OVERFLOW())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);

                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0x70: //BVS
            MACRO_IMMEDIATE();
            if (IF_OVERFLOW())
            {
                int novo_addr = m_program_counter + (int8_t)_read(src_ptr);

                m_ciclos += ((m_program_counter & 0xFF00) != (novo_addr & 0xFF00) ? 2 : 1);
                m_program_counter = novo_addr;
            }
            break;

        case 0x18: //CLC
            SET_CARRY(0);
            break;

        case 0xd8: //CLD
            m_flags &= ~DECIMAL_FLAG; //clear decimal mode
            break;

        case 0x58: //CLI
            SET_INTERRUPT(0);
            break;

        case 0xb8: //CLV
            SET_OVERFLOW(0);
            break;

        //CMP
        case 0xc9:
            MACRO_IMMEDIATE();
            goto CMP;
        case 0xc5:
            MACRO_ZERO_PAGE();
            goto CMP;
        case 0xd5:
            MACRO_ZERO_PAGE_X();
            goto CMP;
        case 0xcd:
            MACRO_ABSOLUTE();
            goto CMP;
        case 0xdd:
            MACRO_ABSOLUTE_X();
            goto CMP;
        case 0xd9:
            MACRO_ABSOLUTE_Y();
            goto CMP;
        case 0xc1:
            MACRO_INDIRECT_X();
            goto CMP;
        case 0xd1:
            MACRO_INDIRECT_Y();
            goto CMP;
CMP:
            aux = _read(src_ptr);
            aux = (m_accumulator - aux);
            SET_CARRY(aux < 0x100);
            SET_SIGN(aux);
            SET_ZERO(aux &= 0xff);
            break;

        //CPX
        case 0xe0:
            MACRO_IMMEDIATE();
            goto CPX;
        case 0xe4:
            MACRO_ZERO_PAGE();
            goto CPX;
        case 0xec:
            MACRO_ABSOLUTE();
            goto CPX;
CPX:
            aux = _read(src_ptr);
            aux = m_registrador_x - aux;
            SET_CARRY(aux < 0x100);
            SET_SIGN(aux);
            SET_ZERO(aux &= 0xff);
            break;

        //CPY
        case 0xc0:
            MACRO_IMMEDIATE();
            goto CPY;
        case 0xc4:
            MACRO_ZERO_PAGE();
            goto CPY;
        case 0xcc:
            MACRO_ABSOLUTE();
            goto CPY;
CPY:
            aux = _read(src_ptr);
            aux = m_registrador_y - aux;
            SET_CARRY(aux < 0x100);
            SET_SIGN(aux);
            SET_ZERO(aux &= 0xff);
            break;

        //DEC
        case 0xc6:
            MACRO_ZERO_PAGE();
            goto DEC;
        case 0xd6:
            MACRO_ZERO_PAGE_X();
            goto DEC;
        case 0xce:
            MACRO_ABSOLUTE();
            goto DEC;
        case 0xde:
            MACRO_ABSOLUTE_X();
            goto DEC;

DEC:
            src = _read(src_ptr);
            src = (src - 1) & 0xff;
            SET_SIGN(src);
            SET_ZERO(src);
            _write(src_ptr, src);
            break;

        //DEX
        case 0xca:
            m_registrador_x--;
            SET_SIGN(m_registrador_x);
            SET_ZERO(m_registrador_x);
            break;

        //DEY
        case 0x88:
            m_registrador_y--;
            SET_SIGN(m_registrador_y);
            SET_ZERO(m_registrador_y);
            break;

        //EOR
        case 0x49:
            MACRO_IMMEDIATE();
            goto EOR;
        case 0x45:
            MACRO_ZERO_PAGE();
            goto EOR;
        case 0x55:
            MACRO_ZERO_PAGE_X();
            goto EOR;
        case 0x4d:
            MACRO_ABSOLUTE();
            goto EOR;
        case 0x5d:
            MACRO_ABSOLUTE_X();
            goto EOR;
        case 0x59:
            MACRO_ABSOLUTE_Y();
            goto EOR;
        case 0x41:
            MACRO_INDIRECT_X();
            goto EOR;
        case 0x51:
            MACRO_INDIRECT_Y();
            goto EOR;

EOR:
            src = (_read(src_ptr) ^ m_accumulator);
            SET_SIGN(src);
            SET_ZERO(src);
            m_accumulator = src;
            break;

        //INC
        case 0xe6:
            MACRO_ZERO_PAGE();
            goto INC;
        case 0xf6:
            MACRO_ZERO_PAGE_X();
            goto INC;
        case 0xee:
            MACRO_ABSOLUTE();
            goto INC;
        case 0xfe:
            MACRO_ABSOLUTE_X();
            goto INC;
INC:
            src = _read(src_ptr);
            src++;
            SET_SIGN(src);
            SET_ZERO(src);
            _write(src_ptr, src);
            break;

        //INX
        case 0xe8:
            m_registrador_x++;
            SET_SIGN(m_registrador_x);
            SET_ZERO(m_registrador_x);
            break;

        //INY
        case 0xc8:
            m_registrador_y++;
            SET_SIGN(m_registrador_y);
            SET_ZERO(m_registrador_y);
            break;

        //JMP
        case 0x4c:
            MACRO_ABSOLUTE();
            m_program_counter = src_ptr;
            break;
        case 0x6c:
            MACRO_ABSOLUTE();
            src = src_ptr+1;
            aux = ((_read((src_ptr&0xff00)|src)<<8) | _read(src_ptr));
            m_program_counter = aux;
            break;

        //JSR
        case 0x20:
            MACRO_ABSOLUTE();
            m_program_counter+=2;
            STACK_PUSH((m_program_counter >> 8) & 0xff);	/* Push return address onto the stack. */
            STACK_PUSH(m_program_counter & 0xff);
            m_program_counter = src_ptr;
            break;

        //lDA
        case 0xa9:
            MACRO_IMMEDIATE();
            goto LDA;
        case 0xa5:
            MACRO_ZERO_PAGE();
            goto LDA;
        case 0xb5:
            MACRO_ZERO_PAGE_X();
            goto LDA;
        case 0xad:
            MACRO_ABSOLUTE();
            goto LDA;
        case 0xbd:
            MACRO_ABSOLUTE_X();
            goto LDA;
        case 0xb9:
            MACRO_ABSOLUTE_Y();
            goto LDA;
        case 0xa1:
            MACRO_INDIRECT_X();
            goto LDA;
        case 0xb1:
            MACRO_INDIRECT_Y();
            goto LDA;

LDA:
            src = _read(src_ptr);
            SET_SIGN(src);
            SET_ZERO(src);
            m_accumulator = (src);
            break;

        //LDX
        case 0xa2:
            MACRO_IMMEDIATE();
            goto LDX;
        case 0xa6:
            MACRO_ZERO_PAGE();
            goto LDX;
        case 0xb6:
            MACRO_ZERO_PAGE_Y();
            goto LDX;
        case 0xae:
            MACRO_ABSOLUTE();
            goto LDX;
        case 0xbe:
            MACRO_ABSOLUTE_Y();
            goto LDX;

LDX:
            src = _read(src_ptr);
            SET_SIGN(src);
            SET_ZERO(src);
            m_registrador_x = (src);
            break;

        //LDY
        case 0xa0:
            MACRO_IMMEDIATE();
            goto LDY;
        case 0xa4:
            MACRO_ZERO_PAGE();
            goto LDY;
        case 0xb4:
            MACRO_ZERO_PAGE_X();
            goto LDY;
        case 0xac:
            MACRO_ABSOLUTE();
            goto LDY;
        case 0xbc:
            MACRO_ABSOLUTE_X();
            goto LDY;

LDY:
            src = _read(src_ptr);
            SET_SIGN(src);
            SET_ZERO(src);
            m_registrador_y = (src);
            break;

        //LSR
        case 0x4a:
            //accumulator
            SET_CARRY(m_accumulator & 0x01);
            m_accumulator >>= 1;
            SET_SIGN(m_accumulator);
            SET_ZERO(m_accumulator);
            break;
        case 0x46:
            MACRO_ZERO_PAGE();
            goto LSR;
        case 0x56:
            MACRO_ZERO_PAGE_X();
            goto LSR;
        case 0x4e:
            MACRO_ABSOLUTE();
            goto LSR;
        case 0x5e:
            MACRO_ABSOLUTE_X();
            goto LSR;

LSR:
            src  =  _read(src_ptr);
            SET_CARRY(src & 0x01);
            src >>= 1;
            SET_SIGN(src);
            SET_ZERO(src);
            _write(src_ptr, src);
            break;

        //NOP
        case 0xea:

            break;

        //ORA
        case 0x09:
            MACRO_IMMEDIATE();
            goto ORA;
        case 0x05:
            MACRO_ZERO_PAGE();
            goto ORA;
        case 0x15:
            MACRO_ZERO_PAGE_X();
            goto ORA;
        case 0x0d:
            MACRO_ABSOLUTE();
            goto ORA;
        case 0x1d:
            MACRO_ABSOLUTE_X();
            goto ORA;
        case 0x19:
            MACRO_ABSOLUTE_Y();
            goto ORA;
        case 0x01:
            MACRO_INDIRECT_X();
            goto ORA;
        case 0x11:
            MACRO_INDIRECT_Y();
            goto ORA;

ORA:
            src = (_read(src_ptr) | m_accumulator);
            SET_SIGN(src);
            SET_ZERO(src);
            m_accumulator = src;
            break;

        //PHA
        case 0x48:
            STACK_PUSH(m_accumulator);
            break;

        //php
        case 0x08:
            src = m_flags | BREAK_FLAG; //simula um bug do processador do nes
            STACK_PUSH(src);
            break;

        //PLA
        case 0x68:
            m_accumulator = STACK_POP();
            SET_SIGN(m_accumulator);
            SET_ZERO(m_accumulator);
            break;

        //plp
        case 0x28:
            m_flags = STACK_POP();
            m_flags |= NOT_USED_FLAG;
            m_flags &= ~BREAK_FLAG;
            break;

        //ROL
        case 0x2a:
            //accumulator
            aux = m_accumulator;
            aux <<= 1;
            if (IF_CARRY()) aux |= 0x1;
            SET_CARRY(aux > 0xff);
            aux &= 0xff;
            SET_SIGN(aux);
            SET_ZERO(aux);
            m_accumulator = aux;
            break;
        case 0x26:
            MACRO_ZERO_PAGE();
            goto ROL;
        case 0x36:
            MACRO_ZERO_PAGE_X();
            goto ROL;
        case 0x2e:
            MACRO_ABSOLUTE();
            goto ROL;
        case 0x3e:
            MACRO_ABSOLUTE_X();
            goto ROL;

ROL:
            aux = _read(src_ptr);
            aux <<= 1;
            if (IF_CARRY()) aux |= 0x1;
            SET_CARRY(aux > 0xff);
            aux &= 0xff;
            SET_SIGN(aux);
            SET_ZERO(aux);
            _write(src_ptr, aux);
            break;


        //ROR
        case 0x6a:
            //accumulator
            aux = m_accumulator;
            if (IF_CARRY()) aux |= 0x100;
            SET_CARRY(aux & 0x01);
            aux >>= 1;
            SET_SIGN(aux);
            SET_ZERO(aux);
            m_accumulator = aux;
            break;
        case 0x66:
            MACRO_ZERO_PAGE();
            goto ROR;
        case 0x76:
            MACRO_ZERO_PAGE_X();
            goto ROR;
        case 0x6e:
            MACRO_ABSOLUTE();
            goto ROR;
        case 0x7e:
            MACRO_ABSOLUTE_X();
            goto ROR;

ROR:
            aux = _read(src_ptr);
            if (IF_CARRY()) aux |= 0x100;
            SET_CARRY(aux & 0x01);
            aux >>= 1;
            SET_SIGN(aux);
            SET_ZERO(aux);
            _write(src_ptr, aux);
            break;


        //RTI
        case 0x40:
            src = STACK_POP();
            m_flags = src;
            m_flags |= NOT_USED_FLAG;
            aux = STACK_POP();
            aux |= (STACK_POP() << 8);	/* Load return address from stack. */
            //aux--;
            m_program_counter = (aux);
            dentro_da_nmi = false;
            Logger::out("CPU") << "Voltando para: 0x" << std::hex << (int)m_program_counter;
            return;
            break;

        //rts
        case 0x60:
            aux = STACK_POP();
            aux |= (STACK_POP() << 8);
            aux++;
            //src += ((STACK_POP()) << 8) + 1;	/* Load return address from stack and add 1. */
            m_program_counter = aux;
            break;

        //SBC
        case 0xe9:
            MACRO_IMMEDIATE();
            goto SBC;
        case 0xe5:
            MACRO_ZERO_PAGE();
            goto SBC;
        case 0xf5:
            MACRO_ZERO_PAGE_X();
            goto SBC;
        case 0xed:
            MACRO_ABSOLUTE();
            goto SBC;
        case 0xfd:
            MACRO_ABSOLUTE_X();
            goto SBC;
        case 0xf9:
            MACRO_ABSOLUTE_Y();
            goto SBC;
        case 0xe1:
            MACRO_INDIRECT_X();
            goto SBC;
        case 0xf1:
            MACRO_INDIRECT_Y();
            goto SBC;

SBC:
            src = _read(src_ptr);
            temp = m_accumulator - src - (IF_CARRY() ? 0 : 1);
            SET_SIGN(temp);
            SET_ZERO(temp & 0xff);
            SET_OVERFLOW(((m_accumulator ^ temp) & 0x80) && ((m_accumulator ^ src) & 0x80));
            SET_CARRY(temp < 0x100);
            m_accumulator = (temp & 0xff);
            break;

        //SEC
        case 0x38:
            SET_CARRY((1));
            break;

        //SED
        case 0xf8: //set decimal
            m_flags |= DECIMAL_FLAG;
            break;

        //sei
        case 0x78:
            SET_INTERRUPT((1));
            break;

        //sta
        case 0x85:
            MACRO_ZERO_PAGE();
            goto STA;
        case 0x95:
            MACRO_ZERO_PAGE_X();
            goto STA;
        case 0x8d:
            MACRO_ABSOLUTE();
            goto STA;
        case 0x9d:
            MACRO_ABSOLUTE_X();
            goto STA;
        case 0x99:
            MACRO_ABSOLUTE_Y();
            goto STA;
        case 0x81:
            MACRO_INDIRECT_X();
            goto STA;
        case 0x91:
            MACRO_INDIRECT_Y();
            goto STA;

STA:

            _write(src_ptr, m_accumulator);
            break;

        //stx
        case 0x86:
            MACRO_ZERO_PAGE();
            goto STX;
        case 0x96:
            MACRO_ZERO_PAGE_Y();
            goto STX;
        case 0x8e:
            MACRO_ABSOLUTE();
            goto STX;
STX:
            _write(src_ptr, m_registrador_x);
            break;

        //sty
        case 0x84:
            MACRO_ZERO_PAGE();
            goto STY;
        case 0x94:
            MACRO_ZERO_PAGE_X();
            goto STY;
        case 0x8c:
            MACRO_ABSOLUTE();
            goto STY;
STY:
            _write(src_ptr, m_registrador_y);
            break;

        //TAX
        case 0xaa:
            SET_SIGN(m_accumulator);
            SET_ZERO(m_accumulator);
            m_registrador_x = m_accumulator;
            break;

        //TAY
        case 0xa8:
            SET_SIGN(m_accumulator);
            SET_ZERO(m_accumulator);
            m_registrador_y = m_accumulator;
            break;

        //TSX
        case 0xba:
            SET_SIGN(m_stack_pointer);
            SET_ZERO(m_stack_pointer);
            m_registrador_x = m_stack_pointer;
            break;

        //TXA
        case 0x8a:
            SET_SIGN(m_registrador_x);
            SET_ZERO(m_registrador_x);
            m_accumulator = m_registrador_x;
            break;

        //TXS
        case 0x9a:
            m_stack_pointer = m_registrador_x;
            break;

        //tya
        case 0x98:
            SET_SIGN(m_registrador_y);
            SET_ZERO(m_registrador_y);
            m_accumulator = m_registrador_y;
            break;
        default:
            //Logger::out("CPU") << "Instrucao invalida.";
            PrintState();
            m_program_counter++;
            crash = true;
            return;
            break;

        }
        m_ciclos += ciclo_instrucao[opcode];
        m_program_counter += tamanho_instrucao[opcode];


        PrintState();
    }
}

std::string CPU_6502::PrintState()
{
    char buffer[256];
    sprintf(buffer, "%x (opcode: %X) A:%2x X:%2x Y:%2x P:%2x SP: %2x CICLOS: %i\n", m_program_counter, _read(m_program_counter), m_accumulator, m_registrador_x, m_registrador_y, m_flags, m_stack_pointer, m_ciclos);
    return std::string(buffer);
}
#include <cstring>
bool CPU_6502::ComparaEstado(const char* str)
{
    char posicao[5] = {0};
    char ac[5] = {0};
    char x[5] = {0};
    char y[5] = {0};
    char p[5] = {0};
    char sp[5] = {0};

    memcpy(posicao, str, 4);
    memcpy(ac, &str[50], 2);
    memcpy(x, &str[55], 2);
    memcpy(y, &str[60], 2);
    memcpy(p, &str[65], 2);
    memcpy(sp, &str[71], 2);

    uint32_t _pos;
    sscanf(posicao, "%X", &_pos);
    uint32_t _ac;
    sscanf(ac, "%X", &_ac);
    uint32_t _x;
    sscanf(x, "%X", &_x);
    uint32_t _y;
    sscanf(y, "%X", &_y);
    uint32_t _p;
    sscanf(p, "%X", &_p);
    uint32_t _sp;
    sscanf(sp, "%X", &_sp);

    if(_pos != m_program_counter) return false;
    if(_ac != m_accumulator) return false;
    if(_x != m_registrador_x) return false;
    if(_y != m_registrador_y) return false;
    if(_p != m_flags) return false;
    if(_sp != m_stack_pointer) return false;

    return true;
}


void CPU_6502::NMI()
{
    //m_program_counter++;
    STACK_PUSH(m_program_counter >> 8);
    STACK_PUSH(m_program_counter & 0xff);
    STACK_PUSH(m_flags);

    Logger::out("CPU") << "NMI: saindo de 0x" << std::hex << (int)m_program_counter;
    m_program_counter = (_read(0xfffb)  << 8) | _read(0xfffa);
    dentro_da_nmi = true;
    Logger::out("CPU") << "NMI: para 0x" << std::hex << (int)m_program_counter;
}

