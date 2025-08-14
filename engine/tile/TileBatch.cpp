#include "engine/tile/TileBatch.hpp"
#include "engine/tile/TileTypes.hpp"
#include "engine/tile/Coords.hpp"

static inline void pushVertex(sf::VertexArray& va, float x, float y, float u, float v) {
    sf::Vertex vert{};
    vert.position  = {x, y};
    vert.texCoords = {u, v};
    vert.color     = sf::Color::White;
    va.append(vert);
}

void TileBatch::addQuad(sf::VertexArray& va, float x, float y, float s, const sf::IntRect& uv) {
    const float x0 = x,     y0 = y;
    const float x1 = x + s, y1 = y + s;

    const float u0 = static_cast<float>(uv.position.x);
    const float v0 = static_cast<float>(uv.position.y);
    const float u1 = static_cast<float>(uv.position.x + uv.size.x);
    const float v1 = static_cast<float>(uv.position.y + uv.size.y);

    // tri 1
    pushVertex(va, x0, y0, u0, v0);
    pushVertex(va, x1, y0, u1, v0);
    pushVertex(va, x1, y1, u1, v1);
    // tri 2
    pushVertex(va, x0, y0, u0, v0);
    pushVertex(va, x1, y1, u1, v1);
    pushVertex(va, x0, y1, u0, v1);
}

void TileBatch::build(const Chunk& chunk, const TileAtlas& atlas) {
    va_.clear();
    va_.setPrimitiveType(sf::PrimitiveType::Triangles);
    tex_ = &atlas.texture();

    // chunk origin in pixels
    const auto orgTiles = chunkOriginTiles(chunk.coord());
    pixelOffset_ = {orgTiles.x * static_cast<float>(TILE_SIZE),
                    orgTiles.y * static_cast<float>(TILE_SIZE)};

    const unsigned W = chunk.width();
    const unsigned H = chunk.height();
    const float    S = static_cast<float>(atlas.tileSize());

    for (unsigned y = 0; y < H; ++y) {
        for (unsigned x = 0; x < W; ++x) {
            TileID t = chunk.get(x, y);
            if (t == Tile::Air) continue;
            addQuad(va_, x * S, y * S, S, atlas.uvFor(t));
        }
    }
}
