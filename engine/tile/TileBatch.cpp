#include "engine/tile/TileBatch.hpp"
#include "engine/tile/TileTypes.hpp"
#include "engine/tile/Coords.hpp"

static inline void pushVertex(sf::VertexArray& va, float x, float y, float u, float v, sf::Color color = sf::Color::White) {
    sf::Vertex vert{};
    vert.position  = {x, y};
    vert.texCoords = {u, v};
    vert.color     = color;
    va.append(vert);
}

void TileBatch::addQuad(sf::VertexArray& va, float x, float y, float s, const sf::IntRect& uv, sf::Color color) {
    const float x0 = x,     y0 = y;
    const float x1 = x + s, y1 = y + s;

    const float u0 = static_cast<float>(uv.position.x);
    const float v0 = static_cast<float>(uv.position.y);
    const float u1 = static_cast<float>(uv.position.x + uv.size.x);
    const float v1 = static_cast<float>(uv.position.y + uv.size.y);

    // tri 1
    pushVertex(va, x0, y0, u0, v0, color);
    pushVertex(va, x1, y0, u1, v0, color);
    pushVertex(va, x1, y1, u1, v1, color);
    // tri 2
    pushVertex(va, x0, y0, u0, v0, color);
    pushVertex(va, x1, y1, u1, v1, color);
    pushVertex(va, x0, y1, u0, v1, color);
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
            
            // Calculate lighting color
            const LightMap& lightMap = chunk.getLightMap();
            unsigned lightLevel = lightMap.getLight(x, y);
            
            
            // Convert light level (0-15) to color multiplier (0.0-1.0)
            float lightFactor = static_cast<float>(lightLevel) / static_cast<float>(MAX_LIGHT_LEVEL);
            
            // During full daylight (level 12+), tiles should appear at full brightness
            // During darkness (level 2), tiles should be very dim but visible
            if (lightLevel >= 10) {
                // Full daylight - show natural tile colors
                lightFactor = 1.0f;
            } else if (lightLevel >= 6) {
                // Dawn/dusk - gradually brighten
                lightFactor = 0.7f + 0.3f * (lightLevel - 6) / 4.0f; // 0.7 to 1.0
            } else {
                // Night/dark - dim but visible
                lightFactor = 0.2f + 0.5f * lightLevel / 6.0f; // 0.2 to 0.7
            }
            
            std::uint8_t brightness = static_cast<std::uint8_t>(255 * lightFactor);
            sf::Color tileColor{brightness, brightness, brightness, 255};
            
            addQuad(va_, x * S, y * S, S, atlas.uvFor(t), tileColor);
        }
    }
    isDirty_ = false;
}

void TileBatch::updateRegion(const Chunk& chunk, const TileAtlas& atlas,
                           unsigned minX, unsigned minY, unsigned maxX, unsigned maxY) {
    // For now, fall back to full rebuild for simplicity
    // TODO: Implement true partial updates with vertex manipulation
    if (isDirty_) {
        build(chunk, atlas);
    }
}
