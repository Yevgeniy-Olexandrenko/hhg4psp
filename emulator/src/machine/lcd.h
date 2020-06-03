#pragma once

#include "constants.h"

class LCD
{
public:
	LCD();

	void  SetBatteryLevel(float level);
	void  Write(int o, u08 s);

	bool  HasNewFrame() const;
	float GetSegmentLevel(int h, int o, int s) const;
	bool  GetSegmentState(int h, int o, int s) const;

private:
	void UpdateLevels();

private:
	u08 m_changes;
	u08 m_segForStore[k_mcuLcdOCount];
	u08 m_segForRender[k_mcuLcdOCount];

	mutable volatile bool m_hasNewFrame;

	float m_maxLevel;
	float m_segLevel[k_mcuLcdHCount][k_mcuLcdOCount][k_mcuLcdSCount];
	float m_latency;
};