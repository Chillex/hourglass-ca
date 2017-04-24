#ifndef LUT_H
#define LUT_H

struct LUT
{
	static const unsigned int resultTable[16];
};

const unsigned int LUT::resultTable[16] =
{
	0, 4, 8, 12, 
	4, 12, 12, 13,
	8, 12, 12, 14,
	12, 13, 14, 15
};

#endif