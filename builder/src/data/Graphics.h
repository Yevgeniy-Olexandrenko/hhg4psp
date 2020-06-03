#pragma once

#include "Config.h"

struct NSVGshape;
class  TextureSpace;
using  SegmentShapes = std::vector<NSVGshape*>;

class Graphics
{
public:
	enum class Type
	{
		FRAME,
		FOREGROUND,
		BACKGROUND,
		SEGMENT,
		count
	};

	enum Flags
	{
		SEGMENT_DO_NOT_FLIP = 1 << 0
	};

	Graphics(const std::string& title, Type type, int x, int y, int w, int h);
	Graphics(const std::string& title, const SegmentShapes& shapes, float svgScale);

	void ComputeFlip(int displayW, int displayH, const Config::Property::DoNotFlip& doNotFlip);

	Type GetType() const;
	int  GetFlags() const;
	int  GetSegmentId() const;
	const std::string & GetTitle() const;

	void SetTextureSpace(const TextureSpace* textureSpace);
	const TextureSpace* GetTextureSpace() const;

	int GetX() const;
	int GetY() const;
	int GetW() const;
	int GetH() const;
	int GetArea() const;
	int GetFlipX() const;

private:
	Type m_type;
	std::string m_title;
	const TextureSpace* m_textureSpace;

	int m_x, m_y;
	int m_w, m_h;
	int m_flip_x;
	int m_flags;
};

struct SortGraphicsBySize
{
	bool operator() (const Graphics* lhs, const Graphics* rhs) const;
};

struct SortGraphicsByType
{
	bool operator() (const Graphics* lhs, const Graphics* rhs) const;
};
