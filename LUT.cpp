#include "LUT.h"

unsigned int LUT::resultTable[256];
unsigned int LUT::colorRTable[256];

const sf::Color LUT::colorLUT[3] = {
	sf::Color(255, 255, 255, 255),	// air
	sf::Color(245, 204, 77, 255),	// sand
	sf::Color(64, 64, 64, 255),		// wall
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

	// color R table
	colorRTable[255] = 0;	// air
	colorRTable[245] = 1;	// sand
	colorRTable[64] = 2;	// wall
}