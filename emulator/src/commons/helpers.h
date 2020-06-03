#pragma once

template<typename def, typename inner = typename def::type>
class safe_enum : public def
{
	typedef typename def::type type;
	inner val;

public:
	safe_enum() {}
	safe_enum(type v) : val((inner)v) {}
	safe_enum(int  v) : val((inner)v) {}
	inner underlying() const { return val; }

	bool operator==(const safe_enum& s) const { return this->val == s.val; }
	bool operator!=(const safe_enum& s) const { return this->val != s.val; }
	bool operator< (const safe_enum& s) const { return this->val < s.val; }
	bool operator<=(const safe_enum& s) const { return this->val <= s.val; }
	bool operator> (const safe_enum& s) const { return this->val > s.val; }
	bool operator>=(const safe_enum& s) const { return this->val >= s.val; }
};

#define simple_safe_enum(name, ...)        \
	struct name ## _def { enum type { __VA_ARGS__ }; };  \
	typedef safe_enum<name ## _def> name;

#define typed_safe_enum(name, storage, ...)        \
	struct name ## _def { enum type { __VA_ARGS__ }; };  \
	typedef safe_enum<name ## _def, storage> name;

template<typename T> inline T lerp(T a, T b, float t) { return a + t * (b - a); }
template<typename T> inline T clamp(T v, T min, T max) { return std::max(min, std::min(max, v)); }
template<typename T> inline T square(T v) { return v * v; }

inline int hash(const std::string& str)
{
	int h = 0;
	for (size_t i = 0; i < str.size(); ++i)
	{
		h = h * 31 + static_cast<int>(str[i]);
	}
	return h;
}

inline u16 rand(u64& seed)
{
	return(((seed = seed * 214013L + 2531011L) >> 16) & 0x7fff);
}