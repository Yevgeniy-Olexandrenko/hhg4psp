#include <algorithm>
#include <cmath>
#include "svg.h"
#include "Graphics.h"

Graphics::Graphics(const std::string& title, Type type, int x, int y, int w, int h)
	: m_type(type)
	, m_title(title)
	, m_textureSpace(nullptr)
	, m_x(x)
	, m_y(y)
	, m_w(w)
	, m_h(h)
{
}

Graphics::Graphics(const std::string& title, const SegmentShapes& shapes, float svgScale)
	: m_type(Type::SEGMENT)
	, m_title(title)
	, m_textureSpace(nullptr)
{
	float x0 = shapes[0]->bounds[0];
	float y0 = shapes[0]->bounds[1];
	float x1 = shapes[0]->bounds[2];
	float y1 = shapes[0]->bounds[3];

	for (size_t i = 1; i < shapes.size(); ++i)
	{
		x0 = std::min(x0, shapes[i]->bounds[0]);
		y0 = std::min(y0, shapes[i]->bounds[1]);
		x1 = std::max(x1, shapes[i]->bounds[2]);
		y1 = std::max(y1, shapes[i]->bounds[3]);
	}

	m_x = int(x0 * svgScale);
	m_y = int(y0 * svgScale);
	m_w = int(x1 * svgScale) - m_x + 1;
	m_h = int(y1 * svgScale) - m_y + 1;

	m_flags = 0x00;
	m_flip_x = m_x;
}

void Graphics::ComputeFlip(int displayW, int displayH, const Config::Property::DoNotFlip& doNotFlip)
{
	if (m_type == Type::SEGMENT)
	{
		m_flip_x = displayW - m_x - m_w;

		int segMinX = m_x, segMaxX = m_x + m_w - 1;
		int segMinY = m_y, segMaxY = m_y + m_h - 1;

		for (auto& region : doNotFlip)
		{
			int regW = int(std::round(displayW * region.m_w));
			int regH = int(std::round(displayH * region.m_h));

			int regMinX = int(std::round(displayW * region.m_x));
			int regMinY = int(std::round(displayH * region.m_y));

			int regMaxX = regMinX + regW - 1;
			int regMaxY = regMinY + regH - 1;

			if (segMinX >= regMinX && segMaxX <= regMaxX && segMinY >= regMinY && segMaxY <= regMaxY)
			{
				int regionFlipX = displayW - regMinX - regW;
				m_flip_x = regionFlipX + regW - (m_flip_x - regionFlipX) - m_w;
				m_flags |= SEGMENT_DO_NOT_FLIP;
				return;
			}
		}
	}
}

Graphics::Type Graphics::GetType() const
{
	return m_type;
}

int Graphics::GetFlags() const
{
	return m_flags;
}

int Graphics::GetSegmentId() const
{
	if (m_title.size() == 5 && m_title[1] == '.' && m_title[3] == '.')
	{
		unsigned char o = m_title[0] - '0';	//	0 - 8
		unsigned char s = m_title[2] - '0';	//	0 - 3
		unsigned char h = m_title[4] - '0';	//	0 - 1
		return s | h << 2 | o << 4;
	}
	return 0xFF;
}

const std::string& Graphics::GetTitle() const
{
	return m_title;
}

void Graphics::SetTextureSpace(const TextureSpace* textureSpace)
{
	m_textureSpace = textureSpace;
}

const TextureSpace* Graphics::GetTextureSpace() const
{
	return m_textureSpace;
}

int Graphics::GetX() const
{
	return m_x;
}

int Graphics::GetY() const
{
	return m_y;
}

int Graphics::GetW() const
{
	return m_w;
}

int Graphics::GetH() const
{
	return m_h;
}

int Graphics::GetArea() const
{
	return m_w * m_h;
}

int Graphics::GetFlipX() const
{
	return m_flip_x;
}

bool SortGraphicsBySize::operator()(const Graphics* lhs, const Graphics* rhs) const
{
	return 
		lhs->GetW() == rhs->GetW()
		? lhs->GetH() > rhs->GetH()
		: lhs->GetW() > rhs->GetW();
}

bool SortGraphicsByType::operator()(const Graphics * lhs, const Graphics * rhs) const
{
	return
		lhs->GetType() == Graphics::Type::SEGMENT && rhs->GetType() == Graphics::Type::SEGMENT
		? lhs->GetSegmentId() < rhs->GetSegmentId()
		: lhs->GetType() < rhs->GetType();
}
