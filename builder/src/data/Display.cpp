#include <algorithm>
#include <cmath>
#include "svg.h"
#include "Display.h"

Display::Display(const Region& region, const NSVGimage* svg)
	: m_x(region.m_x)
	, m_y(region.m_y)
	, m_bgColor(0)
	, m_isValid(false)
{
	m_bgColor = svg->shapes->fill.color;

	float svgScaleW = region.m_w / svg->width;
	float svgScaleH = region.m_h / svg->height;
	m_svgScale = std::min(svgScaleW, svgScaleH);

	m_w = int(std::round(m_svgScale * svg->width));
	m_h = int(std::round(m_svgScale * svg->height));

	m_isValid |= (m_w == region.m_w && std::abs(m_h - region.m_h) <= 1);
	m_isValid |= (m_h == region.m_h && std::abs(m_w - region.m_w) <= 1);
}

s16 Display::GetX() const
{
	return m_x;
}

s16 Display::GetY() const
{
	return m_y;
}

bool Display::IsValid() const
{
	return m_isValid; 
}

float Display::GetSvgScale() const
{
	return m_svgScale; 
}

u16 Display::GetW() const
{
	return m_w; 
}

u16 Display::GetH() const
{
	return m_h; 
}

u32 Display::GetBgColor() const
{
	return m_bgColor;
}
