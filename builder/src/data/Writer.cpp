#include "Writer.h"

namespace Writer
{
	void Write(std::ostream & stream, const u08 & data)
	{
		stream.put(char(data));
	}

	void Write(std::ostream & stream, const u16 & data)
	{
		stream.put(char(data));
		stream.put(char(data >> 8));
	}

	void Write(std::ostream& stream, const s16& data)
	{
		Write(stream, u16(data));
	}

	void Write(std::ostream & stream, const u32 & data)
	{
		stream.put(char(data));
		stream.put(char(data >> 8));
		stream.put(char(data >> 16));
		stream.put(char(data >> 24));
	}

	void Write(std::ostream & stream, const std::string & data)
	{
		stream.put(char(data.length()));
		stream.write(data.c_str(), data.length());
	}

	void Write(std::ostream & stream, const char * data)
	{
		Write(stream, std::string(data));
	}

	void Write(std::ostream & stream, std::ifstream & file)
	{
		file.seekg(0, file.end);
		u16 size = u16(file.tellg());
		file.seekg(0);

		char * buffer = new char[size];
		file.read(buffer, size);
		stream.write(buffer, size);
		delete[] buffer;
	}
}