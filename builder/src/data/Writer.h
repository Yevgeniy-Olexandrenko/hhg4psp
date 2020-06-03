#pragma once

#include <fstream>
#include <sstream>
#include "Types.h"

namespace Writer
{
	void Write(std::ostream & stream, const u08 & data);
	void Write(std::ostream & stream, const u16 & data);
	void Write(std::ostream & stream, const s16 & data);
	void Write(std::ostream & stream, const u32 & data);
	void Write(std::ostream & stream, const std::string & data);
	void Write(std::ostream & stream, const char * data);
	void Write(std::ostream & stream, std::ifstream & file);
}
