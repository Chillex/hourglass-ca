#include <SFML/Graphics.hpp>
#include "Hourglass.h"
#include "LUT.h"

int main()
{
	sf::VideoMode vm(1280, 720);
	sf::RenderWindow window(vm, "Hourglass - Alex Müller");

	sf::View gameView(sf::FloatRect(0, 0, 1920, 1080));
	
	bool simulate = false;

	// create LUT
	LUT::CreateLUT();

	// create the hourglass
	Hourglass hourglass(300, 1000, gameView.getSize());

	sf::Clock deltaClock;
	sf::Time dt;
	while(window.isOpen())
	{
		dt = deltaClock.restart();
		window.setView(gameView);

		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed || (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape))
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				// simulate single step
				if (event.key.code == sf::Keyboard::S)
				{
					hourglass.Simulate();
				}

				// toggle simulation
				if (event.key.code == sf::Keyboard::Space)
				{
					simulate = !simulate;
				}

				if (event.key.code == sf::Keyboard::Right)
				{
					hourglass.Rotate(45.0f);
				}

				if (event.key.code == sf::Keyboard::Left)
				{
					hourglass.Rotate(-45.0f);
				}
			}
		}
		
		// update
		if(simulate)
		{
			hourglass.Simulate();
		}

		window.clear(LUT::colorLUT[2]);

		// draw
		hourglass.Draw(window);

		window.display();
	}

	return 0;
}
