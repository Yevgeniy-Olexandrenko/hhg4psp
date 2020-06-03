#pragma once

#include "Image.h"
#include "TextureSpace.h"

class Texture
{
public:
	Texture(int w, int h, bool hasAlpha);
	~Texture();

	int GetW() const;
	int GetH() const;

	Image * GetImage() const;
	TextureSpace * GetSpace() const;

private:
	Image * m_image;
	TextureSpace * m_space;
	
};