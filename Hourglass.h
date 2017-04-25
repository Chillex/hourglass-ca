#ifndef HOURGLASS_H
#define HOURGLASS_H

#include "LUT.h"
#include <random>

class Hourglass
{
public:
	Hourglass(size_t width, size_t height, const sf::Vector2f& viewSize);
	~Hourglass();

	void Simulate(void);
	void Draw(sf::RenderWindow& window) const;
	void Rotate(float angle);

private:
	sf::Color m_air;
	sf::Color m_wall;
	sf::Color m_sand;

	size_t m_width;
	size_t m_height;
	size_t m_maxDimension;
	
	sf::Uint8* m_pixels;
	sf::Texture m_texture;
	sf::Sprite m_sprite;

	std::mt19937 m_generator;
	std::uniform_real_distribution<float> m_distribution;
	
	bool m_useOffset;

	void DrawEmptyHourglass(void);
	void WriteColorToPixels(sf::Uint8* pixelArray, const sf::Color& color, size_t index);
};

#endif
