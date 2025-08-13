#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdio>
#include "engine/render/Camera.hpp"

int main() {
    // ASCII title to avoid Cocoa UTF-8 edge cases
    const sf::String title("WetTerrarium - bootstrap");
    sf::RenderWindow window(sf::VideoMode({1280u, 720u}), title);
    window.setFramerateLimit(144);
    window.setKeyRepeatEnabled(false); // better pressed/released semantics

    // Camera
    Camera cam;
    cam.init(window.getSize());

    // Simple actor
    sf::RectangleShape box({64.f, 64.f});
    box.setFillColor(sf::Color(180, 220, 255));
    box.setPosition({100.f, 100.f});
    float vx = 120.f; // px/s

    // Font & FPS text
    sf::Font font;
    (void)font.openFromFile("/System/Library/Fonts/Supplemental/Arial Unicode.ttf");
    sf::Text fpsText(font, "", 16);
    fpsText.setFillColor(sf::Color::White);
    fpsText.setPosition({8.f, 8.f});

    // Frame timing
    sf::Clock frameClock;
    float fps = 0.f, accum = 0.f; int frames = 0;

    // Fixed-step sim
    float accumulator = 0.f;
    const float fixed_dt = 1.f / 120.f;

    // Realtime input state (scancode-based, layout-agnostic)
    bool heldW = false, heldA = false, heldS = false, heldD = false, heldQ = false, heldE = false;

    while (window.isOpen()) {
        // --- Events (set/clear "held" state using SCANCODES)
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* r = ev->getIf<sf::Event::Resized>()) {
                cam.onResize({r->size.x, r->size.y});
            } else if (const auto* kp = ev->getIf<sf::Event::KeyPressed>()) {
                switch (kp->scancode) {
                    case sf::Keyboard::Scan::W: heldW = true; break;
                    case sf::Keyboard::Scan::A: heldA = true; break;
                    case sf::Keyboard::Scan::S: heldS = true; break;
                    case sf::Keyboard::Scan::D: heldD = true; break;
                    case sf::Keyboard::Scan::Q: heldQ = true; break;
                    case sf::Keyboard::Scan::E: heldE = true; break;
                    case sf::Keyboard::Scan::Escape: window.close(); break;
                    default: break;
                }
            } else if (const auto* kr = ev->getIf<sf::Event::KeyReleased>()) {
                switch (kr->scancode) {
                    case sf::Keyboard::Scan::W: heldW = false; break;
                    case sf::Keyboard::Scan::A: heldA = false; break;
                    case sf::Keyboard::Scan::S: heldS = false; break;
                    case sf::Keyboard::Scan::D: heldD = false; break;
                    case sf::Keyboard::Scan::Q: heldQ = false; break;
                    case sf::Keyboard::Scan::E: heldE = false; break;
                    default: break;
                }
            }
        }

        // --- Frame delta (seconds)
        const float dt = frameClock.restart().asSeconds();
        accumulator += dt;

        // --- Realtime camera controls (use held-state + dt)
        const float panSpeed = 300.f; // px/s
        const float pan = panSpeed * dt;
        if (heldW) cam.move({0.f, -pan});
        if (heldS) cam.move({0.f,  pan});
        if (heldA) cam.move({-pan, 0.f});
        if (heldD) cam.move({ pan, 0.f});
        if (heldQ) cam.zoom(0.99f);
        if (heldE) cam.zoom(1.01f);

        // --- Fixed-step update
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

        // --- FPS
        accum += dt; frames += 1;
        if (accum >= 0.25f) {
            fps = frames / accum; frames = 0; accum = 0.f;
            char buf[64]; std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
            fpsText.setString(buf);
        }

        // --- Render
        window.clear(sf::Color(8, 10, 16));
        window.setView(cam.view());
        window.draw(box);

        // HUD in default view
        window.setView(window.getDefaultView());
        window.draw(fpsText);
        window.display();
    }
    return 0;
}
