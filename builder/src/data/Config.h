#pragma once

#include <string>
#include <vector>
#include "Types.h"

namespace Yaml { class Node; }

class Config
{
public:
	struct Meta
	{
		u32 m_version;
		std::string m_company;
		std::string m_label;
		std::string m_model;
		u16 m_year;
	};

	struct Hardware
	{
		enum mcu_t { sm5a, sm510 };

		struct Input
		{
			enum type_t   { acl, game_a, game_b, time, alarm, left, right, left_up, left_down, right_up, right_down, cheat };
			enum input_t  { gnd, vcc, mcu_r2, mcu_r3, mcu_r4 };
			enum output_t { mcu_acl, mcu_k1, mcu_k2, mcu_k3, mcu_k4, mcu_alpha, mcu_beta };

			type_t   m_type;
			input_t  m_input;
			output_t m_output;
		};

		mcu_t m_mcu;
		std::string m_rom;
		std::string m_lcd;
		std::vector<Input> m_inputs;
	};

	struct Layout
	{
		struct Layer : Region
		{
			std::string m_file;
		};

		Layer  m_background;
		Region m_display;
		Layer  m_foreground;
	};
	using Layouts = std::vector<Layout>;

	struct Property
	{
		using DoNotFlip = std::vector<NormalizedRegion>;
		DoNotFlip m_doNotFlip;
	};

public: 
	Config(const std::string & filePath);
	bool IsValid() const;

	const std::string & GetName() const;
	const std::string & GetPath() const;

	const Meta     & GetMeta() const;
	const Hardware & GetHardware() const;
	const Layouts  & GetLayouts() const;
	const Property & GetProperty() const;

	std::string GetFilePath(const std::string & file) const;

private:
	void ParsePathAndName(const std::string & filePath);

	void ParseMetaSection(Yaml::Node & node);
	void ParseHardwareSection(Yaml::Node & node);
	void ParseLayoutSection(Yaml::Node & node);
	void ParsePropertySection(Yaml::Node & node);

	void ParseHardwareInputSection(Yaml::Node & node);
	void ParseLayoutRegion(Yaml::Node & node, Region & region);

private:
	std::string m_path;
	std::string m_name;

	Meta     m_meta;
	Hardware m_hardware;
	Layouts  m_layouts;
	Property m_property;

	bool m_isValid;
};
