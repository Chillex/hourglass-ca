#include "Hourglass.h"

#include "OpenCLManager.h"

#include "LUT.h"

#include <fstream>

Hourglass::Hourglass(size_t width, size_t height, const sf::Vector2f& viewSize)
	: m_air(LUT::colorLUT[0])
	, m_wall(LUT::colorLUT[2])
	, m_sand(LUT::colorLUT[1])
	, m_width(width)
	, m_height(height)
	, m_generator(static_cast<unsigned>(time(nullptr)))
	, m_distribution(0.0f, 1.0f)
	, m_useOffset(false)
{
	// create texture
	m_maxDimension = (m_width > m_height) ? m_width : m_height;
	if(!m_texture.create(m_maxDimension, m_maxDimension))
	{
		printf("ERROR::HOURGLASS::CANT_CREATE_TEXTURE: %zu * %zu", m_width, m_height);
	}

	// create pixels
	m_pixels = new sf::Uint8[m_maxDimension * m_maxDimension * 4]; // * 4 because RGBA
	DrawEmptyHourglass();

	// update texture with generated pixels
	m_texture.update(m_pixels);

	// INFO: true paramater resets the texture rect to match new texture
	m_sprite.setTexture(m_texture, true);
	m_sprite.setOrigin(m_maxDimension * 0.5f, m_maxDimension * 0.5f);
	m_sprite.setPosition(viewSize.x * 0.5f, viewSize.y * 0.5f);
	//m_sprite.setPosition(m_maxDimension * 0.5f, m_maxDimension * 0.5f);
}

Hourglass::~Hourglass()
{
}

void Hourglass::Simulate(SimulationMode::Enum simulationMode)
{
	if(simulationMode == SimulationMode::Sequential)
	{
		SimulateSeq();
	}
	else
	{
		SimulateOpenCL(simulationMode);
	}

	m_simulationMode = simulationMode;

	// apply new pixels
	m_texture.update(m_pixels);

	m_useOffset = !m_useOffset;
}

void Hourglass::Draw(sf::RenderWindow& window) const
{
	window.draw(m_sprite);
}

void Hourglass::Rotate(float angle)
{
	float half = m_maxDimension * 0.5f;
	sf::Sprite sprite(m_texture);
	sprite.setOrigin(half, half);
	sprite.setPosition(half, half);
	sprite.rotate(angle);

	sf::RenderTexture renderTexture;
	renderTexture.create(m_maxDimension, m_maxDimension);
	renderTexture.clear(m_wall);
	renderTexture.draw(sprite);
	renderTexture.display();

	sf::Image result = renderTexture.getTexture().copyToImage();
	memcpy(m_pixels, result.getPixelsPtr(), sizeof(sf::Uint8) * m_maxDimension * m_maxDimension * 4);
	m_texture.update(m_pixels);
}

void Hourglass::SimulateSeq()
{
	unsigned state[4];
	unsigned resultState[4];
	unsigned maskLeftTop = ((1 << 2) - 1) << 6;
	unsigned maskRightTop = ((1 << 2) - 1) << 4;
	unsigned maskLeftBottom = ((1 << 2) - 1) << 2;
	unsigned maskRightBottom = ((1 << 2) - 1) << 0;

	for (size_t y = m_useOffset ? 1 : 0; y < m_maxDimension - 1; y += 2)
	{
		for (size_t x = m_useOffset ? 1 : 0; x < m_maxDimension; x += 2)
		{
			size_t pixelIndex[4] = {
				(y * m_maxDimension + x) * 4,			// left top
				(y * m_maxDimension + x + 1) * 4,		// right top
				((y + 1) * m_maxDimension + x) * 4,		// left bottom
				((y + 1) * m_maxDimension + x + 1) * 4	// right bottom
			};

			// get LUT case
			state[0] = LUT::colorRTable[m_pixels[pixelIndex[0]]];
			state[1] = LUT::colorRTable[m_pixels[pixelIndex[1]]];
			state[2] = LUT::colorRTable[m_pixels[pixelIndex[2]]];
			state[3] = LUT::colorRTable[m_pixels[pixelIndex[3]]];

			unsigned lutCase = (state[0] << 6) | (state[1] << 4) | (state[2] << 2) | (state[3] << 0);

			// get result
			unsigned result = LUT::resultTable[lutCase];

			// special case of two sand cells are next to each other with air below them
			if (lutCase == 80 && m_distribution(m_generator) < 0.5f)
			{
				// the cells dont update
				result = lutCase;
			}

			// only if the result is different from the current state
			if (result != lutCase)
			{
				resultState[0] = (result & maskLeftTop) >> 6;
				resultState[1] = (result & maskRightTop) >> 4;
				resultState[2] = (result & maskLeftBottom) >> 2;
				resultState[3] = (result & maskRightBottom) >> 0;

				// write new pixels
				WriteColorToPixels(m_pixels, sf::Color(LUT::colorLUT[resultState[0]]), pixelIndex[0]);
				WriteColorToPixels(m_pixels, sf::Color(LUT::colorLUT[resultState[1]]), pixelIndex[1]);
				WriteColorToPixels(m_pixels, sf::Color(LUT::colorLUT[resultState[2]]), pixelIndex[2]);
				WriteColorToPixels(m_pixels, sf::Color(LUT::colorLUT[resultState[3]]), pixelIndex[3]);
			}
		}
	}
}

