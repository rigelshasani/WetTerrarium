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
    TileID selectedTile = Tile::Stone; // Default selected tile

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
    
    sf::Text fpsText(font, "", 16); // SFML 3 requires font parameter
    if (fontLoaded) {
        fpsText.setFillColor(sf::Color::White);
        fpsText.setPosition({8.f, 8.f});
    }

    sf::Clock frameClock;
    float accum = 0.f; int frames = 0;
    
    // Day/night cycle
    float gameTime = 0.f;                // Game time in seconds
    const float dayLength = 52.f;        // Day/night cycle duration in real seconds (30% longer)
    unsigned currentAmbientLight = 12;   // Current ambient light level

    while (window.isOpen()) {
        while (auto ev = window.pollEvent()) {
            if (ev->is<sf::Event::Closed>()) {
                window.close();
            } else if (const auto* key = ev->getIf<sf::Event::KeyPressed>()) {
                if (key->scancode == sf::Keyboard::Scan::Escape) window.close();
                
                // Handle tile selection keys
                if (key->scancode == sf::Keyboard::Scan::Num1) selectedTile = Tile::Stone;
                else if (key->scancode == sf::Keyboard::Scan::Num2) selectedTile = Tile::Dirt;
                else if (key->scancode == sf::Keyboard::Scan::Num3) selectedTile = Tile::Grass;
                else if (key->scancode == sf::Keyboard::Scan::Num4) selectedTile = Tile::Wood;
                else if (key->scancode == sf::Keyboard::Scan::Num5) selectedTile = Tile::Torch;
                else if (key->scancode == sf::Keyboard::Scan::Num6) selectedTile = Tile::Lantern;
            }
            // NEW: dig/place
            if (const auto* mb = ev->getIf<sf::Event::MouseButtonPressed>()) {
                // Map screen->world using the camera view
                const sf::Vector2f worldPos =
                    window.mapPixelToCoords({mb->position.x, mb->position.y}, cam.view());
                
                if (mb->button == sf::Mouse::Button::Left) {
                    world.setTileAtPixel(worldPos, Tile::Air);   // dig
                } else if (mb->button == sf::Mouse::Button::Right) {
                    world.setTileAtPixel(worldPos, selectedTile); // place selected tile
                }
            }
            cam.handleEvent(*ev);
        }


        const float dt = frameClock.restart().asSeconds();
        cam.update(dt);
        
        // Update day/night cycle
        gameTime += dt;
        float dayProgress = std::fmod(gameTime / dayLength, 1.0f); // 0.0 to 1.0
        
        // Calculate ambient light based on time of day
        // 0.0-0.15 = night, 0.15-0.35 = dawn, 0.35-0.65 = day, 0.65-0.85 = dusk, 0.85-1.0 = night
        unsigned newAmbientLight;
        if (dayProgress < 0.15f) {
            // Night
            newAmbientLight = 2; // Dark
        } else if (dayProgress < 0.35f) {
            // Dawn - longer gradual brightening
            float t = (dayProgress - 0.15f) / 0.2f; // 0.2 duration for smooth transition
            newAmbientLight = static_cast<unsigned>(2 + t * 10); // 2 to 12
        } else if (dayProgress < 0.65f) {
            // Full day - longer daylight period
            newAmbientLight = 12; // Full daylight
        } else if (dayProgress < 0.85f) {
            // Dusk - longer gradual dimming  
            float t = (dayProgress - 0.65f) / 0.2f; // 0.2 duration for smooth transition
            newAmbientLight = static_cast<unsigned>(12 - t * 10); // 12 to 2
        } else {
            // Night
            newAmbientLight = 2; // Dark
        }
        
        // Update lighting if ambient light changed
        if (newAmbientLight != currentAmbientLight) {
            currentAmbientLight = newAmbientLight;
            world.updateAmbientLight(currentAmbientLight);
        }

        // Lazy-load visible chunks around the camera
        world.ensureVisible(cam.view(), /*inflatePixels=*/TILE_SIZE * 8.f, /*keepMarginChunks=*/2);


        // FPS
        accum += dt; frames += 1;
        if (accum >= 0.25f && fontLoaded) {
            const float fps = frames / accum; frames = 0; accum = 0.f;
            char buf[64]; std::snprintf(buf, sizeof(buf), "FPS: %.1f", fps);
            fpsText.setString(buf);
        }

        // Calculate sky color based on time of day with smooth long gradients
        sf::Color skyColor;
        if (dayProgress < 0.15f) {
            // Night - dark purple/blue
            skyColor = sf::Color(15, 8, 35);
        } else if (dayProgress < 0.35f) {
            // Dawn - longer dark to purple/pink gradient
            float t = (dayProgress - 0.15f) / 0.2f; // Smoother over longer period
            skyColor = sf::Color(
                static_cast<std::uint8_t>(15 + t * 105),   // 15 to 120 (purple)
                static_cast<std::uint8_t>(8 + t * 62),     // 8 to 70   (purple tones)
                static_cast<std::uint8_t>(35 + t * 145)    // 35 to 180 (brighter)
            );
        } else if (dayProgress < 0.5f) {
            // Early day - transition from dawn purple to blue
            float t = (dayProgress - 0.35f) / 0.15f; // Longer transition
            skyColor = sf::Color(
                static_cast<std::uint8_t>(120 - t * 35),   // 120 to 85  
                static_cast<std::uint8_t>(70 + t * 100),   // 70 to 170  
                static_cast<std::uint8_t>(180 + t * 55)    // 180 to 235 
            );
        } else if (dayProgress < 0.65f) {
            // Full day - cute blue
            skyColor = sf::Color(85, 170, 235);
        } else if (dayProgress < 0.85f) {
            // Dusk - longer blue to warm orange gradient
            float t = (dayProgress - 0.65f) / 0.2f; // Longer smooth transition
            skyColor = sf::Color(
                static_cast<std::uint8_t>(85 + t * 145),    // 85 to 230  (warm orange)
                static_cast<std::uint8_t>(170 - t * 70),    // 170 to 100 (orange tone)
                static_cast<std::uint8_t>(235 - t * 195)    // 235 to 40  (less blue)
            );
        } else {
            // Night transition - longer orange to dark
            float t = (dayProgress - 0.85f) / 0.15f; // Smoother transition to night
            skyColor = sf::Color(
                static_cast<std::uint8_t>(230 - t * 215),   // 230 to 15  (orange to dark)
                static_cast<std::uint8_t>(100 - t * 92),    // 100 to 8   (dim to dark)
                static_cast<std::uint8_t>(40 - t * 5)       // 40 to 35   (keep some purple)
            );
        }
        
        window.clear(skyColor);
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
