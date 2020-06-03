#include <map>
#include <algorithm>
#include "svg.h"
#include "LayoutBuilder.h"
#include "Display.h"
#include "Texture.h"
#include "Graphics.h"
#include "Writer.h"

LayoutBuilder::LayoutBuilder(const Config & config, int id, NSVGimage * svg, NSVGrasterizer * rasterizer)
	: m_config(config)
	, m_id(id)
	, m_svg(svg)
	, m_rasterizer(rasterizer)
	, m_t_fillDensity(0)
	, m_o_fillDensity(0)
	, m_t_requiredArea(0)
	, m_o_requiredArea(0)
	, m_t_distributedArea(0)
	, m_o_distributedArea(0)
	, m_segmentsNumber(0)
	//, t_assets_format(Image::Format4444.WithColorDithering(Image::DitheringNone).WithAlphaDithering(Image::DitheringNone))
	//, o_assets_format(Image::Format5650.WithColorDithering(Image::DitheringNone).WithAlphaDithering(Image::DitheringNone))
	//, segments_format(Image::Format4444.WithColorDithering(Image::DitheringNone).WithAlphaDithering(Image::DitheringNone))
	, m_t_textureFormat(Image::Format(4, 4, 4, 4, Image::DitheringNone, Image::DitheringNone))
	, m_o_textureFormat(Image::Format(5, 6, 5, 0, Image::DitheringNone, Image::DitheringNone))
	, m_display(nullptr)
	, m_t_texture(nullptr)
	, m_o_texture(nullptr)
	, m_images({ nullptr })
{
}

LayoutBuilder::~LayoutBuilder()
{
	for (auto graphics : m_t_graphics) delete graphics;
	m_t_graphics.clear();

	for (auto graphics : m_o_graphics) delete graphics;
	m_o_graphics.clear();

	for (auto image : m_images) delete image;
	m_images.fill(nullptr);

	delete m_display;
	delete m_t_texture;
	delete m_o_texture;
}

bool LayoutBuilder::Build()
{
	printf("Build layout: %d\n", m_id);

	m_display = new Display(m_config.GetLayouts()[m_id].m_display, m_svg);
	if (m_display->IsValid())
	{
		CollectSegments();
		CollectGraphics();

		for (float fillDensity = 1.0f; fillDensity >= 0.9f; fillDensity -= 0.01f)
		{
			if (m_t_distributedArea < m_t_requiredArea) m_t_fillDensity = fillDensity;
			if (m_o_distributedArea < m_o_requiredArea) m_o_fillDensity = fillDensity;

			printf("T-Texture fill density: %d%%\n", int(100 * m_t_fillDensity));
			printf("O-Texture fill density: %d%%\n", int(100 * m_o_fillDensity));

			CreateTextures();
			PlaceOnTextures();

			if (m_t_distributedArea == m_t_requiredArea && m_o_distributedArea == m_o_requiredArea)
			{
				RenderOnTextures();
				return true;
			}
		}
	}
	return false;
}

void LayoutBuilder::WriteLayout(std::ostream & stream)
{
	//	write textures fixel format
	auto WritePixelFormat = [&](Image::Format& format)
	{
		Writer::Write(stream, u08(format.m_bitsR));
		Writer::Write(stream, u08(format.m_bitsG));
		Writer::Write(stream, u08(format.m_bitsB));
		Writer::Write(stream, u08(format.m_bitsA));
	};
	WritePixelFormat(m_t_textureFormat);
	WritePixelFormat(m_o_textureFormat);

	//	write display rect and bg color
	Writer::Write(stream, m_display->GetX());
	Writer::Write(stream, m_display->GetY());
	Writer::Write(stream, m_display->GetW());
	Writer::Write(stream, m_display->GetH());
	Writer::Write(stream, m_display->GetBgColor());

	//	write layers data
	GraphicsList graphicsLists[size_t(Graphics::Type::count)];
	for (auto graphics : m_t_graphics) graphicsLists[size_t(graphics->GetType())].push_back(graphics);
	for (auto graphics : m_o_graphics) graphicsLists[size_t(graphics->GetType())].push_back(graphics);

	for (int t = 0; t < size_t(Graphics::Type::count); t++)
	{
		GraphicsList & graphicsList = graphicsLists[t];

		Writer::Write(stream, u08(t));
		Writer::Write(stream, u08(graphicsList.size()));

		for (auto graphics : graphicsList)
		{
			if (graphics->GetType() == Graphics::Type::SEGMENT)
			{
				Writer::Write(stream, u08(graphics->GetSegmentId()));
				Writer::Write(stream, u08(graphics->GetFlags()));
				Writer::Write(stream, s16(graphics->GetFlipX()));
			}

			Writer::Write(stream, s16(graphics->GetX()));
			Writer::Write(stream, s16(graphics->GetY()));
			Writer::Write(stream, u16(graphics->GetW()));
			Writer::Write(stream, u16(graphics->GetH()));
			Writer::Write(stream, u16(graphics->GetTextureSpace()->GetX()));
			Writer::Write(stream, u16(graphics->GetTextureSpace()->GetY()));
		}
	}
}

