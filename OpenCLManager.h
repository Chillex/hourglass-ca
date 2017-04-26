#ifndef OPEN_CL_MANAGER_H
#define OPEN_CL_MANAGER_H

#include <CL/cl.hpp>

class OpenCLManager
{
public:
	OpenCLManager();
	~OpenCLManager();

	static std::pair<cl::Device, cl::Context> GetDevice(int platformNR, int deviceNr);
	static std::pair<cl::Device, cl::Context> GetDevice(std::string deviceType);
};

#endif
