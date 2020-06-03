#pragma once

#include <array>
#include "Config.h"
#include "Image.h"

struct NSVGimage;
struct NSVGrasterizer;

class Display;
class Texture;
class Graphics;

class LayoutBuilder
{
public:
	enum class TextureType
	{
		TRANSPARENT,
		OPAQUE
	};

	LayoutBuilder(const Config & config, int id, NSVGimage * svg, NSVGrasterizer * rasterizer);
	~LayoutBuilder();

	bool Build();
	void WriteLayout(std::ostream & stream);
	void WriteTexture(std::ostream & stream, TextureType type);

	int GetId() const;
	int GetNumberOfSegments() const;

private:
	void CollectSegments();
	void CollectGraphics();
	void CreateTextures();
	void PlaceOnTextures();
	void RenderOnTextures();
	void RenderGraphicsOnTexture(const Graphics* graphics, Texture * texture);
	void RenderSegmentsOnTexture(const Graphics* graphics, Texture * texture);
	void SplitRegion(const Region & region, int depthV, int depthH, std::vector<Region> & output);
	void ReadAssets();

private:
	using GraphicsList = std::vector<Graphics *>;

	const Config & m_config;
	const int m_id;

	NSVGimage * m_svg;
	NSVGrasterizer * m_rasterizer;

	float m_t_fillDensity;
	float m_o_fillDensity;

	int m_t_requiredArea;
	int m_o_requiredArea;
	int m_t_distributedArea;
	int m_o_distributedArea;

	int m_segmentsNumber;

	GraphicsList m_t_graphics; // segments + transparent foreground
	GraphicsList m_o_graphics; // background + opaque foreground parts

	Image::Format m_t_textureFormat;
	Image::Format m_o_textureFormat;

	Display * m_display;
	Texture * m_t_texture;
	Texture * m_o_texture;

	std::array<Image *, 3> m_images;
};

