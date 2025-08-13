#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdio>
#include "engine/render/Camera.hpp"

int main() {
    const sf::String title("WetTerrarium - bootstrap");
    sf::RenderWindow window(sf::VideoMode({1280u, 720u}), title);
    window.setFramerateLimit(144);
    window.setKeyRepeatEnabled(false);

    Camera cam;
    cam.init(window.getSize());

    sf::RectangleShape box({64.f, 64.f});
    box.setFillColor(sf::Color(180, 220, 255));
    box.setPosition({100.f, 100.f});
    float vx = 120.f; // px/s

    sf::Font font;
    (void)font.openFromFile("/System/Library/Fonts/Supplemental/Arial Unicode.ttf");
    sf::Text fpsText(font, "", 16);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({8.f, 8.f});

    sf::Clock frameClock;
    float fps = 0.f, accum = 0.f; int frames = 0;

    float accumulator = 0.f;
    const float fixed_dt = 1.f / 120.f;

    while (window.isOpen()) {
        // Events
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) { window.close(); }
            else if (const auto* key = ev->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scan::Escape) window.close();
            }
            cam.handleEvent(*ev);
        }

        // dt
        const float dt = frameClock.restart().asSeconds();
        accumulator += dt;

        // Camera update
        cam.update(dt);

        // Fixed-step sim
        while (accumulator >= fixed_dt) {
            auto p = box.getPosition();
            p.x += vx * fixed_dt;
            const float w = static_cast<float>(window.getSize().x);
            if (p.x <= 0.f || (p.x + box.getSize().x) >= w) {
                vx = -vx;
                p.x = std::max(0.f, std::min(p.x, w - box.getSize().x));
            }
            box.setPosition(p);
            accumulator -= fixed_dt;
        }

        // FPS
        accum += dt; frames += 1;
        if (accum >= 0.25f) {
            fps = frames / accum; frames = 0; accum = 0.f;
            char buf[64]; std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
            fpsText.setString(buf);
        }

        // Render
        window.clear(sf::Color(8, 10, 16));
        cam.applyTo(window);
        window.draw(box);

        window.setView(window.getDefaultView()); // HUD
        window.draw(fpsText);
        window.display();
    }
    return 0;
}