void Hourglass::SimulateOpenCL(SimulationMode::Enum simulationMode)
{
	if(simulationMode != m_simulationMode)
	{
		BuildKernel(simulationMode);
	}

	size_t pixelBufferSize = sizeof(uint32_t) * m_maxDimension * m_maxDimension;
	m_queue.enqueueWriteBuffer(m_pixelsBuffer, CL_TRUE, 0, pixelBufferSize, m_pixels);

	m_queue.enqueueWriteBuffer(m_resultTableBuffer, CL_TRUE, 0, sizeof(cl_uint) * 256, LUT::resultTable);
	m_queue.enqueueWriteBuffer(m_colorRTableBuffer, CL_TRUE, 0, sizeof(cl_uint) * 256, LUT::colorRTable);
	m_queue.enqueueWriteBuffer(m_colorLUTBuffer, CL_TRUE, 0, sizeof(cl_uint) * 3, LUT::colorLUTEndianSwapped);


	cl_float randomValues[256];
	for (size_t i = 0; i < 256; ++i)
	{
		randomValues[i] = m_distribution(m_generator);
	}
	m_queue.enqueueWriteBuffer(m_randomValuesBuffer, CL_TRUE, 0, sizeof(cl_float) * 256, randomValues);

	// arguments
	m_kernel.setArg(0, m_pixelsBuffer);
	m_kernel.setArg(1, m_maxDimension);
	m_kernel.setArg(2, m_useOffset);
	m_kernel.setArg(3, m_resultTableBuffer);
	m_kernel.setArg(4, m_colorRTableBuffer);
	m_kernel.setArg(5, m_colorLUTBuffer);
	m_kernel.setArg(6, m_randomValuesBuffer);

	// start running kernel
	m_queue.enqueueNDRangeKernel(m_kernel, m_offset, m_global, m_local);

	m_queue.enqueueReadBuffer(m_pixelsBuffer, CL_TRUE, 0, pixelBufferSize, m_pixels);
}

void Hourglass::BuildKernel(SimulationMode::Enum simulationMode)
{
	const std::string KERNEL_FILE = "Assets/Kernel/hourglass.cl";
	cl_int err = CL_SUCCESS;
	cl::Program program;
	std::pair<cl::Device, cl::Context> deviceInfo;

	try
	{
		if (simulationMode == SimulationMode::CPU)
			deviceInfo = OpenCLManager::GetDevice("cpu");
		else if (simulationMode == SimulationMode::GPU)
			deviceInfo = OpenCLManager::GetDevice("gpu");
		else
			return;

		// load and build kernel
		std::ifstream sourceFile(KERNEL_FILE);
		if (!sourceFile)
		{
			printf("kernel source file (%s) not found\n", KERNEL_FILE.c_str());
			return;
		}

		std::string sourceCode(std::istreambuf_iterator<char>(sourceFile), (std::istreambuf_iterator<char>()));
		cl::Program::Sources source(1, std::make_pair(sourceCode.c_str(), sourceCode.length() + 1));
		program = cl::Program(deviceInfo.second, source);
		program.build(std::vector<cl::Device>{deviceInfo.first});

		// create kernel
		m_kernel = cl::Kernel(program, "simulate_hourglass", &err);;
		cl::Event event;

		// create queue
		m_queue = cl::CommandQueue(deviceInfo.second, deviceInfo.first, 0, &err);

		// pixel buffer
		size_t pixelBufferSize = sizeof(uint32_t) * m_maxDimension * m_maxDimension;
		m_pixelsBuffer = cl::Buffer(deviceInfo.second, CL_MEM_READ_WRITE, pixelBufferSize);

		// LUT buffers
		m_resultTableBuffer = cl::Buffer(deviceInfo.second, CL_MEM_READ_ONLY, sizeof(cl_uint) * 256);
		m_colorRTableBuffer = cl::Buffer(deviceInfo.second, CL_MEM_READ_ONLY, sizeof(cl_uint) * 256);
		m_colorLUTBuffer = cl::Buffer(deviceInfo.second, CL_MEM_READ_ONLY, sizeof(cl_uint) * 3);

		// random buffer
		m_randomValuesBuffer = cl::Buffer(deviceInfo.second, CL_MEM_READ_ONLY, sizeof(cl_float) * 256);

		// launch kernel
		m_offset = cl::NDRange(0, 0);

		size_t dimensionPow2 = Pow2RoundUp(m_maxDimension);
		m_global = cl::NDRange(dimensionPow2, dimensionPow2);

		size_t maxWorkGroupSize = deviceInfo.first.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
		size_t maxLocal = (maxWorkGroupSize > dimensionPow2) ? dimensionPow2 : maxWorkGroupSize;
		m_local = cl::NDRange(2, maxLocal * 0.5f);
	}
	catch (cl::Error error)
	{
		std::string s;
		program.getBuildInfo(deviceInfo.first, CL_PROGRAM_BUILD_LOG, &s);
		printf("%s\n", s.c_str());
		program.getBuildInfo(deviceInfo.first, CL_PROGRAM_BUILD_OPTIONS, &s);
		printf("%s\n", s.c_str());

		printf("ERROR: %s(%d)\n", error.what(), error.err());
	}
}

