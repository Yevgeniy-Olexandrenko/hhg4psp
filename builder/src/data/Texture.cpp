#include "Texture.h"

Texture::Texture(int w, int h, bool hasAlpha)
	: m_image(new Image(w, h, hasAlpha))
	, m_space(new TextureSpace(0, 0, w, h))
{
}

Texture::~Texture()
{
	delete m_image;
	delete m_space;
}

int Texture::GetW() const
{
	return m_image->GetW();
}

int Texture::GetH() const
{
	return m_image->GetH();
}

Image * Texture::GetImage() const
{
	return m_image;
}

TextureSpace * Texture::GetSpace() const
{
	return m_space;
}