void LayoutBuilder::WriteTexture(std::ostream & stream, TextureType type)
{
	if (type == TextureType::TRANSPARENT)
	{
		m_t_texture->GetImage()->Save(stream);
	}
	else if (type == TextureType::OPAQUE)
	{
		m_o_texture->GetImage()->Save(stream);
	}
}

int LayoutBuilder::GetId() const
{
	return m_id;
}

int LayoutBuilder::GetNumberOfSegments() const
{
	return m_segmentsNumber;
}

void LayoutBuilder::CollectSegments()
{
	printf("Collect segments...\n");

	std::map<std::string, SegmentShapes> shapesToSegmentsMap;
	for (NSVGshape* shape = m_svg->shapes; shape != nullptr; shape = shape->next)
	{
		std::string title = shape->title;
		if (!title.empty() && title[1] == '.' && title[3] == '.')
		{
			shapesToSegmentsMap[title].push_back(shape);
		}
	}

	float svgScale = m_display->GetSvgScale();
	for (auto pair : shapesToSegmentsMap)
	{
		Graphics* segment = new Graphics(pair.first, pair.second, svgScale);
		segment->ComputeFlip(m_display->GetW(), m_display->GetH(), m_config.GetProperty().m_doNotFlip);
		m_t_graphics.push_back(segment);
		m_t_requiredArea += segment->GetArea();
		m_segmentsNumber++;
	}
}

void LayoutBuilder::CollectGraphics()
{
	printf("Collect graphics...\n");

	std::vector<Region> regions;

	const auto & br = m_config.GetLayouts()[m_id].m_background;
	const auto & fr = m_config.GetLayouts()[m_id].m_foreground;

	Region frameParts[] =
	{
		{	//	center top part 
			s16(br.m_x), s16(fr.m_y), u16(br.m_w), u16(br.m_y - fr.m_y)
		},
		{	//	center bottom part 
			s16(br.m_x), s16(br.m_y + br.m_h), u16(br.m_w),	u16(fr.m_h - (br.m_y + br.m_h))
		},
		{	//	left part
			s16(fr.m_x), s16(fr.m_y), u16(br.m_x - fr.m_x),	u16(fr.m_h)
		},
		{	//	right  part
			s16(br.m_x + br.m_w), s16(fr.m_y), u16(fr.m_w - (br.m_x + br.m_w)),	u16(fr.m_h)
		}
	};

	regions.clear();
	SplitRegion(frameParts[0], 2, 0, regions);
	SplitRegion(frameParts[1], 2, 0, regions);
	SplitRegion(frameParts[2], 0, 2, regions);
	SplitRegion(frameParts[3], 0, 2, regions);
	for (auto & region : regions)
	{
		m_o_graphics.push_back(new Graphics(fr.m_file, Graphics::Type::FRAME, region.m_x, region.m_y, region.m_w, region.m_h));
		m_o_requiredArea += m_o_graphics.back()->GetArea();
	}

	regions.clear();
	SplitRegion(br, 1, 1, regions);
	for (auto & region : regions)
	{
		m_t_graphics.push_back(new Graphics(fr.m_file, Graphics::Type::FOREGROUND, region.m_x, region.m_y, region.m_w, region.m_h));
		m_t_requiredArea += m_t_graphics.back()->GetArea();
	}

	regions.clear();
	SplitRegion(br, 1, 1, regions);
	for (auto & region : regions)
	{
		m_o_graphics.push_back(new Graphics(br.m_file, Graphics::Type::BACKGROUND, region.m_x, region.m_y, region.m_w, region.m_h));
		m_o_requiredArea += m_o_graphics.back()->GetArea();
	}
}

void LayoutBuilder::CreateTextures()
{
	printf("Create textures...\n");

	auto NextPow2 = [](unsigned int v) -> unsigned int
	{
		v--; v |= v >> 1; v |= v >> 2; v |= v >> 4;	v |= v >> 8; v |= v >> 16;
		return v + 1;
	};

	if (m_t_texture) delete m_t_texture;
	auto t_enlargedArea = float(m_t_requiredArea / m_t_fillDensity);
	auto t_size = NextPow2(int(0.5f + std::sqrtf(t_enlargedArea)));
	m_t_texture = new Texture(t_size, t_size, true);

	if (m_o_texture) delete m_o_texture;
	auto o_enlargedArea = float(m_o_requiredArea / m_o_fillDensity);
	auto o_size = NextPow2(int(0.5f + std::sqrtf(o_enlargedArea)));
	m_o_texture = new Texture(o_size, o_size, false);
}

