#include "commons.h"
#include "layout.h"

////////////////////////////////////////////////////////////////////////////////

Layout::Asset::Asset()
	: m_data(NULL)
	, m_size(0)
	, m_format(0)
	, m_texture(NULL)
{
}

Layout::Asset::~Asset()
{
	delete[] m_data;
	Free();
}

void Layout::Asset::Bind(u08* data, u32 size)
{
	if (!m_data && data && size)
	{
		m_data = new u08[m_size = size];
		memcpy(m_data, data, m_size);
	}
}

void Layout::Asset::Load()
{
	if (m_data && m_size)
	{
		m_texture = LoadTexture(m_data, m_size, m_format);
	}
}

void Layout::Asset::Free()
{
	platform::video::Free(m_texture);
	m_texture = NULL;
}

////////////////////////////////////////////////////////////////////////////////

Layout::Rect::Rect() : m_x(0), m_y(0), m_w(0), m_h(0) {}

s16 Layout::Rect::X0() const { return m_x; }
s16 Layout::Rect::X1() const { return m_x + m_w - 1; }
s16 Layout::Rect::Y0() const { return m_y; }
s16 Layout::Rect::Y1() const { return m_y + m_h - 1; }

void Layout::Rect::Read(std::istream& stream)
{
	Reader::Read(stream, m_x);
	Reader::Read(stream, m_y);
	Reader::Read(stream, m_w);
	Reader::Read(stream, m_h);
}

platform::video::Rect Layout::Rect::GetDstRect() const
{
	platform::video::Rect rect;
	rect.m_x = m_x; rect.m_y = m_y;
	rect.m_w = m_w; rect.m_h = m_h;
	return rect;
}

Layout::Tile::Tile() : m_tx(0), m_ty(0) {}
Layout::Tile::Tile(std::istream& stream) { Read(stream); }

void Layout::Tile::Read(std::istream& stream)
{
	Rect  ::Read(stream);
	Reader::Read(stream, m_tx);
	Reader::Read(stream, m_ty);
}

platform::video::Rect Layout::Tile::GetSrcRect() const
{
	platform::video::Rect rect;
	rect.m_x = m_tx; rect.m_y = m_ty;
	rect.m_w = m_w;  rect.m_h = m_h;
	return rect;
}

Layout::Segment::Segment() : m_id(0), m_flags(0), m_flip_x(0) {}
Layout::Segment::Segment(std::istream& stream) { Read(stream); }

void Layout::Segment::Read(std::istream& stream)
{
	Reader::Read(stream, m_id);
	Reader::Read(stream, m_flags);
	Reader::Read(stream, m_flip_x);
	Tile  ::Read(stream);
}

platform::video::Rect Layout::Segment::GetDstRect(bool isFlipped) const
{
	platform::video::Rect rect = Tile::GetDstRect();
	if (isFlipped) rect.m_x = m_flip_x;
	return rect;
}

u08 Layout::Segment::GetId() const
{
	return m_id;
}

bool Layout::Segment::IsFlipAllowed() const
{
	return (m_flags & 0x01) == 0;
}

Layout::Layer::Layer()
	: m_x0(INT16_MAX), m_x1(INT16_MIN)
	, m_y0(INT16_MAX), m_y1(INT16_MIN)
{
}

void Layout::Layer::AddTile(const Tile& tile)
{
	if (tile.X0() < m_x0) m_x0 = tile.X0();
	if (tile.X1() > m_x1) m_x1 = tile.X1();
	if (tile.Y0() < m_y0) m_y0 = tile.Y0();
	if (tile.Y1() > m_y1) m_y1 = tile.Y1();
	m_tiles.push_back(tile);
}

int Layout::Layer::GetTilesCount() const
{
	return m_tiles.size();
}

const Layout::Rect Layout::Layer::GetRect() const
{
	Rect rect;
	rect.m_x = m_x0; rect.m_w = m_x1 - m_x0 + 1;
	rect.m_y = m_y0; rect.m_h = m_y1 - m_y0 + 1;
	return rect;
}

const Layout::Tile& Layout::Layer::GetTile(int index) const
{
	return m_tiles[index];
}

////////////////////////////////////////////////////////////////////////////////

void Layout::Load(std::istream& stream)
{
	m_assets[AssetType::TRANSPARENT].m_format = ReadPixelFormat(stream);
	m_assets[AssetType::OPAQUE].m_format = ReadPixelFormat(stream);

	m_display.Read(stream);
	Reader::Read(stream, m_bgColor);

	m_hasFlip = false;
	for(int t = 0; t < LayerType::count; ++t)
	{
		u08 type, count;
		Reader::Read(stream, type);
		Reader::Read(stream, count);
		m_segments.resize(256);

		for (int i = 0; i < count; ++i)
		{
			if (type == LayerType::SEGMENT)
			{
				Segment segment(stream);
				m_segments[segment.GetId()] = segment;
				m_hasFlip |= !segment.IsFlipAllowed();
			}
			else
			{
				Tile tile(stream);
				m_layers[type].AddTile(tile);
			}
		}
	}
}

void Layout::LoadAssets()
{
	for (int i = 0; i < AssetType::count; ++i)
	{
		GetAsset(AssetType(i)).Load();
	}
}

void Layout::FreeAssets()
{
	for (int i = 0; i < AssetType::count; ++i)
	{
		GetAsset(AssetType(i)).Free();
	}
}

Layout::Asset& Layout::GetAsset(AssetType type)
{
	return m_assets[type.underlying()];
}

const Layout::Asset& Layout::GetAsset(AssetType type) const
{
	return m_assets[type.underlying()];
}

platform::video::Texture* Layout::GetAssetTexture(AssetType type) const
{
	return GetAsset(type).m_texture;
}

platform::video::PixelFormat Layout::GetAssetPixelFormat(AssetType type) const
{
	return GetAsset(type).m_format;
}

const Layout::Segment& Layout::GetSegment(u08 id) const
{
	return m_segments[id];
}

const Layout::Layer& Layout::GetLayer(LayerType type) const
{
	return m_layers[type.underlying()];
}

const Layout::Rect& Layout::GetDisplay() const
{
	return m_display;
}

u32 Layout::GetDisplayBgColor() const
{
	return m_bgColor;;
}

bool Layout::IsFlippingSupported() const
{
	return m_hasFlip;
}

platform::video::PixelFormat Layout::ReadPixelFormat(std::istream& stream) const
{
	u32 bits;
	Reader::Read(stream, bits);

	switch (bits)
	{
	/////// A B G R
	case 0x08080808: return platform::video::PixelFormat::ARGB8888;
	case 0x00080808: return platform::video::PixelFormat::RGB888;
	case 0x04040404: return platform::video::PixelFormat::ARGB4444;
	case 0x00050605: return platform::video::PixelFormat::RGB565;
	}
	return platform::video::PixelFormat(0);
}
