#define __CL_ENABLE_EXCEPTIONS
#include "OpenCLManager.h"
#include <iostream>

OpenCLManager::OpenCLManager()
{
}

OpenCLManager::~OpenCLManager()
{
}

std::pair<cl::Device, cl::Context> OpenCLManager::GetDevice(int platformNR, int deviceNr)
{
	std::vector<cl::Device> devices;

	try
	{
		// get available platforms (nvidia, amd, intel, ...)
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		if (platforms.size() == 0)
		{
			std::cout << "No OpenCL platforms available!" << std::endl;
			return std::make_pair(cl::Device(), cl::Context());
		}

		// create a context and get available devices
		cl::Platform platform = platforms[platformNR];
		cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>((platform)()), 0 };
		cl::Context context(CL_DEVICE_TYPE_ALL, properties);

		devices = context.getInfo<CL_CONTEXT_DEVICES>();
		return std::make_pair(devices[deviceNr], context);
	}
	catch(cl::Error error)
	{
		std::cerr << "ERROR: " << error.what() << "(" << error.err() << ")" << std::endl;
	}

	return std::make_pair(cl::Device(), cl::Context());
}

std::pair<cl::Device, cl::Context> OpenCLManager::GetDevice(std::string deviceType)
{
	std::vector<cl::Device> devices;
	int wantedType;

	if (deviceType == "cpu")
		wantedType = CL_DEVICE_TYPE_CPU;
	else if (deviceType == "gpu")
		wantedType = CL_DEVICE_TYPE_GPU;
	else
		return std::make_pair(cl::Device(), cl::Context());

	try
	{
		// get available platforms (nvidia, amd, intel, ...)
		std::vector<cl::Platform> platforms;
		cl::Platform::get(&platforms);

		if (platforms.size() == 0)
		{
			std::cout << "No OpenCL platforms available!" << std::endl;
			return std::make_pair(cl::Device(), cl::Context());
		}

		for (std::vector<cl::Platform>::iterator pl = platforms.begin(); pl < platforms.end(); ++pl)
		{
			// create a context and get available devices
			cl::Platform platform = *pl;
			cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>((platform)()), 0 };
			cl::Context context(CL_DEVICE_TYPE_ALL, properties);

			devices = context.getInfo<CL_CONTEXT_DEVICES>();

			for (std::vector<cl::Device>::iterator de = devices.begin(); de < devices.end(); ++de)
			{
				cl::Device device = *de;

				if (device.getInfo<CL_DEVICE_TYPE>() == wantedType)
					return std::make_pair(device, context);
			}
		}
	}
	catch (cl::Error error)
	{
		std::cerr << "ERROR: " << error.what() << "(" << error.err() << ")" << std::endl;
	}

	return std::make_pair(cl::Device(), cl::Context());
}
