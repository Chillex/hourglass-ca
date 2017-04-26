/*
* kernel that simulates a 2x2 square of an hourglass
*/
__kernel void simulate_hourglass(__global uint* pixels, const ulong dimensions, const uchar offset, __constant uint* resultTable, __constant uint* colorRTable, __constant uint* colorLUT, __constant float* randomValues)
{
	uint row = get_global_id(0) * 2 + offset;
	uint column = get_global_id(1) * 2 + offset;

	if(row >= dimensions - 1 || column >= dimensions - 1)
		return;

	uint maskLeftTop = ((1 << 2) - 1) << 6;
	uint maskRightTop = ((1 << 2) - 1) << 4;
	uint maskLeftBottom = ((1 << 2) - 1) << 2;
	uint maskRightBottom = ((1 << 2) - 1) << 0;

	uint pixelIndex[4] = {
		column + dimensions * row,						// left top
		column + 1 + dimensions * row,				// right top
		column + dimensions * (row + 1),			// left bottom
		column + 1 + dimensions * (row + 1)		// right bottom
	};

	uint state[4] = {
		colorRTable[(pixels[pixelIndex[0]] & 0x000000ff)],
		colorRTable[(pixels[pixelIndex[1]] & 0x000000ff)],
		colorRTable[(pixels[pixelIndex[2]] & 0x000000ff)],
		colorRTable[(pixels[pixelIndex[3]] & 0x000000ff)]
	};

	uint lutCase = (state[0] << 6) | (state[1] << 4) | (state[2] << 2) | (state[3] << 0);

	uint result = resultTable[lutCase];

	if (lutCase == 80 && randomValues[(row * column) % 256] < 0.5f)
	{
		result = lutCase;
	}

	if(result != lutCase)
	{
		uint resultState[] = {
			(result & maskLeftTop) >> 6,
			(result & maskRightTop) >> 4,
			(result & maskLeftBottom) >> 2,
			(result & maskRightBottom) >> 0,
		};
		pixels[pixelIndex[0]] = colorLUT[resultState[0]];
		pixels[pixelIndex[1]] = colorLUT[resultState[1]];
		pixels[pixelIndex[2]] = colorLUT[resultState[2]];
		pixels[pixelIndex[3]] = colorLUT[resultState[3]];
	}
}
