#pragma once

#include "platform.h"

class Layout
{
public:

	////////////////////////////////////////////////////////////////////////////

	class Asset
	{
	public:
		Asset();
		~Asset();

		void Bind(u08* data, u32 size);
		void Load();
		void Free();

	private:
		u08* m_data;
		u32  m_size;
		platform::video::PixelFormat m_format;
		platform::video::Texture* m_texture;
		friend class Layout;
	};

	simple_safe_enum(AssetType,
		TRANSPARENT,
		OPAQUE,
		count,

		FRAME = OPAQUE,
		FOREGROUND = TRANSPARENT,
		BACKGROUND = OPAQUE,
		SEGMENT = TRANSPARENT
	);

	///////////////////////////////////////////////////////////////////////////////

	struct Rect
	{
		s16 m_x, m_y;
		u16 m_w, m_h;

		Rect();
		
		s16 X0() const;	s16 X1() const;
		s16 Y0() const;	s16 Y1() const;

		void Read(std::istream& stream);
		platform::video::Rect GetDstRect() const;
	};
	
	struct Tile : public Rect
	{
		u16 m_tx, m_ty;
		
		Tile();
		Tile(std::istream& stream);

		void Read(std::istream& stream);
		platform::video::Rect GetSrcRect() const;
	};
	
	struct Segment : public Tile
	{
		u08 m_id;
		u08 m_flags;
		u16 m_flip_x;
		
		Segment();
		Segment(std::istream& stream);

		void Read(std::istream& stream);
		platform::video::Rect GetDstRect(bool isFlipped) const;
		
		u08  GetId() const;
		bool IsFlipAllowed() const;
	};

	class Layer
	{
	public:
		Layer();

		void AddTile(const Tile& tile);
		int  GetTilesCount() const;

		const Rect  GetRect() const;
		const Tile& GetTile(int index) const;
		
	private:
		std::vector<Tile> m_tiles;
		s16 m_x0, m_x1, m_y0, m_y1;
	};

	typedef std::vector<Segment> Segments;

	simple_safe_enum(LayerType,
		FRAME,
		FOREGROUND,
		BACKGROUND,
		SEGMENT,
		count
	);

	///////////////////////////////////////////////////////////////////////////////

public:
	void Load(std::istream& stream);
	void LoadAssets();
	void FreeAssets();

	Asset& GetAsset(AssetType type);
	const Asset& GetAsset(AssetType type) const;
	platform::video::Texture* GetAssetTexture(AssetType type) const;
	platform::video::PixelFormat GetAssetPixelFormat(AssetType type) const;

	const Segment& GetSegment(u08 id) const;
	const Layer& GetLayer(LayerType type) const;
	const Rect& GetDisplay() const;

	u32  GetDisplayBgColor() const;
	bool IsFlippingSupported() const;

private:
	platform::video::PixelFormat ReadPixelFormat(std::istream& stream) const;;

private:
	Asset m_assets[AssetType::count];
	Layer m_layers[LayerType::count - 1];

	Rect m_display;
	u32  m_bgColor;

	Segments m_segments;
	bool m_hasFlip;
};

typedef std::vector<Layout> Layouts;
