#ifndef HOURGLASS_H
#define HOURGLASS_H

#include <SFML/Graphics.hpp>

#include <random>

#define __CL_ENABLE_EXCEPTIONS
#include <CL/cl.hpp>

struct SimulationMode
{
	enum Enum
	{
		Sequential = 0,
		CPU,
		GPU
	};
};

class Hourglass
{
public:
	Hourglass(size_t width, size_t height, const sf::Vector2f& viewSize);
	~Hourglass();

	void Simulate(SimulationMode::Enum simulationMode);
	void Draw(sf::RenderWindow& window) const;

	void Rotate(float angle);
	void RemoveSand(sf::Vector2f position, float radius);
	void AddSand(sf::Vector2f position, float radius);

private:
	sf::Color m_air;
	sf::Color m_wall;
	sf::Color m_sand;

	size_t m_width;
	size_t m_height;
	size_t m_maxDimension;
	
	uint8_t* m_pixels;
	sf::Texture m_texture;
	sf::Sprite m_sprite;

	SimulationMode::Enum m_simulationMode;

	// OpenCL stuff
	cl::Buffer m_pixelsBuffer;
	cl::Buffer m_resultTableBuffer;
	cl::Buffer m_colorRTableBuffer;
	cl::Buffer m_colorLUTBuffer;
	cl::Buffer m_randomValuesBuffer;

	cl::CommandQueue m_queue;
	cl::NDRange m_offset;
	cl::NDRange m_global;
	cl::NDRange m_local;

	cl::Kernel m_kernel;

	// random generator
	std::mt19937 m_generator;
	std::uniform_real_distribution<float> m_distribution;
	
	bool m_useOffset;

	void SimulateSeq(void);
	void SimulateOpenCL(SimulationMode::Enum simulationMode);

	void BuildKernel(SimulationMode::Enum simulationMode);

	void DrawEmptyHourglass(void);
	void ReplacePixels(size_t x, size_t y, float radius, sf::Color& oldColor, sf::Color& newColor);

	static void WriteColorToPixels(uint8_t* pixelArray, const sf::Color& color, size_t index);
	static size_t Pow2RoundUp(size_t x);
};

#endif
