#include <map>
#include <algorithm>
#include "svg.h"
#include "Builder.h"
#include "LayoutBuilder.h"
#include "Writer.h"

Builder::Builder(const std::string & configFilePath)
	: m_config(configFilePath)
	, m_svg(nullptr)
	, m_rasterizer(nullptr)
	, m_ok(m_config.IsValid())
{
}

Builder::~Builder()
{
	nsvgDeleteRasterizer(m_rasterizer);
	nsvgDelete(m_svg);

	m_svg = nullptr;
	m_rasterizer = nullptr;

	for (auto layout : m_layoutBuilders) delete layout;
	m_layoutBuilders.clear();
}

bool Builder::Build()
{
	if (m_ok)
	{
		InitLayouts();
		BuildLayouts();
		Write();
	}
	return m_ok;
}

void Builder::InitLayouts()
{
	std::string filePath = m_config.GetFilePath(m_config.GetHardware().m_lcd);
	if (m_svg = nsvgParseFromFile(filePath.c_str(), "px", 96.0f))
	{
		m_rasterizer = nsvgCreateRasterizer();

		for (int id = 0; id < int(m_config.GetLayouts().size()); ++id)
		{
			auto layoutBuilder = new LayoutBuilder(m_config, id, m_svg, m_rasterizer);
			m_layoutBuilders.push_back(layoutBuilder);
		}
		return;
	}
	m_ok = false;
}

void Builder::BuildLayouts()
{
	for (auto layout : m_layoutBuilders)
	{
		if (m_ok)
		{
			m_ok &= layout->Build();

			if (m_ok)
			{
				std::string t_png = "games/_" + m_config.GetName() + std::to_string(layout->GetId()) + "t.png";
				std::ofstream t_png_stream(t_png, std::ofstream::binary);
				layout->WriteTexture(t_png_stream, LayoutBuilder::TextureType::TRANSPARENT);
				t_png_stream.close();

				std::string o_png = "games/_" + m_config.GetName() + std::to_string(layout->GetId()) + "o.png";
				std::ofstream o_png_stream(o_png, std::ofstream::binary);
				layout->WriteTexture(o_png_stream, LayoutBuilder::TextureType::OPAQUE);
				o_png_stream.close();
			}
		}
	}
}

void Builder::Write()
{
	if (zipFile zip = zipOpen(("emulator/" + m_config.GetName() + ".hhg").c_str(), APPEND_STATUS_CREATE))
	{
		WriteHardware(zip);
		WriteSoftware(zip);
		WriteLayouts (zip);
		WriteTextures(zip);

		zipClose(zip, m_config.GetMeta().m_model.c_str());
		return;
	}
	m_ok = false;
}

void Builder::WriteHardware(zipFile & zip)
{
	if (m_ok)
	{
		std::ostringstream stream;

		//	Meta information
		Writer::Write(stream, m_config.GetMeta().m_version);
		Writer::Write(stream, m_config.GetMeta().m_company);
		Writer::Write(stream, m_config.GetMeta().m_label);
		Writer::Write(stream, m_config.GetMeta().m_model);
		Writer::Write(stream, m_config.GetMeta().m_year);

		//	MCU type, LCD info, input config
		Writer::Write(stream, u08(m_config.GetHardware().m_mcu));
		Writer::Write(stream, u08(m_layoutBuilders[0]->GetNumberOfSegments()));
		Writer::Write(stream, u08(m_config.GetHardware().m_inputs.size()));
		for (const auto & input : m_config.GetHardware().m_inputs)
		{
			Writer::Write(stream, u08(input.m_type));
			Writer::Write(stream, u08(input.m_input));
			Writer::Write(stream, u08(input.m_output));
		}

		auto data = stream.str();

		zip_fileinfo zipFileInfo;
		if (zipOpenNewFileInZip(zip, "hardware", &zipFileInfo, 0, 0, 0, 0, "HARDWARE", Z_DEFLATED, Z_BEST_COMPRESSION) == ZIP_OK)
		{
			if (zipWriteInFileInZip(zip, data.data(), data.size()) == ZIP_OK)
			{
				zipCloseFileInZip(zip);
				return;
			}
		}
		m_ok = false;
	}
}

void Builder::WriteSoftware(zipFile & zip)
{
	if (m_ok)
	{
		std::ostringstream stream;
		std::ifstream romFile(m_config.GetFilePath(m_config.GetHardware().m_rom), std::ifstream::binary);
		Writer::Write(stream, romFile);
		romFile.close();

		auto data = stream.str();

		zip_fileinfo zipFileInfo;
		if (zipOpenNewFileInZip(zip, "software", &zipFileInfo, 0, 0, 0, 0, "SOFTWARE", Z_DEFLATED, Z_BEST_COMPRESSION) == ZIP_OK)
		{
			if (zipWriteInFileInZip(zip, data.data(), data.size()) == ZIP_OK)
			{
				zipCloseFileInZip(zip);
				return;
			}
		}
		m_ok = false;
	}
}

void Builder::WriteLayouts(zipFile & zip)
{
	if (m_ok)
	{
		std::ostringstream stream;
		Writer::Write(stream, u08(m_layoutBuilders.size()));
		for (auto layoutBuilder : m_layoutBuilders)
		{
			layoutBuilder->WriteLayout(stream);
		}

		auto data = stream.str();

		zip_fileinfo zipFileInfo;
		if (zipOpenNewFileInZip(zip, "layouts", &zipFileInfo, 0, 0, 0, 0, "LAYOUTS", Z_DEFLATED, Z_BEST_COMPRESSION) == ZIP_OK)
		{
			if (zipWriteInFileInZip(zip, data.data(), data.size()) == ZIP_OK)
			{
				zipCloseFileInZip(zip);
				return;
			}
		}
		m_ok = false;
	}
}

void Builder::WriteTextures(zipFile & zip)
{
	auto WriteTexture = [&](LayoutBuilder * layoutBuilder, LayoutBuilder::TextureType type)
	{
		if (m_ok)
		{
			std::ostringstream stream;
			layoutBuilder->WriteTexture(stream, type);

			auto data = stream.str();
			auto file = "texture" + std::to_string(layoutBuilder->GetId()) + "to"[size_t(type)];

			zip_fileinfo zipFileInfo;
			if (zipOpenNewFileInZip(zip, file.c_str(), &zipFileInfo, 0, 0, 0, 0, "TEXTURE", Z_DEFLATED, Z_BEST_COMPRESSION) == ZIP_OK)
			{
				if (zipWriteInFileInZip(zip, data.data(), data.size()) == ZIP_OK)
				{
					zipCloseFileInZip(zip);
					return;
				}
			}
			m_ok = false;
		}
	};

	for (auto layoutBuilder : m_layoutBuilders)
	{
		WriteTexture(layoutBuilder, LayoutBuilder::TextureType::TRANSPARENT);
		WriteTexture(layoutBuilder, LayoutBuilder::TextureType::OPAQUE);
	}
}
