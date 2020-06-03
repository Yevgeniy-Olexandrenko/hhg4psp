#pragma once

#include <cstdint>

using u32  = uint32_t;
using u16  = uint16_t;
using s16  = int16_t;
using u08  = uint8_t;

struct Region
{
	s16 m_x, m_y;
	u16 m_w, m_h;
};

struct NormalizedRegion
{
	float m_x, m_y;
	float m_w, m_h;
};