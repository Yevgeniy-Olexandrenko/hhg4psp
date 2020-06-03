#include "commons.h"
#include "lcd.h"

LCD::LCD()
	: m_changes(0)
	, m_hasNewFrame(false)
	, m_maxLevel(1.0f)
	, m_latency(0.5f)
{
	memset(m_segForRender, 0, sizeof(m_segForRender));
	memset(m_segForStore, 0, sizeof(m_segForStore));
	memset(m_segLevel, 0, sizeof(m_segLevel));
}

void LCD::SetBatteryLevel(float level)
{
	m_maxLevel = clamp(sqrtf(level), 0.f, 1.f);
}

void LCD::Write(int o, u08 s)
{
	if (o == 0)
	{
		m_changes = 0;
	}

	m_changes |= m_segForStore[o] ^ s;
	m_segForStore[o] = s;

	if (o == k_mcuLcdOCount - 1)
	{
		UpdateLevels();
	}
}

bool LCD::HasNewFrame() const
{
	bool value = m_hasNewFrame;
	m_hasNewFrame = false;
	return value;
}

float LCD::GetSegmentLevel(int h, int o, int s) const
{
	return m_segLevel[h][o][s];
}

bool LCD::GetSegmentState(int h, int o, int s) const
{
	int sbit = h << 2 | s;
	return (m_segForRender[o] >> sbit & 1) != 0;
}

void LCD::UpdateLevels()
{
	if (m_changes == 0)
	{
		memcpy(m_segForRender, m_segForStore, sizeof(m_segForRender));
	}
	float step = k_lcdFrameTime * lerp(k_lcdLatencyCoef0, k_lcdLatencyCoef1, m_latency);
	for (int h = 0; h < k_mcuLcdHCount; ++h)
	{
		for (int o = 0; o < k_mcuLcdOCount; ++o)
		{
			for (int s = 0; s < k_mcuLcdSCount; ++s)
			{
				int sbit = h << 2 | s;
				m_segLevel[h][o][s] = clamp(m_segLevel[h][o][s] + (m_segForRender[o] >> sbit & 1 ? step : -step), 0.f, m_maxLevel);
			}
		}
	} 
	m_hasNewFrame = true;
}
