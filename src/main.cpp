#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdio>
#include <vector>
#include <string>

#include "engine/render/Camera.hpp"
#include "engine/world/World.hpp"
#include "engine/tile/TileAtlas.hpp"

int main() {
    const sf::String title("WetTerrarium - textured world");
    sf::RenderWindow window(sf::VideoMode({1280u, 720u}), title);
    window.setFramerateLimit(144);
    window.setKeyRepeatEnabled(false);

    // Camera
    Camera cam;
    cam.init(window.getSize());

    // Tiles/World
    TileAtlas atlas(TILE_SIZE);
    World world(&atlas, /*seed=*/0);

    // HUD
    sf::Font font;
    bool fontLoaded = false;
    
    // Try multiple font paths for cross-platform compatibility
    const std::vector<std::string> fontPaths = {
        "/System/Library/Fonts/Supplemental/Arial Unicode.ttf", // macOS
        "/System/Library/Fonts/Arial.ttf",                      // macOS fallback
        "C:\\Windows\\Fonts\\arial.ttf",                        // Windows
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",     // Linux
        "/usr/share/fonts/TTF/arial.ttf",                      // Linux alternate
    };
    
    for (const auto& path : fontPaths) {
        if (font.openFromFile(path)) {
            fontLoaded = true;
            break;
        }
    }
    
    sf::Text fpsText;
    if (fontLoaded) {
        fpsText = sf::Text(font, "", 16);
        fpsText.setFillColor(sf::Color::White);
        fpsText.setPosition({8.f, 8.f});
    }

    sf::Clock frameClock;
    float accum = 0.f; int frames = 0;

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = ev->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scan::Escape) window.close();
            }
            // NEW: dig/place
            if (const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                // Map screen->world using the camera view
                const sf::Vector2f worldPos =
                    window.mapPixelToCoords({mb->position.x, mb->position.y}, cam.view());
                if (mb->button == sf::Mouse::Button::Left) {
                    world.setTileAtPixel(worldPos, Tile::Air);   // dig
                } else if (mb->button == sf::Mouse::Button::Right) {
                    world.setTileAtPixel(worldPos, Tile::Stone); // place
                }
            }
            cam.handleEvent(*ev);
        }


        const float dt = frameClock.restart().asSeconds();
        cam.update(dt);

        // Lazy-load visible chunks around the camera
        world.ensureVisible(cam.view(), /*inflatePixels=*/TILE_SIZE * 8.f, /*keepMarginChunks=*/2);


        // FPS
        accum += dt; frames += 1;
        if (accum >= 0.25f && fontLoaded) {
            const float fps = frames / accum; frames = 0; accum = 0.f;
            char buf[64]; std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
            fpsText.setString(buf);
        }

        window.clear(sf::Color(8, 10, 16));
        cam.applyTo(window);
        window.draw(world);

        window.setView(window.getDefaultView());
        if (fontLoaded) {
            window.draw(fpsText);
        }
        window.display();
    }
    return 0;
}
