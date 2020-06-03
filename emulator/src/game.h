#pragma once

#include "layout.h"
#include "pcb.h"

class Game
{
public:
	Game();

	void Load(const std::string& name);
	void Free();

	Layouts& GetLayouts();
	PCB* GetPCB() const;

	const std::string& GetCompany() const;
	const std::string& GetLabel() const;
	const std::string& GetModel() const;

private:
	void ReadHardware(unzFile& zip);
	void ReadSoftware(unzFile& zip);
	void ReadLayouts (unzFile& zip);
	void ReadTextures(unzFile& zip);
	bool UnzipFile(unzFile& zip, const std::string& file, std::string& data);

private:
	Layouts m_layouts;
	PCB* m_pcb;

	u32 m_version;
	std::string m_company;
	std::string m_label;
	std::string m_model;
	u16 m_year;

	u08 m_mcu;
	u08 m_segments;
};