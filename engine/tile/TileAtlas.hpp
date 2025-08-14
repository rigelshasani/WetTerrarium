#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <filesystem>
#include "engine/tile/TileTypes.hpp"

class TileAtlas {
public:
    explicit TileAtlas(unsigned tileSize = TILE_SIZE) : size_(tileSize) { 
        if (tileSize == 0) {
            throw std::invalid_argument("TileAtlas tile size must be greater than 0");
        }
        if (!build()) {
            throw std::runtime_error("Failed to create TileAtlas texture");
        }
    }

    unsigned tileSize() const { return size_; }
    const sf::Texture& texture() const { return tex_; }

    sf::IntRect uvFor(TileID id) const {
        const int col = static_cast<int>(id);
        const int s   = static_cast<int>(size_);
        return sf::IntRect({col * s, 0}, {s, s});
    }

private:
    unsigned    size_;
    sf::Texture tex_;

    static inline std::uint8_t clamp8(int v) {
        return static_cast<std::uint8_t>(std::clamp(v, 0, 255));
    }

    bool build() {
        // Try to load PNG atlas first
        if (loadFromPNG("assets/tiles.png")) {
            return true;
        }
        
        // Fallback to procedural generation
        return buildProcedural();
    }
    
    bool loadFromPNG(const std::filesystem::path& atlasPath) {
        if (!std::filesystem::exists(atlasPath)) {
            return false;
        }
        
        sf::Image img;
        if (!img.loadFromFile(atlasPath)) {
            return false;
        }
        
        // Validate atlas dimensions
        const sf::Vector2u imgSize = img.getSize();
        if (imgSize.y != size_ || imgSize.x != 8 * size_) {
            return false; // Wrong dimensions for 8 tiles
        }
        
        if (!tex_.loadFromImage(img)) {
            return false;
        }
        
        tex_.setRepeated(false);
        tex_.setSmooth(false);
        return true;
    }
    
    bool buildProcedural() {
        const unsigned cols = 8; // Support 8 tile types
        const sf::Vector2u imgSize{cols * size_, size_};
        sf::Image img({imgSize.x, imgSize.y}, sf::Color::Transparent);

        auto shadeFill = [&](unsigned col, sf::Color base) {
            for (unsigned y = 0; y < size_; ++y) {
                const float t = (size_ > 1) ? (static_cast<float>(y) / static_cast<float>(size_ - 1)) : 0.f;
                const float mul = (0.9f + 0.2f * (1.f - t));
                for (unsigned x = 0; x < size_; ++x) {
                    sf::Color c = base;
                    if (x == 0 || y == 0 || x == size_ - 1 || y == size_ - 1) {
                        c = sf::Color(0, 0, 0, 40);
                    } else {
                        c.r = clamp8(int(c.r * mul));
                        c.g = clamp8(int(c.g * mul));
                        c.b = clamp8(int(c.b * mul));
                    }
                    img.setPixel(sf::Vector2u{col * size_ + x, y}, c);
                }
            }
        };

        auto glowFill = [&](unsigned col, sf::Color base) {
            // Special rendering for light sources with glow effect
            for (unsigned y = 0; y < size_; ++y) {
                for (unsigned x = 0; x < size_; ++x) {
                    sf::Color c = base;
                    // Add bright center
                    const float dx = x - size_/2.f;
                    const float dy = y - size_/2.f;
                    const float dist = std::sqrt(dx*dx + dy*dy);
                    const float glow = std::max(0.f, 1.f - dist / (size_/2.f));
                    c.r = clamp8(int(c.r + glow * 100));
                    c.g = clamp8(int(c.g + glow * 80));
                    c.b = clamp8(int(c.b + glow * 40));
                    img.setPixel(sf::Vector2u{col * size_ + x, y}, c);
                }
            }
        };

        shadeFill(0, sf::Color(0, 0, 0, 0));        // Air (transparent)
        shadeFill(1, sf::Color(56, 170, 73));      // Grass (green)
        shadeFill(2, sf::Color(121, 85, 58));      // Dirt (brown)
        shadeFill(3, sf::Color(110, 110, 110));    // Stone (gray)
        shadeFill(4, sf::Color(139, 69, 19));      // Wood (dark brown)
        shadeFill(5, sf::Color(34, 139, 34));      // Leaves (forest green)
        glowFill(6, sf::Color(255, 200, 100));     // Torch (bright orange)
        glowFill(7, sf::Color(255, 255, 200));     // Lantern (bright yellow)

        if (!tex_.loadFromImage(img)) {
            return false;
        }
        tex_.setRepeated(false);
        tex_.setSmooth(false);
        return true;
    }
};
