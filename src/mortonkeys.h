#include <cstdint>
#include <vector>

#define MAXMORTONKEY 0x5555555555555555 // 2D morton key max (32 bits per dimension)

std::vector<uint64_t> mortonkeyX;
std::vector<uint64_t> mortonkeyY;

/*Compute a 256 array of morton keys at compile time*/
template <uint32_t i, uint32_t offset>
struct morton
{
	enum
	{
		value = (morton<i - 1, offset>::value - MAXMORTONKEY) & MAXMORTONKEY
	};
	static void add_values(std::vector<uint64_t>& v)
	{
		morton<i - 1, offset>::add_values(v);
		v.push_back(value<<offset);
	}
};

template <uint32_t offset>
struct morton<0, offset>
{
	enum
	{
		value = 0
	};
	static void add_values(std::vector<uint64_t>& v)
	{
		v.push_back(value);
	}
};

inline uint64_t encodeMortonKey(uint32_t x, uint32_t y){
	uint64_t result = 0;
	result = mortonkeyY[(y >> 16) & 0xFF] |
		mortonkeyX[(x >> 16) & 0xFF];
	result = result << 16 |
		mortonkeyY[(y >> 8) & 0xFF] |
		mortonkeyX[(x >> 8) & 0xFF];
	result = result << 16 |
		mortonkeyY[(y) & 0xFF] |
		mortonkeyX[(x) & 0xFF];
	return result;
}

inline uint32_t FloatToUInt(float x)
{
    // static_assert(
    //     sizeof(float) == sizeof(uint32_t), // Ensure float is 4 bytes
    //     "sizeof(float) != sizeof(uint32_t)"
    // );
    // using uchar = unsigned char; // uchar represents 1 byte.

    // uint32_t xi{0}; // Initialize the result variable (unsigned integer).
    // auto const* i = reinterpret_cast<uchar const*>(&x); // Treat the address of `x` as a pointer to bytes.
    // std::copy(i, i + 4, reinterpret_cast<uchar*>(&xi)); // Copy 4 bytes from `x` into `xi`.

    // return xi; // Return the bitwise representation as an unsigned int.
	return x * 1000;
}

int toGrid(int grid_size, float coord, float min, float max){
    unsigned int position = (coord - min) / (max - min) * (grid_size - 1);
    return static_cast<unsigned int>(position);
}



inline uint64_t encodeFloat(float x, float y, float min, float max){
	uint32_t int_x = toGrid(1048576, x, min, max);
	uint32_t int_y = toGrid(1048576, y, min, max);
	return encodeMortonKey(int_x, int_y);
}


