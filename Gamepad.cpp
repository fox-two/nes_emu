#include "Gamepad.h"
#include <cstdio>


bool _strobe = false;
bool _button_state[8] = {false, false, false, false, false, false, false, false};
int atual = 0;

void Gamepad::setButtonState(int id, bool flag)
{
    _button_state[id] = flag;
}

void gamepad_register_write(uint16_t addr, uint8_t val)
{
    if(val & 0x1) //strobe bit
    {
        _strobe = true;
        atual = 0;
    }
    else
        _strobe = false;
}


uint8_t gamepad_register_read(uint16_t addr)
{
    if(_strobe)
        return _button_state[0];
    atual = (atual >= 8) ? (0) : atual;

    return _button_state[atual++];
}