void LayoutBuilder::PlaceOnTextures()
{
	printf("Place on textures...\n");

	auto Place = [&](GraphicsList & graphicsList, Texture * texture, int & distributedArea)
	{
		std::sort(graphicsList.begin(), graphicsList.end(), SortGraphicsBySize());
		for (auto graphics : graphicsList)
		{
			TextureSpace * textureSpace = texture->GetSpace();
			TextureSpace * allocatedSpace = textureSpace->CutOffSpace(graphics->GetW(), graphics->GetH());
			graphics->SetTextureSpace(allocatedSpace);

			if (allocatedSpace)
				distributedArea += graphics->GetArea();
			else
				printf("Could not place on texture: %d %d %d %d - %s\n", graphics->GetX(), graphics->GetY(), graphics->GetW(), graphics->GetH(), graphics->GetTitle().c_str());
		}
	};

	m_t_distributedArea = 0;
	m_o_distributedArea = 0;

	Place(m_t_graphics, m_t_texture, m_t_distributedArea);
	Place(m_o_graphics, m_o_texture, m_o_distributedArea);
}

void LayoutBuilder::RenderOnTextures()
{
	printf("Render on textures...\n");

	auto Render = [&](GraphicsList & graphicsList, Texture * texture)
	{
		for (auto graphics : graphicsList)
		{
			if (graphics->GetType() == Graphics::Type::SEGMENT)
				RenderSegmentsOnTexture(graphics, texture);
			else
				RenderGraphicsOnTexture(graphics, texture);
		}
	};

	ReadAssets();
	Render(m_t_graphics, m_t_texture);
	Render(m_o_graphics, m_o_texture);
}

void LayoutBuilder::RenderGraphicsOnTexture(const Graphics * graphics, Texture * texture)
{
	if (const TextureSpace* textureSpace = graphics->GetTextureSpace())
	{
		auto & region = graphics->GetType() == Graphics::Type::BACKGROUND
			? m_config.GetLayouts()[m_id].m_background
			: m_config.GetLayouts()[m_id].m_foreground;

		texture->GetImage()->CopyPixels(
			*m_images[size_t(graphics->GetType())],
			graphics->GetX() - region.m_x, graphics->GetY() - region.m_y,
			textureSpace->GetX(), textureSpace->GetY(),
			graphics->GetW(), graphics->GetH()
		);
	}
}

void LayoutBuilder::RenderSegmentsOnTexture(const Graphics * graphics, Texture * texture)
{
	if (const TextureSpace* textureSpace = graphics->GetTextureSpace())
	{
		Image buffer(m_display->GetW(), m_display->GetH(), true);

		for (NSVGshape* shape = m_svg->shapes; shape != nullptr; shape = shape->next)
		{
			shape->flags = (shape->title == graphics->GetTitle() ? NSVG_FLAGS_VISIBLE : 0);
		}
		nsvgRasterize(
			m_rasterizer, m_svg, 0, 0, m_display->GetSvgScale(),
			buffer.GetBytes(), m_display->GetW(), m_display->GetH(), buffer.GetStride()
		);

		buffer.Quantize(m_t_textureFormat, graphics->GetX(), graphics->GetY(), graphics->GetW(), graphics->GetH());
		texture->GetImage()->CopyPixels(
			buffer,
			graphics->GetX(), graphics->GetY(),
			textureSpace->GetX(), textureSpace->GetY(),
			graphics->GetW(), graphics->GetH());
	}
}

void LayoutBuilder::SplitRegion(const Region & region, int depthV, int depthH, std::vector<Region>& output)
{
	if ((depthV == 0 || region.m_w < 8) && (depthH == 0 || region.m_h < 8))
	{
		if (s16(region.m_w) * s16(region.m_h) > 0)
		{
			output.push_back(region);
		}
	}
	else if (depthV > 0)
	{
		Region regionA{ s16(region.m_x), s16(region.m_y), u16(region.m_w >> 1), u16(region.m_h) };
		Region regionB{ s16(region.m_x + (region.m_w >> 1)), s16(region.m_y), u16(region.m_w - (region.m_w >> 1)), u16(region.m_h) };

		SplitRegion(regionA, depthV - 1, depthH, output);
		SplitRegion(regionB, depthV - 1, depthH, output);
	}
	else if (depthH > 0)
	{
		Region regionA{ s16(region.m_x), s16(region.m_y), u16(region.m_w), u16(region.m_h >> 1) };
		Region regionB{ s16(region.m_x), s16(region.m_y + (region.m_h >> 1)), u16(region.m_w), u16(region.m_h - (region.m_h >> 1)) };

		SplitRegion(regionA, depthV, depthH - 1, output);
		SplitRegion(regionB, depthV, depthH - 1, output);
	}
}

void LayoutBuilder::ReadAssets()
{
	const std::string foregroundFile = m_config.GetFilePath(m_config.GetLayouts()[m_id].m_foreground.m_file);
	const std::string backgroundFile = m_config.GetFilePath(m_config.GetLayouts()[m_id].m_background.m_file);

	m_images[size_t(Graphics::Type::FOREGROUND)] = new Image(foregroundFile, m_t_textureFormat);
	m_images[size_t(Graphics::Type::BACKGROUND)] = new Image(backgroundFile, m_o_textureFormat);
	m_images[size_t(Graphics::Type::FRAME)]      = new Image(foregroundFile, m_o_textureFormat);
}
