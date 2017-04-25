#ifndef LUT_H
#define LUT_H

#include <SFML/Graphics.hpp>

struct LUT
{
	static unsigned int resultTable[256];
	static unsigned int colorRTable[256];
	static const sf::Color colorLUT[3];

	static void CreateLUT(void);
};

#endif