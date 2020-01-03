#include <ctime>
#include <SDL/SDL.h>
#include "ppu.h"
#include "Logger.h"
#include "ines_rom_reader.h"
#include "Mapper_base.h"
#include "Gamepad.h"
#include "cpu_6502.h"

MemoryController& getCpuRam();
