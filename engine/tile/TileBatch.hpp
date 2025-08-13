#pragma once
#include <SFML/Graphics.hpp>
#include "engine/tile/Chunk.hpp"

class TileBatch : public sf::Drawable {
public:
    void build(const Chunk& chunk);

private:
    sf::VertexArray va_{sf::PrimitiveType::Triangles};

    static sf::Color colorFor(TileID id);
    static void addQuad(sf::VertexArray& va, float x, float y, float s, sf::Color c);

    void draw(sf::RenderTarget& t, sf::RenderStates s) const override {
        t.draw(va_, s);
    }
};
