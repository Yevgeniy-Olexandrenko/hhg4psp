#ifdef PLATFORM_SDL2

#include <SDL.h>
#include "emulator.h"

int main(int, char**)
{
	Emulator emulator;
	emulator.Init();
	emulator.Loop();
	emulator.Free();
	return 0;
}

#endif