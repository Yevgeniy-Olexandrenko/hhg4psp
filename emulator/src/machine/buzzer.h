#pragma once

class Buzzer
{
public:
	Buzzer();
	void  Write(u08 data);
	float GetSample() const;

private:
	float m_sample;
};