void Hourglass::DrawEmptyHourglass(void)
{
	float halfDimension = m_maxDimension * 0.5f;
	float halfWidth = m_width * 0.5f;
	float halfHeight = m_height * 0.5f;
	float widthBuffer = (m_maxDimension - m_width) * 0.5f;

	float wallSizeIncrease = (halfWidth - 5) / halfHeight;

	for (size_t row = 0; row < m_maxDimension; ++row)
	{
		for (size_t col = 0; col < m_maxDimension; ++col)
		{
			size_t pixelIndex = (row * m_maxDimension + col) * 4;

			float wallSize = widthBuffer;
			if (row <= halfDimension)
			{
				wallSize += wallSizeIncrease * row;
			}
			else
			{
				wallSize += wallSizeIncrease * (m_maxDimension - row);
			}
			
			if (col < wallSize || col > m_maxDimension - wallSize)
			{
				WriteColorToPixels(m_pixels, m_wall, pixelIndex);
			}
			else
			{
				if(row >= 20 && row <= halfDimension)
					WriteColorToPixels(m_pixels, m_sand, pixelIndex);
				else
					WriteColorToPixels(m_pixels, m_air, pixelIndex);
			}
		}
	}
}

void Hourglass::RemoveSand(sf::Vector2f position, float radius)
{
	size_t x = position.x - (m_sprite.getPosition().x - m_maxDimension * 0.5f);
	size_t y = position.y - (m_sprite.getPosition().y - m_maxDimension * 0.5f);

	if (x < 0 || x > m_maxDimension || y < 0 || y > m_maxDimension)
		return;

	ReplacePixels(x, y, radius, m_sand, m_air);
}

void Hourglass::AddSand(sf::Vector2f position, float radius)
{
	size_t x = position.x - (m_sprite.getPosition().x - m_maxDimension * 0.5f);
	size_t y = position.y - (m_sprite.getPosition().y - m_maxDimension * 0.5f);
	
	if (x < 0 || x > m_maxDimension || y < 0 || y > m_maxDimension)
		return;

	ReplacePixels(x, y, radius, m_air, m_sand);
}

void Hourglass::ReplacePixels(size_t x, size_t y, float radius, sf::Color& oldColor, sf::Color& newColor)
{
	for (size_t row = 0; row < m_maxDimension; ++row)
	{
		for (size_t col = 0; col < m_maxDimension; ++col)
		{
			int a = col - x;
			int b = row - y;

			// check if we are inside the radius
			if(a * a + b * b <= radius * radius)
			{
				size_t pixelIndex = (row * m_maxDimension + col) * 4;
				if(m_pixels[pixelIndex] == oldColor.r)
				{
					WriteColorToPixels(m_pixels, newColor, pixelIndex);
				}
			}
		}
	}

	m_texture.update(m_pixels);
}

void Hourglass::WriteColorToPixels(uint8_t* pixelArray, const sf::Color& color, size_t index)
{
	// R
	pixelArray[index] = color.r;
	// G
	pixelArray[index + 1] = color.g;
	// B
	pixelArray[index + 2] = color.b;
	// A
	pixelArray[index + 3] = color.a;
}

size_t Hourglass::Pow2RoundUp(size_t x)
{
	if (x == 0)
		return x;

	--x;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;

	return x + 1;
}