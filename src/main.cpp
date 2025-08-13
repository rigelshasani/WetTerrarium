#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdio>

int main() {
    // Use ASCII title to avoid Cocoa issues
    const sf::String title("WetTerrarium - bootstrap");

    sf::RenderWindow window(sf::VideoMode({1280u, 720u}), title);
    window.setFramerateLimit(144);

    sf::Font font;
    (void)font.openFromFile("/System/Library/Fonts/Supplemental/Arial Unicode.ttf"); // ignore failure

    sf::Text fpsText(font, "", 16);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({8.f, 8.f});

    sf::Clock frameClock;
    float fps = 0.f, accum = 0.f;
    int frames = 0;

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = ev->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scan::Escape) window.close();
            }
        }

        float dt = frameClock.restart().asSeconds();
        accum += dt; frames += 1;
        if (accum >= 0.25f) {
            fps = frames / accum; frames = 0; accum = 0.f;
            char buf[64]; std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
            fpsText.setString(buf);
        }

        window.clear(sf::Color(8, 10, 16));
        window.draw(fpsText);
        window.display();
    }
    return 0;
}
