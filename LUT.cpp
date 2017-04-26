#include "LUT.h"

#include <SFML/Graphics/Color.hpp>

cl_uint LUT::resultTable[256];
cl_uint LUT::colorRTable[256];
cl_uint LUT::colorLUTEndianSwapped[3];

const cl_uint LUT::colorLUT[3] = {
	sf::Color(255, 255, 255, 255).toInteger(),	// air
	sf::Color(245, 204, 77, 255).toInteger(),	// sand
	sf::Color(64, 64, 64, 255).toInteger(),		// wall
};

void LUT::CreateLUT()
{
	for (size_t i = 0; i < 256; ++i)
	{
		resultTable[i] = i;
		colorRTable[i] = i;
	}

	// special cases
	// 00 = air
	// 01 = sand
	// 10 = wall
	resultTable[0b01000000] = 0b00000100;
	resultTable[0b00010000] = 0b00000001;
	resultTable[0b01000100] = 0b00000101;
	resultTable[0b00010001] = 0b00000101;
	resultTable[0b01000001] = 0b00000101;
	resultTable[0b00010100] = 0b00000101;
	resultTable[0b01010001] = 0b00010101;
	resultTable[0b01010100] = 0b01000101;
	resultTable[0b01010000] = 0b00000101;
	// sand and wall cases
	resultTable[0b01100000] = 0b00100100;
	resultTable[0b01000010] = 0b00000110;
	resultTable[0b01100010] = 0b00100110;
	resultTable[0b10010000] = 0b10000001;
	resultTable[0b10011000] = 0b10001001;
	resultTable[0b00011000] = 0b00001001;
	resultTable[0b01011000] = 0b01001001;
	resultTable[0b01010010] = 0b00010110;
	resultTable[0b01001000] = 0b00001001;
	resultTable[0b00010010] = 0b00000110;

	// color R table
	colorRTable[255] = 0;	// air
	colorRTable[245] = 1;	// sand
	colorRTable[64] = 2;	// wall

	// swapped color LUT
	for(size_t i = 0; i < 3; ++i)
	{
		colorLUTEndianSwapped[i] = swapEndian(colorLUT[i]);
	}
}

cl_uint LUT::swapEndian(cl_uint num)
{
	return	((num >> 24) & 0xff) | // move byte 3 to byte 0
		((num << 8) & 0xff0000) | // move byte 1 to byte 2
		((num >> 8) & 0xff00) | // move byte 2 to byte 1
		((num << 24) & 0xff000000); // byte 0 to byte 3
}
