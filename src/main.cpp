#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdio>

#include "engine/render/Camera.hpp"
#include "engine/world/World.hpp"

int main() {
    const sf::String title("WetTerrarium - world bootstrap");
    sf::RenderWindow window(sf::VideoMode({1280u, 720u}), title);
    window.setFramerateLimit(144);
    window.setKeyRepeatEnabled(false);

    Camera cam; 
    cam.init(window.getSize());

    World world(/*seed=*/0);

    // HUD
    sf::Font font; (void)font.openFromFile("/System/Library/Fonts/Supplemental/Arial Unicode.ttf");
    sf::Text fpsText(font, "", 16);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({8.f, 8.f});

    sf::Clock frameClock;
    float accum = 0.f; int frames = 0;

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) window.close();
            else if (const auto* key = ev->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scan::Escape) window.close();
            }
            cam.handleEvent(*ev);
        }

        const float dt = frameClock.restart().asSeconds();
        cam.update(dt);

        // Ensure chunks around the camera’s current view exist (lazy-load)
        world.ensureVisible(cam.view(), /*inflatePixels=*/TILE_SIZE * 8.f);

        // FPS
        accum += dt; frames += 1;
        if (accum >= 0.25f) {
            const float fps = frames / accum; frames = 0; accum = 0.f;
            char buf[64]; std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
            fpsText.setString(buf);
        }

        window.clear(sf::Color(8, 10, 16));
        cam.applyTo(window);
        window.draw(world);

        window.setView(window.getDefaultView());
        window.draw(fpsText);
        window.display();
    }
    return 0;
}
