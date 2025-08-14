#pragma once
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cstdint>
#include <stdexcept>
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
        const unsigned cols = 4;
        const sf::Vector2u imgSize{cols * size_, size_};
        sf::Image img(imgSize, sf::Color::Transparent);

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

        shadeFill(0, sf::Color(0, 0, 0, 0));
        shadeFill(1, sf::Color(56, 170, 73));
        shadeFill(2, sf::Color(121, 85, 58));
        shadeFill(3, sf::Color(110, 110, 110));

        if (!tex_.loadFromImage(img)) {
            return false;
        }
        tex_.setRepeated(false);
        tex_.setSmooth(false);
        return true;
    }
};
