#include <algorithm>
#include "Config.h"
#include "Yaml.hpp"
#include "magic_enum.hpp"

Config::Config(const std::string & filePath)
	: m_isValid(false)
{
	ParsePathAndName(filePath);
	try
	{
		Yaml::Node root;
		Yaml::Parse(root, filePath.c_str());

		if (root.IsMap() && root.Size() == 4)
		{
			ParseMetaSection(root["meta"]);
			ParseHardwareSection(root["hardware"]);
			ParseLayoutSection(root["layouts"]);
			ParsePropertySection(root["property"]);

			m_isValid = true;
		}
	}
	catch (const Yaml::Exception e)
	{}
}

bool Config::IsValid() const
{
	return m_isValid;
}

const std::string & Config::GetName() const
{
	return m_name;
}

const std::string & Config::GetPath() const
{
	return m_path;
}

const Config::Meta & Config::GetMeta() const
{
	return m_meta;
}

const Config::Hardware & Config::GetHardware() const
{
	return m_hardware;
}

const Config::Layouts & Config::GetLayouts() const
{
	return m_layouts;
}

const Config::Property & Config::GetProperty() const
{
	return m_property;
}

std::string Config::GetFilePath(const std::string & file) const
{
	return m_path + file;
}

void Config::ParsePathAndName(const std::string & filePath)
{
	int a = filePath.find_last_of('/');
	int b = filePath.find_last_of('.');

	m_path = filePath.substr(0, a + 1);
	m_name = filePath.substr(a + 1, b - a - 1);
}

void Config::ParseMetaSection(Yaml::Node & node)
{
	if (node.IsMap() && node.Size() == 5)
	{
		m_meta.m_version = node["version"].As<int>();
		m_meta.m_company = node["company"].As<std::string>();
		m_meta.m_label = node["label"].As<std::string>();
		m_meta.m_model = node["model"].As<std::string>();
		m_meta.m_year = node["year"].As<int>();
	}
}

void Config::ParseHardwareSection(Yaml::Node & node)
{
	if (node.IsMap() && node.Size() == 4)
	{
		m_hardware.m_mcu = magic_enum::enum_cast<Hardware::mcu_t>(node["mcu"].As<std::string>()).value();
		m_hardware.m_rom = node["rom"].As<std::string>();
		m_hardware.m_lcd = node["lcd"].As<std::string>();
		ParseHardwareInputSection(node["input"]);
	}
}

void Config::ParseLayoutSection(Yaml::Node & node)
{
	if (node.IsSequence())
	{
		m_layouts.clear();
		for (size_t index = 0; index < std::min(node.Size(), size_t(0x0F)); ++index)
		{
			Yaml::Node & layoutNode = node[index];
			if (layoutNode.IsMap() && layoutNode.Size() == 3)
			{
				Layout layout;

				Yaml::Node & displayNode = layoutNode["display"];
				ParseLayoutRegion(displayNode, layout.m_display);

				Yaml::Node & backgroundNode = layoutNode["background"];
				ParseLayoutRegion(backgroundNode, layout.m_background);
				layout.m_background.m_file = backgroundNode["file"].As<std::string>();

				Yaml::Node & foregroundNode = layoutNode["foreground"];
				ParseLayoutRegion(foregroundNode, layout.m_foreground);
				layout.m_foreground.m_file = foregroundNode["file"].As<std::string>();

				m_layouts.push_back(layout);
			}
		}
	}
}

void Config::ParsePropertySection(Yaml::Node & node)
{
	if (node.IsMap())
	{
		Yaml::Node& regionsNode = node["do_not_flip"];
		if (regionsNode.IsSequence() && !m_layouts.empty())
		{
			const Region & baseLayoutDisplay = m_layouts[0].m_display;
			for (size_t index = 0; index < regionsNode.Size(); ++index)
			{
				Region regionForBaseLayout;
				ParseLayoutRegion(regionsNode[index], regionForBaseLayout);

				m_property.m_doNotFlip.push_back({
					float(regionForBaseLayout.m_x) / float(baseLayoutDisplay.m_w),
					float(regionForBaseLayout.m_y) / float(baseLayoutDisplay.m_h),
					float(regionForBaseLayout.m_w) / float(baseLayoutDisplay.m_w),
					float(regionForBaseLayout.m_h) / float(baseLayoutDisplay.m_h),
				});
			}
		}
	}
}

void Config::ParseHardwareInputSection(Yaml::Node & node)
{
	if (node.IsSequence())
	{
		m_hardware.m_inputs.clear();
		for (size_t index = 0; index < node.Size(); ++index)
		{
			Yaml::Node & inputNode = node[index];
			if (inputNode.IsMap() && inputNode.Size() == 3)
			{
				auto type = magic_enum::enum_cast<Hardware::Input::type_t>(inputNode["type"].As<std::string>());
				auto input = magic_enum::enum_cast<Hardware::Input::input_t>(inputNode["input"].As<std::string>());
				auto output = magic_enum::enum_cast<Hardware::Input::output_t>(inputNode["output"].As<std::string>());

				if (type.has_value() && input.has_value() && output.has_value())
				{
					m_hardware.m_inputs.push_back(Hardware::Input({ type.value(), input.value(), output.value() }));
				}
			}
		}
	}
}

void Config::ParseLayoutRegion(Yaml::Node & node, Region & region)
{
	if (node.IsMap() && node.Size() >= 4)
	{
		region.m_x = node["x"].As<int>();
		region.m_y = node["y"].As<int>();
		region.m_w = node["w"].As<int>();
		region.m_h = node["h"].As<int>();
	}
}
