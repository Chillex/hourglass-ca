#include <SFML/Graphics.hpp>
#include "Hourglass.h"
#include "LUT.h"
#include "FPSCounter.h"

int main()
{
	sf::VideoMode vm(1280, 720);
	sf::RenderWindow window(vm, "Hourglass - Alex Müller");

	sf::View gameView(sf::FloatRect(0, 0, 1920, 1080));
	window.setMouseCursorVisible(false);
	
	bool removeSand = false;
	bool addSand = false;
	bool simulate = false;

	SimulationMode::Enum simulationMode = SimulationMode::Sequential;

	// create LUT
	LUT::CreateLUT();

	// create the hourglass
	Hourglass hourglass(300, 1000, gameView.getSize());

	// mouse cursor (brush)
	float cursorSize = 10.0f;
	sf::CircleShape cursor(cursorSize);
	cursor.setFillColor(sf::Color::Transparent);
	cursor.setOutlineColor(sf::Color::Red);
	cursor.setOutlineThickness(2.0f);

	// clear color
	sf::Color clearColor(LUT::colorLUT[2]);

	// FPS counter
	FPSCounter fpsCounter("Assets/Font/digital_counter_7.ttf");
	
	// mode display
	sf::Font font;
	font.loadFromFile("Assets/Font/digital_counter_7.ttf");

	sf::Text modeText("Sequential Mode", font, 24);
	modeText.setPosition(10, 44);
	modeText.setFillColor(sf::Color(255, 64, 64, 255));

	sf::Clock deltaClock;
	sf::Time dt;
	while(window.isOpen())
	{
		dt = deltaClock.restart();
		window.setView(gameView);

		sf::Vector2i mousePosPixel = sf::Mouse::getPosition(window);
		sf::Vector2f mousePosWorld = window.mapPixelToCoords(mousePosPixel);

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				// change simulation mode
				if (event.key.code == sf::Keyboard::M)
				{
					switch (simulationMode)
					{
					case SimulationMode::Sequential:
						simulationMode = SimulationMode::CPU;
						modeText.setString("CPU Mode");
						break;
					case SimulationMode::CPU:
						simulationMode = SimulationMode::GPU;
						modeText.setString("GPU Mode");
						break;
					case SimulationMode::GPU:
						simulationMode = SimulationMode::Sequential;
						modeText.setString("Sequential Mode");
						break;
					}
				}

				// simulate single step
				if (event.key.code == sf::Keyboard::S)
				{
					hourglass.Simulate(simulationMode);
				}

				// toggle simulation
				if (event.key.code == sf::Keyboard::Space)
				{
					simulate = !simulate;
				}

				// rotate hourglass
				if (event.key.code == sf::Keyboard::Right)
				{
					hourglass.Rotate(45.0f);
				}

				if (event.key.code == sf::Keyboard::Left)
				{
					hourglass.Rotate(-45.0f);
				}

				// change brush size
				if (event.key.code == sf::Keyboard::Equal)
				{
					++cursorSize;
				}

				if (event.key.code == sf::Keyboard::Dash)
				{
					if(cursorSize > 1.0f)
						--cursorSize;
				}
			}

			if(event.type == sf::Event::MouseButtonPressed)
			{
				// remove sand
				if(event.mouseButton.button == sf::Mouse::Left)
				{
					removeSand = true;
				}

				// add sand
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					addSand = true;
				}
			}

			if (event.type == sf::Event::MouseButtonReleased)
			{
				// remove sand
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					removeSand = false;
				}

				// add sand
				if (event.mouseButton.button == sf::Mouse::Right)
				{
					addSand = false;
				}
			}
		}
		
		// update
		fpsCounter.Update(dt);

		if (removeSand)
		{
			hourglass.RemoveSand(mousePosWorld, cursorSize);
		}

		if (addSand)
		{
			hourglass.AddSand(mousePosWorld, cursorSize);
		}
		
		if(simulate)
		{
			hourglass.Simulate(simulationMode);
		}

		// set cursor
		cursor.setRadius(cursorSize);
		cursor.setOrigin(cursorSize, cursorSize);
		cursor.setPosition(mousePosWorld.x, mousePosWorld.y);

		window.clear(clearColor);

		// draw
		hourglass.Draw(window);
		fpsCounter.Draw(window);
		window.draw(modeText);
		window.draw(cursor);

		window.display();
	}

	return 0;
}
