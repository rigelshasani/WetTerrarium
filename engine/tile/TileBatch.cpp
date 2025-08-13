#include "engine/tile/TileBatch.hpp"
#include "engine/tile/TileTypes.hpp"

sf::Color TileBatch::colorFor(TileID id) {
    switch (id) {
        case Tile::Grass: return sf::Color(56, 170, 73);
        case Tile::Dirt:  return sf::Color(121, 85, 58);
        case Tile::Stone: return sf::Color(110, 110, 110);
        default:          return sf::Color::Transparent;
    }
}

static inline void pushVertex(sf::VertexArray& va, float x, float y, sf::Color c) {
    sf::Vertex v{};
    v.position = {x, y};
    v.color    = c;
    va.append(v);
}

void TileBatch::addQuad(sf::VertexArray& va, float x, float y, float s, sf::Color c) {
    const float x0 = x,     y0 = y;
    const float x1 = x + s, y1 = y + s;
    // tri 1: (x0,y0)-(x1,y0)-(x1,y1)
    pushVertex(va, x0, y0, c);
    pushVertex(va, x1, y0, c);
    pushVertex(va, x1, y1, c);
    // tri 2: (x0,y0)-(x1,y1)-(x0,y1)
    pushVertex(va, x0, y0, c);
    pushVertex(va, x1, y1, c);
    pushVertex(va, x0, y1, c);
}

void TileBatch::build(const Chunk& chunk) {
    va_.clear();
    va_.setPrimitiveType(sf::PrimitiveType::Triangles); // SFML 3

    const unsigned W = chunk.width();
    const unsigned H = chunk.height();
    const float    S = static_cast<float>(TILE_SIZE);

    for (unsigned y = 0; y < H; ++y) {
        for (unsigned x = 0; x < W; ++x) {
            TileID t = chunk.get(x, y);
            if (t == Tile::Air) continue;
            addQuad(va_, x * S, y * S, S, colorFor(t));
        }
    }
}
