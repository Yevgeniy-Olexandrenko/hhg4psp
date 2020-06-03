#pragma once

#include "mcu.h"
#include "lcd.h"
#include "buzzer.h"

class PCB : public MCU::IO
{
public:
	simple_safe_enum(Input,
		UP,
		DOWN,
		LEFT,
		RIGHT,
		LEFT_UP,
		LEFT_DOWN,
		RIGHT_UP,
		RIGHT_DOWN,
		ACTION,
		GAME_A,
		GAME_B,
		TIME,
		ALARM,
		ACL,
		CHEAT,
		TEST,
		count
	);

	typed_safe_enum(ISignal, u08,
		MCU_R2 = 0x02,
		MCU_R3 = 0x04,
		MCU_R4 = 0x08,
		GND    = 0x00,
		VCC    = 0xFF
	);

	typed_safe_enum(OSignal, u08,
		MCU_K1    = 0x01,
		MCU_K2    = 0x02,
		MCU_K3    = 0x04,
		MCU_K4    = 0x08,
		MCU_ALPHA = 0x10,
		MCU_BETA  = 0x20,
		MCU_ACL   = 0x40
	);

public:
	PCB();
	~PCB();

	void BuildDevices(const u08* rom);
	void BuildInput(ISignal iSignal, OSignal oSignal, Input input);

	void SetInputFlipped(bool isFlipped);
	void SetInput(u16 inputMask);
	void Clock();

	MCU * GetMCU() const;
	LCD * GetLCD() const;
	Buzzer * GetBuzzer() const;

	bool IsCheatEnabled() const;

public:
	virtual Flg  ReadAcl() const;
	virtual Flg  ReadAlpha() const;
	virtual Flg  ReadBeta() const;
	virtual u08  ReadK() const;
	virtual void WriteR(u08 data);
	virtual void WriteLCD(int o, u08 s);

private:
	struct Line
	{
		Line();
		Line(ISignal iSignal, OSignal oSignal);

		bool m_isUsed;
		bool m_isActive;
		ISignal m_in;
		OSignal m_out;
	};

	MCU    * m_mcu;
	LCD    * m_lcd;
	Buzzer * m_buz;

	Line m_lines[PCB::Input::count];
	u08  m_mcuOut;

	bool m_isInputFlipped;
	bool m_isCheatPressed;
	bool m_isCheatEnabled;
};
