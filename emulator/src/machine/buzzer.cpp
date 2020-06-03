#include "commons.h"
#include "buzzer.h"

Buzzer::Buzzer()
	: m_sample(0)
{
}

void Buzzer::Write(u08 data)
{
	m_sample = (data & 0x01) != 0 ? +1.f : -1.f;
}

float Buzzer::GetSample() const
{
	return m_sample;
}
