#pragma once

namespace Reader
{
	inline void Read(std::istream& stream, u08& data)
	{
		data = u08(stream.get());
	}

	inline  void Read(std::istream& stream, u16& data)
	{
		data = u08(stream.get()) | u08(stream.get()) << 8;
	}

	inline void Read(std::istream& stream, s16& data)
	{
		Read(stream, reinterpret_cast<u16&>(data));
	}

	inline void Read(std::istream& stream, u32& data)
	{
		data = u08(stream.get()) | u08(stream.get()) << 8 | u08(stream.get()) << 16 | u08(stream.get()) << 24;
	}

	inline void Read(std::istream& stream, std::string& data)
	{
		int size = stream.get();
		char* buffer = new char[size];
		stream.read(buffer, size);
		data.assign(buffer, size);
		delete[] buffer;
	}
}
