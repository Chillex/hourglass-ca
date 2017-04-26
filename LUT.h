#ifndef LUT_H
#define LUT_H

#include <CL/cl_ext.h>

struct LUT
{
	static cl_uint resultTable[256];
	static cl_uint colorRTable[256];
	static const cl_uint colorLUT[3];
	static cl_uint colorLUTEndianSwapped[3];

	static void CreateLUT(void);
	static cl_uint swapEndian(cl_uint num);
};

#endif