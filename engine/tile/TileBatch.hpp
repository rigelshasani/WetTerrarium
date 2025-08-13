#pragma once
#include <SFML/Graphics.hpp>
#include "engine/tile/Chunk.hpp"

class TileBatch : public sf::Drawable {
public:
    void build(const Chunk& chunk);

private:
    sf::VertexArray va_{sf::PrimitiveType::Triangles};
    sf::Vector2f pixelOffset_{0.f, 0.f}; // chunk origin in pixels

    static sf::Color colorFor(TileID id);
    static void addQuad(sf::VertexArray& va, float x, float y, float s, sf::Color c);

    void draw(sf::RenderTarget& t, sf::RenderStates s) const override {
        s.transform.translate(pixelOffset_);
        t.draw(va_, s);
    }
};
