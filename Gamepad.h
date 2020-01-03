#ifndef GAMEPAD_H
#define GAMEPAD_H

#include <cstdint>



class Gamepad
{
public:
    enum BUTTONS
    {
        BUTTON_A,
        BUTTON_B,
        BUTTON_SELECT,
        BUTTON_START,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT
    };
    static void setButtonState(int id, bool flag);


protected:

private:

    Gamepad();
};

void gamepad_register_write(uint16_t addr, uint8_t val);
uint8_t gamepad_register_read(uint16_t addr);


#endif // GAMEPAD_H
