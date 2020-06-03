#pragma once

#include <string>
#include <vector>
#include "zip.h"
#include "Config.h"

struct NSVGimage;
struct NSVGrasterizer;
class  LayoutBuilder;

class Builder
{
public:
	Builder(const std::string & configFilePath);
	~Builder();

	bool Build();

private:
	void InitLayouts();
	void BuildLayouts();
	void Write();

	void WriteHardware(zipFile & zip);
	void WriteSoftware(zipFile & zip);
	void WriteLayouts (zipFile & zip);
	void WriteTextures(zipFile & zip);

private:
	Config m_config;

	NSVGimage * m_svg;
	NSVGrasterizer * m_rasterizer;

	std::vector<LayoutBuilder *> m_layoutBuilders;
	bool m_ok;
};
