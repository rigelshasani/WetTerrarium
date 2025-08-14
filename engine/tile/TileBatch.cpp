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
    // Use exact tile boundaries to prevent background bleeding
    const float x0 = x,     y0 = y;
    const float x1 = x + s, y1 = y + s;

    // Use exact texture coordinates to avoid bleeding between atlas tiles
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
            
            // For underground stone tiles, use simplified lighting to avoid banding
            const auto orgTiles = chunkOriginTiles(chunk.coord());
            const int worldY = orgTiles.y + static_cast<int>(y);
            
            sf::Color tileColor;
            if (t == Tile::Stone && worldY >= 8) {
                // Underground stone: use simplified lighting that reduces variation
                float lightFactor;
                if (lightLevel >= 10) {
                    lightFactor = 0.7f; // Bright areas (near torches) but not full bright
                } else if (lightLevel >= 8) {
                    lightFactor = 0.6f; // Medium bright
                } else {
                    lightFactor = 0.5f; // Base underground brightness
                }
                
                std::uint8_t brightness = static_cast<std::uint8_t>(255 * lightFactor);
                tileColor = sf::Color{brightness, brightness, brightness, 255};
            } else {
                // Normal lighting calculation for surface tiles and non-stone
                float lightFactor = static_cast<float>(lightLevel) / static_cast<float>(MAX_LIGHT_LEVEL);
                
                if (lightLevel >= 10) {
                    lightFactor = 1.0f;
                } else if (lightLevel >= 6) {
                    lightFactor = 0.7f + 0.3f * (lightLevel - 6) / 4.0f;
                } else {
                    lightFactor = 0.2f + 0.5f * lightLevel / 6.0f;
                }
                
                std::uint8_t brightness = static_cast<std::uint8_t>(255 * lightFactor);
                tileColor = sf::Color{brightness, brightness, brightness, 255};
            }
            
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
