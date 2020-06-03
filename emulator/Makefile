TARGET  = ngw_emu
OBJS    = ./src/emulator.o ./src/game.o ./src/visual.o \
          ./src/machine/mcu.o ./src/machine/lcd.o ./src/machine/buzzer.o ./src/machine/pcb.o \
          ./src/platform/psp_platform.o ./src/platform/psp_main.o

INCDIR   = ./src
DEFINES  = -DPLATFORM_PSP
CFLAGS   = -O2 -G4 -Wall $(DEFINES)
CXXFLAGS = -fno-exceptions -fno-rtti
ASFLAGS	 =

LIBDIR   =
LIBS     = -lstdc++ -lpng -lz -lm -lpspvram
LDFLAGS	 =

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = ngw_emu
#PSP_EBOOT_ICON= ./res/ICON0.png
#PSP_EBOOT_PIC1= ./res/PIC1.png
#PSP_EBOOT_SND0= ./res/SND0.at3

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak