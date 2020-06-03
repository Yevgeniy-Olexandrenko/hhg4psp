#include "commons.h"
#include "game.h"

static PCB::Input inputs[] = 
{
	PCB::Input::ACL, PCB::Input::GAME_A, PCB::Input::GAME_B, PCB::Input::TIME, PCB::Input::ALARM,
	PCB::Input::LEFT, PCB::Input::RIGHT, 
	PCB::Input::LEFT_UP, PCB::Input::LEFT_DOWN, PCB::Input::RIGHT_UP, PCB::Input::RIGHT_DOWN,
	PCB::Input::CHEAT 
};

static PCB::ISignal isignals[] = 
{
	PCB::ISignal::GND, PCB::ISignal::VCC,
	PCB::ISignal::MCU_R2, PCB::ISignal::MCU_R3, PCB::ISignal::MCU_R4
};

static PCB::OSignal osignals[] =
{ 
	PCB::OSignal::MCU_ACL,
	PCB::OSignal::MCU_K1, PCB::OSignal::MCU_K2, PCB::OSignal::MCU_K3, PCB::OSignal::MCU_K4,
	PCB::OSignal::MCU_ALPHA, PCB::OSignal::MCU_BETA 
};

Game::Game()
	: m_pcb(NULL)
	, m_version(0)
	, m_year(0)
	, m_mcu(0)
	, m_segments(0)
{
}

void Game::Load(const std::string& name)
{
	platform::Dbg("begin game load: %s\n", name.c_str());

	if (unzFile zip = unzOpen((name + ".hhg").c_str()))
	{
		ReadHardware(zip);
		ReadSoftware(zip);
		ReadLayouts(zip);
		ReadTextures(zip);
		unzClose(zip);
	}

	platform::Dbg("end game load\n");
}

void Game::Free()
{
	m_layouts.clear();

	delete m_pcb;
	m_pcb = NULL;
}

Layouts& Game::GetLayouts()
{
	return m_layouts;
}

PCB* Game::GetPCB() const
{
	return m_pcb;
}

const std::string& Game::GetCompany() const
{
	return m_company;
}

const std::string& Game::GetLabel() const
{
	return m_label;
}

const std::string& Game::GetModel() const
{
	return m_model;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void Game::ReadHardware(unzFile& zip)
{
	std::string data;
	if (UnzipFile(zip, "hardware", data))
	{
		std::istringstream stream(data);

		Reader::Read(stream, m_version);
		Reader::Read(stream, m_company);
		Reader::Read(stream, m_label);
		Reader::Read(stream, m_model);
		Reader::Read(stream, m_year);

		Reader::Read(stream, m_mcu);
		Reader::Read(stream, m_segments);
		m_pcb = new PCB();

		u08 inputCount;
		Reader::Read(stream, inputCount);

		u08 type, input, output;
		for (u08 i = 0; i < inputCount; ++i)
		{
			Reader::Read(stream, type);
			Reader::Read(stream, input);
			Reader::Read(stream, output);
			m_pcb->BuildInput(isignals[input], osignals[output], inputs[type]);
		}
	}
}

void Game::ReadSoftware(unzFile& zip)
{
	std::string data;
	if (m_pcb && UnzipFile(zip, "software", data))
	{
		m_pcb->BuildDevices((u08*)data.data());
	}
}

void Game::ReadLayouts(unzFile& zip)
{
	std::string data;
	if (m_pcb && UnzipFile(zip, "layouts", data))
	{
		std::istringstream stream(data);

		u08 layouts;
		Reader::Read(stream, layouts);
		m_layouts.resize(layouts);

		for (size_t i = 0; i < m_layouts.size(); ++i)
		{
			m_layouts[i].Load(stream);
		}
	}
}

void Game::ReadTextures(unzFile& zip)
{
	for (size_t i = 0; i < m_layouts.size(); ++i)
	{
		for (int t = 0; t < Layout::AssetType::count; ++t)
		{
			std::string file = "texture" + std::to_string(i) + "to"[t];
			std::string data;
			if (UnzipFile(zip, file.c_str(), data))
			{
				m_layouts[i].GetAsset(Layout::AssetType(t)).Bind((u08*)data.data(), data.size());
			}
		}
	}
}

bool Game::UnzipFile(unzFile& zip, const std::string& file, std::string& data)
{
	if (unzLocateFile(zip, file.c_str(), 1) == UNZ_OK)
	{
		unz_file_info info;
		if (unzGetCurrentFileInfo(zip, &info, 0, 0, 0, 0, 0, 0) == UNZ_OK)
		{
			if (unzOpenCurrentFile(zip) == UNZ_OK)
			{
				data.resize(info.uncompressed_size);
				unzReadCurrentFile(zip, (voidp)data.data(), data.size());

				unzCloseCurrentFile(zip);
				return true;
			}
		}
	}
	return false;
}
