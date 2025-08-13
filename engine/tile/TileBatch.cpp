#include "engine/tile/TileBatch.hpp"
#include "engine/tile/TileTypes.hpp"
#include "engine/tile/Coords.hpp"

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
    // tri 1
    pushVertex(va, x0, y0, c);
    pushVertex(va, x1, y0, c);
    pushVertex(va, x1, y1, c);
    // tri 2
    pushVertex(va, x0, y0, c);
    pushVertex(va, x1, y1, c);
    pushVertex(va, x0, y1, c);
}

void TileBatch::build(const Chunk& chunk) {
    va_.clear();
    va_.setPrimitiveType(sf::PrimitiveType::Triangles);

    // chunk origin in pixels
    const auto orgTiles = chunkOriginTiles(chunk.coord());
    pixelOffset_ = {orgTiles.x * static_cast<float>(TILE_SIZE),
                    orgTiles.y * static_cast<float>(TILE_SIZE)};

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
