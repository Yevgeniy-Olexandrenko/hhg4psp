#include <algorithm>
#include "TextureSpace.h"

TextureSpace::TextureSpace(int x, int y, int w, int h)
	: m_x(x), m_y(y), m_w(w), m_h(h)
{
}

TextureSpace::~TextureSpace()
{
	m_spaces.clear();
}

int TextureSpace::GetX() const
{
	return m_x;
}

int TextureSpace::GetY() const
{
	return m_y;
}

int TextureSpace::GetW() const
{
	return m_w;
}

int TextureSpace::GetH() const
{
	return m_h;
}

int TextureSpace::GetLargestFreeArea() const
{
	if (IsEmpty()) return m_w * m_h;

	int largestFreeArea = 0;
	for (const TextureSpace & space : m_spaces)
	{
		largestFreeArea = std::max(space.GetLargestFreeArea(), largestFreeArea);
	}
	return largestFreeArea;
}

bool TextureSpace::IsEmpty() const
{
	return m_spaces.empty();
}

TextureSpace * TextureSpace::CutOffSpace(int w, int h)
{
	if (IsEmpty())
	{
		if (w <= m_w && h <= m_h)
		{
			TextureSpace rSpaceA(m_x + w, m_y, m_w - w, m_h);
			TextureSpace dSpaceA(m_x, m_y + h, w, m_h - h);

			TextureSpace rSpaceB(m_x + w, m_y, m_w - w, h);
			TextureSpace dSpaceB(m_x, m_y + h, m_w, m_h - h);

			int areaDeltaA = std::abs(rSpaceA.GetLargestFreeArea() - dSpaceA.GetLargestFreeArea());
			int areaDeltaB = std::abs(rSpaceB.GetLargestFreeArea() - dSpaceB.GetLargestFreeArea());

			if (areaDeltaA > areaDeltaB)
			{
				m_spaces.push_back(rSpaceA);
				m_spaces.push_back(dSpaceA);
			}
			else
			{
				m_spaces.push_back(rSpaceB);
				m_spaces.push_back(dSpaceB);
			}
			m_w = w;
			m_h = h;
			return this;
		}
	}
	else
	{
		std::sort(m_spaces.begin(), m_spaces.end());
		for (TextureSpace & space : m_spaces)
		{
			if (TextureSpace * result = space.CutOffSpace(w, h))
			{
				return result;
			}
		}
	}
	return nullptr;
}

bool TextureSpace::operator<(const TextureSpace & other) const
{
	return GetLargestFreeArea() > other.GetLargestFreeArea() && m_x < other.m_x && m_y < other.m_y;
}
