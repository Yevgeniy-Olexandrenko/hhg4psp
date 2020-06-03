#pragma once

#include <vector>

class TextureSpace
{
public:
	TextureSpace(int x, int y, int w, int h);
	~TextureSpace();

	int GetX() const;
	int GetY() const;
	int GetW() const;
	int GetH() const;
	int GetLargestFreeArea() const;

	bool IsEmpty() const;
	TextureSpace * CutOffSpace(int w, int h);
	bool operator<(const TextureSpace & other) const;

private:
	int m_x, m_y;
	int m_w, m_h;

	std::vector<TextureSpace> m_spaces;
};
