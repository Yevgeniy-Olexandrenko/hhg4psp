#include "commons.h"
#include "emulator.h"

//	Useful info:
//	https://redream.io/posts/improving-audio-video-synchronization-multi-sync

const std::string k_defaultGame = "gnw_helmet";

Emulator::Emulator()
	: m_drawer(&m_game)
	, m_fraction(0)
	, m_autoInput(0)
{
}

void Emulator::Init()
{
	platform::Dbg("emulator init\n");

	platform::Init(this);
	m_game.Load(k_defaultGame);	
	m_drawer.Init(m_drawer.GetLayoutId(), m_drawer.IsFlipped());
}

void Emulator::Loop()
{
	platform::Dbg("emulator loop\n");

	platform::audio::Resume();
	while(!platform::input::IsQuitRequested())
	{
		if (m_game.GetPCB()->GetLCD()->HasNewFrame())
		{
//@@		UpdateAutoPlay();
			UpdateLayoutSwitch();
			UpdateBatteryLevel();
			m_drawer.Draw();
		}
	}
	platform::audio::Pause();
}

void Emulator::Free()
{
	platform::Dbg("emulator free\n");

	m_drawer.Free();
	m_game.Free();

	platform::Free();
}

void Emulator::ProduceAudioSample(float & sample, int & count)
{
	const float samplesPerClock = (float)k_audioFrequencyHz / (float)k_mcuClockFreqHz;
	const float samplesProduced = samplesPerClock > 1 ? (float)(int(samplesPerClock)) : 1;

	for (; m_fraction < 1.f; m_fraction += samplesPerClock)
	{
		u16 input = platform::input::GetInputMask() | m_autoInput;
		m_game.GetPCB()->SetInput(input);
		m_game.GetPCB()->Clock();
	}
	m_fraction -= samplesProduced;

	sample = m_game.GetPCB()->GetBuzzer()->GetSample();
	count = (int)samplesProduced;
}

void Emulator::UpdateLayoutSwitch()
{
	int layoutId = m_drawer.GetLayoutId();
	bool isFlipped = m_drawer.IsFlipped();

	if (platform::input::IsLayoutSwitchRequested())
	{
		if (++layoutId > int(m_game.GetLayouts().size()) - 1) layoutId = 0;
	}

	if (platform::input::IsLayoutFlipRequested())
	{
		isFlipped ^= true;
	}

	if (layoutId != m_drawer.GetLayoutId() || isFlipped != m_drawer.IsFlipped())
	{
		m_drawer.Free();
		m_drawer.Init(layoutId, isFlipped);
		m_game.GetPCB()->SetInputFlipped(isFlipped);
	}
}

void Emulator::UpdateBatteryLevel()
{
	float batteryLevel = platform::other::GetBatteryLevel();
	m_game.GetPCB()->GetLCD()->SetBatteryLevel(batteryLevel);
}

void Emulator::UpdateAutoPlay()
{
	//	3.2.1 - left up egg -> 1.1.1
	//	2.0.0 - left down egg -> 1.0.0
	//	8.2.1 - right up egg -> 1.3.1
	//	0.3.1 - right down egg -> 1.3.0

	static struct { int h0, o0, s0; int h1, o1, s1; u16 mask; u32 time = 0; } segments[]
	{
		{1, 3, 2, 1, 1, 1, platform::input::MASK_LEFT_UP},
		{0, 2, 0, 0, 1, 0, platform::input::MASK_LEFT_DOWN},
		{1, 8, 2, 1, 1, 3, platform::input::MASK_RIGHT_UP},
		{1, 0, 3, 0, 1, 3, platform::input::MASK_RIGHT_DOWN} 
	};

	int indexOfOldest = -1;
	u32 timeOfOldest = 0xFFFFFFFF;

	for (int i = 0; i < 4; ++i)
	{
		auto& segment = segments[i];
		bool isSegmentActive = m_game.GetPCB()->GetLCD()->GetSegmentState(segment.h0, segment.o0, segment.s0);

		if (isSegmentActive != (segment.time != 0))
		{
			segment.time = isSegmentActive ? platform::other::GetTime() : 0;
		}

		if (segment.time != 0 && segment.time < timeOfOldest)
		{
			timeOfOldest = segment.time;
			indexOfOldest = i;
		}
	}

	m_autoInput = 0;
	if (indexOfOldest != -1 && !m_game.GetPCB()->GetLCD()->GetSegmentState(segments[indexOfOldest].h1, segments[indexOfOldest].o1, segments[indexOfOldest].s1))
	{
		m_autoInput = segments[indexOfOldest].mask;
	}
}
