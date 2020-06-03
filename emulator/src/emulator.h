#pragma once

#include "platform.h"
#include "game.h"
#include "drawer.h"

class Emulator : public platform::AudioSampleProducer
{
public:
	Emulator();

	void Init();
	void Loop();
	void Free();

protected:
	virtual void ProduceAudioSample(float & sample, int & count);
	void UpdateLayoutSwitch();
	void UpdateBatteryLevel();
	void UpdateAutoPlay();

private:
	Game   m_game;
	Drawer m_drawer;

	volatile float m_fraction;
	u16 m_autoInput;
};
