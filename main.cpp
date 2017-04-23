#include <SFML/Graphics.hpp>

int main()
{
	sf::VideoMode vm(1280, 720);
	sf::RenderWindow window(vm, "Hourglass - Alex Müller");

	sf::View gameView(sf::FloatRect(0, 0, 1280, 720));

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
		}
		
		// update

		window.clear();

		// draw

		window.display();
	}

	return 0;
}