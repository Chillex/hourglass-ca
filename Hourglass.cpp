#include "Hourglass.h"

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

void Hourglass::Simulate()
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
			if(lutCase == 80 && m_distribution(m_generator) < 0.5f)
			{
				// the cells dont update
				result = lutCase;
			}

			resultState[0] = (result & maskLeftTop) >> 6;
			resultState[1] = (result & maskRightTop) >> 4;
			resultState[2] = (result & maskLeftBottom) >> 2;
			resultState[3] = (result & maskRightBottom) >> 0;

			// write new pixels
			WriteColorToPixels(m_pixels, LUT::colorLUT[resultState[0]], pixelIndex[0]);
			WriteColorToPixels(m_pixels, LUT::colorLUT[resultState[1]], pixelIndex[1]);
			WriteColorToPixels(m_pixels, LUT::colorLUT[resultState[2]], pixelIndex[2]);
			WriteColorToPixels(m_pixels, LUT::colorLUT[resultState[3]], pixelIndex[3]);
		}
	}

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
