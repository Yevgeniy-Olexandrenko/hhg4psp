#pragma once

#include "Types.h"

struct NSVGimage;

class Display
{
public:
	Display(const Region& region, const NSVGimage* svg);

	s16 GetX() const;
	s16 GetY() const;
	u16 GetW() const;
	u16 GetH() const;
	u32 GetBgColor() const;

	bool  IsValid() const;
	float GetSvgScale() const;

private:
	s16 m_x, m_y;
	u16 m_w, m_h;
	u32 m_bgColor;

	bool  m_isValid;
	float m_svgScale;
	
};
