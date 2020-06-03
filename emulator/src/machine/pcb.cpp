#include "commons.h"
#include "pcb.h"

PCB::PCB()
	: m_mcu(NULL)
	, m_lcd(NULL)
	, m_buz(NULL)
	, m_isInputFlipped(false)
	, m_isCheatPressed(false)
	, m_isCheatEnabled(false)
{
}

PCB::~PCB()
{
	delete m_mcu;
	delete m_lcd;
	delete m_buz;
}

void PCB::BuildDevices(const u08* rom)
{
	m_buz = new Buzzer();
	m_lcd = new LCD();
	m_mcu = new MCU(rom, *this);
	m_mcu->Reset();
}

void PCB::BuildInput(ISignal iSignal, OSignal oSignal, Input input)
{
	m_lines[input.underlying()] = Line(iSignal, oSignal);
}

void PCB::SetInputFlipped(bool isFlipped)
{
	m_isInputFlipped = isFlipped;
}

void PCB::SetInput(u16 inputMask)
{
	for (int i = 0; i < Input::count; ++i)
	{
		Input input = (Input)i;
		bool active = (inputMask & (1 << i)) != 0;

		if (m_isInputFlipped)
		{
			switch (input.underlying())
			{
			case Input::LEFT: input = Input::RIGHT; break;
			case Input::RIGHT: input = Input::LEFT; break;
			case Input::LEFT_UP: input = Input::RIGHT_UP; break;
			case Input::RIGHT_UP: input = Input::LEFT_UP; break;
			case Input::LEFT_DOWN: input = Input::RIGHT_DOWN; break;
			case Input::RIGHT_DOWN: input = Input::LEFT_DOWN; break;
			default: break;
			}
		}

		if (m_lines[input.underlying()].m_isUsed)
		{
			if (input == Input::CHEAT)
			{
				if (active && !m_isCheatPressed)
				{
					m_isCheatEnabled ^= true;
				}
				m_isCheatPressed = active;
				active = m_isCheatEnabled;
			}
			m_lines[input.underlying()].m_isActive = active;
		}
	}
}

void PCB::Clock()
{
	m_mcu->Clock();
}

MCU * PCB::GetMCU() const
{
	return m_mcu;
}

LCD * PCB::GetLCD() const
{
	return m_lcd;
}

Buzzer * PCB::GetBuzzer() const
{
	return m_buz;
}

bool PCB::IsCheatEnabled() const
{
	return m_isCheatEnabled;
}

Flg PCB::ReadAcl() const
{
	for(int i = 0; i < PCB::Input::count; ++i)
	{
		const Line & line = m_lines[i];
		if (line.m_in == ISignal::VCC && line.m_out == OSignal::MCU_ACL && line.m_isActive)
		{
			return true;
		}
	}
	return false;
}

Flg PCB::ReadAlpha() const
{
	for (int i = 0; i < PCB::Input::count; ++i)
	{
		const Line & line = m_lines[i];
		if (line.m_in == ISignal::GND && line.m_out == OSignal::MCU_ALPHA && line.m_isActive)
		{
			return false;
		}
	}
	return true;
}

Flg PCB::ReadBeta() const
{
	for (int i = 0; i < PCB::Input::count; ++i)
	{
		const Line & line = m_lines[i];
		if (line.m_in == ISignal::GND && line.m_out == OSignal::MCU_BETA && line.m_isActive)
		{
			return false;
		}
	}
	return true;
}

u08 PCB::ReadK() const
{
	u08 data = 0;
	for (int i = 0; i < PCB::Input::count; ++i)
	{
		const Line & line = m_lines[i];
		if (line.m_out >= OSignal::MCU_K1 && line.m_out <= OSignal::MCU_K4)
		{
			bool isISignalActive = false;
			if (line.m_in >= ISignal::MCU_R2 && line.m_in <= ISignal::MCU_R4)
			{
				isISignalActive = (m_mcuOut & line.m_in.underlying()) != 0;
			}
			else if (line.m_in == ISignal::VCC)
			{
				isISignalActive = true;
			}
			if (isISignalActive && line.m_isActive)
			{
				data |= line.m_out.underlying();
			}
		}
	}
	return data;
}

void PCB::WriteR(u08 data)
{
	m_mcuOut = data;
	m_buz->Write(data);
}

void PCB::WriteLCD(int o, u08 s)
{
	m_lcd->Write(o, s);
}

PCB::Line::Line()
	: m_isUsed(false)
	, m_isActive(false)
{}

PCB::Line::Line(ISignal iSignal, OSignal oSignal)
	: m_isUsed(true)
	, m_isActive(false)
	, m_in(iSignal)
	, m_out(oSignal)
{}
