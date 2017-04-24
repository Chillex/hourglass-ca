#include "Hourglass.h"
#include "LUT.h"

Hourglass::Hourglass(size_t width, size_t height, const sf::Vector2f& viewSize)
	: m_air(255, 255, 255, 255)
	, m_wall(64, 64, 64, 255)
	, m_sand(245, 204, 77, 255)
	, m_width(width)
	, m_height(height)
	, m_useOffset(false)
{
	// create texture
	if(!m_texture.create(m_width, m_height))
	{
		printf("ERROR::HOURGLASS::CANT_CREATET_TEXTURE: %zu * %zu", m_width, m_height);
	}

	// create pixels
	m_pixels = new sf::Uint8[m_width * m_height * 4]; // * 4 because RGBA
	DrawEmptyHourglass();

	// update texture with generated pixels
	m_texture.update(m_pixels);

	// INFO: true paramater resets the texture rect to match new texture
	m_sprite.setTexture(m_texture, true);
	m_sprite.setOrigin(m_width * 0.5f, m_height * 0.5f);
	m_sprite.setPosition(viewSize.x * 0.5f, viewSize.y * 0.5f);
}

Hourglass::~Hourglass()
{
}

void Hourglass::Simulate()
{
	sf::Uint8* pixelBuffer = m_pixels; // * 4 because RGBA

	for (size_t y = m_useOffset ? 1 : 0; y < m_height - 1; y += 2)
	{
		for (size_t x = m_useOffset ? 1 : 0; x < m_width; x += 2)
		{
			size_t leftTop = (y * m_width + x) * 4;
			size_t rightTop = (y * m_width + x + 1) * 4;
			size_t leftBot = ((y + 1) * m_width + x) * 4;
			size_t rightBot = ((y + 1) * m_width + x + 1) * 4;

			// get LUT case
			char state[4] = {
				(m_pixels[leftTop] == m_sand.r) ? 1 : 0,
				(m_pixels[rightTop] == m_sand.r) ? 1 : 0,
				(m_pixels[leftBot] == m_sand.r) ? 1 : 0,
				(m_pixels[rightBot] == m_sand.r) ? 1 : 0
			};
			unsigned int lutCase = (state[0] << 0) | (state[1] << 1) | (state[2] << 2) | (state[3] << 3);
			
			// get result
			unsigned int result = LUT::resultTable[lutCase];
			char resultState[4] = {
				(result & (1 << 0)) >> 0,
				(result & (1 << 1)) >> 1,
				(result & (1 << 2)) >> 2,
				(result & (1 << 3)) >> 3
			};

			WriteColorToPixels(pixelBuffer, (resultState[0] == 1) ? m_sand : m_air, leftTop);
			WriteColorToPixels(pixelBuffer, (resultState[1] == 1) ? m_sand : m_air, rightTop);
			WriteColorToPixels(pixelBuffer, (resultState[2] == 1) ? m_sand : m_air, leftBot);
			WriteColorToPixels(pixelBuffer, (resultState[3] == 1) ? m_sand : m_air, rightBot);
		}
	}

	// apply buffer
	m_pixels = pixelBuffer;
	m_texture.update(m_pixels);

	m_useOffset = !m_useOffset;
}

void Hourglass::Draw(sf::RenderWindow& window) const
{
	window.draw(m_sprite);
}

void Hourglass::DrawEmptyHourglass(void)
{
	float halfHeight = m_height * 0.5f;
	float halfWidth = m_width * 0.5f;

	float wallSizeIncrease = (halfWidth - 10) / halfHeight;

	for (size_t row = 0; row < m_height; ++row)
	{
		for (size_t col = 0; col < m_width; ++col)
		{
			size_t pixelIndex = (row * m_width + col) * 4;

			float wallSize = 0.0f;
			if (row <= halfHeight)
			{
				wallSize = wallSizeIncrease * row;
			}
			else
			{
				wallSize = wallSizeIncrease * (m_height - row);
			}
			
			if (col < wallSize || col > m_width - wallSize)
			{
				WriteColorToPixels(m_pixels, m_wall, pixelIndex);
			}
			else
			{
				if(row >= 20 && row <= halfHeight)
					WriteColorToPixels(m_pixels, m_sand, pixelIndex);
				else
					WriteColorToPixels(m_pixels, m_air, pixelIndex);
			}
		}
	}
}

void Hourglass::WriteColorToPixels(sf::Uint8* pixelArray, const sf::Color& color, size_t index)
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
